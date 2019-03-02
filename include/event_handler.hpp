#ifndef __EVENT_HANDLE__
#define __EVENT_HANDLE__

#include <tuple>

template <typename... TEvents>
struct EventHandler
{
    using Tuple = std::tuple<TEvents...>;
};

#endif // !__EVENT_HANDLE__
