#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <poll.h>

#include "chat_types.h"
#include "../../lib/config.hpp"

#define MAX_PENDING_CONNECTIONS 32
#define MAX_CONNECTIONS 500

struct User {
    char name[20];
    int socket_fd;
};

User global_connected_users[MAX_CONNECTIONS];
int global_connected_users_size = 0;

void chatSendBackMessage(int socket_fd, const char *sender, uint32_t timestamp,
                         const char *message) {
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

void chatSendBackWhisper(int socket_fd, const char *sender, uint32_t timestamp,
                         const char *message) {
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

void chatSendUserList(int socket_fd) {
    Packet p;
    p.id = MSG_USERLIST;
    p.body = (byte*) calloc(500*20, sizeof(byte));

    int offset = 0;
    for (int i = 0; i < global_connected_users_size; i++) {
        int len = strlen(global_connected_users[i].name) +1;
        memcpy(p.body + offset, global_connected_users[i].name, len);
        offset += len;
    }

    p.size = offset;

    sendPacket(socket_fd, p);

    free(p.body);
}

int handleNewConnection(int socket_fd, pollfd *new_connection) {
    memset(new_connection, 0, sizeof(*new_connection));

    // accept incoming connection (non-blocking)
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int next_socket_fd = accept(socket_fd, (struct sockaddr *) &client_address,
                                &client_address_len);
    if (next_socket_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            fprintf(stderr, "ERROR: Could not accept incoming connection.\n");
        }

        return 0;
    }

    // set socket file descriptor to non-blocking mode
    fcntl(next_socket_fd, F_SETFL, O_NONBLOCK);

    new_connection->fd = next_socket_fd;
    new_connection->events = POLLIN;

    return 1;
}

int getUserIndexBySocket(int socket_fd) {
    for (int i = 0; i < global_connected_users_size; i++) {
        if (socket_fd == global_connected_users[i].socket_fd) {
            return i;
        }
    }
    assert(false);
    return -1;
}

int getUserIndexByName(char name[20]) {
    for (int i = 0; i < global_connected_users_size; i++) {
        if (!strcmp(name, global_connected_users[i].name)) {
            return i;
        }
    }
    assert(false);
    return -1;
}

void handleNewMessage(int *index, pollfd *sockets_to_poll, int *size) {
    int socket_fd = sockets_to_poll[*index].fd;

    printf("New message from socket %d\n", socket_fd);

    // NOTE: only reads one packet at a time
    Packet p;
    int received = receivePacket(socket_fd, &p);

    // connection is closed
    if (!received) {
        puts("Connection closed.");
        for (int j = *index; j < (*size); j++) {
            sockets_to_poll[j] = sockets_to_poll[j+1];
        }
        *size -= 1;
        *index -= 1;

        int user_index = getUserIndexBySocket(socket_fd);
        for (int j = user_index; j < global_connected_users_size-1; j++) {
            global_connected_users[j] = global_connected_users[j+1];
        }
        global_connected_users_size--;

        for (int i = 0; i < global_connected_users_size; i++) {
            chatSendUserList(global_connected_users[i].socket_fd);
        }
        return;
    }

    switch (p.id) {
        case MSG_CONNECT:
            {
                printf("CONNECT: %d %d %s\n", p.id, p.size, p.body);
                User temp = {"", socket_fd};
                global_connected_users[global_connected_users_size] = temp;
                strcpy(global_connected_users[global_connected_users_size].name,
                        (const char *) p.body);
                global_connected_users_size++;
                
                for (int i = 0; i < global_connected_users_size; i++) {
                    chatSendUserList(global_connected_users[i].socket_fd);
                }
                //chatSendBackMessage(socket_fd, "Server", 321941092, "Connected.");
            } break;
        case MSG_SEND_MESSAGE:
            {
                for (int i = 0; i < global_connected_users_size; i++) {
                    chatSendBackMessage(global_connected_users[i].socket_fd,
                            global_connected_users[getUserIndexBySocket(socket_fd)].name,
                            time(NULL),
                            (const char*) p.body);
                }
                printf("SEND MESSAGE: %d %d %s\n", p.id, p.size, p.body);
            } break;
        case MSG_SEND_WHISPER:
            {
                int len_destination = strlen((const char *) p.body);
                char *destination = (char *) calloc(len_destination + 1,
                                                    sizeof(*destination));
                memcpy(destination, p.body, len_destination);

                int len_message = p.size - len_destination - 1;
                char *message = (char *) calloc(len_message, sizeof(*message));
                memcpy(message, p.body + len_destination + 1, len_message);
                printf("SEND WHISPER: %d %d %s %s\n", p.id, p.size, destination, message);

                int send_socket = global_connected_users[getUserIndexByName(destination)].socket_fd;
                chatSendBackWhisper(send_socket,
                        global_connected_users[getUserIndexBySocket(socket_fd)].name,
                        time(NULL),
                        message);

                free(destination);
                free(message);
            } break;
    }

    free(p.body);
}

int main(int argc, char **argv) {

    // create TCP socket
    int listen_fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
    if (listen_fd < 0) {
        fprintf(stderr, "ERROR: Could not create socket.\n");
        return -1;
    }

    // server address initialization
    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(CHAT_SERVER_PORT);

    // bind socket file descriptor to socket
    if (bind(listen_fd, (struct sockaddr *) &server_address,
             sizeof(server_address)) < 0) {
        fprintf(stderr, "ERROR: Could not bind file descriptor to socket.\n");
        return -1;
    }

    // start listening
    if (listen(listen_fd, MAX_PENDING_CONNECTIONS)) {
        fprintf(stderr, "ERROR: Could not start listening.\n");
    }
    fprintf(stdout, "Listening...\n");
    

    struct pollfd sockets_to_poll[MAX_CONNECTIONS];
    memset(sockets_to_poll, 0, sizeof(sockets_to_poll));

    int last_socket_fd_id = -1;

    ++last_socket_fd_id;
    sockets_to_poll[last_socket_fd_id].fd = listen_fd;
    sockets_to_poll[last_socket_fd_id].events = POLLIN;
    
    for (;;) {
        int ret = poll(sockets_to_poll, last_socket_fd_id+1, 0);
        if (ret < 0) {
            printf("ERROR: poll() failed with return code %d\n", ret);
            break;
        }

        // nothing to read in any socket
        if (ret == 0) {
            usleep(5000);
            continue;
        }

        for (int i = 0; i <= last_socket_fd_id; i++) {
            if (sockets_to_poll[i].revents == 0) {
                continue;
            }

            // data received
            if (sockets_to_poll[i].revents == POLLIN) {

                // handle new connections
                if (sockets_to_poll[i].fd == listen_fd) {
                    for (;;) {
                        pollfd new_connection = {};
                        int ret = handleNewConnection(listen_fd, &new_connection);

                        if (ret) {
                            // update socket counter
                            sockets_to_poll[++last_socket_fd_id] = new_connection;

                            fprintf(stdout, "Connection received.\n");
                        } else {
                            break;
                        }
                    }
                }

                // handle other sockets
                else {
                    handleNewMessage(&i, sockets_to_poll, &last_socket_fd_id);
                }
            } else {
                assert(false);
            }
        }
    }

    return 0;
}
