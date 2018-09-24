#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

#include <stdio.h>
#include <assert.h>

#include "render-types.hpp"
#include "vector.hpp"

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


// TODO: dar créditos a quem fez isso pela primeira vez
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


Model loadWavefrontModel(const char *obj_filename, const char *texture_filename, FaceType face_type) {
    Model model = {};

    model.vertices = (Vector *) malloc(100000 * sizeof(Vector));
    model.faces = (Face *) malloc(100000 * sizeof(Face));
    model.normals = (Normal *) malloc(100000 * sizeof(Normal));
    model.texture_coords = (TextureCoord *) malloc(100000 * sizeof(TextureCoord));

    if (face_type == VERTEX_ALL || face_type == VERTEX_TEXTURE) {
        // TODO: do we need to free this?
        SDL_Surface *sur = IMG_Load(texture_filename);
        glGenTextures(1, &model.texture_id);
        glBindTexture(GL_TEXTURE_2D, model.texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, 4, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, sur->pixels);
        glBindTexture(GL_TEXTURE_2D, 0);
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
                // TODO
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


void drawModel(Model model) {
    glBindTexture(GL_TEXTURE_2D, model.texture_id);
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
void drawSphere(Vector center, float radius) {
    int gradation = 10;

    GLfloat mat_ambient[] = { 0.4, 0.0, 0.0, 1.0 };
    GLfloat mat_diffuse[] = { 1.0, 0.1, 0.1, 1.0 };
    GLfloat mat_specular[] = { 1.0, 0.1, 0.1, 1.0 };
    GLfloat mat_shininess[] = { 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glPushMatrix();
    glTranslatef(center.x, center.y, 0.2);
    //glRotatef(mouse_angle, 0, 0, 1);
    //glScalef(0.2, 0.2, 0.2);
    for (float alpha = 0.0; alpha < M_PI; alpha += M_PI/gradation) {
        glBegin(GL_TRIANGLE_STRIP);
        for (float beta = 0.0; beta < 2.01*M_PI; beta += M_PI/gradation) {
            float x = radius*cos(beta)*sin(alpha);
            float y = radius*sin(beta)*sin(alpha);
            float z = radius*cos(alpha);
            glVertex3f(x, y, z);
            x = radius*cos(beta)*sin(alpha + M_PI/gradation);
            y = radius*sin(beta)*sin(alpha + M_PI/gradation);
            z = radius*cos(alpha + M_PI/gradation);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
    glPopMatrix();
}