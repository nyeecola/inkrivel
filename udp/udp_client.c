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
        fprintf(stderr, "ERROR: Failed to create UDP socket.\n");
        exit(1);
    }

    fcntl(file_descriptor, F_SETFL, O_NONBLOCK);

    return file_descriptor;
}

hostent* DNSLookUp() {

    hostent *server = gethostbyname(SERVER_ADDRESS);
    if ( server == NULL ) {
        fprintf(stderr, "ERROR: Failed to resolve server address.\n");
        exit(1);
    }

    return server;
}

sockaddr_in InitializeAddr(hostent *server_raw_address) {
 
    sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    memcpy(&server_address.sin_addr.s_addr, server_raw_address->h_addr_list[0], server_raw_address->h_length);

    return server_address;
}

int main(int argc, char **argv) {

    int socket_file_descriptor = createSocket();

    hostent *server_raw_address = DNSLookUp();

    sockaddr_in server_address = InitializeAddr(server_raw_address);

    sockaddr *server_address2 = (sockaddr*) &server_address;

    long count = 0;

    while(1)
    {

        sendto(socket_file_descriptor, &count, sizeof(count), 0, server_address2, sizeof(server_address));

        sockaddr addr = {0};
        socklen_t len;

        recvfrom(socket_file_descriptor, &count, sizeof(count), 0, &addr, &len);

        if ( count > 1000 ) break;

        printf("%ld\n", count);
    }

    shutdown(socket_file_descriptor, SHUT_RDWR);

    return 0;
}