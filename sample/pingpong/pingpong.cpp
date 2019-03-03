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
        if (e.counter % 1000000 == 0)
            std::cout << "pong received: " << e.counter << std::endl;
        publish(PingEvent{e.counter+1});
    }
};

struct PongActor : public Actor<EventHandler<PingEvent>, EventPublisher<PongEvent>>
{
    void onEvent(PingEvent e)
    {
        publish(PongEvent{e.counter});
    }
};

int main()
{
    std::cout << "Starting PingPong play !!" << std::endl;

    auto coordinator = Executor{
        Executor{PingActor{}},
        Executor{PongActor{}}
    };

    coordinator.mainloop();

    std::cout << "Kthxbye!" << std::endl;
    return 0;
}
