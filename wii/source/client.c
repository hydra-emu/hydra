#include <network.h>
#include <panic.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <packet.h>

int32_t socket = 0;

char* response_buffer = NULL;
#define MAX_RESPONSE_SIZE 32768
char localip[16] = {0};
char gateway[16] = {0};
char netmask[16] = {0};

void InitializeClient()
{
	if (if_config ( localip, netmask, gateway, true, 20) < 0) {
        panic("Failed to get network configuration");
    }

    socket = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (socket == INVALID_SOCKET) {
        printf("errno: %d\n", errno);
        panic("Failed to create socket");
    }

    // connect to 192.168.1.4:1234
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_len = sizeof(server);
    server.sin_port = htons(1234);
    server.sin_addr.s_addr = inet_addr("192.168.1.4");

    if (net_connect(socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        panic("Failed to connect to server");
    }

    response_buffer = malloc(MAX_RESPONSE_SIZE);
    memset(response_buffer, 0, MAX_RESPONSE_SIZE);

    hc_client_version_t version;
    version.version = (HC_PROTOCOL_VERSION >> 8) | (HC_PROTOCOL_VERSION << 8);
    uint32_t packet_size = 0;
    void* packet = malloc_packet(HC_PACKET_TYPE_version, &version, sizeof(version), &packet_size);
    net_write(socket, packet, packet_size);
    free_packet(packet);

    printf("Written!\n");
    int i = 0;
    while (true) {
        int bytes = net_read(socket, response_buffer, MAX_RESPONSE_SIZE);
        printf("Read %d bytes\n", bytes);
        for (int j = 0; j < bytes; j++) {
            printf("%02x", response_buffer[j]);
        }
        memset(response_buffer, 0, MAX_RESPONSE_SIZE);
        i++;
        if (i > 3) {
            break;
        }
    }
    panic("Connection closed");
}