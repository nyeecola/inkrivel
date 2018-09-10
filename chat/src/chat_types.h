#pragma once

#define MAX_MESSAGE_SIZE 65000

typedef unsigned char byte;

enum MessageId {
    MSG_CONNECT,
    MSG_SEND_MESSAGE,
    MSG_RCV_MESSAGE,
    MSG_SEND_WHISPER,
    MSG_RCV_WHISPER,
    MSG_USERLIST
};

struct Packet {
    uint8_t id;
    uint16_t size;
    byte *body;
};

void sendPacket(int socket_fd, Packet p) {
    byte *buffer = (byte *) calloc(p.size + 3, sizeof(*buffer));

    buffer[0] = p.id;
    buffer[1] = (byte) (p.size >> 8);
    buffer[2] = (byte) ((p.size << 8) >> 8);
    memcpy(&buffer[3], p.body, p.size);

    printf("%d %d %s\n", buffer[0], (buffer[1] << 8) | buffer[2], &buffer[3]);

    int n = write(socket_fd, buffer, p.size + 3);

    if (n < 0) {
        fprintf(stderr, "ERROR: Failed to send message to server.\n");
    } else {
        fprintf(stdout, "Message sent.\n");
    }

    free(buffer);
}

