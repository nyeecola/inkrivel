#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 17555

#define MAX_MESSAGE_SIZE 255

void *listen_server(void *arg) {
    for (;;) {
        // avoid burning cycles
        usleep(10000);

        // socket file descriptor
        int socket_fd = (int) arg;

        // read messages from server
        char buffer[MAX_MESSAGE_SIZE+1] = {0};
        int n = read(socket_fd, buffer, MAX_MESSAGE_SIZE);
        if (n > 0) {
            fprintf(stdout, "%s", buffer);
            fflush(stdout);
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    // create TCP blocking socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
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
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    pthread_t listener;
    pthread_create(&listener, NULL, listen_server, (void *) socket_fd);

    // main loop
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

    return 0;
}