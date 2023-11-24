#include <functional>

namespace hydra
{

    struct ScopeGuard
    {
        ScopeGuard(std::function<void()> f) : f_(f) {}

        ~ScopeGuard()
        {
            f_();
        }

    private:
        std::function<void()> f_;

        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
        ScopeGuard(ScopeGuard&&) = delete;
        ScopeGuard& operator=(ScopeGuard&&) = delete;
    };

} // namespace hydra