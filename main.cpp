#include <iostream>
#include <vector>

#include "actor/actor.hpp"
#include "engine/executor.hpp"
#include "events/event_handler.hpp"

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

    // auto dispatchor = Executor{a, b, c, c};
    // auto dispatchor2 = Executor{a, b};
    // auto main = Executor{dispatchor, dispatchor2};

    // using Events = typename decltype(main)::Events;
    // using Events = typename decltype(dispatchor)::Events;

    // std::vector<Events> data;
    // data.push_back(Events{FooEvent{10}});
    // data.push_back(Events{BarEvent{10, 100}});
    // data.push_back(Events{FooEvent{7}});

    // for(auto& d : data) {
        // dispatchor.dispatch(d);
        // dispatchor2.onEvent(d);
    //     main.dispatch(d);
    // }

    // dispatchor.dispatch(Events{FooEvent{123}});

    // main.dispatch(Events{FooEvent{123}});
    // main.pull();

    auto d1 = Executor{a};
    auto d2 = Executor{b};
    auto coordinator = Executor{d1, d2};

    d1.onEvent(FooEvent{1});
    d1.pull();

    // decltype(coordinator)::PublishedEvents xs = 1;
    coordinator.pull();
    d1.pull();
    d2.pull();
    coordinator.pull();
    d1.pull();
    d2.pull();
    coordinator.pull();
    d1.pull();
    d2.pull();
    coordinator.pull();


    // std::thread th1([&](){
    //     while(true) d1.pull();
    // });

    // std::thread th2([&](){
    //     while(true) d2.pull();
    // });

    // d1.onEvent(FooEvent{0});

    // while(true)
    //     coordinator.pull();

    // th1.join();
    // th2.join();

    return 0;
}
