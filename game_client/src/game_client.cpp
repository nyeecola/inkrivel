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
    //SDL_GL_SetSwapInterval(0);

    // OpenGL properties
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    //glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    // OpenGL projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    makeFrustum(45, (float) SCREEN_WIDTH / SCREEN_HEIGHT, 0.1, 100);

    stbtt_initfont();

    // Load models
    Models models = {0};
    models.green_character[0] = loadWavefrontModel("../assets/slime.obj", "../assets/slime_green.png", VERTEX_ALL, 1024);
    models.green_character[1] = loadWavefrontModel("../assets/slime.obj", "../assets/slime_green.png", VERTEX_ALL, 1024);
    models.green_character[2] = loadWavefrontModel("../assets/slime.obj", "../assets/slime_green.png", VERTEX_ALL, 1024);
    models.green_character[3] = loadWavefrontModel("../assets/slime.obj", "../assets/slime_green.png", VERTEX_ALL, 1024);
    models.pink_character[0] = loadWavefrontModel("../assets/slime.obj", "../assets/slime_pink.png", VERTEX_ALL, 1024);
    models.pink_character[1] = loadWavefrontModel("../assets/slime.obj", "../assets/slime_pink.png", VERTEX_ALL, 1024);
    models.pink_character[2] = loadWavefrontModel("../assets/slime.obj", "../assets/slime_pink.png", VERTEX_ALL, 1024);
    models.pink_character[3] = loadWavefrontModel("../assets/slime.obj", "../assets/slime_pink.png", VERTEX_ALL, 1024);

#if 0
    models.map = loadWavefrontModel("../assets/map7.obj", "../assets/map3.png",
                                    VERTEX_ALL, MAP_TEXTURE_SIZE);
#else
    models.map = loadWavefrontModel("../assets/cherie.obj", "../assets/map3.png",
                                    VERTEX_ALL, MAP_TEXTURE_SIZE);
#endif

    // TODO: MUST BE INITIALIZED PROPERLY
    InputPacket input = {0};
    input.player_id = my_id;
    DrawPacket draw = {0};

    const Uint8 *kb_state = SDL_GetKeyboardState(NULL);
    bool running = true;

    uint64_t last_time = getTimestamp();
    float light_x = 0;
    float light_y = 0;

    while (running) {
        uint64_t cur_time = getTimestamp();
        float dt = (float) (cur_time - last_time) / 1000.0f; // milliseconds
        last_time = cur_time;

        light_x += dt;
        light_y += dt / 2;

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        // handle events
        {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
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
        input.swimming = kb_state[SDL_SCANCODE_SPACE];

#if 1
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
#endif


#if 1
        DrawPacket temp_draw;
        draw.num_paint_points = 0;
        while (recvfrom(socket_file_descriptor, &temp_draw, sizeof(temp_draw),
                        0, NULL, NULL) != ERROR) {
            if (draw.frame == 0) {
                draw = temp_draw;
                continue;
            }

            if (temp_draw.frame > draw.frame) {
                DrawPacket aux = draw;
                draw = temp_draw;

                memcpy(&draw.paint_points[draw.num_paint_points],
                       aux.paint_points,
                       aux.num_paint_points * sizeof(*aux.paint_points));
                draw.num_paint_points += aux.num_paint_points;
            } else {
                memcpy(&draw.paint_points[draw.num_paint_points],
                        temp_draw.paint_points,
                        temp_draw.num_paint_points * sizeof(*temp_draw.paint_points));
                draw.num_paint_points += temp_draw.num_paint_points;
            }
        }
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            fprintf(stderr, "ERROR: Unespected error while recieving draw packet. %s.\n", strerror(errno));
            exit(1);
        }
#else
        if (recvfrom(socket_file_descriptor, &draw, sizeof(draw), 0, NULL, NULL) == ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while recieving draw packet. %s.\n", strerror(errno));
                exit(1);
            }
        }
#endif

        // apply paint
        // TODO: be careful with order of packets
        for (uint32_t i = 0; i < draw.num_paint_points; i++) {
            uint32_t color;
            if (draw.paint_points[i].team) {
                color = 0xFFFF1FFF;
            } else {
                color = 0xFF1FFF1F;
            }

            paintCircle(models.map, models.map.faces[draw.paint_points[i].face],
                        draw.paint_points[i].pos, draw.paint_points[i].radius,
                        color, true);
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
            GLfloat light_position[] = {
                (float) (LIGHT_END_X - LIGHT_START_X) * light_x / (float) TIMER_DURATION_IN_SECONDS,
                (float) (LIGHT_END_Y - LIGHT_START_Y) * light_y / (float) TIMER_DURATION_IN_SECONDS,
                8, 1
            };
            GLfloat ambient[] = { 0.2, 0.2, 0.2, 1 };
            GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1 };
            GLfloat specular[] = { 0.6, 0.6, 0.6, 1 };
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

            // default is (1, 0, 0)
            glLighti(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
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
            GLfloat mat_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
            GLfloat mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
            GLfloat mat_specular[] = { 0.5, 0.5, 0.5, 1.0 };
#endif
            GLfloat mat_shininess[] = { 1 };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
            glPushMatrix();
            glScalef(MAP_SCALE, MAP_SCALE, MAP_SCALE);
            drawModel(models.map, draw.respawn_timer[my_id] >= 0);
            glPopMatrix();
        }

        // draw projectiles
        for (uint32_t i = 0; i < draw.num_projectiles; i++) {
            float r, g, b;
            if (draw.projectiles_team[i]) {
                r = 1 * 0.6;
                g = 0 * 0.6;
                b = 1 * 0.6;
            } else {
                r = 0 * 0.6;
                g = 1 * 0.6;
                b = 0 * 0.6;
            }
            if (draw.respawn_timer[my_id] >= 0) { // grayscale
                float brightness = 0.2126*r + 0.7152*g + 0.0722*b;
                drawSphere(draw.projectiles_pos[i], draw.projectiles_radius[i],
                           brightness, brightness, brightness);
            } else { // normal
                drawSphere(draw.projectiles_pos[i], draw.projectiles_radius[i], r, g, b);
            }
        }

        //glClear(GL_DEPTH_BUFFER_BIT);

        // draw players
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (draw.online[i] && draw.respawn_timer[i] == -1) {
                GLfloat mat_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
                GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
                GLfloat mat_specular[] = { 0.7, 0.7, 0.7, 1.0 };
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

                // model specific stuff
                switch (draw.model_id[i]) {
                    case TEST:
                        glScalef(TEST_SCALE);
                        break;
                    case ROLO:
                        glScalef(ROLO_SCALE);
                        break;
                    case SNIPER:
                        glScalef(SNIPER_SCALE);
                        break;
                    case ASSAULT:
                        glScalef(ASSAULT_SCALE);
                        break;
                    case BUCKET:
                        glScalef(BUCKET_SCALE);
                        break;
                    default:
                        assert(false);
                }

                if (draw.swimming[i]) {
                    glTranslatef(0, 0, -0.4);
                }

                if (i % 2) {
                    drawModel(models.pink_character[draw.model_id[i]],
                              draw.respawn_timer[my_id] >= 0);
                } else {
                    drawModel(models.green_character[draw.model_id[i]],
                              draw.respawn_timer[my_id] >= 0);
                }

                glPopMatrix();

#if DEBUG
                // TODO: make this code work client side
                // draw slime hitsphere
                drawSphere(draw.pos[i], draw.hit_radius[i], 1, 0, 0);
#endif
            }
        }

        // draw ammo bar
        if (input.swimming && draw.respawn_timer[my_id] == -1) {
            assert(draw.ammo[my_id] >= 0);

            float x = draw.pos[my_id].x;
            float y = draw.pos[my_id].y;

            float r, g, b;
            if (my_id % 2) {
                r = 1;
                g = 0;
                b = 1;
            } else {
                r = 0;
                g = 1;
                b = 0;
            }
            
            float ratio = draw.ammo[my_id] / (float) STARTING_AMMO;

            drawRect(x + AMMO_BOX_X_OFFSET, y + AMMO_BOX_Y_OFFSET,
                     AMMO_BOX_WIDTH, AMMO_BOX_HEIGHT, 0, 0, 0);
            drawRect(x + AMMO_BOX_X_OFFSET + AMMO_BOX_BORDER,
                     y + AMMO_BOX_Y_OFFSET + AMMO_BOX_BORDER,
                     AMMO_BOX_WIDTH - AMMO_BOX_BORDER * 2,
                     (AMMO_BOX_HEIGHT - AMMO_BOX_BORDER * 2) * ratio, r, g, b);
        }

        // font
        {
            prepareDrawFont();

            stbtt_print(TIMER_X, TIMER_Y, draw.timer);
            if (draw.respawn_timer[my_id] >= 0) {
                char text[2] = {(char) (draw.respawn_timer[my_id] + '0'), 0};
                stbtt_print(RESPAWN_TIMER_X, RESPAWN_TIMER_Y, text, 1, 0, 0);
            }

            endDrawFont();
        }

        SDL_GL_SwapWindow(window);
    }

    close(socket_file_descriptor);

    return 0;
}
