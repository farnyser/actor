#include <iostream>
#include <variant>

struct FooEvent { std::uint8_t counter{0}; };
struct BarEvent { std::uint8_t counter{0}; std::uint64_t data{0}; };
using Event = std::variant<FooEvent, BarEvent>;

class MyActor
{
public:
    void onEvent(FooEvent e)
    {
        std::cout << "foo " << (int)e.counter << std::endl;
    }
};

class MyOtherActor
{
public:
    void onEvent(BarEvent e)
    {
        std::cout << "bar " << (int)e.counter << " " << e.data << std::endl;
    }
};

class MyCmbinedActor
{
public:
    void onEvent(FooEvent e)
    {
        std::cout << "cfoo " << (int)e.counter << std::endl;
    }
    void onEvent(BarEvent e)
    {
        std::cout << "cbar " << (int)e.counter << " " << e.data << std::endl;
    }
};

using Actor = std::variant<MyActor, MyOtherActor, MyCmbinedActor>;

struct Transitions
{
    void operator()(MyActor& a, FooEvent e) { a.onEvent(e); }

    template<typename TActor, typename TEvent>
    void operator()(TActor& a, const TEvent& e) {}
};

// template <typename... TA>
// struct TR
// {
//     template<typename TActor, typename TEvent>
//     void operator()(TActor& a, const TEvent& e)
//     {
//     }
// };

// template <typename TA0, typename... TA>
// struct TR<TA0, TA...> : TA0, TR<TA...>
// {
//     using TA0::onEvent;
//     using TA::onEvent...;

//     template<typename TActor, typename TEvent>
//     void operator()(TActor& a, const TEvent& e, decltype(TActor::onEvent(TEvent{}))* ignore = nullptr)
//     {
//         onEvent(e);
//     }
// };

template <typename... TA0>
struct TR : TA0...
{
    // using TA0::onEvent...;

    template<typename TActor, typename TEvent>
    void operator()(TActor& a, const TEvent& e, decltype(TActor::onEvent(TEvent{}))* ignore = nullptr)
    {
        a.onEvent(e);
    }

    template<typename... T>
    void operator()(T... p)
    {
    }
};

int main()
{
    std::cout << "Hello, World!" << std::endl;

    Actor a{MyActor{}};
    Actor b{MyOtherActor{}};
    Actor c{MyCmbinedActor{}};

    std::visit(TR<MyActor>{}, a, Event{FooEvent{1}});
    std::visit(TR<MyActor>{}, a, Event{BarEvent{1, 100}});

    std::visit(TR<MyOtherActor>{}, a, Event{FooEvent{2}});
    std::visit(TR<MyOtherActor>{}, b, Event{FooEvent{2}});
    std::visit(TR<MyOtherActor>{}, b, Event{BarEvent{2, 100}});

    std::visit(TR<MyActor, MyOtherActor>{}, a, Event{FooEvent{3}});
    std::visit(TR<MyActor, MyOtherActor>{}, b, Event{FooEvent{3}});
    std::visit(TR<MyActor, MyOtherActor>{}, a, Event{BarEvent{3, 100}});
    std::visit(TR<MyActor, MyOtherActor>{}, b, Event{BarEvent{3, 100}});

    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, a, Event{FooEvent{4}});
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, b, Event{FooEvent{4}});
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, c, Event{FooEvent{4}});
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, a, Event{BarEvent{4, 100}});
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, b, Event{BarEvent{4, 100}});
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, c, Event{BarEvent{4, 100}});


    return 0;
}
