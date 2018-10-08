#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

#include <stdio.h>
#include <assert.h>

#include "../../lib/render-types.hpp"
#include "../../lib/vector.hpp"
#include "../../lib/udp.hpp"
#include "../../lib/drawing.hpp"
#include "../../lib/config.hpp"

#define MAX(a, b) ((a) > (b) ? a : b)
#define MIN(a, b) ((a) < (b) ? a : b)

int main(int argc, char **argv) {

    int my_id;
    sscanf(argv[1], "%d", &my_id);

    int socket_file_descriptor = createUDPSocket();
    hostent *raw_server_address = DNSLookUp();
    sockaddr_in server_address = InitializeClientAddr(raw_server_address);
    sockaddr *server_adapted_address = (sockaddr *) &server_address;

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

    // Load models
    Models models = {0};
    models.character[0] = loadWavefrontModel("../assets/slime.obj", "../assets/slime.png", VERTEX_ALL);
    models.character[1] = loadWavefrontModel("../assets/slime.obj", "../assets/slime.png", VERTEX_ALL);
    models.character[2] = loadWavefrontModel("../assets/slime.obj", "../assets/slime.png", VERTEX_ALL);
    models.character[3] = loadWavefrontModel("../assets/slime.obj", "../assets/slime.png", VERTEX_ALL);
    models.map = loadWavefrontModel("../assets/map7.obj", "../assets/map2.png", VERTEX_ALL);

    // TODO: MUST BE INITIALIZED PROPERLY
    InputPacket input = {0};
    input.player_id = my_id;
    DrawPacket draw = {0};

    const Uint8 *kb_state = SDL_GetKeyboardState(NULL);
    bool running = true;
    input.running = true;
    while (running) {
        // handle events
        {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                    input.running = false;
                }
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    input.shooting = true;
                }
                if (e.type == SDL_MOUSEBUTTONUP) {
                    input.shooting = false;
                }
            }
        }

        {
            int int_mouse_x, int_mouse_y;
            SDL_GetMouseState(&int_mouse_x, &int_mouse_y);
            float mouse_x = int_mouse_x;
            float mouse_y = int_mouse_y;
            float mouse_angle;
            {
                mouse_x -= SCREEN_WIDTH/2;
                mouse_y -= SCREEN_HEIGHT/2;
                float norm = sqrt(mouse_x * mouse_x + mouse_y * mouse_y);
                mouse_x /= norm;
                mouse_y /= norm;
                mouse_angle = atan2(mouse_y, mouse_x) * 180 / M_PI;
                mouse_angle += 90;
                mouse_angle *= -1;
            }
            input.mouse_angle = mouse_angle;
        }
        input.up = kb_state[SDL_SCANCODE_W];
        input.down = kb_state[SDL_SCANCODE_S];
        input.right = kb_state[SDL_SCANCODE_D];
        input.left = kb_state[SDL_SCANCODE_A];
        input.especial = kb_state[SDL_SCANCODE_E];

        if (sendto(socket_file_descriptor, &input, sizeof(input), 0, server_adapted_address, sizeof(server_address)) == ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while sending input packet. %s.\n", strerror(errno));
                exit(1);
            }
        } else {
#if 0
            printf("mouse_X, mouse_Y = %f %f\n", input.mouse_x, input.mouse_y);
            //if (input.up) printf("W"); else printf(" ");
            //if (input.down) printf("S"); else printf(" ");
            //if (input.right) printf("D"); else printf(" ");
            //if (input.left) printf("A"); else printf(" ");
            //if (input.shooting) printf("L"); else printf(" ");
            //if (input.especial) printf("E"); else printf(" ");
            //if (input.running) printf("R"); else printf(" ");
            //printf("\n");
#endif
        }


        if (recvfrom(socket_file_descriptor, &draw, sizeof(draw), 0, NULL, NULL) == ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while recieving draw packet. %s.\n", strerror(errno));
                exit(1);
            }
        }

        // apply paint
        // TODO: be careful with order of packets
        for (uint32_t i = 0; i < draw.num_paint_points; i++) {
            //printf("i %d\n", i);
            paintCircle(models.map, MAP_SCALE,
                        &models.map.faces[draw.paint_points_faces[i]],
                        draw.paint_points_pos[i], draw.paint_points_radius[i],
                        0x1F, 0xFF, 0x1F);
        }

        // render
        glClearColor(0.4, 0.6, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // set camera position
        glTranslatef(-draw.pos[my_id].x, -draw.pos[my_id].y, -8);

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
            glScalef(MAP_SCALE, MAP_SCALE, MAP_SCALE);
            drawModel(models.map);
            glPopMatrix();
        }

        // draw projectiles
        for (uint32_t i = 0; i < draw.num_projectiles; i++) {
            drawSphere(draw.projectiles_pos[i], draw.projectiles_radius[i], 0, 1, 0);
        }

        glClear(GL_DEPTH_BUFFER_BIT);

        // draw slimes
        for(int i = 0; i < MAX_PLAYERS ; i++) {
            if ( draw.online[i] ) {
                GLfloat mat_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
                GLfloat mat_diffuse[] = { 0.3, 0.3, 0.3, 1.0 };
                GLfloat mat_specular[] = { 0.6, 0.6, 0.6, 1.0 };
                GLfloat mat_shininess[] = { 1.0 };
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

                glPushMatrix();
                glTranslatef(draw.pos[i].x, draw.pos[i].y, draw.pos[i].z);
                // From AllegroGL`s math.c
                glRotatef((2*acos(draw.rotations[i].w)) * 180 / M_PI,
                          draw.rotations[i].x,
                          draw.rotations[i].y,
                          draw.rotations[i].z);
                glRotatef(draw.mouse_angle[i], 0, 0, 1);
                switch (draw.model_id[i]) {
                    case TEST:
                        glScalef(TEST_SCALE);
                        break;
                    default:
                        assert(false);
                }
                drawModel(models.character[draw.model_id[i]]);
                glPopMatrix();

#if DEBUG
                // TODO: make this code work client side
                // draw slime hitsphere
                drawSphere(draw.pos[i], draw.hit_radius[i], 1, 0, 0);
#endif
            }

        }

        SDL_GL_SwapWindow(window);
    }

    close(socket_file_descriptor);

    return 0;
}
