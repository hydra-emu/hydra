#include "server.hxx"
#include <error_factory.hxx>
#include <protocol/packet.h>
#include <thread>
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
#include <arpa/inet.h>
#include <unistd.h>
#else
#pragma message("TODO: include winsock2.h")
#endif

namespace hydra
{

    struct socket_wrapper
    {
        socket_wrapper(uint32_t socket) : socket_(socket) {}

        void read(void* data, uint32_t size)
        {
            if (recv(socket_, data, size, 0) < 0)
            {
                printf("recv() failed\n");
                close();
            }
        }

        int send(void* data, uint32_t size)
        {
            return ::send(socket_, data, size, 0);
        }

        uint32_t get()
        {
            return socket_;
        }

        bool is_open()
        {
            return socket_ != 0;
        }

        void close()
        {
            if (socket_)
            {
                if (::close(socket_) < 0)
                    printf("Warning: close() failed\n");
                socket_ = 0;
            }
        }

    private:
        uint32_t socket_ = 0;
    };

    struct packet_wrapper
    {
        packet_wrapper(socket_wrapper& socket, uint8_t type, void* body, uint32_t body_size)
            : socket_(socket)
        {
            packet_ = malloc_packet(type, body, body_size, &packet_size_);
        }

        ~packet_wrapper()
        {
            if (packet_)
                free_packet(packet_);
        }

        void send()
        {
            if (socket_.send(packet_, packet_size_) < 0)
            {
                printf("send() failed\n");
                socket_.close();
            }
        }

    private:
        socket_wrapper& socket_;
        void* packet_ = nullptr;
        uint32_t packet_size_ = 0;
    };

    server_t::server_t()
    {
        socket_ = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(1234);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(socket_, (sockaddr*)&addr, sizeof(addr)) < 0)
            throw ErrorFactory::generate_exception(__func__, __LINE__, "bind() failed");
        if (listen(socket_, 10) < 0)
            throw ErrorFactory::generate_exception(__func__, __LINE__, "listen() failed");
        sockaddr_in server_addr;
        socklen_t server_addr_len = sizeof(server_addr);
        if (getsockname(socket_, (sockaddr*)&server_addr, &server_addr_len) < 0)
            throw ErrorFactory::generate_exception(__func__, __LINE__, "getsockname() failed");
        printf("Listening on address %s:%d...\n", inet_ntoa(server_addr.sin_addr),
               ntohs(server_addr.sin_port));

        accept_loop();
    }

    server_t::~server_t()
    {
        if (close(socket_) < 0)
            printf("Warning: close() failed\n");
    }

    void client_loop(socket_wrapper client_socket, sockaddr_in client_addr)
    {
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));
        while (client_socket.is_open())
        {
            uint8_t packet_type;
            client_socket.read(&packet_type, 1);

            uint32_t packet_size;
            client_socket.read(&packet_size, 4);

            switch (packet_type)
            {
                case HC_PACKET_TYPE_version:
                {
                    hc_client_version_t version;
                    client_socket.read(&version, sizeof(version));
                    printf("Client version: %04x\n", version.version);
                    hc_server_version_ack_t version_ack;
                    version_ack.response = (version.version == HC_PROTOCOL_VERSION)
                                               ? HC_RESPONSE_OK
                                               : HC_RESPONSE_ERROR;
                    packet_wrapper wrapper(client_socket, HC_PACKET_TYPE_version_ack, &version_ack,
                                           sizeof(version_ack));
                    wrapper.send();
                    if (version_ack.response == HC_RESPONSE_ERROR)
                    {
                        printf("Client version does not match server version: %04x!\n",
                               HC_PROTOCOL_VERSION);
                        client_socket.close();
                    }
                    else
                    {
                        printf("Client version matches server version!\n");
                    }
                    break;
                }
            }
        }
        printf("Connection closed from %s:%d\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));
    }

    void server_t::accept_loop()
    {
        while (true)
        {
            sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client_socket = accept(socket_, (sockaddr*)&client_addr, &client_addr_len);
            if (client_socket < 0)
            {
                printf(__func__, __LINE__, "accept() failed");
            }
            else
            {
                std::thread client_thread([client_socket, client_addr] {
                    socket_wrapper wrapper(client_socket);
                    client_loop(wrapper, client_addr);
                });
                client_thread.detach();
            }
        }
    }

} // namespace hydra