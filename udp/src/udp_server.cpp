#include "udp.hpp"

void BindAddressToSocket(sockaddr_in server_address, int socket_file_descriptor);

int main(int argc, char **argv) {

    int socket_file_descriptor = createUDPSocket();

    sockaddr_in server_address = InitializeServerAddr();

    BindAddressToSocket(server_address, socket_file_descriptor);

    InputPacket input_packet = {0};
    DrawPacket draw_packet = {0};

    draw_packet.id = 0;

    printf("Server is up\n");

    for(ever) {

        sockaddr addr = {0};
        socklen_t len = sizeof(addr);

        if ( recvfrom(socket_file_descriptor, &input_packet, sizeof(input_packet), 0, &addr, &len) != ERROR ) {

            printf("Pacote Input %hu recebido.\n", input_packet.id);

            sendto(socket_file_descriptor, &draw_packet, sizeof(draw_packet), 0, &addr, len);

            printf("Pacote Draw %hu enviado.\n", draw_packet.id);

            draw_packet.id = ( draw_packet.id + 1 ) % DRAW_ID_WINDOW;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while sending input packet. %s %d.\n", strerror(errno), errno);
                exit(1);
            }
        }
    }
    
    close(socket_file_descriptor);

    return 0;
}


void BindAddressToSocket(sockaddr_in server_address, int socket_file_descriptor) {

    sockaddr *addr = (sockaddr *) &server_address;

    if (bind(socket_file_descriptor, addr, sizeof(server_address)) == ERROR) {
        fprintf(stderr, "ERROR: Could not bind file descriptor to socket. %s.\n", strerror(errno));
        exit(1);
    }
}
