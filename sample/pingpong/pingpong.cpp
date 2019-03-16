#include <iostream>
#include <vector>
#include <chrono>

#include "actor/actor.hpp"
#include "engine/executor.hpp"
#include "engine/coordinator.hpp"
#include "tools/latency.hpp"

#define PING_PONG_ITERATION 5000000

struct PingEvent { std::uint32_t counter{0}; std::chrono::high_resolution_clock::time_point timestamp; };
struct PongEvent { std::uint32_t counter{0}; std::chrono::high_resolution_clock::time_point timestamp; };

struct PingActor : public Actor<EventHandler<PongEvent>>
{
    pg::latency<500 * 1000, 300000> latency;

    template <typename P>
    void onStart(P& bus)
    {
        bus.publish(PingEvent{0, std::chrono::high_resolution_clock::now()});
    }

    template <typename P>
    void onEvent(const PongEvent& e, P& bus)
    {
        latency.add(std::chrono::high_resolution_clock::now() - e.timestamp);

        if (e.counter == PING_PONG_ITERATION)
        {
            latency.generate<std::ostream, std::chrono::nanoseconds>(std::cout, "ns");
            exit(0);
        }

        bus.publish(PingEvent{e.counter+1, std::chrono::high_resolution_clock::now()});
    }
};

struct PongActor : public Actor<EventHandler<PingEvent>>
{
    template <typename P>
    void onEvent(const PingEvent& e, P& bus)
    {
        bus.publish(PongEvent{e.counter, e.timestamp});
    }
};

int main()
{
    std::cout << "Starting PingPong play !!" << std::endl;

    auto coordinator = Coordinator{"Main",
        Executor{"PingExecutor", PingActor{}},
        Executor{"PongExecutor", PongActor{}}
    };

    // decltype(coordinator)::Events xxx = 1;
    coordinator.mainloop();

    std::cout << "Kthxbye!" << std::endl;
    return 0;
}
