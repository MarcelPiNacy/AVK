#pragma once
#include <cstdint>
#include <atomic>
#include <thread>
#include "platform.h"
#include "algorithm_thread.h"



struct spin_lock
{
	std::atomic<bool> ctrl;

	template <bool Optimistic = true>
	void lock() noexcept
	{
		while (true)
		{
			for (uint_fast8_t i = 0; i < 8; ++i)
			{
				bool e = ctrl.load(std::memory_order_acquire);
				if (!e)
					if (ctrl.compare_exchange_weak(e, true, std::memory_order_acquire, std::memory_order_relaxed))
						return;
				platform::yield_cpu();
			}
			std::this_thread::yield();
		}
	}

	void unlock() noexcept
	{
		ctrl.store(false, std::memory_order_release);
	}
};