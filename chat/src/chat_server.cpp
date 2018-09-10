#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "chat_types.h"

#define SERVER_PORT 17555
#define MAX_PENDING_CONNECTIONS 5

void chatSendBackMessage(int socket_fd, const char *sender, uint32_t timestamp, const char *message) {
    int len_sender = strlen(sender) + 1;
    int len_message = strlen(message) + 1;
    int len_timestamp = sizeof(uint32_t);


    Packet p;
    p.id = MSG_RCV_MESSAGE;
    p.size = len_sender + len_timestamp + len_message;
    p.body = (byte *) calloc(p.size, sizeof(*p.body)); 
    memcpy(p.body, sender, len_sender);
    memcpy(p.body + len_sender, &timestamp, len_timestamp);
    memcpy(p.body + len_sender + len_timestamp, message, len_message);

    sendPacket(socket_fd, p);

    free(p.body);
}

void chatSendBackWhisper(int socket_fd, const char *sender, uint32_t timestamp, const char *message) {
    int len_sender = strlen(sender) + 1;
    int len_message = strlen(message) + 1;
    int len_timestamp = sizeof(uint32_t);


    Packet p;
    p.id = MSG_RCV_WHISPER;
    p.size = len_sender + len_timestamp + len_message;
    p.body = (byte *) calloc(p.size, sizeof(*p.body)); 
    memcpy(p.body, sender, len_sender);
    memcpy(p.body + len_sender, &timestamp, len_timestamp);
    memcpy(p.body + len_sender + len_timestamp, message, len_message);

    sendPacket(socket_fd, p);

    free(p.body);
}

void chatSendUserList(int socket_fd, const char *userlist) {
    // TODO
}


int main(int argc, char **argv) {

    // create TCP socket
    int socket_fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "ERROR: Could not create socket.\n");
        return -1;
    }

    // server address initialization
    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    // bind socket file descriptor to socket
    if (bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "ERROR: Could not bind file descriptor to socket.\n");
        return -1;
    }

    // start listening
    if (listen(socket_fd, MAX_PENDING_CONNECTIONS)) {
        fprintf(stderr, "ERROR: Could not start listening.\n");
    }
    fprintf(stdout, "Listening...\n");
    
    // TODO: stop using this, figure out a better way or use linked lists instead
    int sockets[500];
    memset(sockets, -1, sizeof(sockets));

    // next socket
    int next_socket_fd;
    int last_socket_fd_id = -1;

    for (;;) {
        // avoid wasting cycles
        usleep(10000);

        // accept incoming connection (non-blocking)
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        next_socket_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_address_len);
        if (next_socket_fd < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            fprintf(stderr, "ERROR: Could not accept incoming connection.\n");
        }

        // handle new connection if one exists 
        if (next_socket_fd > 0) {
            // set socket file descriptor to non-blocking mode
            fcntl(next_socket_fd, F_SETFL, O_NONBLOCK);

            // update socket counter
            sockets[++last_socket_fd_id] = next_socket_fd;
            fprintf(stdout, "Connection received.\n");

#if 0
            // send a hello message to client
            int n = write(next_socket_fd, "Hello from server!\n", 200);
            if (n < 0) {
                fprintf(stderr, "ERROR: Failed to talk to client.\n");
            }
#endif
        }

        // loop through all sockets
        for (int i = 0; i <= last_socket_fd_id; i++) {
            if (sockets[i] == -1) continue; // TODO: stop doing this, fix sockets list

            // read next information on socket if any (non-blocking)
            byte *buffer = (byte *) calloc(MAX_MESSAGE_SIZE, sizeof(*buffer));;

            // TODO: read multiple packets together

            int n = read(sockets[i], buffer, MAX_MESSAGE_SIZE);
            if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Something went wrong when reading from socket %d.\n", sockets[i]);
                free(buffer);
                continue;
            }

            // skip sockets with no message
            if (n <= 0) {
                free(buffer);
                continue;
            }

            // if a new message was received, log it ...
#if 0
            fprintf(stdout, "Client %d: %s\n", sockets[i], buffer);
            char send_buffer[400] = {0};

            // ... and send it to other users
            for (int j = 0; j <= last_socket_fd_id; j++) {
                if (j == i) continue; // don't send to the user who sent it to us
                if (sockets[j] == -1) continue; // TODO: stop doing this

                sprintf(send_buffer, "User %d: %s\n", sockets[j], buffer);
                n = write(sockets[j], send_buffer, 400);
                if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    fprintf(stderr, "ERROR: Something went wrong when writing from socket %d.\n", sockets[j]);
                    continue;
                }
            }
#else
            Packet p = {};            
            p.id = buffer[0];
            p.size = buffer[1] << 8;
            p.size |= buffer[2];
            p.body = (byte *) calloc(p.size, sizeof(*p.body));
            memcpy(p.body, &buffer[3], p.size);

            switch (p.id) {
                case MSG_CONNECT:
                    {
                        printf("CONNECT: %d %d %s\n", p.id, p.size, p.body);
                        chatSendBackMessage(sockets[i], "Server", 321941092, "Connected.");
                    } break;
                case MSG_SEND_MESSAGE:
                    {
                        printf("SEND MESSAGE: %d %d %s\n", p.id, p.size, p.body);
                    } break;
                case MSG_SEND_WHISPER:
                    {
                        int len_destination = strlen(p.body);
                        char *destination = (char *) calloc(len_destination + 1, sizeof(*destination));;
                        memcpy(destination, p.body, len_destination);

                        int len_message = p.size - len_destination - 1;
                        char *message = (char *) calloc(len_message, sizeof(*message));
                        memcpy(message, p.body + len_destination + 1, len_message);
                        printf("SEND WHISPER: %d %d %s %s\n", p.id, p.size, destination, message);

                        free(destination);
                        free(message);
                    } break;
            }

            free(p.body);
            free(buffer);
#endif
        }
    }

    return 0;
}
