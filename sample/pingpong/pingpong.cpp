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
    latency<30 * 1000 * 1000, 300000> latency;

    void onStart()
    {
        publish(PingEvent{0, std::chrono::high_resolution_clock::now()});
    }

    void onEvent(PongEvent e)
    {
        latency.add(std::chrono::high_resolution_clock::now() - e.timestamp);

        if (e.counter > 0 && e.counter % 100000 == 0)
        {
            latency.generate<std::ostream, std::chrono::nanoseconds>(std::cout);
            exit(0);
        }

        publish(PingEvent{e.counter+1, std::chrono::high_resolution_clock::now()});
    }
};

struct PongActor : public Actor<EventHandler<PingEvent>, EventPublisher<PongEvent>>
{
    void onStart() {}
    void onEvent(PingEvent e)
    {
        publish(PongEvent{e.counter, e.timestamp});
    }
};

int main()
{
    std::cout << "Starting PingPong play !!" << std::endl;

    auto coordinator = Executor{"Main",
        Executor{"PingExecutor", PingActor{}},
        Executor{"PongExecutor",PongActor{}}
    };

    // decltype(coordinator)::Events xxx = 1;
    coordinator.mainloop();

    std::cout << "Kthxbye!" << std::endl;
    return 0;
}
