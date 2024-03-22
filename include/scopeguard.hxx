#include <functional>

namespace hydra
{

    struct scope_guard
    {
        scope_guard(std::function<void()> f) : f_(f) {}

        ~scope_guard()
        {
            f_();
        }

    private:
        std::function<void()> f_;

        scope_guard(const scope_guard&) = delete;
        scope_guard& operator=(const scope_guard&) = delete;
        scope_guard(scope_guard&&) = delete;
        scope_guard& operator=(scope_guard&&) = delete;
    };

} // namespace hydra