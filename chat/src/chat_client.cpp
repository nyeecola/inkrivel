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

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 17555

void *listenServer(void *arg) {
    for (;;) {
        // socket file descriptor
        long socket_fd = (long) arg;

        Packet p;
        int received = receivePacket(socket_fd, &p);

        // the socket is blocking, so the function must always return 1
        assert(received); 

        switch (p.id) {
            case MSG_RCV_MESSAGE:
                {
                    int len_sender = strlen((const char *) p.body) + 1;
                    int len_timestamp = sizeof(uint32_t);
                    int len_message = p.size - len_sender - len_timestamp;

                    char *sender = (char *) calloc(len_sender, sizeof(*sender));
                    //uint32_t timestamp = ((uint32_t *) p.body)[len_sender+1];
                    uint32_t timestamp = p.body[len_sender];
                    timestamp |= p.body[len_sender+1] << 8;
                    timestamp |= p.body[len_sender+2] << 16;
                    timestamp |= p.body[len_sender+3] << 24;
                    char *message = (char *) calloc(len_message, sizeof(*message));

                    memcpy(sender, p.body, len_sender);
                    memcpy(message, p.body + len_sender + len_timestamp, len_message);

                    printf("RCV MESSAGE: %d %d %s %d %s\n", p.id, p.size, sender,
                                                            timestamp, message);

                    free(message);
                    free(sender);
                }
                break;
            case MSG_RCV_WHISPER:
                {
                    printf("RCV WHISPER: %d %d %s\n", p.id, p.size, p.body);
                }
                break;
            case MSG_USERLIST:
                {
                    int len_destination = strlen((const char *) p.body);
                    char *destination = (char *) calloc(len_destination + 1,
                                                        sizeof(*destination));
                    memcpy(destination, p.body, len_destination);

                    int len_message = p.size - len_destination - 1;
                    char *message = (char *) calloc(len_message, sizeof(*message));
                    memcpy(message, p.body + len_destination + 1, len_message);
                    printf("SEND WHISPER: %d %d %s %s\n", p.id, p.size, destination,
                                                          message);

                    free(destination);
                    free(message);
                }
                break;
        }

        free(p.body);
    }

    return NULL;
}

void chatConnect(int socket_fd) {
    char name[] = "Italo";

    Packet p;
    p.id = MSG_CONNECT;
    p.size = sizeof(name);
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

int main(int argc, char **argv) {
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
    server_address.sin_port = htons(SERVER_PORT);

    // connect to server
    int err = connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (err < 0) {
        fprintf(stderr, "%d %d\n", err, errno);
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return -1;
    }

    // change TCP socket to non-blocking mode
    //fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    pthread_t listener;
    pthread_create(&listener, NULL, listenServer, (void *) socket_fd);

    // main loop
#if 0
    for (;;) {
        // avoid burning cycles
        usleep(10000);

        char buffer[MAX_MESSAGE_SIZE+1] = {0};
        fgets(buffer, MAX_MESSAGE_SIZE, stdin);
        int n = write(socket_fd, buffer, MAX_MESSAGE_SIZE+1);
        if (n < 0) {
            fprintf(stderr, "ERROR: Failed to send message to server.\n");
        } else {
            //fprintf(stdout, "Message sent.\n");
        }
    }
#else
        chatConnect(socket_fd);
        usleep(1000000);
        chatSendMessage(socket_fd, "BLZ?");
        usleep(1000000);
        chatSendWhisper(socket_fd, "Andreza", "oi");
        usleep(1000000);
#endif

    return 0;
}
