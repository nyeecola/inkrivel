#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

#include <stdio.h>
#include <assert.h>

#include "render-types.hpp"

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

#define max(a, b) ((a) > (b) ? a : b)
#define min(a, b) ((a) < (b) ? a : b)

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

    model.vertices = (Vertex *) malloc(100000 * sizeof(Vertex));
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
            Vertex v = {};

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

            Vertex v = model.vertices[f.vertices[j]];

            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();
    }

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

    // load models
    //Model slime = loadWavefrontModel("assets/slime.obj", "assets/slime.png", VERTEX_ALL);
    //Model slime = loadWavefrontModel("assets/rolo.obj", "assets/slime.png", VERTEX_NORMAL);
    Model slime = loadWavefrontModel("assets/sniper.obj", "assets/slime.png", VERTEX_NORMAL);
    Model map = loadWavefrontModel("assets/map.obj", "assets/map.png", VERTEX_ALL);

    float tran_x = 0;
    float tran_y = 0;
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
            tran_x += mouse_x * 0.02;
            tran_y -= mouse_y * 0.02;
        }


        // render

        glClearColor(0.4, 0.6, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // set camera position
        glTranslatef(-tran_x, -tran_y, -8);

        // draw sun
        {
            glEnable(GL_LIGHT0);
            GLfloat light_position[] = { 100, 100, 150, 1 };
            GLfloat ambient[] = { 0.5, 0.5, 0.5, 1 };
            GLfloat diffuse_specular[] = { 0.7, 0.7, 0.7, 1 };
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_specular);
            glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse_specular);

            // default is (1, 0, 0)
            glLighti(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 4.0);
            //glLighti(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1);
            //glLighti(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1);
        }

        // draw slime
        { 
            GLfloat mat_ambient[] = { 0.8, 0.8, 0.8, 1.0 };
            GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
            GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
            GLfloat mat_shininess[] = { 1.0 };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
            glPushMatrix();
            glTranslatef(tran_x, tran_y, 0.2);
            glRotatef(mouse_angle, 0, 0, 1);
            glScalef(0.2, 0.2, 0.2);
            drawModel(slime);
            glPopMatrix();
        }

        // draw map
        { 
            GLfloat mat_ambient[] = { 0.8, 0.8, 0.8, 1.0 };
            GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
            GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
            GLfloat mat_shininess[] = { 1.0 };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
            glPushMatrix();
            glScalef(1.0, 1.0, 1.0);
            drawModel(map);
            glPopMatrix();
        }

        SDL_GL_SwapWindow(window);
    }

    return 0;
}

