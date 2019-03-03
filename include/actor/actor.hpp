#ifndef __ENGINE_ACTOR__
#define __ENGINE_ACTOR__

#include <variant>
#include "events/event_handler.hpp"

template <typename... T>
struct Actor
{
};

template <typename THandler, typename TPublisher>
struct Actor<THandler, TPublisher> : public THandler, public TPublisher
{
};

template <typename THandler>
struct Actor<THandler> : public THandler, public EventPublisher<std::monostate>
{
};

#endif // !__ENGINE_ACTOR__
