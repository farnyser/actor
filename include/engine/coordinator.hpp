#ifndef __ENGINE_COORDINATOR__
#define __ENGINE_COORDINATOR__

#include "tools/variant.hpp"
#include "tools/queue_lock.hpp"
#include "events/event_helper.hpp"
#include <vector>

template <typename... TA0>
struct Coordinator
{
    using Actors = typename VariantToReducedVariant<std::variant<TA0...>>::Variant;
    using Events = typename _GetEvents<TA0...>::Events;
    using PublishedEvents = typename _GetPublishedEvents<TA0...>::PublishedEvents;
    std::string name;

    Coordinator(std::string name = "") : name(name)
    {
    }

    Coordinator(TA0&&... actors) : Coordinator()
    {
        addExecutor(actors...);
    }

    Coordinator(std::string name, TA0&&... actors) : Coordinator(name)
    {
        addExecutor(actors...);
    }

    void onStart()
    {
        for(auto& actor : actors)
            std::visit([&](auto &a){ start(a); }, actor);
    }

    void mainloop()
    {
        onStart();

        while(true)
        {
            pull();
        }

        for(auto& thread : threads)
            thread.join();
    }

private:
    std::vector<Actors> actors;
    std::vector<std::thread> threads;

    template <typename A0, typename... A>
    void addExecutor(A0&& a0, A&&... a)
    {
        actors.push_back(Actors{std::move(a0)});
        addExecutor(a...);
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

    template<typename TEvent>
    void dispatch(const TEvent& e)
    {
        for (auto& a : actors)
            std::visit([&](auto& aa, auto& ee) { event(aa, ee); }, a, e);
    }

    template <typename TActor>
    void start(TActor& actor, decltype(TActor{}.onStart())* ignore = nullptr)
    {
        threads.push_back(actor.spawn());
    }

    template <typename TActor, typename TEvent>
    void event(TActor& actor, TEvent& e, decltype(TActor{}.onEvent(e))* ignore = nullptr)
    {
        actor.onEvent(e);
    }

    void addExecutor() {}

    template <typename... T>
    bool spawn(T&... ignore) { return false; }

    template <typename... T>
    void start(T&... ignore) { }

    template <typename... T>
    void event(T&... ignore) { }
};

#endif // !__ENGINE_COORDINATOR__
