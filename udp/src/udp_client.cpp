#include "udp.hpp"

int main(int argc, char **argv) {

    int socket_file_descriptor = createUDPSocket();

    hostent *raw_server_address = DNSLookUp();

    sockaddr_in server_address = InitializeClientAddr(raw_server_address);

    sockaddr *server_adapted_address = (sockaddr *) &server_address;

    InputPacket input_packet = {0};
    DrawPacket draw_packet = {0};

    input_packet.id = 0;

    for(ever) {

        if (sendto(socket_file_descriptor, &input_packet, sizeof(input_packet), 0, server_adapted_address, sizeof(server_address)) == ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while sending input packet. %s.\n", strerror(errno));
                exit(1);
            }
        } else {
            printf("Pacote Input %hu enviado.\n", input_packet.id);
            input_packet.id = ( input_packet.id + 1 ) % INPUT_ID_WINDOW;
        }

        usleep(50000);

        if (recvfrom(socket_file_descriptor, &draw_packet, sizeof(draw_packet), 0, NULL, NULL) == ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "ERROR: Unespected error while recieving draw packet. %s.\n", strerror(errno));
                exit(1);
            }
        } else {
           printf("Pacote Draw %hu recebido.\n", draw_packet.id);
        }
    }

    close(socket_file_descriptor);

    return 0;
}
