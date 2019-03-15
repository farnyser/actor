#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include "tools/single_consumer.hpp"
#include "tools/latency.hpp"

struct PingEvent { std::uint32_t counter{0}; std::chrono::high_resolution_clock::time_point timestamp; };

struct PingActor
{
    pg::latency<20 * 1000 * 1000, 300000> latency;

    template <typename P>
    void onStart(P& bus)
    {
        bus.publish(PingEvent{0, std::chrono::high_resolution_clock::now()});
    }

    template <typename P>
    void onEvent(const PingEvent& e, P& bus)
    {
        latency.add(std::chrono::high_resolution_clock::now() - e.timestamp);

        if (e.counter > 0 && e.counter % 1000000 == 0)
        {
            latency.generate<std::ostream, std::chrono::nanoseconds>(std::cout, "ns");
            exit(0);
        }

        bus.publish(PingEvent{e.counter+1, std::chrono::high_resolution_clock::now()});
    }
};

struct PongActor
{
    template <typename P>
    void onEvent(const PingEvent& e, P& bus)
    {
        bus.publish(PingEvent{e.counter, e.timestamp});
    }
};

int main()
{
    std::cout << "Starting PingPong play !!" << std::endl;

    PingActor ping;
    PongActor pong;

    pg::lockfree::SingleConsumer<PingEvent, 100> queue;
    pg::lockfree::SingleConsumer<PingEvent, 100> queue2;
    pg::latency<20 * 1000 * 1000, 300000> latency;

    std::thread pingThread{[&](){
        std::cout << "ping" << std::endl;

        while(!queue.try_push([](PingEvent& e){
            e.counter = 0;
            e.timestamp = std::chrono::high_resolution_clock::now();
        }));

        while(true)
        {
            queue2.try_consume([&](auto& e) {
                latency.add(std::chrono::high_resolution_clock::now() - e.timestamp);

                if (e.counter > 0 && e.counter % 1000000 == 0)
                {
                    latency.generate<std::ostream, std::chrono::nanoseconds>(std::cout, "ns");
                    exit(0);
                }

                while(!queue.try_push([&](PingEvent& ee){
                    ee.counter = e.counter+1;
                    ee.timestamp = std::chrono::high_resolution_clock::now();
                }));
            });
        }
    }};

    std::thread pongThead{[&](){
        std::cout << "pong" << std::endl;

        while(true)
        {
            queue.try_consume([&](auto& e) {
                while(!queue2.try_push([&](PingEvent& ee){
                    ee.counter = e.counter;
                    ee.timestamp = e.timestamp;
                }));
            });
        }
    }};

    pingThread.join();
    std::cout << "Kthxbye!" << std::endl;
    return 0;
}
