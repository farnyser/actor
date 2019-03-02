#ifndef __EVENT_HELPER__
#define __EVENT_HELPER__

#include "tools/variant.hpp"

template <typename... T>
struct _GetEvents
{
};

template <typename T0>
struct _GetEvents<T0>
{
    using Events = typename TupleToReducedVariant<typename T0::Tuple>::Variant;
    using Tuple = typename TupleToVariant<Events>::Tuple;
};

template <typename T0, typename... T>
struct _GetEvents<T0, T...>
{
    using Events = typename TupleToReducedVariant<decltype(std::tuple_cat(typename T0::Tuple{}, typename _GetEvents<T...>::Tuple{}))>::Variant;
    using Tuple = typename TupleToVariant<Events>::Tuple;
};

#endif // !__EVENT_HELPER__
