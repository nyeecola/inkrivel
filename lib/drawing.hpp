#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

#include <stdio.h>
#include <assert.h>

#include "render-types.hpp"
#include "vector.hpp"
#include "map.hpp"
#include "physics.hpp"

void loadLibraries() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        exit(1);
    }
}


// http://webserver2.tecgraf.puc-rio.br/ftp_pub/lfm/OpenGL_Transformation.pdf
// This creates a symmetric frustum.
// It converts to 6 params (l, r, b, t, n, f) for glFrustum()
// from given 4 params (fovy, aspect, near, far)
void makeFrustum (double fov_y, double aspect_ratio, double front, double back) {
    const double DEG2RAD = M_PI / 180;

    double tangent = tan(fov_y/2 * DEG2RAD);   // tangent of half fovY
    double height = front * tangent;          // half height of near plane
    double width = height * aspect_ratio;     // half width of near plane

    // params: left, right, bottom, top, near, far
    glFrustum(-width, width, -height, height, front, back);
}

uint32_t getLuminance(uint32_t pixel) {
    float r = pixel & 0x000000FF;
    float g = (pixel & 0x0000FF00) >> 8;
    float b = (pixel & 0x00FF0000) >> 16;
    r /= 255;
    g /= 255;
    b /= 255;

    float luminance = 0.2126*r + 0.7152*g + 0.0722*b;
    int luminance_int = (int) (luminance * 0xFF);

    pixel = 0xFF000000;
    pixel |= luminance_int;
    pixel |= luminance_int << 8;
    pixel |= luminance_int << 16;

    return pixel;
}


Model loadWavefrontModel(const char *obj_filename, const char *texture_filename, FaceType face_type) {
    Model model = {};

    // NOTE: Objects bigger than the constants are undefined behavior
    model.vertices = (Vector *) malloc(MAX_OBJ_VERTICES * sizeof(Vector));
    model.faces = (Face *) malloc(MAX_OBJ_FACES * sizeof(Face));
    model.normals = (Normal *) malloc(MAX_OBJ_VERTICES * sizeof(Normal));
    model.texture_coords = (TextureCoord *) malloc(MAX_OBJ_VERTICES * sizeof(TextureCoord));

    if (face_type == VERTEX_ALL || face_type == VERTEX_TEXTURE) {
        // NOTE: this can be freed if it`s not a map model

        // normal texture
        {
            SDL_Surface *sur = IMG_Load(texture_filename);
            assert(sur);

            glGenTextures(1, &model.texture_id);
            glBindTexture(GL_TEXTURE_2D, model.texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, 4, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         sur->pixels);
            glBindTexture(GL_TEXTURE_2D, 0);

            model.texture_image = sur;
        }

        // black-white version of texture
        {
            SDL_Surface *bw_sur = IMG_Load(texture_filename);

            int w = bw_sur->w;
            int h = bw_sur->h;
            int size = w * h;

            uint32_t *pixels = (uint32_t *) bw_sur->pixels;

            for (int i = 0; i < size; i++) {
                uint32_t pixel = getLuminance(pixels[i]);
                pixels[i] = pixel;
            }

            glGenTextures(1, &model.texture_bw_id);
            glBindTexture(GL_TEXTURE_2D, model.texture_bw_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, bw_sur->pixels);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    FILE *f = fopen(obj_filename, "r");
    char type[40] = {};
    while ((fscanf(f, " %s", type)) != EOF) {
        if (!strcmp(type, "v")) {
            Vector v = {};

            fscanf(f, " %f %f %f", &v.x, &v.y, &v.z);
            model.vertices[++model.num_vertices] = v;
        } else if (!strcmp(type, "vn")) {
            Normal n = {};

            fscanf(f, " %f %f %f", &n.x, &n.y, &n.z);
            model.normals[++model.num_normals] = n;
        } else if (!strcmp(type, "vt")) {
            TextureCoord t = {};

            fscanf(f, " %f %f", &t.x, &t.y);
            t.y = 1 - t.y;
            model.texture_coords[++model.num_texture_coords] = t;
        } else if (!strcmp(type, "f")) {
            Face face = {};

            if (face_type == VERTEX_ONLY) {
                fscanf(f, " %d %d %d", &face.vertices[0], &face.vertices[1], &face.vertices[2]);
            } else if (face_type == VERTEX_NORMAL) {
                fscanf(f, " %d//%d %d//%d %d//%d", &face.vertices[0], &face.normals[0], &face.vertices[1],
                                                   &face.normals[1], &face.vertices[2], &face.normals[2]);
            } else if (face_type == VERTEX_TEXTURE) {
                // NOTE: not supported
                assert(false);
            } else if (face_type == VERTEX_ALL) {
                fscanf(f, " %d/%d/%d %d/%d/%d %d/%d/%d", &face.vertices[0], &face.texture_coords[0],
                                                         &face.normals[0], &face.vertices[1],
                                                         &face.texture_coords[1], &face.normals[1],
                                                         &face.vertices[2], &face.texture_coords[2],
                                                         &face.normals[2]);
            } else {
                assert(false);
            }

            model.faces[model.num_faces++] = face;
        }
    }

    return model;
}


void drawModel(Model model, bool grayscale = false) {
    if (grayscale) {
        glBindTexture(GL_TEXTURE_2D, model.texture_bw_id);
    } else {
        glBindTexture(GL_TEXTURE_2D, model.texture_id);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    for (int i = 0; i < model.num_faces; i++) {
        Face f = model.faces[i];

        glBegin(GL_TRIANGLES);

        for (int j = 0; j <= 2; j++) {

            if (model.num_normals) {
                Normal n = model.normals[f.normals[j]];
                glNormal3f(n.x, n.y, n.z);
            }

            if (model.num_texture_coords) {
                TextureCoord t = model.texture_coords[f.texture_coords[j]];
                glTexCoord2f(t.x, t.y);
            }

            Vector v = model.vertices[f.vertices[j]];

            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Base for sphere code:
// https://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
void drawSphere(Vector center, float radius, float r, float g, float b) {
    int gradation = 30;

    GLfloat mat_ambient[] = { r * 0.4f, g * 0.4f, b * 0.4f, 1.0 };
    GLfloat mat_diffuse[] = { r * 1.0f, g * 1.0f, b * 1.0f, 1.0 };
    GLfloat mat_specular[] = { r * 1.0f, g * 1.0f, b * 1.0f, 1.0 };
    GLfloat mat_shininess[] = { 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);
    //glRotatef(mouse_angle, 0, 0, 1);
    //glScalef(0.2, 0.2, 0.2);
    for (float alpha = 0.0; alpha < M_PI; alpha += M_PI/gradation) {
        glBegin(GL_TRIANGLE_STRIP);
        for (float beta = 0.0; beta < 2.01*M_PI; beta += M_PI/gradation) {
            float x = radius*cos(beta)*sin(alpha);
            float y = radius*sin(beta)*sin(alpha);
            float z = radius*cos(alpha);
            Vector norm(x,y,z);
            norm.normalize();
            glNormal3f(norm.x, norm.y, norm.z);
            glVertex3f(x, y, z);
            x = radius*cos(beta)*sin(alpha + M_PI/gradation);
            y = radius*sin(beta)*sin(alpha + M_PI/gradation);
            z = radius*cos(alpha + M_PI/gradation);
            Vector norm2(x,y,z);
            norm2.normalize();
            glNormal3f(norm2.x, norm2.y, norm2.z);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
    glPopMatrix();
}

void drawRect(float x, float y, float w, float h, float r, float g, float b) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3f(r, g, b);
    glVertex3f(x, y, 0);
    glColor3f(r, g, b);
    glVertex3f(x + w, y, 0);
    glColor3f(r, g, b);
    glVertex3f(x + w, y + h, 0);
    glColor3f(r, g, b);
    glVertex3f(x, y + h, 0);
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}

// TODO: stop assuming the next line
// NOTE: assumes texture size is 1024x1024
// NOTE: max radius for now is 100
// NOTE: color must be in 0xFFBBGGRR format
#define MAX_INK_SPOT_RADIUS 100
void paintCircle(Model map_model, Face paint_face, Vector center, float radius, uint32_t color, bool opengl) {
    assert(radius <= 100);

    Vector v0 = MAP_SCALE * map_model.vertices[paint_face.vertices[0]];
    Vector v1 = MAP_SCALE * map_model.vertices[paint_face.vertices[1]];
    Vector v2 = MAP_SCALE * map_model.vertices[paint_face.vertices[2]];

    float u, v, w;
    barycentric(center, v0, v1, v2, u, v, w);

    TextureCoord tex_v0 = map_model.texture_coords[paint_face.texture_coords[0]];
    TextureCoord tex_v1 = map_model.texture_coords[paint_face.texture_coords[1]];
    TextureCoord tex_v2 = map_model.texture_coords[paint_face.texture_coords[2]];

    uint32_t *pixels = (uint32_t *) map_model.texture_image->pixels;

    int diameter = (int) radius * 2;
    int int_radius = (int) radius;
    int *ink_spot = (int *) malloc(diameter * diameter * sizeof(*ink_spot));

    float tex_x, tex_y;
    tex_x = (u * tex_v0.x + v * tex_v1.x + w * tex_v2.x) * 1023 - int_radius;
    tex_y = (u * tex_v0.y + v * tex_v1.y + w * tex_v2.y) * 1023 - int_radius;

    for (int k1 = 0; k1 < diameter; k1++) {
        for (int k2 = 0; k2 < diameter; k2++) {
            int y = tex_y + k1;
            int x = tex_x + k2;

            if (x > 1023 || x < 0 || y < 0 || y > 1023) continue;

            int k1_minus_radius = k1 - int_radius;
            int k2_minus_radius = k2 - int_radius;
            if (k1_minus_radius * k1_minus_radius +
                    k2_minus_radius * k2_minus_radius <= radius * radius) {
                // update texture image
                pixels[y * 1024 + x] = color;
            }

            // set pixel color to match new texture color
            ink_spot[k1 * diameter + k2] = pixels[y * 1024 + x];
        }
    }

    if (opengl) {
        // normal texture
        glBindTexture(GL_TEXTURE_2D, map_model.texture_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, tex_x, tex_y,
                        diameter, diameter, GL_RGBA, GL_UNSIGNED_BYTE,
                        (const void *) ink_spot);

        // black-white texture
        for (int k1 = 0; k1 < diameter; k1++) {
            for (int k2 = 0; k2 < diameter; k2++) {
                ink_spot[k1 * diameter + k2] = getLuminance(ink_spot[k1*diameter+k2]);
            }
        }
        glBindTexture(GL_TEXTURE_2D, map_model.texture_bw_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, tex_x, tex_y,
                        diameter, diameter, GL_RGBA, GL_UNSIGNED_BYTE,
                        (const void *) ink_spot);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    free(ink_spot);
}

#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"
#endif

unsigned char ttf_buffer[1<<22];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

void stbtt_initfont(void)
{
   fread(ttf_buffer, 1, 1<<22, fopen("../assets/Inconsolata-Regular.ttf", "rb"));
   stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
   // can free ttf_buffer at this point
   glGenTextures(1, &ftex);
   glBindTexture(GL_TEXTURE_2D, ftex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
   // can free temp_bitmap at this point
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void stbtt_print(float x, float y, char *text, float r=1.0f, float g=1.0f, float b=1.0f) {
   // assume orthographic projection with units = screen pixels, origin at top left
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, ftex);
   glBegin(GL_QUADS);
   glColor3f(r, g, b);
   while (*text) {
      if (*text >= 32 && *text < 128) {
         stbtt_aligned_quad q;
         stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
         stbtt_bakedchar baked_char = cdata[*text - 32]; // TODO: remove this magic number
         //printf("%f %f %f %f %f\n", q.x0, q.y0, q.x1, q.y1, baked_char.yoff);
         if (*text == '2' || *text == '4' || *text == '7') baked_char.yoff -= 1.0f;
         if (*text == ':') baked_char.yoff -= 2.0f;

         glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y0-baked_char.yoff);
         glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y0-baked_char.yoff);
         glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y1-baked_char.yoff);
         glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y1-baked_char.yoff);
      }
      ++text;
   }
   glColor3f(1, 1, 1);
   glEnd();
}

void prepareDrawFont() {
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    //glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, -1, 1.0); // origin in bottom left
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void endDrawFont() {
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}
