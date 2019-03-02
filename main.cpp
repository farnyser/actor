#include <iostream>
#include <vector>

#include "actor/actor.hpp"
#include "engine/executor.hpp"
#include "events/event_handler.hpp"

struct FooEvent { std::uint8_t counter{0}; };
struct BarEvent { std::uint8_t counter{0}; std::uint64_t data{0}; };

struct MyActor : public Actor<EventHandler<FooEvent>, EventPublisher<BarEvent>>
{
    void onEvent(FooEvent e)
    {
        std::cout << "foo " << (int)e.counter << std::endl;
        publish(BarEvent{5, 55});
    }
};

struct MyOtherActor : public Actor<EventHandler<BarEvent>>
{
    void onEvent(BarEvent e)
    {
        std::cout << "bar " << (int)e.counter << " " << e.data << std::endl;
    }
};

struct MyCombinedActor : public Actor<EventHandler<FooEvent, BarEvent>, EventPublisher<FooEvent>>
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

    auto dispatchor = Executor{a, b, c, c};
    auto dispatchor2 = Executor{a, b};
    auto main = Executor{dispatchor, dispatchor2};

    // using Events = typename decltype(main)::Events;
    using Events = typename decltype(dispatchor)::Events;

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

    main.dispatch(Events{FooEvent{123}});
    main.pull();

    return 0;
}
