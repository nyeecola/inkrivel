#include "../../lib/udp.hpp"

void BindAddressToSocket(sockaddr_in server_address, int socket_file_descriptor);

int main(int argc, char **argv) {

    int socket_file_descriptor = createUDPSocket();

    sockaddr_in server_address = InitializeServerAddr();

    BindAddressToSocket(server_address, socket_file_descriptor);

    InputPacket input_packet = {0};
    //DrawPacket draw_packet = {0};

    //draw_packet.id = 0;

    printf("Server is up\n");

    float x = 0;
    float y = 0;

    for(ever) {

        sockaddr addr = {0};
        socklen_t len = sizeof(addr);

        if ( recvfrom(socket_file_descriptor, &input_packet, sizeof(input_packet), 0, &addr, &len) != ERROR ) {

            //printf("Pacote Input %hu recebido.\n", input_packet.id);


            //printf("Pacote Draw %hu enviado.\n", draw_packet.id);

            //draw_packet.id = ( draw_packet.id + 1 ) % DRAW_ID_WINDOW;

            printf("%f  %f   ", x, y);
            if (input_packet.foward) y += 1;
            if (input_packet.back) y -= 1;
            if (input_packet.right) x += 1;
            if (input_packet.left) x -= 1;
            if (input_packet.shooting) printf("POWPOW");
            if (input_packet.especial) printf("ESPECIALZÂO");
            printf("\n");

            //sendto(socket_file_descriptor, &draw_packet, sizeof(draw_packet), 0, &addr, len);


        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while sending input packet. %s %d.\n", strerror(errno), errno);
                exit(1);
            }
        }
    }
    
    close(socket_file_descriptor);

    return 0;
}


void BindAddressToSocket(sockaddr_in server_address, int socket_file_descriptor) {

    sockaddr *addr = (sockaddr *) &server_address;

    if (bind(socket_file_descriptor, addr, sizeof(server_address)) == ERROR) {
        fprintf(stderr, "ERROR: Could not bind file descriptor to socket. %s.\n", strerror(errno));
        exit(1);
    }
}




















/*

int main() {


    // Create player slime
    Character slime;
    slime.pos = {0, 0, 0};
    slime.hit_radius = 0.25;
    slime.model = loadWavefrontModel("assets/slime.obj", "assets/slime.png", VERTEX_ALL);
    slime.speed = 0.02;
    slime.dir = {0, 0, 0};

    // Create map
    Map map;
    map.model = loadWavefrontModel("assets/map.obj", "assets/map.png", VERTEX_ALL);
    map.characterList[0] = &slime;

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
        if (slime.dir.len()) {
            for (int i = 0; i < map.model.num_faces; i++) {
                Face *cur = &map.model.faces[i];

                Vector v1 = map.model.vertices[cur->vertices[2]] - map.model.vertices[cur->vertices[0]];
                Vector v2 = map.model.vertices[cur->vertices[1]] - map.model.vertices[cur->vertices[0]];
                Vector normal = v1.cross(v2);
                normal.normalize();
                if (normal.z < 0) {
                    normal = normal * -1;
                }

                Vector up = {0, 0, 1};

                float cosine = normal.dot(up);

                float angle = acos(cosine) * 180 / M_PI;

                // Sphere-Triangle collision from: http://realtimecollisiondetection.net/blog/?p=103
                if (angle > 60) {
                    Vector A = map.model.vertices[cur->vertices[0]] - slime.pos;
                    Vector B = map.model.vertices[cur->vertices[1]] - slime.pos;
                    Vector C = map.model.vertices[cur->vertices[2]] - slime.pos;
                    float rr = slime.hit_radius * slime.hit_radius;
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

                    if (!separated && slime.dir.dot(normal) > 0) {
                        normal.z = 0;
                        normal.normalize();
                        normal *= slime.speed * normal.dot(slime.dir) / (normal.len()*slime.dir.len());

                        slime.dir -= normal;
                    }
                }
            }
        }

        // Slime movement
        if (slime.dir.len() > slime.speed) {
            slime.dir.normalize();
            slime.dir *= slime.speed;
        }
        slime.pos += slime.dir;

    }

    */