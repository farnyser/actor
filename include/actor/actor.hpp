#ifndef __ENGINE_ACTOR__
#define __ENGINE_ACTOR__

#include <variant>
#include "events/event_handler.hpp"

template <typename... T>
struct Actor
{
};

template <typename THandler>
struct Actor<THandler> : public THandler
{
};

#endif // !__ENGINE_ACTOR__
