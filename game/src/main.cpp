#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

#include <stdio.h>
#include <assert.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define max(a, b) ((a) > (b) ? a : b)
#define min(a, b) ((a) < (b) ? a : b)

SDL_Window *getWindow(){
    SDL_Window *window = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }
    window = SDL_CreateWindow("InkRivel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return NULL;
    }

    return window;
}

// This creates a symmetric frustum.
// It converts to 6 params (l, r, b, t, n, f) for glFrustum()
// from given 4 params (fovy, aspect, near, far)
void makeFrustum (double fovY, double aspectRatio, double front, double back) {
    const double DEG2RAD = 3.14159265 / 180;

    double tangent = tan(fovY/2 * DEG2RAD);   // tangent of half fovY
    double height = front * tangent;          // half height of near plane
    double width = height * aspectRatio;      // half width of near plane

    // params: left, right, bottom, top, near, far
    glFrustum(-width, width, -height, height, front, back);
}

typedef struct {
    float x;
    float y;
    float z;
} Vertex;

typedef struct {
    float x;
    float y;
    float z;
} Normal;

typedef struct {
    float x;
    float y;
} Texture;

typedef struct {
    int a;
    int b;
    int c;
    int na;
    int nb;
    int nc;
    int ta;
    int tb;
    int tc;
} Face;

typedef struct {
    Vertex *vertices;
    int num_vertices;
    Face *faces;
    int num_faces;
    Normal *normals;
    int num_normals;
    Texture *textures;
    int num_textures;
    GLuint texture_id;
} Model;

typedef enum {
    VERTEX_ONLY,
    VERTEX_NORMAL,
    VERTEX_TEXTURE,
    ALL
} FaceType;

Model load_wavefront_model(char *obj_filename, char *texture_filename, FaceType face_type) {
    SDL_Surface *sur = IMG_Load(texture_filename);

    Model model = {0};

    model.vertices = (Vertex *) malloc(10000 * sizeof(Vertex));
    model.faces = (Face *) malloc(10000 * sizeof(Face));
    model.normals = (Normal *) malloc(10000 * sizeof(Normal));
    model.textures = (Texture *) malloc(10000 * sizeof(Texture));

    glGenTextures(1, &model.texture_id);
    glBindTexture(GL_TEXTURE_2D, model.texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, sur->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    FILE *f = fopen(obj_filename, "r");
    char type[40] = {0};
    while ((fscanf(f, " %s", type)) != EOF) {
        if (!strcmp(type, "v")) {
            Vertex v = {0};

            fscanf(f, " %f %f %f", &v.x, &v.y, &v.z);
            model.vertices[++model.num_vertices] = v;
        } else if (!strcmp(type, "vn")) {
            Normal n = {0};

            fscanf(f, " %f %f %f", &n.x, &n.y, &n.z);
            model.normals[++model.num_normals] = n;
        } else if (!strcmp(type, "vt")) {
            Texture t = {0};

            fscanf(f, " %f %f", &t.x, &t.y);
            t.y = 1 - t.y;
            model.textures[++model.num_textures] = t;
        } else if (!strcmp(type, "f")) {
            Face face = {0};

            if (face_type == VERTEX_ONLY) {
                fscanf(f, " %d %d %d", &face.a, &face.b, &face.c);
            } else if (face_type == VERTEX_NORMAL) {
                fscanf(f, " %d//%d %d//%d %d//%d", &face.a, &face.na, &face.b, &face.nb, &face.c, &face.nc);
            } else if (face_type == VERTEX_TEXTURE) {
                // TODO
            } else if (face_type == ALL) {
                fscanf(f, " %d/%d/%d %d/%d/%d %d/%d/%d", &face.a, &face.ta, &face.na, &face.b, &face.tb, &face.nb, &face.c, &face.tc, &face.nc);
            } else {
                assert(false);
            }

            model.faces[model.num_faces++] = face;
        }
    }
    return model;
}

void draw_model(Model model) {
    glBindTexture(GL_TEXTURE_2D, model.texture_id);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    for (int i = 0; i < model.num_faces; i++) {
        Face f = model.faces[i];
        glBegin(GL_TRIANGLES);
        if (model.num_normals) {
            glNormal3f(model.normals[f.na].x,
                       model.normals[f.na].y,
                       model.normals[f.na].z);
        }
        if (model.num_textures) {
            glTexCoord2f(model.textures[f.ta].x,
                         model.textures[f.ta].y);
        }
        glVertex3f(model.vertices[f.a].x,
                   model.vertices[f.a].y,
                   model.vertices[f.a].z);
        if (model.num_normals) {
            glNormal3f(model.normals[f.nb].x,
                       model.normals[f.nb].y,
                       model.normals[f.nb].z);
        }
        if (model.num_textures) {
            glTexCoord2f(model.textures[f.tb].x,
                         model.textures[f.tb].y);
        }
        glVertex3f(model.vertices[f.b].x,
                   model.vertices[f.b].y,
                   model.vertices[f.b].z);
        if (model.num_normals) {
            glNormal3f(model.normals[f.nc].x,
                       model.normals[f.nc].y,
                       model.normals[f.nc].z);
        }
        if (model.num_textures) {
            glTexCoord2f(model.textures[f.tc].x,
                         model.textures[f.tc].y);
        }
        glVertex3f(model.vertices[f.c].x,
                   model.vertices[f.c].y,
                   model.vertices[f.c].z);
        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

int main() {
    SDL_Window *window = getWindow();
    assert(window);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    //glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    // load slime model
    Model slime = load_wavefront_model("assets/slime.obj", "assets/slime.png", ALL);

    float tranx = 0;
    float trany = 0;
    const Uint8 *kbState = SDL_GetKeyboardState(NULL);
    while (1) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                goto end;
            }
        }

        if (kbState[SDL_SCANCODE_W]) {
            trany += 0.1;
        }
        if (kbState[SDL_SCANCODE_A]) {
            tranx -= 0.1;
        }
        if (kbState[SDL_SCANCODE_S]) {
            trany -= 0.1;
        }
        if (kbState[SDL_SCANCODE_D]) {
            tranx += 0.1;
        }

        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        makeFrustum(45, 4.0/3, 0.1, 100);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, -10);

        // draw light
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
            glLighti(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 3.5);
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
            glTranslatef(tranx, trany, 4);
            glScalef(0.5, 0.5, 0.5);
            draw_model(slime);
            glPopMatrix();
        }

        SDL_GL_SwapWindow(window);
    }

end:
    return 0;
}
