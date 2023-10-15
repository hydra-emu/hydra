#include <network.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <packet.h>
#include <text.h>
#include <unistd.h>
#include <ogcsys.h>
#include "menu.h"

void* malloc_packet(uint8_t type, void* body, uint32_t body_size, uint32_t* packet_size)
{
    uint8_t* packet = (uint8_t*)malloc(sizeof(uint8_t) + sizeof(uint32_t) + body_size);
    memset(packet, type, 1);
    uint32_t body_size_n = __builtin_bswap32(body_size);
    memcpy(packet + 1, &body_size_n, 4);
    if (body_size != 0)
        memcpy(packet + 1 + 4, body, body_size);
    *packet_size = sizeof(uint8_t) + sizeof(uint32_t) + body_size;
    return packet;
}

void free_packet(void* packet)
{
    free(packet);
}

int32_t socket = 0;

char* response_buffer = NULL;
size_t response_max_size = 0;
char localip[16] = {0};
char gateway[16] = {0};
char netmask[16] = {0};
bool initialized = false;
uint32_t mq_handle = 0;

static lwp_t init_thread = (lwp_t)NULL;
static lwp_t packet_thread = (lwp_t)NULL;

void* InitializeClient_Impl(void*);
void* LoopClient_Impl(void*);

void InitializeClient()
{
    s32 res = if_config( localip, netmask, gateway, true, 1);
    if (res < 0) {
        Printf("Failed to get network configuration - %d", res);
        return;
    }

    if (MQ_Init(&mq_handle, 10) < 0) {
        Printf("Failed to initialize message queue");
        return;
    }

    LWP_CreateThread(&init_thread,
        InitializeClient_Impl,
        NULL,
        NULL,
        1024,
        50
    );
}

void* InitializeClient_Impl(void* arg)
{
    socket = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (socket == INVALID_SOCKET) {
        Printf("Failed to create socket - %d", errno);
        return 0;
    }

    response_buffer = malloc(1024);
    response_max_size = 1024;

    // connect to 192.168.1.4:1234
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_len = sizeof(server);
    server.sin_port = htons(1234);
    server.sin_addr.s_addr = inet_addr("192.168.1.4");

    s32 res = net_connect(socket, (struct sockaddr*)&server, sizeof(server));
    int tries = 0;
    while (res < 0) {
        tries++;
        Printf("Attempt %d: Failed to connect to server - %d, trying again in 5 seconds...", tries, res);
        if (tries > 5) {
            Printf("Failed to connect to server - %d. Giving up.", res);
            return 0;
        }
        sleep(5);
        res = net_connect(socket, (struct sockaddr*)&server, sizeof(server));
    }
    Printf("Connected to server");
    initialized = true;
    return 0;
}

void* SendPacket_Impl(void* arg)
{
    int pt = (int)arg;
    void* packet = NULL;
    uint32_t packet_size = 0;
    switch (pt) {
        case HC_PACKET_TYPE_version:
        {
            Printf("Sending version packet");
            hc_client_version_t version;
            version.version = (HC_PROTOCOL_VERSION >> 8) | (HC_PROTOCOL_VERSION << 8);
            packet = malloc_packet(HC_PACKET_TYPE_version, &version, sizeof(version), &packet_size);            
            break;
        }
        case HC_PACKET_TYPE_video:
        {
            hc_client_video_t video;
            video.format[0] = 'r';
            video.format[1] = 'g';
            video.format[2] = 'b';
            video.format[3] = 'a';
            packet = malloc_packet(HC_PACKET_TYPE_video, &video, sizeof(video), &packet_size);
            break;
        }
        default:
        {
            Printf("Unknown packet type: %d", packet);
            break;
        }
    }

    if (packet == NULL) {
        Printf("packet is NULL!");
        return NULL;
    }

    int ret = net_write(socket, packet, packet_size);
    free_packet(packet);
    if (ret != packet_size) {
        Printf("Tried to write %d bytes, but only wrote %d", packet_size, ret);
        return NULL;
    }
    uint8_t response_type;
    uint32_t response_size;
    int ret1 = net_read(socket, &response_type, 1);
    int ret2 = net_read(socket, &response_size, 4);
    if (ret1 != 1 || ret2 != 4) {
        Printf("Failed to read response header");
        return NULL;
    }
    response_size = __builtin_bswap32(response_size);
    if (response_size > response_max_size) {
        response_buffer = realloc(response_buffer, response_size);
        response_max_size = response_size;
    }
    int response_index = 0;
    while (response_index < response_size) {
        int read = response_size - response_index;
        if (read > 32768) {
            read = 32768;
        }
        int ret = net_read(socket, response_buffer + response_index, read);
        if (ret < 0) {
            Printf("Failed to read response body");
            return NULL;
        }
        response_index += ret;
    }
    for (int i = 0; i < 400; i++) {
        for (int j = 0; j < 480; j++) {
            GRRLIB_SetPixelTotexImg(i, j, emulator_texture, ((uint32_t*)response_buffer)[i + j * 400]);
        }
    }
    GRRLIB_FlushTex(emulator_texture);
    return NULL;
}

void SendPacket(int packet)
{
    while (!initialized) {
        Printf("Waiting for initialization");
        sleep(1);
    }

    if (packet_thread != (lwp_t)NULL) {
        LWP_JoinThread(packet_thread, NULL);
    }

    LWP_CreateThread(&packet_thread,
        SendPacket_Impl,
        (void*)packet,
        NULL,
        4096,
        1
    );
}