#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>

#include "chat_types.h"

#include "../../lib/config.hpp"

void chatConnect(int socket_fd, char name[20]) {
    Packet p;
    p.id = MSG_CONNECT;
    p.size = strlen(name) +1;
    p.body = (byte *) calloc(p.size, sizeof(*p.body));
    memcpy(p.body, name, p.size);

    sendPacket(socket_fd, p);

    free(p.body);
}

void chatSendMessage(int socket_fd, const char *message) {
    Packet p;
    p.id = MSG_SEND_MESSAGE;
    p.size = strlen(message) + 1;
    p.body = (byte *) calloc(p.size, sizeof(*p.body));
    memcpy(p.body, message, p.size);

    sendPacket(socket_fd, p);

    free(p.body);
}

void chatSendWhisper(int socket_fd, const char *destination, const char *message) {
    int len_destination = strlen(destination) + 1;
    int len_message = strlen(message) + 1;

    Packet p;
    p.id = MSG_SEND_WHISPER;
    p.size = len_destination + len_message;
    p.body = (byte *) calloc(p.size, sizeof(*p.body));
    memcpy(p.body, destination, len_destination);
    memcpy(p.body + len_destination, message, len_message);

    sendPacket(socket_fd, p);

    free(p.body);
}

int createSocket() {
    // create TCP blocking socket
    long socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "ERROR: Failed to create TCP socket.\n");
        return -1;
    }

    // load host by address
    struct hostent *server = gethostbyname(SERVER_ADDRESS);
    if (!server) {
        fprintf(stderr, "ERROR: Failed to resolve server address.\n");
        return -1;
    }

    // server address struct initialization
    struct sockaddr_in server_address = {0};
    memcpy(&server_address.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(CHAT_SERVER_PORT);

    // connect to server
    int err = connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (err < 0) {
        fprintf(stderr, "%d %d\n", err, errno);
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return -1;
    }
    
    return socket_fd;
}

#if 0
int main(int argc, char **argv) {
        chatConnect(socket_fd);
        usleep(1000000);
        chatSendMessage(socket_fd, "BLZ?");
        usleep(1000000);
        chatSendWhisper(socket_fd, "Andreza", "oi");
        usleep(1000000);

        while (1) {
            usleep(1000000);
        }

    return 0;
}
#endif
