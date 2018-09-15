#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 20000

typedef struct hostent hostent;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

int createSocket() {

    int file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    if (file_descriptor < 0) {
        fprintf(stderr, "ERROR: Failed to create UDP socket. %s.\n", strerror(errno));
        exit(1);
    }

    fcntl(file_descriptor, F_SETFL, O_NONBLOCK);

    return file_descriptor;
}

sockaddr_in InitializeAddr() {

    sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    return server_address;
}

void BindAddressToSocket(sockaddr_in server_address, int socket_file_descriptor) {

    sockaddr *addr = (sockaddr *) &server_address;

    if (bind(socket_file_descriptor, addr, sizeof(*addr)) == -1) {
        fprintf(stderr, "ERROR: Could not bind file descriptor to socket. %s.\n", strerror(errno));
        exit(1);
    }
}

int main(int argc, char **argv) {

    int socket_file_descriptor = createSocket();

    sockaddr_in server_address = InitializeAddr();

    BindAddressToSocket(server_address, socket_file_descriptor);

    long count = 0;

    while(1) {

        sockaddr addr = {0};
        socklen_t len;

        recvfrom(socket_file_descriptor, &count, sizeof(count), 0, &addr, &len);

        printf("%ld\n", count);

        count++;

        sockaddr_in *test = (sockaddr_in*) &addr;

        printf("%d\n", test->sin_port);

        sendto(socket_file_descriptor, &count, sizeof(count), 0, &addr, len);
    }
    
    shutdown(socket_file_descriptor, SHUT_RDWR);

    return 0;
}