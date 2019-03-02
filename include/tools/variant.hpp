#ifndef __VARIANT_HELPER__
#define __VARIANT_HELPER__

#include <tuple>
#include <variant>

template <typename... T>
struct TupleToVariant {};

template <typename... T>
struct VariantToReducedVariant {};

template <typename... T>
struct TupleToReducedVariant {};

template <typename... T>
struct TupleToVariant<std::tuple<T...>>
{
    using Variant = typename std::variant<T...>;
};

template <typename... T>
struct TupleToVariant<std::variant<T...>>
{
    using Tuple = typename std::tuple<T...>;
};

template <typename... T>
struct MyCat{};

template <typename T0>
struct MyCat<T0>
{
    using Tuple = typename std::tuple<T0>;
    using Variant = typename std::variant<T0>;
};

template <bool Enable, typename T0, typename... T>
struct MyCatI {};

template <typename T0, typename... T>
struct MyCatI<true, T0, T...>
{
    using Tuple = typename MyCat<T...>::Tuple;
    using Variant = typename TupleToVariant<Tuple>::Variant;
};

template <typename T0, typename... T>
struct MyCatI<false, T0, T...>
{
    using Tuple = decltype(std::tuple_cat(std::tuple<T0>{}, typename MyCat<T...>::Tuple{}));
    using Variant = typename TupleToVariant<Tuple>::Variant;
};


template <typename T0, typename... T>
struct MyCat<T0, T...> : public MyCatI<(std::is_same<T0, T>::value + ...) == 1, T0, T...>
{
};

template <typename... T>
struct TupleToReducedVariant<std::tuple<T...>>
{
    using Variant = typename MyCat<T...>::Variant;
};

template <typename... T>
struct VariantToReducedVariant<std::variant<T...>>
{
    using Variant = typename MyCat<T...>::Variant;
};

#endif // __VARIANT_HELPER__
