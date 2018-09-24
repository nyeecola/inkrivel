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

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 27222

#define INPUT_ID_WINDOW 200
#define DRAW_ID_WINDOW 200

typedef struct hostent hostent;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;


typedef struct {
    uint8_t id;
    uint8_t player_id;
    float mouse_x;
    float mouse_y;
    bool foward;
    bool back;
    bool right;
    bool left;
    bool shooting;
    bool especial;
    bool running;
} InputPacket;

typedef struct {
	uint8_t id;
    bool online[MAX_PLAYERS];
    Vector pos[MAX_PLAYERS];
    float mouse_angle[MAX_PLAYERS];
    uint8_t model_id[MAX_PLAYERS];
} DrawPacket;


int createUDPSocket() {

    int file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    if (file_descriptor == ERROR) {
        fprintf(stderr, "ERROR: Failed to create UDP socket. %s.\n", strerror(errno));
        exit(1);
    }

   fcntl(file_descriptor, F_SETFL, O_NONBLOCK);
    
    return file_descriptor;
}


sockaddr_in InitializeServerAddr() {

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