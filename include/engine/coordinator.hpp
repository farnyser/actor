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
    std::string name;

    Coordinator(TA0&&... actors)
    {
        addExecutor(actors...);
    }

    Coordinator(std::string name, TA0&&... actors) : name(name)
    {
        addExecutor(actors...);
    }

    void mainloop()
    {
        onStart();

        for(auto& thread : threads)
            thread.join();
    }

    template<typename TEvent>
    void publish(const TEvent& e)
    {
        for (auto& a : actors)
            std::visit([&](auto& aa) { event(aa, e ); }, a);
    }

private:
    std::vector<Actors> actors;
    std::vector<std::thread> threads;

    void onStart()
    {
        for(auto& actor : actors)
            std::visit([&](auto &a){ start(a); }, actor);
    }

    template <typename A0, typename... A>
    void addExecutor(A0&& a0, A&&... a)
    {
        actors.push_back(Actors{std::move(a0)});
        addExecutor(a...);
    }

    template <typename TActor>
    void start(TActor& actor)
    {
        threads.push_back(actor.spawn(*this));
    }

    template <typename TActor, typename TEvent>
    void event(TActor& actor, const TEvent& e, decltype(TActor{}.onEvent(e))* ignore = nullptr)
    {
        actor.onEvent(e);
    }

    void addExecutor() {}

    template <typename... T>
    void event(T&... ignore) { }
};

#endif // !__ENGINE_COORDINATOR__
