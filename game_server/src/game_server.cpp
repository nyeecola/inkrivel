#include "../../lib/render-types.hpp"
#include "../../lib/vector.hpp"
#include "../../lib/udp.hpp"
#include "../../lib/drawing.hpp"

#include "../../lib/character.hpp"
#include "../../lib/map.hpp"
//#include "base/character-test.cpp"

void bindAddressToSocket(sockaddr_in server_address, int socket_file_descriptor) {
    sockaddr *addr = (sockaddr *) &server_address;

    if (bind(socket_file_descriptor, addr, sizeof(server_address)) == ERROR) {
        fprintf(stderr, "ERROR: Could not bind file descriptor to socket. %s.\n",
                strerror(errno));
        exit(1);
    }
}

int main(int argc, char **argv) {
    sockaddr player_address[MAX_PLAYERS] = {0};
    bool online[MAX_PLAYERS] = {0};

    int socket_file_descriptor = createUDPSocket();
    sockaddr_in server_address = initializeServerAddr();
    bindAddressToSocket(server_address, socket_file_descriptor);

    // TODO: MUST BE INITIALIZED PROPERLY
    InputPacket input = {0};
    DrawPacket draw = {0};

    printf("Server is up\n");

    // Create players
    Character player[MAX_PLAYERS];
    for(int i = 0; i < MAX_PLAYERS; i++){
        player[i].pos = {0, 0, 0.35};
        player[i].hit_radius = 0.25;
        player[i].speed = 0.02;
        player[i].dir = {0, 0, 0};
        player[i].rotation = {0, 0, 0, 1};
    }

    // Create map
    Map map;
    map.model = loadWavefrontModel("../assets/map7.obj",
            "../assets/map2.png",
            VERTEX_ALL);
    map.scale = MAP_SCALE;
    for(int i = 0; i < MAX_PLAYERS; i++){
        map.characterList[i] = &player[i];
    }

    uint 
    for(ever) {
        sockaddr addr = {0};
        socklen_t len = sizeof(addr);

        if (recvfrom(socket_file_descriptor, &input, sizeof(input), 0, &addr, &len)
                != ERROR) {

#if 0
            printf("mouse_X, mouse_Y = %f %f\n", input.mouse_x, input.mouse_y);
            // if (input.foward) printf("W"); else printf(" ");
            // if (input.back) printf("S"); else printf(" ");
            // if (input.right) printf("D"); else printf(" ");
            // if (input.left) printf("A"); else printf(" ");
            // if (input.shooting) printf("L"); else printf(" ");
            // if (input.especial) printf("E"); else printf(" ");
            // if (input.running) printf("R"); else printf(" ");
            // printf("\n");
#endif

            uint8_t id = input.player_id;

            // TODO: check if this is a good way to achive what we want
            if (!online[id]) {
                online[id] = true;
                draw.online[id] = true;
                memcpy(&player_address[id], &addr, sizeof(sockaddr));
            }

            // mouse position relative to the middle of the window
            draw.mouse_angle[id] = input.mouse_angle;

            // move object in the direction of the keys
            // TODO: use delta_time
            player[id].dir = {0, 0, 0};
            if (input.down) {
                player[id].dir.y -= 1;
            }
            if (input.up) {
                player[id].dir.y += 1;
            }
            if (input.left) {
                player[id].dir.x -= 1;
            }
            if (input.right) {
                player[id].dir.x += 1;
            }
            player[id].dir.normalize();
            player[id].dir *= player[id].speed;

            // collision
            Vector max_z = {0, 0, -200};
            Vector normal_sum = {0, 0, 0};
            Vector paint_max_z = {0, 0, -200};
            int paint_face;
            collidesWithMap(map, player[id], normal_sum, max_z, paint_max_z, paint_face);

            // TODO: fix this
            draw.paint = true;
            draw.paint_face = paint_face;
            draw.paint_max_z = paint_max_z;
            draw.paint_radius = 40;

            // TODO: get this from lobby server
            for (int i = 0; i < MAX_PLAYERS; i++) {
                draw.model_id[i] = TEST;
            }

            // rotation
            {
                player[id].rotation = getRotationQuat({0,0,1}, normal_sum);
                draw.rotations[id] = player[id].rotation;
            }

            // player movement
            {
                if (player[id].dir.len() > player[id].speed) {
                    player[id].dir.normalize();
                    player[id].dir *= player[id].speed;
                }
                player[id].pos += player[id].dir;
                if (max_z.z > 0) {
                    player[id].pos.z += 0.5*(max_z.z - player[id].pos.z);
                }
                draw.pos[id] = player[id].pos;
            }

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
