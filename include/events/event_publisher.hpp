#ifndef __EVENT_PUBLISHER__
#define __EVENT_PUBLISHER__

#include <tuple>
#include <queue>

template <typename... TEvents>
struct EventPublisher
{
    using PublishedEvents = std::variant<TEvents...>;
};

#endif // !__EVENT_PUBLISHER__
