#include <iostream>
#include <vector>

#include "actor/actor.hpp"
#include "engine/executor.hpp"
#include "engine/coordinator.hpp"

struct FooEvent { std::uint32_t counter{0}; };
struct BarEvent { std::uint32_t counter{0}; std::uint64_t data{0}; };

struct MyActor : public Actor<EventHandler<FooEvent>, EventPublisher<BarEvent>>
{
    MyActor() = default;
    MyActor(MyActor&&) = default;
    MyActor(const MyActor&) = delete;
    MyActor& operator=(const MyActor&) = delete;

    template <typename TPublisher>
    void onStart(TPublisher& bus)
    {
        bus.publish(BarEvent{0, 55});
    }

    template <typename TPublisher>
    void onEvent(FooEvent e, TPublisher& bus)
    {
        std::cout << "foo " << (int)e.counter << std::endl;
        bus.publish(BarEvent{e.counter+1, 55});
    }
};

struct MyOtherActor : public Actor<EventHandler<BarEvent>, EventPublisher<FooEvent>>
{
    template <typename TPublisher>
    void onEvent(BarEvent e, TPublisher& bus)
    {
        std::cout << "bar " << (int)e.counter << " " << e.data << std::endl;
        bus.publish(FooEvent{e.counter+1});
    }
};

struct MyCombinedActor : public Actor<EventHandler<FooEvent, BarEvent>>
{
    void onEvent(FooEvent e)
    {
        std::cout << "cfoo " << (int)e.counter << std::endl;
    }

    void onEvent(BarEvent e)
    {
        std::cout << "cbar " << (int)e.counter << " " << e.data << std::endl;
    }
};

int main()
{
    std::cout << "Hello, World!" << std::endl;

    auto coordinator = Coordinator {
        Executor{MyActor{}},
        Executor{MyOtherActor{}, MyCombinedActor{}},
        Executor{MyCombinedActor{}}
    };

    coordinator.mainloop();

    std::cout << "Kthxbye!" << std::endl;
    return 0;
}
