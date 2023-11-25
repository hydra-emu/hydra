// Helper structs for observer pattern

#pragma once

#include <algorithm>
#include <vector>

namespace hydra
{
    struct Subject;

    struct Observer
    {
        explicit Observer(Subject* subject);

        virtual ~Observer() = default;
        virtual void update() = 0;

        Observer(const Observer&) = delete;
        Observer& operator=(const Observer&) = delete;

    protected:
        Subject* subject_;
    };

    struct Subject
    {
        void notify()
        {
            for (auto& observer : observers_)
            {
                observer->update();
            }
        }

        void attach(Observer* observer)
        {
            observers_.push_back(observer);
        }

        void detach(Observer* observer)
        {
            observers_.erase(std::remove(observers_.begin(), observers_.end(), observer),
                             observers_.end());
        }

    private:
        std::vector<Observer*> observers_;
    };

    inline Observer::Observer(Subject* subject) : subject_(subject)
    {
        subject_->attach(this);
    }
} // namespace hydra