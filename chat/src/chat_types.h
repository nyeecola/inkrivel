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

    //printf("%d %d %s\n", buffer[0], (buffer[1] << 8) | buffer[2], &buffer[3]);

    int n = write(socket_fd, buffer, p.size + 3);

    if (n < 0) {
        fprintf(stderr, "ERROR: Failed to send message to server.\n");
    } else {
        fprintf(stdout, "Message sent.\n");
    }

    free(buffer);
}

// TODO: caller must remember to free body_buffer
int receivePacket(int socket_fd, Packet *p) {
    int n = read(socket_fd, &p->id, 1);
    if (n != 1) {
        return 0;
    }

    n = 0;
    byte size_buffer[2];
    while (n != 2) {
        int x = read(socket_fd, &size_buffer[n], 2 - n);
        if (x > 0) {
            n += x;
        }
        assert(x >= 0 || errno == EAGAIN || errno == EWOULDBLOCK);
    }
    p->size = size_buffer[0] << 8;
    p->size |= size_buffer[1];

    n = 0;
    byte *body_buffer = (byte *) malloc(p->size * sizeof(byte));
    while (n != p->size) {
        int x = read(socket_fd, &body_buffer[n], p->size - n);
        if (x > 0) {
            n += x;
        }
        assert(x >= 0 || errno == EAGAIN || errno == EWOULDBLOCK);
    }
    p->body = body_buffer;

    return 1;
}

