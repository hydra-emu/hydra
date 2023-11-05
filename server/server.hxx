#include <hsystem.h>

#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#elif defined(HYDRA_WINDOWS)
#pragma message("TODO: include winsock2.h")
#endif
#include "corewrapper.hxx"

namespace hydra
{
    class server_t final
    {
    public:
        server_t();
        ~server_t();

        void accept_loop();

    private:
        uint32_t socket_;
    };
} // namespace hydra