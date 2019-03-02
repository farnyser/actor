#include <iostream>
#include <vector>
#include <variant>
#include <tuple>
#include "variant.hpp"

struct FooEvent { std::uint8_t counter{0}; };
struct BarEvent { std::uint8_t counter{0}; std::uint64_t data{0}; };
// using Event = std::variant<FooEvent, BarEvent>;

template <typename... TEvents>
struct EventHandler
{
    using Tuple = std::tuple<TEvents...>;
};

class MyActor : public EventHandler<FooEvent>
{
public:
    void onEvent(FooEvent e)
    {
        std::cout << "foo " << (int)e.counter << std::endl;
    }
};

class MyOtherActor : public EventHandler<BarEvent>
{
public:
    void onEvent(BarEvent e)
    {
        std::cout << "bar " << (int)e.counter << " " << e.data << std::endl;
    }
};

class MyCmbinedActor : public EventHandler<FooEvent, BarEvent>
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

template <typename... T>
struct _GetEvents
{
};

template <typename T0>
struct _GetEvents<T0>
{
    using Tuple = typename T0::Tuple;
    using Events = typename TupleToVariant<Tuple>::Variant;
};

template <typename T0, typename... T>
struct _GetEvents<T0, T...>
{
    using Tuple = decltype(std::tuple_cat(typename T0::Tuple{}, typename _GetEvents<T...>::Tuple{}));
    using Events = typename TupleToVariant<Tuple>::Variant;
};

template <typename... TA0>
struct TR : TA0...
{
    template<typename TActor, typename TEvent>
    void operator()(TActor& a, const TEvent& e, decltype(TActor::onEvent(TEvent{}))* ignore = nullptr)
    {
        a.onEvent(e);
    }

    template<typename... T>
    void operator()(T... p)
    {
    }

    template<typename TActor, typename TEvent>
    void dispatch(TActor& a, const TEvent& e)
    {
        std::cout << "dispatch called" << std::endl;
        // std::visit(*this, a, std::variant<TEvent>(e));
        std::visit(*this, a, (e));
    }

    using Events = typename _GetEvents<TA0...>::Events;
};

template <typename T>
auto Event(T&& e) { return std::variant<T>{e}; }

int main()
{
    std::cout << "Hello, World!" << std::endl;

    Actor a{MyActor{}};
    Actor b{MyOtherActor{}};
    Actor c{MyCmbinedActor{}};

    std::visit(TR<MyActor>{}, a, Event(FooEvent{1}));
    std::visit(TR<MyActor>{}, a, Event(BarEvent{1, 100}));

    std::visit(TR<MyOtherActor>{}, a, Event(FooEvent{2}));
    std::visit(TR<MyOtherActor>{}, b, Event(FooEvent{2}));
    std::visit(TR<MyOtherActor>{}, b, Event(BarEvent{2, 100}));

    std::visit(TR<MyActor, MyOtherActor>{}, a, Event(FooEvent{3}));
    std::visit(TR<MyActor, MyOtherActor>{}, b, Event(FooEvent{3}));
    std::visit(TR<MyActor, MyOtherActor>{}, a, Event(BarEvent{3, 100}));
    std::visit(TR<MyActor, MyOtherActor>{}, b, Event(BarEvent{3, 100}));

    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, a, Event(FooEvent{4}));
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, b, Event(FooEvent{4}));
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, c, Event(FooEvent{4}));
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, a, Event(BarEvent{4, 100}));
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, b, Event(BarEvent{4, 100}));
    std::visit(TR<MyActor, MyOtherActor, MyCmbinedActor>{}, c, Event(BarEvent{4, 100}));

    auto dispatchor = TR<MyActor, MyOtherActor, MyCmbinedActor>{};

    typename MyCat<int>::Variant xxxx = 1;
    typename MyCat<int, float>::Variant xxx = 1;
    typename MyCat<int, int>::Variant xxzx = 1;
    typename MyCat<int, float, int>::Variant xxzzx = 1;
    typename MyCat<float, char*, int>::Variant zzezez = (char*)&"aa";
    typename MyCat<char*, float, int>::Variant xxzzzx = (char*)&"aa";
    typename MyCat<char*, float, int, char*>::Variant azazazaza = (char*)&"aa";
    typename MyCat<int, float, int, float>::Variant azazzzzazaza1 = 1;
    typename MyCat<int, float, int, float>::Variant azazzzzazaza2 = 1.0f;


    // dispatchor.dispatch(a, FooEvent{9});

    std::vector<decltype(dispatchor)::Events> data;
    data.push_back(std::variant<FooEvent, BarEvent>{FooEvent{10}});
    data.push_back(std::variant<FooEvent, BarEvent>{BarEvent{10, 100}});

    for(auto& d : data)
        dispatchor.dispatch(c, d);

    return 0;
}
