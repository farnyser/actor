#include <iostream>
#include <vector>

#include "actor/actor.hpp"
#include "engine/executor.hpp"

struct FooEvent { std::uint32_t counter{0}; };
struct BarEvent { std::uint32_t counter{0}; std::uint64_t data{0}; };

struct MyActor : public Actor<EventHandler<FooEvent>, EventPublisher<BarEvent>>
{
    void onEvent(FooEvent e)
    {
        std::cout << "foo " << (int)e.counter << std::endl;
        publish(BarEvent{e.counter+1, 55});
    }
};

struct MyOtherActor : public Actor<EventHandler<BarEvent>, EventPublisher<FooEvent>>
{
    void onEvent(BarEvent e)
    {
        std::cout << "bar " << (int)e.counter << " " << e.data << std::endl;
        publish(FooEvent{e.counter+1});
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

    auto a{MyActor{}};
    auto b{MyOtherActor{}};
    auto c{MyCombinedActor{}};

    auto d1 = Executor{a};
    auto d2 = Executor{b, c};
    auto d3 = Executor{c};
    auto coordinator = Executor{d1, d2, d3};

    d1.onEvent(FooEvent{1});

    auto th1 = d1.spawn();
    auto th2 = d2.spawn();
    auto th3 = d3.spawn();

    coordinator.mainloop();

    th1.join(); th2.join(); th3.join();

    std::cout << "Kthxbye!" << std::endl;
    return 0;
}
