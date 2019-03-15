#ifndef __EVENT_HELPER__
#define __EVENT_HELPER__

#include "tools/variant.hpp"
#include "events/event_handler.hpp"

template<typename...T> struct Executor;

template <typename... T>
struct _GetEvents
{
};

template <typename T0>
struct _GetEvents<T0>
{
    using Events = typename VariantToReducedVariant<typename T0::Events>::Variant;
};

template <typename T0, typename... T>
struct _GetEvents<T0, T...>
{
private:
    using p1 = typename T0::Events;
    using p2 = typename _GetEvents<T...>::Events;
    using t1 = typename TupleToVariant<p1>::Tuple;
    using t2 = typename TupleToVariant<p2>::Tuple;
public:
    using Events = typename TupleToReducedVariant<decltype(std::tuple_cat(t1{}, t2{}))>::Variant;
};

#endif // !__EVENT_HELPER__
