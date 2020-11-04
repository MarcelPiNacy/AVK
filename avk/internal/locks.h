#pragma once
#include <cstdint>
#include <atomic>
#include "platform.h"
#include "algorithm_thread.h"



struct spin_lock
{
	std::atomic<bool> ctrl;

	template <bool Optimistic = true>
	void lock() noexcept
	{
		if constexpr (Optimistic)
		{
			while (ctrl.exchange(true, std::memory_order_acquire))
			{
				platform::yield_cpu();
			}
		}
		else
		{
			while (true)
			{
				bool e = ctrl.load(std::memory_order_acquire);
				if (!e)
				{
					if (ctrl.compare_exchange_weak(e, true, std::memory_order_acquire, std::memory_order_relaxed))
					{
						break;
					}
				}
				platform::yield_cpu();
			}
		}
	}

	void unlock() noexcept
	{
		ctrl.store(false, std::memory_order_release);
	}
};