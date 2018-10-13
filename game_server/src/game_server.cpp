#include <pthread.h>

#include "../../lib/render-types.hpp"
#include "../../lib/vector.hpp"
#include "../../lib/udp.hpp"
#include "../../lib/drawing.hpp"

#include "../../lib/base.hpp"
#include "../../lib/map.hpp"
//#include "base/character-test.cpp"

#define THREAD_MUTEX_DELAY 200
#define MAX_INPUT_BUFFER_SIZE 200

// milliseconds
#define TICK_TIME 10

void bindAddressToSocket(sockaddr_in server_address, int socket_fd) {
    sockaddr *addr = (sockaddr *) &server_address;

    if (bind(socket_fd, addr, sizeof(server_address)) == ERROR) {
        fprintf(stderr, "ERROR: Could not bind file descriptor to socket. %s.\n",
                strerror(errno));
        exit(1);
    }
}

sockaddr player_address[MAX_PLAYERS] = {0};
sockaddr addr = {0};
int socket_fd;
int global_buffer_index = 0;
PacketBuffer global_input_buffer[2] = { PacketBuffer(INPUT, MAX_INPUT_BUFFER_SIZE),
                                        PacketBuffer(INPUT, MAX_INPUT_BUFFER_SIZE) };
bool should_die = false;
bool online[MAX_PLAYERS] = {0};
// TODO: MUST BE INITIALIZED PROPERLY
DrawPacket draw = {0};

void *listenInputs(void *arg) {
    while (!should_die) {
        socklen_t len = sizeof(addr);

        InputPacket input = {0};

        if (recvfrom(socket_fd, &input, sizeof(input), 0, &addr, &len) != ERROR) {
            int success = global_input_buffer[global_buffer_index].insert(INPUT, &input);
            assert(success);

            // TODO: check if this is a good way to achive what we want
            if (!online[input.player_id]) {
                printf("Player %d connected.\n", input.player_id);
                online[input.player_id] = true;
                draw.online[input.player_id] = true;
                memcpy(&player_address[input.player_id], &addr, sizeof(sockaddr));
            }
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while recieving input packet. %s.\n", strerror(errno));
                exit(1);
            }
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    socket_fd = createUDPSocket();
    sockaddr_in server_address = initializeServerAddr();
    bindAddressToSocket(server_address, socket_fd);

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
    map.model = loadWavefrontModel("../assets/map7.obj", "../assets/map2.png", VERTEX_ALL);
    for(int i = 0; i < MAX_PLAYERS; i++){
        map.characterList[i] = &player[i];
    }

    int num_projectiles = 0;
    Projectile projectiles[MAX_PROJECTILES] = {};

    {
        pthread_t listener;
        pthread_create(&listener, NULL, listenInputs, (void *) 0);
    }

    uint64_t tick_count = 0;
    uint64_t last_time = getTimestamp();
    uint64_t accumulated_time = 0;

    int game_timer = TIMER_DURATION_IN_SECONDS * 1000;

    for(ever) {
        uint64_t cur_time = getTimestamp();
        uint64_t dt = cur_time - last_time; // TODO: maybe in seconds later in the future
        last_time = cur_time;

        accumulated_time += dt;
        game_timer -= dt;

        // do tick
        if (accumulated_time > TICK_TIME) {
            global_buffer_index = !global_buffer_index;
            usleep(THREAD_MUTEX_DELAY); // polite thread safety mechanism

            // reset paint commands
            draw.num_paint_points = 0;

            while (global_input_buffer[!global_buffer_index].packets) {
                int index = --global_input_buffer[!global_buffer_index].packets;

                InputPacket input = global_input_buffer[!global_buffer_index].input_buffer[index];

                uint8_t id = input.player_id;

                // mouse position relative to the middle of the window
                draw.mouse_angle[id] = input.mouse_angle;

                // move object in the direction of the keys
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
                if (input.shooting) {
                    float mouse_angle = input.mouse_angle * -1;
                    mouse_angle -= 90;
                    Vector looking(0, 0, 0);
                    looking.x = cos(mouse_angle * M_PI / 180);
                    looking.y = -sin(mouse_angle * M_PI / 180);
                    looking.normalize();

                    assert(num_projectiles < MAX_PROJECTILES);
                    projectiles[num_projectiles].pos = player[id].pos + Vector(0, 0, 0.25);
                    // always shooting straight, z can be changed if that's not what you want
                    projectiles[num_projectiles].dir = looking;
                    projectiles[num_projectiles].radius = 0.04;
                    projectiles[num_projectiles].speed = 0.03;
                    projectiles[num_projectiles].team = id % 2;

                    num_projectiles++;
                }
                player[id].dir.normalize();
                player[id].dir *= player[id].speed;
            }

            for(int id = 0; id < MAX_PLAYERS; id++) {
                if (!online[id]) continue;

                // collision
                Vector max_z = {0, 0, -200};
                Vector normal_sum = {0, 0, 0};
                Vector paint_max_z = {0, 0, -200};
                int paint_face;
                collidesWithMap(map, player[id], normal_sum, max_z,
                                paint_max_z, paint_face);

                // paint
                {
                    PaintPoint pp = {};
                    pp.team = id % 2;
                    pp.pos = paint_max_z;
                    pp.face = paint_face;
                    // TODO: fix this number and do this only for ROLO
                    pp.radius = 40;
                    draw.paint_points[draw.num_paint_points++] = pp;

                    uint8_t r, g, b;
                    if (id % 2) {
                        r = 0xFF;
                        g = 0x1F;
                        b = 0xFF;
                    } else {
                        r = 0x1F;
                        g = 0xFF;
                        b = 0x1F;
                    }

                    paintCircle(map.model, MAP_SCALE,
                                &map.model.faces[pp.face],
                                pp.pos, pp.radius,
                                r, g, b, false);
                }

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
            }

            // projectile simulation
            {
                for (int i = 0; i < num_projectiles; i++) {

                    // check collision
                    Vector paint_pos;
                    int paint_face;
                    bool collides = projectileCollidesWithMap(map, projectiles[i],
                            paint_pos, paint_face);

                    if (collides) {
                        // paint
                        {
                            PaintPoint pp = {};
                            pp.team = projectiles[i].team;
                            pp.pos = paint_pos;
                            pp.face = paint_face;
                            //draw.paint_points_radius[draw.num_paint_points] = projectiles[i].radius * projectiles[i].radius;
                            // TODO: fix this number
                            pp.radius = 40;
                            draw.paint_points[draw.num_paint_points++] = pp;

                            uint8_t r, g, b;
                            if (pp.team) {
                                r = 0xFF;
                                g = 0x1F;
                                b = 0xFF;
                            } else {
                                r = 0x1F;
                                g = 0xFF;
                                b = 0x1F;
                            }

                            paintCircle(map.model, MAP_SCALE,
                                        &map.model.faces[pp.face],
                                        pp.pos, pp.radius,
                                        r, g, b, false);
                        }

                        for (int j = i; j < num_projectiles - 1; j++) {
                            projectiles[j] = projectiles[j + 1];
                        }
                        num_projectiles--;
                        i--;

                        continue;
                    } else {
                        projectiles[i].dir += Vector(0, 0, -GRAVITY);
                        projectiles[i].pos += projectiles[i].dir * projectiles[i].speed;
                    }

                    draw.projectiles_pos[i] = projectiles[i].pos;
                    draw.projectiles_radius[i] = projectiles[i].radius;
                    draw.projectiles_team[i] = projectiles[i].team;
                }
                draw.num_projectiles = num_projectiles;
            }

            // timer
            {
                int seconds = game_timer / 1000;
                sprintf(draw.timer, "%02d:%02d", seconds / 60, seconds % 60);

                if (game_timer <= 0) {
                    float scores[3];
                    getPaintResults(map.model, scores);
                    printf("Green: %f\nPink: %f\nNone: %f\n", scores[0], scores[1], scores[2]);
                    should_die = true;
                    break;
                }
            }

            // end of tick
            draw.frame = tick_count++;

            for(int i = 0; i < MAX_PLAYERS; i++) {
                if (online[i]) {
                    if ( sendto(socket_fd, &draw, sizeof(draw), 0, &player_address[i], sizeof(sockaddr)) != ERROR ) {
                        //printf("Pacote Draw %hu recebido.\n", draw.id);
                    } else {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            fprintf(stderr, "ERROR: Unespected error while sending draw packet. %s %d.\n", strerror(errno), errno);
                            exit(1);
                        }
                    }
                }
            }

            accumulated_time -= TICK_TIME;
        }
    }

    close(socket_fd);

    return 0;
}
