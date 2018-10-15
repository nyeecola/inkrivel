#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "config.hpp"
#include "vector.hpp"
#include "render-types.hpp"

#define INPUT_ID_WINDOW 200
#define DRAW_ID_WINDOW 200

typedef struct hostent hostent;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

typedef enum {
    INPUT,
    DRAW
} PacketType;

typedef struct {
    uint64_t timestamp;
    uint8_t player_id;
    float mouse_angle;
    bool up;
    bool down;
    bool right;
    bool left;
    bool shooting;
    bool especial;
    bool running;
} InputPacket;

typedef struct {
    uint64_t team; // 0 or 1, meaning even or odd
    Vector pos;
    uint32_t face;
    float radius;
} PaintPoint;

typedef struct {
    uint64_t frame;
    bool online[MAX_PLAYERS];
    Vector pos[MAX_PLAYERS];
    float mouse_angle[MAX_PLAYERS];
    CharacterId model_id[MAX_PLAYERS];
    Quat rotations[MAX_PLAYERS];
    int8_t respawn_timer[MAX_PLAYERS]; // -1 means not dead

    uint32_t num_projectiles;
    Vector projectiles_pos[MAX_PROJECTILES];
    float projectiles_radius[MAX_PROJECTILES];
    uint8_t projectiles_team[MAX_PROJECTILES];

    uint32_t num_paint_points;
    PaintPoint paint_points[MAX_PAINT_POINTS];

    char timer[12]; // NULL terminated
} DrawPacket;

class PacketBuffer {
public:
    InputPacket *input_buffer;
    DrawPacket *draw_buffer;
    uint8_t max_size;
    uint8_t packets;

    PacketBuffer(PacketType type, uint8_t max_size) {
        this->max_size = max_size;
        this->packets = 0;
        this->draw_buffer = NULL;
        this->input_buffer = NULL;
        switch( type ) {
            case INPUT:
                input_buffer = (InputPacket*) malloc(sizeof(InputPacket) * max_size);
                break;
            
            case DRAW:
                draw_buffer = (DrawPacket*) malloc(sizeof(DrawPacket) * max_size);
                break;
            
            default:
                printf("Warning: Unespected packet type while creating object.\n");
                break;
        }
    }

    bool insert(PacketType type, void* packet) {
        if ( this->packets == this->max_size ) {
            printf("Estourou o buffer\n");
            return false;
        }
        else {
            if ( type == DRAW && this->draw_buffer == NULL ){
                printf("Can't insert draw packet to incompatible buffer type.\n");
                return false;
            }
            if ( type == INPUT && this->input_buffer == NULL ){
                printf("Can't insert input packet to incompatible buffer type.\n");
                return false;
            }
            int i;
            switch( type ) {
                case INPUT: {
                    InputPacket *in = (InputPacket*) packet;
                    for(i = this->packets - 1; i >= 0; i--) {
                        if ( this->input_buffer[i].timestamp < in->timestamp ) {
                            this->input_buffer[i+1] = this->input_buffer[i];
                        } else {
                            break;
                        }
                    }
                    this->input_buffer[i+1] = *in;
                    this->packets++;
                    return true;
                }
                case DRAW: {
                    DrawPacket *draw = (DrawPacket*) packet;
                    for(i = this->packets - 1; i >= 0; i--) {
                        if ( this->draw_buffer[i].frame < draw->frame ) {
                            this->draw_buffer[i+1] = this->draw_buffer[i];
                        } else {
                            break;
                        }
                    }
                    this->draw_buffer[i+1] = *draw;
                    this->packets++;
                    return true;
                }
                default: {
                    printf("Warning: Unespected packet type while inserting.\n");
                    return false;
                }
            }
        }
    }

#if 0
    void removeOlderFramesThan(int timestamp) {
        int removed = 0;
        int i;
        if ( this->input_buffer != NULL ) {
            for (i = this->packets - 1; i >= 0; i--) {
                if ( this->input_buffer[i].timestamp < timestamp ) {
                    removed++;
                } else {
                    break;
                }
            }
        }
        if ( this->draw_buffer != NULL ) {
            for (i = this->packets - 1; i >= 0; i--) {
                if ( this->draw_buffer[i].timestamp < timestamp ) {
                    removed++;
                } else {
                    break;
                }
            }
        }
        this->packets -= removed;
    }
#endif

    void print() {
        if ( this->input_buffer != NULL ) {
            printf("INPUT: ");
            for(int i = 0; i < this->packets; i++) {
                printf("%lu ", this->input_buffer[i].timestamp);
            }
            printf("\n");
        }
        if ( this->draw_buffer != NULL ) {
            printf("DRAW: ");
            for(int i = 0; i < this->packets; i++) {
                printf("%lu ", this->draw_buffer[i].frame);
            }
            printf("\n");
        }
    }

    void destroy() {
        if ( this->draw_buffer != NULL ) {
            free(this->draw_buffer);
        }
        if ( this->input_buffer != NULL ) {
            free(this->input_buffer);
        }
        //TODO: Caso inserir pacotes novos, destrua a fila associada aqui
    }
};

int createUDPSocket() {
    int file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    if (file_descriptor == ERROR) {
        fprintf(stderr, "ERROR: Failed to create UDP socket. %s.\n", strerror(errno));
        exit(1);
    }

    fcntl(file_descriptor, F_SETFL, O_NONBLOCK);

    return file_descriptor;
}


sockaddr_in initializeServerAddr() {

    sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    return server_address;
}


sockaddr_in InitializeClientAddr(hostent *server_raw_address) {

    sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    memcpy(&server_address.sin_addr.s_addr, server_raw_address->h_addr, server_raw_address->h_length);

    return server_address;
}


hostent* DNSLookUp() {

    hostent *server = gethostbyname(SERVER_ADDRESS);
    if ( server == NULL ) {
        fprintf(stderr, "ERROR: Failed to resolve server address.%s.\n", strerror(errno));
        exit(1);
    }

    return server;
}
