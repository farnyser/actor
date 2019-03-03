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

    Executor(TA0... actors) : Executor()
    {
        addActor(actors...);
    }

    template<typename TEvent>
    void onEvent(TEvent e, decltype(Events{TEvent{}})* ignore = nullptr)
    {
        // std::cout << "on event called !" << (this) << std::endl;
        dispatch(Events{e});
    }

    template<typename... T>
    void onEvent(T... ignore)
    {
        // std::cout << "on event is sad... " << (this) << std::endl;
    }

    template <typename F>
    void onPull(F f)
    {
        queue->try_consume(f);
    }

    template <typename TEvent>
    void publish(TEvent e, decltype(PublishedEvents{TEvent{}})* ignore = nullptr)
    {
        std::cout << "> publish called..." << (this) << std::endl;
        while(!queue->try_push([&](auto& buffer) { buffer = e; }));
        std::cout << "> publish done !" << (this) << std::endl;
    }

    template <typename... T>
    void publish(T... t)
    {
        // std::cout << "publish is sad... " << (this) << std::endl;
    }

    template<typename TEvent>
    void dispatch(const TEvent& e)
    {
        // std::cout << "dispatch called !" << (this) << std::endl;

         std::visit([&](auto& ev) {
            publish(ev);
        }, e);

        for (auto& a : actors)
            std::visit(callback, a, e);
    }

    void pull()
    {
        for (auto& a : actors)
        {
            std::visit([&](auto &aa) {
                aa.onPull([&](auto &ee) {
                    dispatch(ee);
                });
             }, a);
        }
    }

    void mainloop()
    {
        while(true)
        {
            pull();
        }
    }

private:
    std::vector<Actors> actors;
    std::vector<Actors*> publishers;
    ExecutorActor<Actors> callback;
    pg::adaptor::QueueLock<PublishedEvents>* queue;

    void addActor() {}

    template <typename A0, typename... A>
    void addActor(A0& a0, A... a)
    {
        actors.push_back(Actors{a0});
        addActor(a...);
    }
};

#endif // !__ENGINE_EXECUTOR__
