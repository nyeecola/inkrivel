#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

#include <stdio.h>
#include <assert.h>

#include "render-types.hpp"
#include "vector.hpp"
#include "base/character.hpp"
#include "base/map.hpp"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define DEBUG 0

#define MAX(a, b) ((a) > (b) ? a : b)
#define MIN(a, b) ((a) < (b) ? a : b)

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
        // TODO: do we need to free this? For the map be careful, because we need the pointer
        SDL_Surface *sur = IMG_Load(texture_filename);
        assert(sur);
        glGenTextures(1, &model.texture_id);
        glBindTexture(GL_TEXTURE_2D, model.texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, 4, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, sur->pixels);
        glBindTexture(GL_TEXTURE_2D, 0);

        model.texture_image = sur;
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

// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
bool rayIntersectsTriangle(Map map, Vector rayOrigin, Vector rayVector, Face* inTriangle, Vector& outIntersectionPoint) {
    const float EPSILON = 0.0000001;
    Vector vertex0 = map.scale * map.model.vertices[inTriangle->vertices[0]];
    Vector vertex1 = map.scale * map.model.vertices[inTriangle->vertices[1]];
    Vector vertex2 = map.scale * map.model.vertices[inTriangle->vertices[2]];
    Vector edge1, edge2, h, s, q;
    float a,f,u,v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = rayVector.cross(edge2);
    a = edge1.dot(h);
    if (a > -EPSILON && a < EPSILON)
        return false;
    f = 1/a;
    s = rayOrigin - vertex0;
    u = f * (s.dot(h));
    if (u < 0.0 || u > 1.0)
        return false;
    q = s.cross(edge1);
    v = f * rayVector.dot(q);
    if (v < 0.0 || u + v > 1.0)
        return false;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * edge2.dot(q);
    if (t > EPSILON) { // ray intersection
        outIntersectionPoint = rayOrigin + rayVector * t;
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return false;
}

// Sphere-Triangle collision from: http://realtimecollisiondetection.net/blog/?p=103
bool sphereCollidesTriangle(Vector sphere_center, float sphere_radius, Vector triangle0, Vector triangle1, Vector triangle2) {
    Vector A = triangle0 - sphere_center;
    Vector B = triangle1 - sphere_center;
    Vector C = triangle2 - sphere_center;
    float rr = sphere_radius * sphere_radius;
    Vector V = (B - A).cross(C - A);
    float d = A.dot(V);
    float e = V.dot(V);

    bool sep1 = d*d > rr*e;

    float aa = A.dot(A);
    float ab = A.dot(B);
    float ac = A.dot(C);
    float bb = B.dot(B);
    float bc = B.dot(C);
    float cc = C.dot(C);

    bool sep2 = (aa > rr) && (ab > aa) && (ac > aa);
    bool sep3 = (bb > rr) && (ab > bb) && (bc > bb);
    bool sep4 = (cc > rr) && (ac > cc) && (bc > cc);

    Vector AB = B - A;
    Vector BC = C - B;
    Vector CA = A - C;

    float d1 = ab - aa;
    float d2 = bc - bb;
    float d3 = ac - cc;

    float e1 = AB.dot(AB);
    float e2 = BC.dot(BC);
    float e3 = CA.dot(CA);

    Vector Q1 = A*e1 - d1*AB;
    Vector Q2 = B*e2 - d2*BC;
    Vector Q3 = C*e3 - d3*CA;
    Vector QC = C*e1 - Q1;
    Vector QA = A*e2 - Q2;
    Vector QB = B*e3 - Q3;

    bool sep5 = (Q1.dot(Q1) > rr * e1 * e1) && (Q1.dot(QC) > 0);
    bool sep6 = (Q2.dot(Q2) > rr * e2 * e2) && (Q2.dot(QA) > 0);
    bool sep7 = (Q3.dot(Q3) > rr * e3 * e3) && (Q3.dot(QB) > 0);

    bool separated = sep1 || sep2 || sep3 || sep4 || sep5 || sep6 || sep7;

    return !separated;
}

// https://www.gamedev.net/forums/topic/566295-normal-to-a-quaternion/
Quat getRotationQuat(const Vector& from, const Vector& to) {
     Quat result;
     Vector H = from + to;
     H.normalize();

     result.w = from.dot(H);
     result.x = from.y*H.z - from.z*H.y;
     result.y = from.z*H.x - from.x*H.z;
     result.z = from.x*H.y - from.y*H.x;
     return result;
}

// https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
void barycentric(Vector p, Vector a, Vector b, Vector c, float &u, float &v, float &w)
{
    Vector v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

// NOTE: assumes texture size is 1024x1024
// NOTE: max radius for now is 100
#define MAX_INK_SPOT_RADIUS 100
void paintCircle(Map map, Face *paint_face, Vector center, float radius, uint8_t r, uint8_t g, uint8_t b) {
    assert (radius <= 100);

    Vector v0 = map.scale * map.model.vertices[paint_face->vertices[0]];
    Vector v1 = map.scale * map.model.vertices[paint_face->vertices[1]];
    Vector v2 = map.scale * map.model.vertices[paint_face->vertices[2]];

    float u, v, w;
    barycentric(center, v0, v1, v2, u, v, w);

    TextureCoord tex_v0 = map.model.texture_coords[paint_face->texture_coords[0]];
    TextureCoord tex_v1 = map.model.texture_coords[paint_face->texture_coords[1]];
    TextureCoord tex_v2 = map.model.texture_coords[paint_face->texture_coords[2]];

    unsigned char *pixels = (unsigned char *) map.model.texture_image->pixels;

    float tex_x, tex_y;
    tex_x = u * tex_v0.x + v * tex_v1.x + w * tex_v2.x;
    tex_y = u * tex_v0.y + v * tex_v1.y + w * tex_v2.y;

    int ink_spot[MAX_INK_SPOT_RADIUS][MAX_INK_SPOT_RADIUS] = {};
    for (int k1 = 0; k1 < MAX_INK_SPOT_RADIUS; k1++) {
        for (int k2 = 0; k2 < MAX_INK_SPOT_RADIUS; k2++) {
            int y = tex_y * 1023 - MAX_INK_SPOT_RADIUS/2 + k1;
            int x = tex_x * 1023 - MAX_INK_SPOT_RADIUS/2 + k2;

            if ((k1-MAX_INK_SPOT_RADIUS/2) * (k1-MAX_INK_SPOT_RADIUS/2)
                + (k2-MAX_INK_SPOT_RADIUS/2) * (k2-MAX_INK_SPOT_RADIUS/2) <= radius * radius) {
                // use paint color
                ink_spot[k1][k2] = r;
                ink_spot[k1][k2] |= g << 8;
                ink_spot[k1][k2] |= b << 16;
                ink_spot[k1][k2] |= 0xFF << 24;
            } else {
                // use previous color
                ink_spot[k1][k2] = pixels[4 * (y * 1024 + x) + 0] << 0;
                ink_spot[k1][k2] |= pixels[4 * (y * 1024 + x) + 1] << 8;
                ink_spot[k1][k2] |= pixels[4 * (y * 1024 + x) + 2] << 16;
                ink_spot[k1][k2] |= pixels[4 * (y * 1024 + x) + 3] << 24;
            }

            // store current color
            pixels[4 * (y * 1024 + x) + 0] = (char) (ink_spot[k1][k2]);
            pixels[4 * (y * 1024 + x) + 1] = (char) (ink_spot[k1][k2] >> 8);
            pixels[4 * (y * 1024 + x) + 2] = (char) (ink_spot[k1][k2] >> 16);
            pixels[4 * (y * 1024 + x) + 3] = (char) (ink_spot[k1][k2] >> 24);
        }
    }

    glBindTexture(GL_TEXTURE_2D, map.model.texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
            tex_x * 1023 - MAX_INK_SPOT_RADIUS / 2, tex_y * 1023 - MAX_INK_SPOT_RADIUS / 2,
            MAX_INK_SPOT_RADIUS, MAX_INK_SPOT_RADIUS, GL_RGBA, GL_UNSIGNED_BYTE,
            (const void *) ink_spot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main() {
    loadLibraries();

    SDL_Window *window = NULL;
    window = SDL_CreateWindow("InkRivel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // ignoring return value
    SDL_GL_CreateContext(window);

    // OpenGL properties
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    //glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    // OpenGL projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    makeFrustum(45, (float) SCREEN_WIDTH / SCREEN_HEIGHT, 0.1, 100);

    // Create player slime
    Character slime;
    slime.pos = {0, 0, 0.35};
    slime.hit_radius = 0.25;
    slime.model = loadWavefrontModel("assets/slime.obj", "assets/slime.png", VERTEX_ALL);
    slime.speed = 0.02;
    slime.dir = {0, 0, 0};

    // Create map
    Map map;
    map.model = loadWavefrontModel("assets/map7.obj", "assets/map.png", VERTEX_ALL);
    map.characterList[0] = &slime;
    map.scale = 0.4;

    const Uint8 *kb_state = SDL_GetKeyboardState(NULL);
    bool running = true;
    while (running) {

        // handle events
        {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                }
            }
        }

        // mouse position relative to the middle of the window
        float mouse_x, mouse_y;
        float mouse_angle;
        {
            int int_mouse_x, int_mouse_y;
            SDL_GetMouseState(&int_mouse_x, &int_mouse_y);
            mouse_x = int_mouse_x;
            mouse_y = int_mouse_y;
            mouse_x -= SCREEN_WIDTH/2;
            mouse_y -= SCREEN_HEIGHT/2;
            float norm = sqrt(mouse_x * mouse_x + mouse_y * mouse_y);
            mouse_x /= norm;
            mouse_y /= norm;
            mouse_angle = atan2(mouse_y, mouse_x) * 180 / M_PI;
            mouse_angle += 90;
            mouse_angle *= -1;
        }

        // move object in the direction of the mouse when W is pressed
        // TODO: use delta_time
        if (kb_state[SDL_SCANCODE_W]) {
            slime.dir.x = mouse_x;
            slime.dir.y = -mouse_y;
            slime.dir.normalize();
            slime.dir *= slime.speed;
        }
        else {
            slime.dir = {0, 0, 0};
        }

        // collision
        Vector next_pos = slime.pos + slime.dir;
        Vector max_z = {0, 0, -200};
        Vector rotation_points[4] = {{-200, -200, -200}, {-200, -200, -200}, {-200, -200, -200}, {-200, -200, -200}};
        Vector rotation_normals[4] = {{-200, -200, -200}, {-200, -200, -200}, {-200, -200, -200}, {-200, -200, -200}};

        // DEBUG: currently use for painting
        Vector paint_max_z = {0, 0, -200};
        Face *paint_face;

        for (int i = 0; i < map.model.num_faces; i++) {
            Face *cur = &map.model.faces[i];

            Vector vertex0 = map.scale * map.model.vertices[cur->vertices[0]];
            Vector vertex1 = map.scale * map.model.vertices[cur->vertices[1]];
            Vector vertex2 = map.scale * map.model.vertices[cur->vertices[2]];

            Vector v1 = vertex2 - vertex0;
            Vector v2 = vertex1 - vertex0;
            Vector normal = v1.cross(v2);
            normal.normalize();
            if (normal.z < 0) {
                normal = normal * -1;
            }

            Vector up = {0, 0, 1};

            float cosine = normal.dot(up);

            float angle = acos(cosine) * 180 / M_PI;

            // walls
            if (angle > 60 && (vertex0.z > next_pos.z || vertex1.z > next_pos.z || vertex2.z > next_pos.z)) {
                bool collides = sphereCollidesTriangle(next_pos, slime.hit_radius, vertex0, vertex1, vertex2);

                if (collides) {
                    Vector v = slime.pos - vertex0;
                    float d = v.dot(normal);
                    Vector collision = d*normal;

                    Vector reaction_v = slime.dir + collision;
                    slime.dir.x = reaction_v.x * slime.dir.x > 0 ? reaction_v.x : 0;
                    slime.dir.y = reaction_v.y * slime.dir.y > 0 ? reaction_v.y : 0;
                    slime.dir.z = reaction_v.z * slime.dir.z > 0 ? reaction_v.z : 0;
                }
            }

            // floors
            else {
                Vector sky[4];
                sky[0] = {slime.pos.x + slime.hit_radius, slime.pos.y, 200};
                sky[1] = {slime.pos.x - slime.hit_radius, slime.pos.y, 200};
                sky[2] = {slime.pos.x, slime.pos.y + slime.hit_radius, 200};
                sky[3] = {slime.pos.x, slime.pos.y - slime.hit_radius, 200};

                Vector ground = {0, 0, -1};

                Vector intersect_v;
                for (int j = 0; j < 4; j++) {
                    bool intersect = rayIntersectsTriangle(map, sky[j], ground, cur, intersect_v);
                    if (intersect) {
                        if (max_z.z < intersect_v.z) {
                            max_z = intersect_v;
                        }

                        if (rotation_points[j].z < intersect_v.z) {
                            rotation_points[j] = intersect_v;
                            rotation_normals[j] = normal;
                        }
                    }
                }

                // DEBUG: this makes the slime paint the ground where it walks
                Vector sky_slime = {slime.pos.x, slime.pos.y, 200};
                bool intersect = rayIntersectsTriangle(map, sky_slime, ground, cur, intersect_v);
                if (intersect && intersect_v.z > paint_max_z.z) {
                    paint_max_z = intersect_v;
                    paint_face = cur;
                }
            }
        }

        paintCircle(map, paint_face, paint_max_z, 40, 0x1F, 0xFF, 0x1F);


        Vector normal_sum = {0,0,0};
        for (int j = 0; j < 4; j++) {
            normal_sum += rotation_normals[j];
        }
        normal_sum.normalize();
        slime.rotation = getRotationQuat({0,0,1}, normal_sum);

        // Slime movement
        if (slime.dir.len() > slime.speed) {
            slime.dir.normalize();
            slime.dir *= slime.speed;
        }
        slime.pos += slime.dir;
        if (max_z.z > 0) {
            slime.pos.z += 0.5*(max_z.z - slime.pos.z);
        }

        // render

        glClearColor(0.4, 0.6, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // set camera position
        glTranslatef(-slime.pos.x, -slime.pos.y, -8);

        // draw sun
        {
            glEnable(GL_LIGHT0);
            GLfloat light_position[] = { 0.8, 0.5, 5, 1 };
            GLfloat ambient[] = { 0.1, 0.1, 0.1, 1 };
            GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1 };
            GLfloat specular[] = { 0.4, 0.4, 0.4, 1 };
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

            // default is (1, 0, 0)
            glLighti(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 2.8);
            //glLighti(GL_LIGHT0, GL_LINEAR_ATTENUATION, 3);
            //glLighti(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1);
        }

        // draw slime hitsphere
#if DEBUG
        drawSphere(slime.pos, slime.hit_radius);
#endif

        // draw map
        {
#if 0
            GLfloat mat_ambient[] = { 0.24, 0.20, 0.075, 1.0 };
            GLfloat mat_diffuse[] = { 0.75, 0.6, 0.22, 1.0 };
            GLfloat mat_specular[] = { 0.6282, 0.556, 0.366, 1.0 };
#else
            GLfloat mat_ambient[] = { 0.17, 0.17, 0.17, 1.0 };
            GLfloat mat_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
            GLfloat mat_specular[] = { 0.5, 0.5, 0.5, 1.0 };
#endif
            GLfloat mat_shininess[] = { 1 };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
            glPushMatrix();
            glScalef(map.scale, map.scale, map.scale);
            drawModel(map.model);
            glPopMatrix();
        }

        glClear(GL_DEPTH_BUFFER_BIT);

        // draw slime
        {
#if 0
            GLfloat mat_ambient[] = { 0.24, 0.20, 0.075, 1.0 };
            GLfloat mat_diffuse[] = { 0.75, 0.6, 0.22, 1.0 };
            GLfloat mat_specular[] = { 0.6282, 0.556, 0.366, 1.0 };
#else
            GLfloat mat_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
            GLfloat mat_diffuse[] = { 0.3, 0.3, 0.3, 1.0 };
            GLfloat mat_specular[] = { 0.6, 0.6, 0.6, 1.0 };
#endif
            GLfloat mat_shininess[] = { 1 };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
            glPushMatrix();
            glTranslatef(slime.pos.x, slime.pos.y, slime.pos.z);
            // From AllegroGL`s math.c
            glRotatef((2*acos(slime.rotation.w)) * 180 / M_PI, slime.rotation.x, slime.rotation.y, slime.rotation.z);
            glRotatef(mouse_angle, 0, 0, 1);
            glScalef(0.2, 0.2, 0.2);
            //glScalef(0.5, 0.5, 0.5);
            drawModel(slime.model);
            glPopMatrix();
        }

        SDL_GL_SwapWindow(window);
    }

    return 0;
}

