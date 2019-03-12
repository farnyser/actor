#ifndef __EVENT_PUBLISHER__
#define __EVENT_PUBLISHER__

#include <tuple>
#include <queue>

template <typename... TEvents>
struct EventPublisher
{
    using PublishedEvents = std::variant<TEvents...>;

    EventPublisher() = default;

    template <typename TEvent>
    void publish(TEvent e)
    {
        queue.push(e);
    }

    template <typename F>
    void onPull(F f)
    {
        if(queue.empty())
            return;

        f(queue.front());
        queue.pop();
    }

private:
    std::queue<PublishedEvents> queue;
};

#endif // !__EVENT_PUBLISHER__
