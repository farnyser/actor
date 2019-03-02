#include <iostream>
#include <vector>

#include "engine/executor.hpp"
#include "event_handler.hpp"
#include "event_helper.hpp"

struct FooEvent { std::uint8_t counter{0}; };
struct BarEvent { std::uint8_t counter{0}; std::uint64_t data{0}; };

struct MyActor : public EventHandler<FooEvent>
{
    void onEvent(FooEvent e)
    {
        std::cout << "foo " << (int)e.counter << std::endl;
    }
};

struct MyOtherActor : public EventHandler<BarEvent>
{
    void onEvent(BarEvent e)
    {
        std::cout << "bar " << (int)e.counter << " " << e.data << std::endl;
    }
};

struct MyCombinedActor : public EventHandler<FooEvent, BarEvent>
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

    auto dispatchor = Executor{a, b, c};
    using Events = typename decltype(dispatchor)::Events;

    std::vector<Events> data;
    data.push_back(Events{FooEvent{10}});
    data.push_back(Events{BarEvent{10, 100}});
    data.push_back(Events{FooEvent{7}});

    for(auto& d : data) {
        dispatchor.dispatch(d);
    }

    auto dispatchor2 = Executor{c};
    // auto dispatchor3 = Executor{dispatchor, dispatchor2};


    return 0;
}
