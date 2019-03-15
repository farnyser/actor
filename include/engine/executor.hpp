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
    std::string name;

    Executor(std::string name = "") : name(name)
    {
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

    template <typename TPublisher>
    void onStart(TPublisher& bus)
    {
        for(auto& actor : actors)
            std::visit([&](auto &a){ start(a, bus); }, actor);
    }

    template<typename TEvent>
    void onEvent(const TEvent& e, decltype(Events{TEvent{}})* ignore = nullptr)
    {
        while(!inbound->try_push([&](auto& buffer){
            buffer = e;
        }));
    }

    template<typename... T>
    void onEvent(T... ignore) { }

    template <typename P>
    auto spawn(P& bus)
    {
        return std::thread{[&]()
        {
            onStart(bus);

            while(true)
            {
                dispatch(bus);
            }
        }};
    }

private:
    std::vector<Actors> actors;
    pg::lockfree::SingleConsumer<Events, SIZE>* inbound;

    template <typename A0, typename... A>
    void addActor(A0&& a0, A&&... a)
    {
        actors.push_back(Actors{std::move(a0)});
        addActor(a...);
    }

    template <typename TPublisher>
    void dispatch(TPublisher& bus)
    {
        while(inbound->try_consume([&](auto& e) {
            dispatch(e, bus);
        }));
    }

    template<typename TEvent, typename TPublisher>
    void dispatch(const TEvent& e, TPublisher& bus)
    {
        for (auto& a : actors)
            std::visit([&](auto& aa, auto& ee) { event(aa, ee, bus); }, a, e);
    }

    template <typename TActor, typename TPublisher>
    void start(TActor& actor, TPublisher& bus, decltype(TActor{}.onStart((Publisher&)*((Publisher*)nullptr)))* ignore = nullptr)
    {
        actor.onStart(bus);
    }

    template <typename TActor, typename TPublisher>
    void start(TActor& actor, TPublisher& bus, decltype(TActor{}.onStart())* ignore = nullptr)
    {
        actor.onStart();
    }

    template <typename TActor, typename TEvent, typename TPublisher>
    void event(TActor& actor, const TEvent& e, TPublisher& bus, decltype(TActor{}.onEvent(e, (Publisher&)*((Publisher*)nullptr)))* ignore = nullptr)
    {
        actor.onEvent(e, bus);
    }

    template <typename TActor, typename TEvent, typename TPublisher>
    void event(TActor& actor, const TEvent& e, TPublisher& bus, decltype(TActor{}.onEvent(e))* ignore = nullptr)
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
};

#endif // !__ENGINE_EXECUTOR__
