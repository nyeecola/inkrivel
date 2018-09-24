#include "game_server.hpp"

void BindAddressToSocket(sockaddr_in server_address, int socket_file_descriptor);

int main(int argc, char **argv) {

    sockaddr player_address[MAX_PLAYERS] = {0};
    bool online[MAX_PLAYERS] = {0};

    int socket_file_descriptor = createUDPSocket();

    sockaddr_in server_address = InitializeServerAddr();

    BindAddressToSocket(server_address, socket_file_descriptor);

    // TODO : MUST BE INITIALIZED PROPERLY
    InputPacket input = {0};
    DrawPacket draw = {0};

    printf("Server is up\n");

    // Create players
    Character player[MAX_PLAYERS];
    for(int i = 0; i < MAX_PLAYERS; i++){
        player[i].pos = {0, 0, 0};
        player[i].hit_radius = 0.25;
        player[i].speed = 0.02;
        player[i].dir = {0, 0, 0};
    }

    // Create map
    Map map;
    for(int i = 0; i < MAX_PLAYERS; i++){
        map.characterList[i] = &player[i];
        map.model = loadWavefrontModel("../assets/map.obj", "../assets/map.png", VERTEX_ALL);
    }

    for(ever) {

        sockaddr addr = {0};
        socklen_t len = sizeof(addr);

        if ( recvfrom(socket_file_descriptor, &input, sizeof(input), 0, &addr, &len) != ERROR ) {
            //printf("Pacote Input %hu recebido.\n", input_packet.id);
            //printf("Pacote Input %hu enviado.\n", input.id);
            printf("mouse_X, mouse_Y = %f %f\n", input.mouse_x, input.mouse_y);
            // if (input.foward) printf("W"); else printf(" ");
            // if (input.back) printf("S"); else printf(" ");
            // if (input.right) printf("D"); else printf(" ");
            // if (input.left) printf("A"); else printf(" ");
            // if (input.shooting) printf("L"); else printf(" ");
            // if (input.especial) printf("E"); else printf(" ");
            // if (input.running) printf("R"); else printf(" ");
            // printf("\n");

            uint8_t id = input.player_id;

            if ( online[id] ) {
                if ( memcmp(&addr, &player_address[id], sizeof(sockaddr)) != 0 ) {
                    fprintf(stderr, "ERROR: Conflicting player IDs.");
                    exit(1);
                }
            } else {
                online[id] = true;
                draw.online[id] = true;
                memcpy(&player_address[id], &addr, sizeof(sockaddr));
            }

            // mouse position relative to the middle of the window
            float mouse_x, mouse_y;
            float mouse_angle;
            {
                mouse_x = input.mouse_x;
                mouse_y = input.mouse_y;
                mouse_x -= SCREEN_WIDTH/2;
                mouse_y -= SCREEN_HEIGHT/2;
                float norm = sqrt(mouse_x * mouse_x + mouse_y * mouse_y);
                mouse_x /= norm;
                mouse_y /= norm;
                mouse_angle = atan2(mouse_y, mouse_x) * 180 / M_PI;
                mouse_angle += 90;
                mouse_angle *= -1;
            }
            draw.mouse_angle[id] = mouse_angle;

            // move object in the direction of the mouse when W is pressed
            // TODO: use delta_time
            if (input.foward) {
                player[id].dir.x = mouse_x;
                player[id].dir.y = -mouse_y;
                player[id].dir.normalize();
                player[id].dir *= player[id].speed;
            }
            else {
                player[id].dir = {0, 0, 0};
            }

            // collision
            if (player[id].dir.len()) {
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
                        Vector A = map.model.vertices[cur->vertices[0]] - player[id].pos;
                        Vector B = map.model.vertices[cur->vertices[1]] - player[id].pos;
                        Vector C = map.model.vertices[cur->vertices[2]] - player[id].pos;
                        float rr = player[id].hit_radius * player[id].hit_radius;
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

                        if (!separated && player[id].dir.dot(normal) > 0) {
                            normal.z = 0;
                            normal.normalize();
                            normal *= player[id].speed * normal.dot(player[id].dir) / (normal.len()*player[id].dir.len());

                            player[id].dir -= normal;
                        }
                    }
                }
            }

            // player movement
            if (player[id].dir.len() > player[id].speed) {
                player[id].dir.normalize();
                player[id].dir *= player[id].speed;
            }
            player[id].pos += player[id].dir;
            draw.pos[id] += player[id].dir;


            for(int i = 0; i < MAX_PLAYERS ; i++) {
                if ( online[i] ) {
                    if ( sendto(socket_file_descriptor, &draw, sizeof(draw), 0, &player_address[i], sizeof(sockaddr)) != ERROR ) {
                        //printf("Pacote Draw %hu recebido.\n", draw.id);
                    } else {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            fprintf(stderr, "ERROR: Unespected error while sending draw packet. %s %d.\n", strerror(errno), errno);
                            exit(1);
                        }
                    }
                }
            }
            draw.id = ( draw.id + 1 ) % DRAW_ID_WINDOW;


        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while recieving input packet. %s.\n", strerror(errno));
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
