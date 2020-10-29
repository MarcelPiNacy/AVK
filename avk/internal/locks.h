#pragma once
#include <cstdint>
#include <atomic>
#include "platform.h"



// NON-DETERMINISTIC ACQUISITION ORDER
struct optimistic_spinlock
{
	std::atomic_bool flag;

	void lock() noexcept
	{
		while (true)
		{
			bool desired = false;
			if (!flag.load(std::memory_order_acquire))
				if (flag.compare_exchange_weak(desired, true, std::memory_order_acquire, std::memory_order_relaxed))
					break;
			platform::yield_cpu();
		}
	}

	void unlock() noexcept
	{
		flag.store(false, std::memory_order_release);
	}
};



struct rw_spinlock
{
	struct control_block
	{
		uint32_t
			write : 1,
			read_count : 31;
	};

	std::atomic<control_block> ctrl;
	
	void lock_read() noexcept
	{
		uint8_t retries = 16;
		control_block c, nc;
		while (true)
		{
			c = ctrl.load(std::memory_order_acquire);
			if (!c.write)
			{
				nc.read_count = c.read_count + 1;
				nc.write = false;
				if (ctrl.compare_exchange_weak(c, nc, std::memory_order_acquire, std::memory_order_relaxed))
					return;
			}

			if (retries != 0)
			{
				--retries;
				platform::yield_cpu();
			}
			else
			{
				platform::yield_thread();
			}
		}
	}

	void lock_write() noexcept
	{
		uint8_t retries = 16;
		control_block c;
		constexpr control_block nc = { true, 0 };

		while (true)
		{
			c = ctrl.load(std::memory_order_acquire);
			if (c.read_count == 0)
				if (ctrl.compare_exchange_weak(c, nc, std::memory_order_acquire, std::memory_order_relaxed))
					return;

			if (retries != 0)
			{
				--retries;
				platform::yield_cpu();
			}
			else
			{
				platform::yield_thread();
			}
		}
	}

	void unlock_read() noexcept
	{
		control_block c, nc;
		while (true)
		{
			c = ctrl.load(std::memory_order_acquire);
			nc.read_count = c.read_count - 1;
			nc.write = false;
			if (ctrl.compare_exchange_weak(c, nc, std::memory_order_release, std::memory_order_relaxed))
				break;
			platform::yield_cpu();
		}
	}

	void unlock_write() noexcept
	{
		ctrl.store(control_block(), std::memory_order_release);
	}
};



// DETERMINISTIC ACQUISITION ORDER
struct ticket_spinlock
{
	std::atomic<uint32_t> head;
	std::atomic<uint32_t> tail;

	void lock() noexcept
	{
		uint8_t retries = 16;
		const auto desired = head.fetch_add(1, std::memory_order_acquire);
		while (true)
		{
			auto t = tail.load(std::memory_order_acquire);
			if (t == desired)
				break;
			platform::yield_cpu();
		}
	}

	void unlock() noexcept
	{
		(void)tail.fetch_add(1, std::memory_order_release);
	}
};