#include <iostream>
#include <vector>
#include <chrono>

#include "actor/actor.hpp"
#include "engine/executor.hpp"
#include "tools/latency.hpp"

struct PingEvent { std::uint32_t counter{0}; std::chrono::high_resolution_clock::time_point timestamp; };
struct PongEvent { std::uint32_t counter{0}; std::chrono::high_resolution_clock::time_point timestamp; };

struct PingActor : public Actor<EventHandler<PongEvent>, EventPublisher<PingEvent>>
{
    latency<10 * 1000 * 1000, 300000> latency;

    void onStart()
    {
        publish(PingEvent{0, std::chrono::high_resolution_clock::now()});
    }

    void onEvent(PongEvent e)
    {
        latency.add(std::chrono::high_resolution_clock::now() - e.timestamp);

        if (e.counter > 0 && e.counter % 100000 == 0)
        {
            latency.generate(std::cout);
            exit(0);
        }

        publish(PingEvent{e.counter+1, std::chrono::high_resolution_clock::now()});
    }
};

struct PongActor : public Actor<EventHandler<PingEvent>, EventPublisher<PongEvent>>
{
    void onEvent(PingEvent e)
    {
        publish(PongEvent{e.counter, e.timestamp});
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
