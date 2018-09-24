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
    draw.map_scale = 0.4;

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
    for(int i = 0; i < MAX_PLAYERS; i++){
        map.model = loadWavefrontModel("../assets/map7.obj",
                                       "../assets/map.png",
                                       VERTEX_ALL);
        map.characterList[i] = &player[i];
        map.scale = 0.4;
    }

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

            // TODO: make this client side
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

            // TODO: create a function for collision
            // collision
            Vector next_pos = player[id].pos + player[id].dir;
            Vector max_z = {0, 0, -200};
            Vector rotation_points[4] = {{-200, -200, -200}, {-200, -200, -200},
                                         {-200, -200, -200}, {-200, -200, -200}};
            Vector rotation_normals[4] = {{-200, -200, -200}, {-200, -200, -200},
                                          {-200, -200, -200}, {-200, -200, -200}};

            // DEBUG: currently use for painting
            Vector paint_max_z = {0, 0, -200};
            int paint_face;

            for (int i = 0; i < map.model.num_faces; i++) {
                Face *cur = &map.model.faces[i];

                Vector vertex0 = map.scale * map.model.vertices[cur->vertices[0]];
                Vector vertex1 = map.scale * map.model.vertices[cur->vertices[1]];
                Vector vertex2 = map.scale * map.model.vertices[cur->vertices[2]];

                Vector v1 = vertex2 - vertex0;
                Vector v2 = vertex1 - vertex0;
                Vector normal = v1.cross(v2);
                normal.normalize();
                if (normal.z < 0) {
                    normal = normal * -1;
                }

                Vector up = {0, 0, 1};

                float cosine = normal.dot(up);

                float angle = acos(cosine) * 180 / M_PI;

                // walls
                if (angle > 60 &&
                        (vertex0.z > next_pos.z ||
                         vertex1.z > next_pos.z ||
                         vertex2.z > next_pos.z)) {
                    bool collides = sphereCollidesTriangle(next_pos,
                                                           player[id].hit_radius,
                                                           vertex0, vertex1, vertex2);

                    if (collides) {
                        Vector v = player[id].pos - vertex0;
                        float d = v.dot(normal);
                        Vector collision = d*normal;

                        Vector reaction_v = player[id].dir + collision;
                        player[id].dir.x = reaction_v.x * player[id].dir.x > 0 ? reaction_v.x : 0;
                        player[id].dir.y = reaction_v.y * player[id].dir.y > 0 ? reaction_v.y : 0;
                        player[id].dir.z = reaction_v.z * player[id].dir.z > 0 ? reaction_v.z : 0;
                    }
                }

                // floors
                else {
                    Vector sky[4];
                    sky[0] = {player[id].pos.x + player[id].hit_radius, player[id].pos.y, 200};
                    sky[1] = {player[id].pos.x - player[id].hit_radius, player[id].pos.y, 200};
                    sky[2] = {player[id].pos.x, player[id].pos.y + player[id].hit_radius, 200};
                    sky[3] = {player[id].pos.x, player[id].pos.y - player[id].hit_radius, 200};

                    Vector ground = {0, 0, -1};

                    Vector intersect_v;
                    for (int j = 0; j < 4; j++) {
                        bool intersect = rayIntersectsTriangle(map, sky[j], ground, cur, intersect_v);
                        if (intersect) {
                            if (max_z.z < intersect_v.z) {
                                max_z = intersect_v;
                            }

                            if (rotation_points[j].z < intersect_v.z) {
                                rotation_points[j] = intersect_v;
                                rotation_normals[j] = normal;
                            }
                        }
                    }

                    // DEBUG: this makes the slime paint the ground where it walks
                    Vector sky_slime = {player[id].pos.x, player[id].pos.y, 200};
                    bool intersect = rayIntersectsTriangle(map, sky_slime, ground,
                                                           cur, intersect_v);
                    if (intersect && intersect_v.z > paint_max_z.z) {
                        paint_max_z = intersect_v;
                        paint_face = i;
                    }
                }
            }

            // TODO: fix this
            draw.paint = true;
            draw.paint_face = paint_face;
            draw.paint_max_z = paint_max_z;
            draw.paint_radius = 40;

            // rotation
            {
                Vector normal_sum = {0,0,0};
                for (int j = 0; j < 4; j++) {
                    normal_sum += rotation_normals[j];
                }
                normal_sum.normalize();
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
