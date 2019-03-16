#ifndef __ENGINE_COORDINATOR__
#define __ENGINE_COORDINATOR__

#include "tools/variant.hpp"
#include "tools/queue_lock.hpp"
#include "events/event_helper.hpp"

#include <vector>
#include <memory>

template <typename TCoordinator>
struct PublisherCallback
{
    TCoordinator& coordinator;
    size_t executorId;

    PublisherCallback(TCoordinator& coordinator, size_t executorId) : coordinator(coordinator), executorId(executorId)
    {
    }

    template <typename TEvent>
    void publish(const TEvent& e)
    {
        coordinator.publish(e, executorId);
    }
};

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
    void publish(const TEvent& e, size_t executorId)
    {
        for (auto& a : actors)
            std::visit([&](auto& aa) { event(aa, e, executorId ); }, a);
    }

private:
    std::vector<Actors> actors;
    std::vector<std::unique_ptr<PublisherCallback<Coordinator<TA0...>>>> callbacks;
    std::vector<std::thread> threads;

    void onStart()
    {
        for(size_t i = 0; i < actors.size(); i++)
            std::visit([&](auto &a){ start(a, i); }, actors[i]);
    }

    template <typename A0, typename... A>
    void addExecutor(A0&& a0, A&&... a)
    {
        actors.push_back(Actors{std::move(a0)});
        addExecutor(a...);
    }

    template <typename TExecutor>
    void start(TExecutor& executor, size_t id)
    {
        callbacks.emplace_back(std::make_unique<PublisherCallback<Coordinator<TA0...>>>(*this, id));
        threads.push_back(executor.spawn(*(callbacks[callbacks.size()-1]), actors.size()));
    }

    template <typename TExecutor, typename TEvent>
    void event(TExecutor& executor, const TEvent& e, size_t executorId, decltype(TExecutor{}.publish(e, 0))* ignore = nullptr)
    {
        executor.publish(e, executorId);
    }

    void addExecutor() {}

    template <typename... T>
    void event(T&... ignore) { }
};

#endif // !__ENGINE_COORDINATOR__
