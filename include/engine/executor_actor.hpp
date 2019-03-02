#ifndef __EXECUTOR_ACTOR__
#define __EXECUTOR_ACTOR__

#include "tools/variant.hpp"

template <typename... TActorBase>
struct ExecutorActor
{
};

template <typename... TActorBase>
struct ExecutorActor<std::variant<TActorBase...>> : TActorBase...
{
    template<typename TActor, typename TEvent>
    void operator()(TActor& a, const TEvent& e, decltype(TActor::onEvent(e))* ignore = nullptr)
    {
        a.onEvent(e);
    }

    template<typename... T>
    void operator()(T... p)
    {
    }
};

template <typename... TActorBase>
struct PublisherActor
{
};

template <typename... TActorBase>
struct PublisherActor<std::variant<TActorBase...>> : TActorBase...
{
    template<typename TActor, typename F>
    void operator()(TActor& a, F f, decltype(TActor::onPull(f))* ignore = nullptr)
    {
        a.onPull(f);
    }

    template<typename... T>
    void operator()(T... p)
    {
    }
};

#endif // !__EXECUTOR_ACTOR__
