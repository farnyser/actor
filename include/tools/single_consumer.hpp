#ifndef __SINGLE_CONSUMER_HPP__
#define __SINGLE_CONSUMER_HPP__

#include <atomic>
#include <memory>

namespace pg
{
	namespace lockfree
	{
		template <typename T, size_t SIZE>
		class SingleConsumer
		{
            private:
                std::atomic<unsigned long> read{0}, write{0};
                unsigned long r{0}, w{0};
                std::unique_ptr<std::array<T, SIZE>> buffer_ptr;
                std::array<T, SIZE>& buffer;

            public:
                SingleConsumer() : buffer_ptr(new std::array<T, SIZE>()), buffer(*buffer_ptr) {
                }

                template <typename F>
                bool try_consume(F f) {
                    if(r < write) {
                        f(buffer[r%SIZE]);
                        r++;
                        read++;
                        return true;
                    }
                    return false;
                }

                template <typename F>
                bool try_push(F f) {
                    if(w - read < SIZE) {
                        f(buffer[w%SIZE]);
                        write++;
                        w++;
                        return true;
                    }
                    return false;
                }
        };
    }
}

#endif
