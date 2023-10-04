#include <network.h>
#include <panic.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t socket = 0;

char* response_buffer = NULL;
#define MAX_RESPONSE_SIZE 32768
char localip[16] = {0};
char gateway[16] = {0};
char netmask[16] = {0};

void InitializeHTTP()
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

    // GET /screen HTTP/1.1
    // Host: 192.168.1.4:1234
    // User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/118.0
    // Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8
    // Accept-Language: en-US,en;q=0.5
    // Accept-Encoding: gzip, deflate
    // Connection: keep-alive
    // Upgrade-Insecure-Requests: 1
    const char* request =
        "GET /screen HTTP/1.1\r\n"
        "Host: 192.168.1.4:1234\r\n"
        "User-Agent: hydra-wii/0.1\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        // TODO: keep-alive?
        "Connection: close\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "\r\n";
    net_write(socket, request, strlen(request));

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