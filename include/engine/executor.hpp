#ifndef __ENGINE_EXECUTOR__
#define __ENGINE_EXECUTOR__

#include "tools/variant.hpp"
#include "event_helper.hpp"
#include <string>
#include <vector>

template <typename... TA0>
struct Executor : TA0...
{
    using Actors = typename std::variant<TA0...>;
    using Events = typename _GetEvents<TA0...>::Events;
    using Tuple = typename _GetEvents<TA0...>::Tuple;

    Executor() {}

    Executor(const std::string& name, TA0... actors) : name(name)
    {
        addActor(actors...);
        // std::cout << "Executor " << name << " with " << this->actors.size() << " actors " << std::endl;
    }

    template<typename TActor, typename TEvent>
    void operator()(TActor& a, const TEvent& e, decltype(TActor::onEvent(e))* ignore = nullptr)
    {
        // std::cout << "Executor " << name << " operator()(a,e) " << std::endl;
        a.onEvent(e);
    }

    template<typename... T>
    void operator()(T... p)
    {
        // std::cout << "Executor " << name << " operator()(...) " << std::endl;
    }

    template<typename TEvent>
    void onEvent(TEvent e)
    {
        // std::cout << "Executor " << name << " onEvent(e) " << std::endl;
        dispatch(Events{e});
    }

    template<typename TEvent>
    void dispatch(const TEvent& e)
    {
        // std::cout << "Executor " << name << " dispatch(e) " << std::endl;
        for (auto& a : actors)
            dispatch(a, e);
    }


private:
    std::string name;
    std::vector<Actors> actors;

    template<typename TActor, typename TEvent>
    void dispatch(TActor& a, const TEvent& e)
    {
        std::visit(*this, a, e);
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
