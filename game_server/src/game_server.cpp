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
#define TICK_TIME 11

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

        usleep(500);
    }

    return NULL;
}

void createProjectile(InputPacket input, Character *player, int model_id, int id, Projectile *projectiles, int *num_proj) {
    assert(projectiles);
    assert(num_proj);

    // projectile XY direction
    float mouse_angle = input.mouse_angle * -1;
    mouse_angle -= 90;
    Vector looking(0, 0, 0);
    looking.x = cos(mouse_angle * M_PI / 180);
    looking.y = -sin(mouse_angle * M_PI / 180);
    looking.normalize();

    // projectile XYZ direction
    {
        float dot = looking.dot(player->normal_sum);
        float squared_normal = player->normal_sum.lenSq();
        float tmp = dot/squared_normal;
        Vector scaled_normal = player->normal_sum * tmp;
        looking = looking - scaled_normal;
        looking.normalize();
    }

    // create projectile
    Projectile proj;
    proj.pos = player->pos + Vector(0, 0, 0.25); // TODO: stop de mijar
    proj.dir = looking;
    proj.radius = 0.04;
    proj.speed = 0.03;
    proj.team = id % 2;
    proj.damage = 0;

    // projectile damage
    switch (model_id){
        case TEST:
            proj.damage = TEST_PROJECTILE_DAMAGE;
            break;
        case ROLO:
            assert(false);
            break;
        case ASSAULT:
            proj.damage = ASSAULT_PROJECTILE_DAMAGE;
            {
                Vector tmp(-proj.dir.y, proj.dir.x, proj.dir.z);
                tmp.normalize();

                if (player->alternate_fire_assault) {
                    proj.pos = proj.pos + tmp * 0.07; // TODO: check if this value is good
                } else {
                    proj.pos = proj.pos - tmp * 0.07; // TODO: check if this value is good
                }
                player->alternate_fire_assault = !player->alternate_fire_assault;
            }
            break;
        case SNIPER:
            proj.damage = SNIPER_PROJECTILE_DAMAGE;
            break;
        case BUCKET:
            proj.damage = BUCKET_PROJECTILE_DAMAGE;
            break;
        default:
            assert(false);
    }

    // add projectile to queue
    assert(*num_proj < MAX_PROJECTILES);
    projectiles[*num_proj] = proj;
    *num_proj = *num_proj + 1;
}

int main(int argc, char **argv) {
    int num_players = atoi(argv[1]);

    for (int i = 0; i < num_players; i++) {
        draw.model_id[i] = (CharacterId) atoi(argv[i + 2]);
    }

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
        player[i].health = STARTING_HEALTH;
        player[i].ammo = STARTING_AMMO;
        player[i].atk_delay = ATK_DELAY;
        player[i].starting_atk_delay = ATK_DELAY;
        player[i].swimming = false;
    }

    // Create map
    Map map;
#if 1
    map.model = loadWavefrontModel("../assets/map/test_map.obj", "../assets/map/test_map.png", VERTEX_ALL, MAP_TEXTURE_SIZE);
#else
    map.model = loadWavefrontModel("../assets/cherie.obj", "../assets/map3.png", VERTEX_ALL, MAP_TEXTURE_SIZE);
#endif

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
            uint64_t tick_start = getTimestamp();

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
                player[id].swimming = input.swimming;

                if (input.shooting && !player[id].swimming && player[id].atk_delay <= 0 && player[id].ammo) {
                    // update ammo
                    if (draw.model_id[id] == ROLO) {
                        player[id].ammo -= 0.1;
                    } else {
                        player[id].ammo -= 1;
                    }
                    if (player[id].ammo < 0) { // needed in the future
                        player[id].ammo = 0;
                    }

                    if (draw.model_id[id] == ROLO) {
                        // paint
                        PaintPoint pp = {};
                        pp.team = id % 2;
                        pp.pos = player[id].paint_max_z;
                        pp.face = player[id].paint_face;
                        // TODO: fix this number and do this only for ROLO
                        pp.radius = 40;
                        draw.paint_points[draw.num_paint_points++] = pp;

                        uint32_t color;
                        if (id % 2) {
                            color = 0xFFFF1FFF;
                        } else {
                            color = 0xFF1FFF1F;
                        }

                        paintCircle(map.model, map.model.faces[pp.face],
                                    pp.pos, pp.radius, color, false);
                    } else { // everyone else can shoot normally
                        switch (draw.model_id[id]) {
                            case ASSAULT: {
                            } break;
                            case SNIPER: {
                            } break;
                            case BUCKET: {
                            } break;
                            default:
                                assert(false);
                        }
                        player[id].atk_delay = player[id].starting_atk_delay;

                        // create projectile
                        createProjectile(input, &player[id], draw.model_id[id], id, projectiles, &num_projectiles);
                    }
                }

                player[id].dir.normalize();
                player[id].dir *= player[id].speed;
            }

            // player-related simulation
            for(int id = 0; id < MAX_PLAYERS; id++) {
                if (!online[id]) continue;

                draw.swimming[id] = player[id].swimming;
                draw.ammo[id] = (int) player[id].ammo;

                if (player[id].atk_delay > 0) {
                    player[id].atk_delay -= TICK_TIME;
                }

                // respawn
                if (player[id].dead) {
                    player[id].respawn_timer -= (float) TICK_TIME / 1000.0f;
                    if (player[id].respawn_timer < 0) {
                        player[id].dead = false;
                        player[id].ammo = STARTING_AMMO;
                        draw.respawn_timer[id] = -1;
                    } else {
                        draw.respawn_timer[id] = (int) player[id].respawn_timer;
                        continue;
                    }
                } else {
                    draw.respawn_timer[id] = -1;
                }

                // collision
                Vector player_z = {0, 0, -200};
                Vector normal_sum = {0, 0, 0};
                Vector paint_max_z = {0, 0, -200};
                int paint_face;
                collidesWithMap(map, player[id], normal_sum, player_z,
                                paint_max_z, paint_face);

                // TODO: do not store this value in the future, there is no need to
                player[id].paint_max_z = paint_max_z;
                player[id].paint_face = paint_face;

                // rotation
                {
                    // TODO: do not store this value in the future, there is no need to
                    player[id].normal_sum = normal_sum;

                    player[id].rotation = getRotationQuat({0,0,1}, normal_sum);
                    draw.rotations[id] = player[id].rotation;
                }

                // player movement
                {
                    if (player[id].dir.len() > player[id].speed) {
                        player[id].dir.normalize();
                        player[id].dir *= player[id].speed;
                    }
                    if (player[id].swimming) {
                        Vector max_z = {0, 0, -200};
                        int face_max_z = -1;
                        for (int i = 0; i < map.model.num_faces; i++) {
                            Face *cur = &map.model.faces[i];

                            Vector ground = {0, 0, -1};
                            Vector sky = {player[id].pos.x + player[id].hit_radius,
                                          player[id].pos.y, 200};
                            Vector intersection;
                            bool intersect = rayIntersectsTriangle(map, sky, ground,
                                                                   cur, intersection);

                            if (intersect && intersection.z > max_z.z) {
                                max_z = intersection;
                                face_max_z = i;
                            }
                        }

                        if (face_max_z != -1) {
                            Face paint_face = map.model.faces[face_max_z];

                            int fv0 = paint_face.vertices[0];
                            int fv1 = paint_face.vertices[1];
                            int fv2 = paint_face.vertices[2];
                            Vector v0 = MAP_SCALE * map.model.vertices[fv0];
                            Vector v1 = MAP_SCALE * map.model.vertices[fv1];
                            Vector v2 = MAP_SCALE * map.model.vertices[fv2];

                            float u, v, w;
                            barycentric(max_z, v0, v1, v2, u, v, w);

                            int ftc0 = paint_face.texture_coords[0];
                            int ftc1 = paint_face.texture_coords[1];
                            int ftc2 = paint_face.texture_coords[2];
                            TextureCoord tex_v0 = map.model.texture_coords[ftc0];
                            TextureCoord tex_v1 = map.model.texture_coords[ftc1];
                            TextureCoord tex_v2 = map.model.texture_coords[ftc2];

                            int tex_x, tex_y;
                            tex_x = (u * tex_v0.x + v * tex_v1.x + w * tex_v2.x) * (MAP_TEXTURE_SIZE-1);
                            tex_y = (u * tex_v0.y + v * tex_v1.y + w * tex_v2.y) * (MAP_TEXTURE_SIZE-1);

                            uint32_t *pixels = (uint32_t *) map.model.texture_image->pixels;

                            if (id % 2) {
                                if (pixels[tex_y * MAP_TEXTURE_SIZE + tex_x] == 0xFFFF1FFF) {
                                    // player ammo
                                    if (player[id].ammo < STARTING_AMMO) {
                                        player[id].ammo += AMMO_RECHARGE_RATE;
                                    }
                                    player[id].dir *= SWIM_GOOD_FACTOR;
                                } else {
                                    player[id].dir *= SWIM_BAD_FACTOR;
                                }
                            } else {
                                if (pixels[tex_y * MAP_TEXTURE_SIZE + tex_x] == 0xFF1FFF1F) {
                                    // player ammo
                                    if (player[id].ammo < STARTING_AMMO) {
                                        player[id].ammo += AMMO_RECHARGE_RATE;
                                    }
                                    player[id].dir *= SWIM_GOOD_FACTOR;
                                } else {
                                    player[id].dir *= SWIM_BAD_FACTOR;
                                }
                            }
                        }
                    }
                    player[id].pos += player[id].dir;
                    if (player_z.z > 0) {
                        player[id].pos.z += 0.5*(player_z.z - player[id].pos.z);
                    }
                    draw.pos[id] = player[id].pos;
                }
            }

            // projectile simulation
            {
                printf("projectiles %d\n", num_projectiles);
                for (int i = 0; i < num_projectiles; i++) {

                    // check collision with player
                    for (int j = 0; j < MAX_PLAYERS; j++) {
                        if (!online[j] || projectiles[i].team == (j % 2)) continue;

                        float offset_z = 0;
                        switch (draw.model_id[j]) {
                            case TEST:
                                offset_z = TEST_HITBOX_Z_OFFSET; 
                                break;
                            case ROLO:
                                offset_z = ROLO_HITBOX_Z_OFFSET; 
                                break;
                            case ASSAULT:
                                offset_z = ASSAULT_HITBOX_Z_OFFSET; 
                                break;
                            case SNIPER:
                                offset_z = SNIPER_HITBOX_Z_OFFSET; 
                                break;
                            case BUCKET:
                                offset_z = BUCKET_HITBOX_Z_OFFSET; 
                                break;
                            default:
                                assert(false);
                        }

                        Vector player_hitbox_center = player[j].pos +
                                                      Vector(0, 0, offset_z);
                        Vector dist_vector = player_hitbox_center - projectiles[i].pos;
                        if (dist_vector.len() < player[j].hit_radius +
                                                projectiles[i].radius) {
                            player[j].health -= projectiles[i].damage;

                            // respawn
                            if (player[j].health <= 0) {
                                player[j].pos = Vector(0, 0, 20);
                                player[j].health = STARTING_HEALTH;
                                player[j].dead = true;
                                player[j].respawn_timer = RESPAWN_DELAY;
                            }

                            for (int k = i; k < num_projectiles - 1; k++) {
                                projectiles[k] = projectiles[k + 1];
                            }
                            num_projectiles--;
                            i--;

                            goto next;
                        }
                    }

                    // check collision with map
                    {
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

                                uint32_t color;
                                if (pp.team) {
                                    color = 0xFFFF1FFF;
                                } else {
                                    color = 0xFF1FFF1F;
                                }

                                paintCircle(map.model, map.model.faces[pp.face],
                                            pp.pos, pp.radius, color, false);
                            }

                            for (int j = i; j < num_projectiles - 1; j++) {
                                projectiles[j] = projectiles[j + 1];
                            }
                            num_projectiles--;
                            i--;

                            goto next;
                        } else {
                            projectiles[i].dir += Vector(0, 0, -GRAVITY);
                            projectiles[i].pos += projectiles[i].dir * projectiles[i].speed;
                        }
                    }

                    draw.projectiles_pos[i] = projectiles[i].pos;
                    draw.projectiles_radius[i] = projectiles[i].radius;
                    draw.projectiles_team[i] = projectiles[i].team;
next:;
                }
                draw.num_projectiles = num_projectiles;
            }

            // game timer
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

            uint64_t tick_end = getTimestamp();
            printf("Tick time: %lums\n", tick_end - tick_start);

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

        usleep(500);
    }

    close(socket_fd);

    return 0;
}
