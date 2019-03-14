#ifndef __ENGINE_EXECUTOR__
#define __ENGINE_EXECUTOR__

#include "tools/variant.hpp"
#include "tools/single_consumer.hpp"
#include "tools/queue_lock.hpp"
#include "events/event_helper.hpp"
#include <thread>
#include <vector>

#define SIZE 100

struct Publisher
{
    template <typename T>
    void publish(T& t){}
};

template <typename... TA0>
struct Executor
{
    using Actors = typename VariantToReducedVariant<std::variant<TA0...>>::Variant;
    using Events = typename _GetEvents<TA0...>::Events;
    using PublishedEvents = typename _GetPublishedEvents<TA0...>::PublishedEvents;
    std::string name;

    Executor(std::string name = "") : name(name)
    {
        outbound = new pg::lockfree::SingleConsumer<PublishedEvents, SIZE>();
        inbound = new pg::lockfree::SingleConsumer<Events, SIZE>();
    }

    Executor(TA0&&... actors) : Executor()
    {
        addActor(actors...);
    }

    Executor(std::string name, TA0&&... actors) : Executor(name)
    {
        addActor(actors...);
    }

    void onStart()
    {
        for(auto& actor : actors)
            std::visit([&](auto &a){ start(a); }, actor);
    }

    template<typename TEvent>
    void onEvent(TEvent e, decltype(Events{TEvent{}})* ignore = nullptr)
    {
        while(!inbound->try_push([&](auto& buffer){
            buffer = e;
        }));
    }

    template<typename... T>
    void onEvent(T... ignore) { }

    template <typename F>
    void onPull(F f)
    {
        outbound->try_consume(f);
    }

    auto spawn()
    {
        return std::thread{[&]()
        {
            onStart();

            while(true)
            {
                dispatch();
            }
        }};
    }

    template <typename TEvent>
    void publish(TEvent e, decltype(PublishedEvents{TEvent{}})* ignore = nullptr)
    {
        while(!outbound->try_push([&](auto& buffer) { buffer = e; }));
    }

private:
    std::vector<Actors> actors;
    pg::lockfree::SingleConsumer<PublishedEvents, SIZE>* outbound;
    pg::lockfree::SingleConsumer<Events, SIZE>* inbound;

    template <typename A0, typename... A>
    void addActor(A0&& a0, A&&... a)
    {
        actors.push_back(Actors{std::move(a0)});
        addActor(a...);
    }

    void dispatch()
    {
        while(inbound->try_consume([&](auto& e) {
            dispatch(e);
        }));
    }

    template<typename TEvent>
    void dispatch(const TEvent& e)
    {
        for (auto& a : actors)
            std::visit([&](auto& aa, auto& ee) { event(aa, ee); }, a, e);
    }

    template <typename TActor>
    void start(TActor& actor, decltype(TActor{}.onStart((Publisher&)*((Publisher*)nullptr)))* ignore = nullptr)
    {
        actor.onStart(*this);
    }

    template <typename TActor>
    void start(TActor& actor, decltype(TActor{}.onStart())* ignore = nullptr)
    {
        actor.onStart();
    }

    template <typename TActor, typename TEvent>
    void event(TActor& actor, TEvent& e, decltype(TActor{}.onEvent(e, (Publisher&)*((Publisher*)nullptr)))* ignore = nullptr)
    {
        actor.onEvent(e, *this);
    }

    template <typename TActor, typename TEvent>
    void event(TActor& actor, TEvent& e, decltype(TActor{}.onEvent(e))* ignore = nullptr)
    {
        actor.onEvent(e);
    }

    void addActor() {}

    template <typename... T>
    bool spawn(T&... ignore) { return false; }

    template <typename... T>
    void start(T&... ignore) { }

    template <typename... T>
    void event(T&... ignore) { }

    template <typename... T>
    void publish(T&... ignore) { }
};

#endif // !__ENGINE_EXECUTOR__
