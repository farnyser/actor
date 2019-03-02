#ifndef __ENGINE_EXECUTOR__
#define __ENGINE_EXECUTOR__

#include "tools/variant.hpp"
#include "events/event_helper.hpp"
#include "engine/executor_actor.hpp"
#include <vector>

template <typename... TA0>
struct Executor
{
    using Actors = typename VariantToReducedVariant<std::variant<TA0...>>::Variant;
    using Events = typename _GetEvents<TA0...>::Events;
    using PublishedEvents = typename _GetPublishedEvents<TA0...>::PublishedEvents;

    Executor() {}

    Executor(TA0... actors)
    {
        addActor(actors...);
    }

    template<typename TEvent>
    void onEvent(TEvent e)
    {
        dispatch(Events{e});
    }

    template<typename TEvent>
    void dispatch(const TEvent& e)
    {
        for (auto& a : actors)
            std::visit(callback, a, e);
    }

    void pull()
    {
        for (auto& a : actors)
        {
            std::visit([&](auto &aa) {
                // std::cout << "???" << std::endl;
                aa.onPull([&](auto &ee) {
                //     std::cout << "got something ?" << std::endl;
                    dispatch(ee);
                });
             }, a);

            // std::visit(PublisherActor{}, a);
        }
    }

private:
    std::vector<Actors> actors;
    ExecutorActor<Actors> callback;

    void addActor() {}

    template <typename A0, typename... A>
    void addActor(A0& a0, A... a)
    {
        actors.push_back(Actors{a0});
        addActor(a...);
    }
};

#endif // !__ENGINE_EXECUTOR__
