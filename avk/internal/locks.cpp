#include "locks.h"

void spinlock::lock()
{
	for (;; platform::yield_cpu())
	{
		bool e = ctrl.load(std::memory_order_acquire);
		if (e) continue;
		if (ctrl.compare_exchange_weak(e, true, std::memory_order_acquire, std::memory_order_relaxed))
			break;
	}
}

void spinlock::unlock()
{
	ctrl.store(false, std::memory_order_release);
}

void ticket_spinlock::lock()
{
	auto k = head.fetch_add(1, std::memory_order_acquire);
	while (tail.load(std::memory_order_acquire) != k)
		platform::yield_cpu();
}

void ticket_spinlock::unlock()
{
	(void)tail.fetch_add(1, std::memory_order_release);
}
