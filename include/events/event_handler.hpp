#ifndef __EVENT_HANDLE__
#define __EVENT_HANDLE__

#include <tuple>
#include "tools/queue_lock.hpp"

template <typename... TEvents>
struct EventHandler
{
    using Events = std::variant<TEvents...>;
};

template <typename... TEvents>
struct EventPublisher
{
    using PublishedEvents = std::variant<TEvents...>;

    EventPublisher() : queue(new pg::adaptor::QueueLock<PublishedEvents>{}) {}

    template <typename TEvent>
    void publish(TEvent e)
    {
        while(!queue->try_push([&](auto& buffer) { buffer = e; }));
    }

    template <typename F>
    void onPull(F f)
    {
        queue->try_consume(f);
    }

private:
    pg::adaptor::QueueLock<PublishedEvents>* queue;
};

#endif // !__EVENT_HANDLE__
