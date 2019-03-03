#ifndef __ENGINE_EXECUTOR__
#define __ENGINE_EXECUTOR__

#include "tools/variant.hpp"
#include "tools/queue_lock.hpp"
#include "events/event_helper.hpp"
#include "engine/executor_actor.hpp"
#include <vector>

template <typename... TA0>
struct Executor
{
    using Actors = typename VariantToReducedVariant<std::variant<TA0...>>::Variant;
    using Events = typename _GetEvents<TA0...>::Events;
    using PublishedEvents = typename _GetPublishedEvents<TA0...>::PublishedEvents;

    Executor()
    {
        queue = new pg::adaptor::QueueLock<PublishedEvents>();
    }

    Executor(TA0&&... actors) : Executor()
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
        dispatch(Events{e});
    }

    template<typename... T>
    void onEvent(T... ignore)
    {
    }

    template <typename F>
    void onPull(F f)
    {
        queue->try_consume(f);
    }

    template<typename TEvent>
    void dispatch(const TEvent& e)
    {
        for (auto& a : actors)
            std::visit(callback, a, e);
    }

    void mainloop()
    {
        onStart();

        while(true)
        {
            pull();
            dispatch();
        }

        for(auto& thread : threads)
            thread.join();
    }

    auto spawn()
    {
        return std::thread{[&]()
        {
            onStart();

            while(true)
                pull();
        }};
    }

private:
    std::vector<Actors> actors;
    std::vector<Actors*> publishers;
    std::vector<std::thread> threads;
    ExecutorActor<Actors> callback;
    pg::adaptor::QueueLock<PublishedEvents>* queue;

    void addActor() {}

    template <typename A0, typename... A>
    void addActor(A0&& a0, A&&... a)
    {
        actors.push_back(Actors{std::move(a0)});
        addActor(a...);
    }

    void pull()
    {
        for (auto& a : actors)
        {
            std::visit([&](auto &aa) {
                aa.onPull([&](auto &ee) {
                    std::visit([&](auto& ev) {
                        publish(ev);
                    }, ee);
                });
             }, a);
        }
    }

    void dispatch()
    {
        std::vector<PublishedEvents> events;
        while(queue->try_consume([&](auto& e) { events.push_back(e); }));

        for (auto& e : events)
            dispatch(e);
    }


    template <typename TEvent>
    void publish(TEvent e, decltype(PublishedEvents{TEvent{}})* ignore = nullptr)
    {
        while(!queue->try_push([&](auto& buffer) { buffer = e; }));
    }

    template <typename... T>
    void publish(T&... ignore) { }

    template <typename TActor>
    bool spawn(TActor& actor, decltype(TActor{}.spawn())* ignore = nullptr)
    {
        threads.push_back(actor.spawn());
        return true;
    }

    template <typename... T>
    bool spawn(T&... ignore) { return false; }

    template <typename TActor>
    void start(TActor& actor, decltype(TActor{}.onStart())* ignore = nullptr)
    {
        if (!spawn(actor))
            actor.onStart();
    }

    template <typename... T>
    void start(T&... ignore) { }
};

#endif // !__ENGINE_EXECUTOR__
