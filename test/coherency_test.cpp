#include <iostream>
#include <string>

#include "actor/actor.hpp"
#include "engine/executor.hpp"
#include "engine/coordinator.hpp"

struct EventA { std::uint32_t counter{0}; std::string data; };
struct EventB { std::uint32_t counter{0}; };
struct EventC { };
struct EventD { std::string data; };

#define debug(X)
// #define debug(X) X

struct Base
{
    std::thread::id self;

    void registerId(const char* name = "")
    {
        self = std::this_thread::get_id();
        std::cout << name << " is on thread " << self << std::endl;
    }

    void printErrorIfIdInvalid()
    {
        if(self != std::this_thread::get_id())
            std::cerr << "Invalid thread caller !" << std::endl;
    }
};

struct IncrementalCheck
{
    size_t previous { (size_t)-1 };

    void setNewValueAndCheck(size_t newValue)
    {
        if(newValue != previous+1)
            std::cerr << "invalid counter" << std::endl;

        previous = newValue;
    }
};

struct ActorX1 : public Actor<EventHandler<EventD>>, Base
{
    template <typename P>
    void onStart(P& bus)
    {
        registerId("X1");
        bus.publish(EventD{});
    }

    template <typename P>
    void onEvent(const EventD& e, P& bus)
    {
        printErrorIfIdInvalid();

        debug(std::cout << "ActorX1 got D" << std::endl);
        bus.publish(EventD{});
    }
};

struct ActorX2 : public Actor<EventHandler<EventB>>, Base
{
    IncrementalCheck bEventCheck;

    template <typename P>
    void onStart(P& bus)
    {
        registerId("X2");
        bus.publish(EventA{0, "Hello World !"});
    }

    template <typename P>
    void onEvent(const EventB& e, P& bus)
    {
        printErrorIfIdInvalid();
        debug(std::cout << "ActorX2 got B " << e.counter << std::endl);

        bEventCheck.setNewValueAndCheck(e.counter);
        bus.publish(EventA{e.counter+1, "Hello World !"});
        bus.publish(EventC{});
    }
};

struct ActorY : public Actor<EventHandler<EventA>>, Base
{
    IncrementalCheck aEventCheck;

    void onStart()
    {
        registerId("Y");
    }

    template <typename P>
    void onEvent(const EventA& e, P& bus)
    {
        printErrorIfIdInvalid();
        debug(std::cout << "ActorY got A " << e.counter << std::endl);

        aEventCheck.setNewValueAndCheck(e.counter);
        bus.publish(EventB{e.counter});
    }
};

struct ActorZ : public Actor<EventHandler<EventA, EventB, EventC>>, Base
{
    IncrementalCheck aEventCheck;
    IncrementalCheck bEventCheck;

    void onStart()
    {
        registerId("Z");
    }

    void onEvent(const EventA& e)
    {
        printErrorIfIdInvalid();
        debug(std::cout << "ActorZ got A " << e.counter << std::endl);

        aEventCheck.setNewValueAndCheck(e.counter);
        if (e.counter == 1000000)
            exit(0);
    }

    void onEvent(const EventB& e)
    {
        printErrorIfIdInvalid();
        debug(std::cout << "ActorZ got B " << std::endl);
        bEventCheck.setNewValueAndCheck(e.counter);
    }

    void onEvent(const EventC&)
    {
        printErrorIfIdInvalid();
        debug(std::cout << "ActorZ got C " << std::endl);
    }
};

int main()
{
    std::cout << "Starting coherency test !!" << std::endl;

    auto coordinator = Coordinator{
        Executor{"X", ActorX1{}, ActorX2{}},
        Executor{"Y", ActorY{}},
        Executor{"Z", ActorZ{}}
    };

    coordinator.mainloop();

    std::cout << "Kthxbye!" << std::endl;
    return 0;
}
