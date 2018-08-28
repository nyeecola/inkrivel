#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

#include <stdio.h>
#include <assert.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

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

int main() {
    SDL_Window *window = getWindow();
    assert(window);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    float rotx = 0;
    float roty = 0;
    while (1) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                goto end;
            }
        }

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        makeFrustum(45, 4.0/3, 0.1, 100);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, -10);
        glRotatef(rotx, 1, 0, 0);
        glRotatef(roty, 0, 1, 0);

        //glPushMatrix();
        glBegin(GL_POLYGON);

        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 1, 0);
        glVertex3f(1, 1, 0);
        glVertex3f(1, 0, 0);

        glEnd();

        glBegin(GL_POLYGON);

        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 1);
        glVertex3f(0, 1, 1);
        glVertex3f(1, 1, 1);
        glVertex3f(1, 0, 1);

        glEnd();

        glBegin(GL_POLYGON);

        glColor3f(0, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 1);
        glVertex3f(0, 1, 1);
        glVertex3f(0, 1, 0);

        glEnd();

        glBegin(GL_POLYGON);

        glColor3f(1, 1, 1);
        glVertex3f(1, 0, 0);
        glVertex3f(1, 0, 1);
        glVertex3f(1, 1, 1);
        glVertex3f(1, 1, 0);

        glEnd();

        glBegin(GL_POLYGON);

        glColor3f(1, 0, 1);
        glVertex3f(0, 1, 0);
        glVertex3f(0, 1, 1);
        glVertex3f(1, 1, 1);
        glVertex3f(1, 1, 0);

        glEnd();

        glBegin(GL_POLYGON);

        glColor3f(0, 1, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 1);
        glVertex3f(1, 0, 1);
        glVertex3f(1, 0, 0);

        glEnd();
        //glPopMatrix();
        
        rotx += 1;
        roty += 0.9;

        //glFlush();
        SDL_GL_SwapWindow(window);
    }

end:
    return 0;
}
