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
    using Tuple = typename T0::Tuple;
    using Events = typename TupleToReducedVariant<Tuple>::Variant;
};

template <typename T0, typename... T>
struct _GetEvents<T0, T...>
{
    using Tuple = decltype(std::tuple_cat(typename T0::Tuple{}, typename _GetEvents<T...>::Tuple{}));
    using Events = typename TupleToReducedVariant<Tuple>::Variant;
};

#endif // !__EVENT_HELPER__
