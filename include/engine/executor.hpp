#ifndef __ENGINE_EXECUTOR__
#define __ENGINE_EXECUTOR__

#include "tools/variant.hpp"
#include "event_helper.hpp"
#include <vector>

template <typename... TA0>
struct Executor : TA0...
{
    using Actors = typename std::variant<TA0...>;
    using Events = typename _GetEvents<TA0...>::Events;
    using Tuple = typename _GetEvents<TA0...>::Tuple;

    Executor(TA0... actors)
    {
        addActor(actors...);
    }

    template<typename TActor, typename TEvent>
    void operator()(TActor& a, const TEvent& e, decltype(TActor::onEvent(TEvent{}))* ignore = nullptr)
    {
        a.onEvent(e);
    }

    template<typename... T>
    void operator()(T... p)
    {
    }

    template<typename TEvent>
    void dispatch(const TEvent& e)
    {
        // std::cout << "dispatch called" << std::endl;
        for(auto& a : actors)
            dispatch(a, e);
    }


private:
    std::vector<Actors> actors;

    template<typename TActor, typename TEvent>
    void dispatch(TActor& a, const TEvent& e)
    {
        // std::visit([&](auto& aa, auto& ee) {
        //     std::cout << "visit called" << std::endl;
        //     operator()(aa, ee);
        // }, a, e);
        std::visit(*this, a, (e));
    }

    void addActor() {}

    template <typename A0, typename... A>
    void addActor(A0& a0, A... a)
    {
        actors.push_back(Actors{a0});
        addActor(a...);
    }
};

#endif // !__ENGINE_EXECUTOR__
