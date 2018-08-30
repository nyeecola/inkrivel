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

void drawCube() {
    glColor3f(1, 1, 1);

    glBegin(GL_POLYGON);
    glTexCoord2f(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(0, 1, 0);
    glTexCoord2f(1, 1);
    glVertex3f(1, 1, 0);
    glTexCoord2f(1, 0);
    glVertex3f(1, 0, 0);
    glEnd();

    glBegin(GL_POLYGON);
    glTexCoord2f(0, 0);
    glVertex3f(0, 0, 1);
    glTexCoord2f(0, 1);
    glVertex3f(0, 1, 1);
    glTexCoord2f(1, 1);
    glVertex3f(1, 1, 1);
    glTexCoord2f(1, 0);
    glVertex3f(1, 0, 1);
    glEnd();

    glBegin(GL_POLYGON);
    glTexCoord2f(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(0, 0, 1);
    glTexCoord2f(1, 1);
    glVertex3f(0, 1, 1);
    glTexCoord2f(1, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    glBegin(GL_POLYGON);
    glTexCoord2f(0, 0);
    glVertex3f(1, 0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(1, 0, 1);
    glTexCoord2f(1, 1);
    glVertex3f(1, 1, 1);
    glTexCoord2f(1, 0);
    glVertex3f(1, 1, 0);
    glEnd();

    glBegin(GL_POLYGON);
    glTexCoord2f(0, 0);
    glVertex3f(0, 1, 0);
    glTexCoord2f(0, 1);
    glVertex3f(0, 1, 1);
    glTexCoord2f(1, 1);
    glVertex3f(1, 1, 1);
    glTexCoord2f(1, 0);
    glVertex3f(1, 1, 0);
    glEnd();

    glBegin(GL_POLYGON);
    glTexCoord2f(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(0, 0, 1);
    glTexCoord2f(1, 1);
    glVertex3f(1, 0, 1);
    glTexCoord2f(1, 0);
    glVertex3f(1, 0, 0);
    glEnd();
}

typedef struct {
    float x;
    float y;
    float z;
} Vertex;

typedef struct {
    int a;
    int b;
    int c;
} Face;

typedef struct {
    Vertex *vertices;
    int num_vertices;
    Face *faces;
    int num_faces;
} Model;

Model load_wavefront_model(char *filename) {
    Model model = {0};
    model.vertices = (Vertex *) malloc(10000 * sizeof(Vertex));
    model.faces = (Face *) malloc(10000 * sizeof(Face));
    FILE *f = fopen(filename, "r");
    char type[40] = {0};
    while ((fscanf(f, " %s", type)) != EOF) {
        if (!strcmp(type, "v")) {
            Vertex v = {0};
            fscanf(f, " %f %f %f", &v.x, &v.y, &v.z);
            model.vertices[model.num_vertices++] = v;
        } else if (!strcmp(type, "f")) {
            Face face = {0};
            fscanf(f, " %d %d %d", &face.a, &face.b, &face.c);
            model.faces[model.num_faces++] = face;
        }
    }
    return model;
}

void draw_model(Model model) {
    srand(37);
    for (int i = 0; i < model.num_faces; i++) {
        Face f = model.faces[i];
        glBegin(GL_TRIANGLES);
        //float r = (rand() % 1000) / 1000.0;
        float g = (rand() % 1000) / 1000.0;
        //float b = (rand() % 1000) / 1000.0;
        glColor3f(0.3, min(0.45 + g, 1), 0.3);
        glVertex3f(model.vertices[f.a].x,
                   model.vertices[f.a].y,
                   model.vertices[f.a].z);
        glVertex3f(model.vertices[f.b].x,
                   model.vertices[f.b].y,
                   model.vertices[f.b].z);
        glVertex3f(model.vertices[f.c].x,
                   model.vertices[f.c].y,
                   model.vertices[f.c].z);
        glEnd();
    }
}

char *load_img(char *filename) {
    
}

int main() {
    SDL_Window *window = getWindow();
    assert(window);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // DEBUG
    Model model = load_wavefront_model("assets/model.obj");
    printf("%d %d\n", model.num_faces, model.num_vertices);

    // DEBUG
    SDL_Surface *wood_surface = IMG_Load("assets/wood.png");
    printf("%d %d %d\n", wood_surface->w, wood_surface->h, wood_surface->pitch);
    unsigned int *pixels = (unsigned int *) wood_surface->pixels;
    printf("%x %u %u %u %u\n", pixels[0], pixels[0] & 0xff000000, pixels[0] & 0x00ff0000, pixels[0] & 0x0000ff00, pixels[0] & 0x000000ff);

    glEnable(GL_TEXTURE_2D);
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    float rotx = 0;
    float roty = 0;
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

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_FILTER, GL_LINEAR);
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND/GL_REPLACE/GL_MODULATE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexImage2D(GL_TEXTURE_2D, 0, 4, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, wood_surface->pixels);

        glPushMatrix();
        glRotatef(rotx, 1, 0, 0);
        glRotatef(roty, 0, 1, 0);
        drawCube();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(tranx, trany, -10);
        drawCube();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-tranx, -trany, -10);
        glRotatef(90, 1, 0, 0);
        glScalef(0.5, 0.5, 0.5);
        draw_model(model);
        glPopMatrix();

        rotx += 1;
        roty += 0.9;

        SDL_GL_SwapWindow(window);
    }

end:
    return 0;
}
