#include <iostream>
#include <vector>

#include "actor/actor.hpp"
#include "engine/executor.hpp"

struct PingEvent { std::uint32_t counter{0}; };
struct PongEvent { std::uint32_t counter{0}; };

struct PingActor : public Actor<EventHandler<PongEvent>, EventPublisher<PingEvent>>
{
    void onStart()
    {
        publish(PingEvent{0});
    }

    void onEvent(PongEvent e)
    {
        std::cout << "pong received: " << e.counter << std::endl;
        publish(PingEvent{e.counter+1});
    }
};

struct PongActor : public Actor<EventHandler<PingEvent>, EventPublisher<PongEvent>>
{
    void onEvent(PingEvent e)
    {
        publish(PongEvent{e.counter+1});
    }
};

int main()
{
    std::cout << "Starting PingPong play !!" << std::endl;

    auto d1 = Executor{PingActor{}};
    auto d2 = Executor{PongActor{}};
    auto coordinator = Executor{d1, d2};

    auto th1 = d1.spawn();
    auto th2 = d2.spawn();

    coordinator.mainloop();

    th1.join(); th2.join();

    std::cout << "Kthxbye!" << std::endl;
    return 0;
}
