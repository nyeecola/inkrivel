#include "game_client.hpp"

int main(int argc, char **argv) {

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
    models.character[0] = loadWavefrontModel("assets/slime.obj", "assets/slime.png", VERTEX_ALL);
    models.character[1] = loadWavefrontModel("assets/slime.obj", "assets/slime.png", VERTEX_ALL);
    models.character[2] = loadWavefrontModel("assets/slime.obj", "assets/slime.png", VERTEX_ALL);
    models.character[3] = loadWavefrontModel("assets/slime.obj", "assets/slime.png", VERTEX_ALL);
    models.map = loadWavefrontModel("assets/map.obj", "assets/map.png", VERTEX_ALL);

    InputPacket input = {0};
    input.id = 0;


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
            input.mouse_x = int_mouse_x;
            input.mouse_y = int_mouse_y;
            input.mouse_x -= SCREEN_WIDTH/2;
            input.mouse_y -= SCREEN_HEIGHT/2;
            float norm = sqrt(input.mouse_x * input.mouse_x + input.mouse_y * input.mouse_y);
            input.mouse_x /= norm;
            input.mouse_y /= norm;
        }
        input.foward = kb_state[SDL_SCANCODE_W];
        input.back = kb_state[SDL_SCANCODE_S];
        input.right = kb_state[SDL_SCANCODE_D];
        input.left = kb_state[SDL_SCANCODE_A];
        input.especial = kb_state[SDL_SCANCODE_E];

        if (sendto(socket_file_descriptor, &input, sizeof(input), 0, server_adapted_address, sizeof(server_address)) == ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while sending input packet. %s.\n", strerror(errno));
                exit(1);
            }
        } else {
            printf("Pacote Input %hu enviado.\n", input.id);
            printf("id = %d\n", input.id);
            printf("mouse_X, mouse_Y = %f %f\n", input.mouse_x, input.mouse_y);
            if (input.foward) printf("W"); else printf(" ");
            if (input.back) printf("S"); else printf(" ");
            if (input.right) printf("D"); else printf(" ");
            if (input.left) printf("A"); else printf(" ");
            if (input.shooting) printf("L"); else printf(" ");
            if (input.especial) printf("E"); else printf(" ");
            if (input.running) printf("R"); else printf(" ");
            printf("\n");
            input.id = ( input.id + 1 ) % INPUT_ID_WINDOW;
        }


        usleep(200000);
/*
        if (recvfrom(socket_file_descriptor, &draw_packet, sizeof(draw_packet), 0, NULL, NULL) == ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while recieving draw packet. %s.\n", strerror(errno));
                exit(1);
            }
        } else {
           printf("Pacote Draw %hu recebido.\n", draw_packet.id);
        }
*/
    }

    close(socket_file_descriptor);

    return 0;
}





























/*




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
            glTranslatef(slime.pos.x, slime.pos.y, 0.2);
            glRotatef(mouse_angle, 0, 0, 1);
            glScalef(0.2, 0.2, 0.2);
            drawModel(slime.model);
            glPopMatrix();
        }

        // draw slime hitsphere
#if DEBUG
        drawSphere(slime.pos, slime.hit_radius);
#endif

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
            drawModel(map.model);
            glPopMatrix();
        }

        SDL_GL_SwapWindow(window);
    }

    return 0;
}




    InputPacket input_packet = {0};
    DrawPacket draw_packet = {0};

    input_packet.id = 0;

    for(ever) {


        if (recvfrom(socket_file_descriptor, &draw_packet, sizeof(draw_packet), 0, NULL, NULL) == ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while recieving draw packet. %s.\n", strerror(errno));
                exit(1);
            }
        } else {
           printf("Pacote Draw %hu recebido.\n", draw_packet.id);
        }
    }


    return 0;
}
*/