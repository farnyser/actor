#ifndef __SINGLE_CONSUMER_HPP__
#define __SINGLE_CONSUMER_HPP__

#include <atomic>
#include <memory>
#include <vector>

namespace pg
{
	namespace lockfree
	{
		template <typename T, size_t SIZE>
		class SingleConsumer
		{
			private:
                const size_t producerCount;
				unsigned long rp;

			public:
				class PublisherImpl
				{
					private:
						std::atomic<unsigned long> read{0}, write{0};
						unsigned long r{0}, w{0};
						std::unique_ptr<std::array<T, SIZE>> buffer_ptr;
						std::array<T, SIZE>& buffer;

					public:
						PublisherImpl() : buffer_ptr(new std::array<T, SIZE>()), buffer(*buffer_ptr) {
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

                SingleConsumer(size_t producerCount = 1) : producerCount(producerCount)
                {
                    Publisher = new PublisherImpl[producerCount];
                }

                ~SingleConsumer() { delete[] Publisher; }

				template <typename F>
				bool try_consume(F f) {
					for(size_t i = 0; i < producerCount; i++) {
						auto&p = Publisher[rp++ % producerCount];
						if(p.try_consume(f))
							return true;
					}
					return false;
				}

				PublisherImpl* Publisher;
		};
	}
}

#endif
