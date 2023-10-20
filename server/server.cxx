#include "server.hxx"
#include <cstring>
#include <error_factory.hxx>
#include <protocol/packet.h>
#include <thread>
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
#include <arpa/inet.h>
#include <unistd.h>
#else
#pragma message("TODO: include winsock2.h")
#endif
#include "glad.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <GLFW/glfw3.h>
#include <hydra/core.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <netinet/tcp.h>
#include <stb_image_write.h>

void* get_proc_address = nullptr;
GLuint fbo = 0;
void* context = nullptr;
std::array<uint8_t, 400 * 480 * 4> buffer;
hydra::core_wrapper_t* core_;

namespace hydra
{

    void* malloc_packet(uint8_t type, void* body, uint32_t body_size, uint32_t* packet_size)
    {
        uint8_t* packet = (uint8_t*)malloc(sizeof(uint8_t) + sizeof(uint32_t) + body_size);
        memset(packet, type, 1);
        memcpy(packet + 1, &body_size, 4);
        if (body_size != 0)
            memcpy(packet + 1 + 4, body, body_size);
        *packet_size = sizeof(uint8_t) + sizeof(uint32_t) + body_size;
        return packet;
    }

    void free_packet(void* packet)
    {
        free(packet);
    }

    void init_gl()
    {
        if (!glfwInit())
            printf("glfwInit() failed\n");
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        GLFWwindow* window = glfwCreateWindow(640, 480, "", nullptr, nullptr);
        if (!window)
            printf("glfwCreateWindow() failed\n");
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            printf("gladLoadGLLoader() failed\n");
        get_proc_address = (void*)glfwGetProcAddress;
        context = (void*)glfwGetCurrentContext();

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 400, 480);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    }

    void* read_other_callback(hc_other_e other)
    {
        switch (other)
        {
            case hc_other_e::HC_OTHER_GL_GET_PROC_ADDRESS:
            {
                return get_proc_address;
            }
            case hc_other_e::HC_OTHER_GL_FBO:
            {
                return &fbo;
            }
            case hc_other_e::HC_OTHER_GL_CONTEXT:
            {
                return context;
            }
            default:
            {
                return nullptr;
            }
        }
    }

    void poll_input_callback() {}

    int8_t read_input_callback(uint8_t player, hc_input_e button)
    {
        return 0;
    }

    void video_callback(const uint8_t* data, uint32_t width, uint32_t height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

        for (uint32_t y = 0; y < height / 2; y++)
        {
            for (uint32_t x = 0; x < width; x++)
            {
                std::swap(buffer[(y * width + x) * 4 + 0],
                          buffer[((height - 1 - y) * width + x) * 4 + 0]);
                std::swap(buffer[(y * width + x) * 4 + 1],
                          buffer[((height - 1 - y) * width + x) * 4 + 1]);
                std::swap(buffer[(y * width + x) * 4 + 2],
                          buffer[((height - 1 - y) * width + x) * 4 + 2]);
                std::swap(buffer[(y * width + x) * 4 + 3],
                          buffer[((height - 1 - y) * width + x) * 4 + 3]);
            }
        }
    }

    void audio_callback(const int16_t* data, uint32_t size) {}

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
        const int enable = 1;
        if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
            throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                   "setsockopt(SO_REUSEADDR) failed");
        if (setsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE, (void*)&enable, sizeof(int) < 0))
            throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                   "setsockopt(SO_KEEPALIVE) failed");
        if (setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(int) < 0))
            throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                   "setsockopt(TCP_NODELAY) failed");
        if (setsockopt(socket_, IPPROTO_TCP, TCP_QUICKACK, (void*)&enable, sizeof(int) < 0))
            throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                   "setsockopt(TCP_QUICKACK) failed");
        if (auto res = bind(socket_, (sockaddr*)&addr, sizeof(addr)); res < 0)
        {
            printf("bind() failed: %d\n", errno);
            throw ErrorFactory::generate_exception(__func__, __LINE__, "bind() failed");
        }
        if (listen(socket_, 10) < 0)
            throw ErrorFactory::generate_exception(__func__, __LINE__, "listen() failed");
        sockaddr_in server_addr;
        socklen_t server_addr_len = sizeof(server_addr);
        if (getsockname(socket_, (sockaddr*)&server_addr, &server_addr_len) < 0)
            throw ErrorFactory::generate_exception(__func__, __LINE__, "getsockname() failed");
        printf("Listening on address %s:%d...\n", inet_ntoa(server_addr.sin_addr),
               ntohs(server_addr.sin_port));

        init_gl();

        core_ = new core_wrapper_t(std::filesystem::path("/home/offtkp/cores/libAlber.so"));
        core_->hc_set_read_other_callback_p(read_other_callback);
        core_->core_handle = core_->hc_create_p();
        core_->hc_load_file_p(core_->core_handle, "rom", "/home/offtkp/Roms/3DS/zelda.3ds");
        core_->hc_set_poll_input_callback_p(core_->core_handle, poll_input_callback);
        core_->hc_set_read_input_callback_p(core_->core_handle, read_input_callback);
        core_->hc_set_video_callback_p(core_->core_handle, video_callback);
        core_->hc_set_audio_callback_p(core_->core_handle, audio_callback);
        for (int i = 0; i < 600; i++)
            core_->hc_run_frame_p(core_->core_handle);
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
            printf("Received packet type: %d\n", packet_type);

            uint32_t packet_size;
            client_socket.read(&packet_size, 4);
            printf("Received packet size: %d\n", packet_size);

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
                case HC_PACKET_TYPE_video:
                {
                    hc_client_video_t video;
                    client_socket.read(&video, sizeof(video));
                    packet_wrapper wrapper(client_socket, HC_PACKET_TYPE_video_ack, buffer.data(),
                                           buffer.size());
                    wrapper.send();
                    break;
                }
                case HC_PACKET_TYPE_step:
                {
                    hc_client_step_t step;
                    client_socket.read(&step, sizeof(step));
                    for (uint16_t i = 0; i < step.frames; i++)
                    {
                        core_->hc_run_frame_p(core_->core_handle);
                    }
                    hc_server_step_ack_t step_ack;
                    step_ack.response = HC_RESPONSE_OK;
                    packet_wrapper wrapper(client_socket, HC_PACKET_TYPE_step_ack, &step_ack,
                                           sizeof(step_ack));
                    wrapper.send();
                    break;
                }
                case HC_PACKET_TYPE_discord_plays_special_input:
                {
                    hc_client_discord_plays_special_input_t discord_plays_special_input;
                    client_socket.read(&discord_plays_special_input,
                                       sizeof(discord_plays_special_input));
                    // AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
                    packet_wrapper wrapper(client_socket,
                                           HC_PACKET_TYPE_discord_plays_special_input_ack,
                                           buffer.data(), buffer.size());
                    break;
                }
                default:
                    printf("Unknown packet type: %d\n", packet_type);
                    break;
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
                socket_wrapper wrapper(client_socket);
                client_loop(wrapper, client_addr);
            }
        }
    }

} // namespace hydra