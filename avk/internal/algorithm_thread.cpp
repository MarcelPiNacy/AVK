#include "algorithm_thread.h"
#include "enforce.h"
#include <Windows.h>
#include <atomic>
#pragma comment(lib, "Synchronization.lib")

// TODO: This synchronization is probably overkill: Maybe remove atomics or reduce the number of generation counters...

extern std::atomic<bool> should_continue_global;

static sort_function_pointer sort_function;
static HANDLE thread_handle;
static std::atomic<uint32_t> head;
static std::atomic<uint32_t> tail;

static DWORD WINAPI algorithm_thread_entry_point(void* unused) noexcept
{
	auto& main_array = *(::main_array*)nullptr; //WHEEZE
	while (should_continue_global.load(std::memory_order_acquire))
	{
		auto h = head.load(std::memory_order_acquire);
		(void)WaitOnAddress(&head, &h, sizeof(head), INFINITE);
		sort_function(main_array);
		(void)tail.fetch_add(1, std::memory_order_release);
		(void)WakeByAddressAll(&tail);
	}
	return 0;
}

namespace algorithm_thread
{
	void assign_sort(sort_function_pointer sort) noexcept
	{
		await();
		sort_function = sort;
		(void)head.fetch_add(1, std::memory_order_acquire);
		WakeByAddressAll(&head);
	}

	void launch() noexcept
	{
		thread_handle = CreateThread(nullptr, 1 << 21, algorithm_thread_entry_point, nullptr, CREATE_SUSPENDED, nullptr);
		enforce(thread_handle != nullptr);
		ResumeThread(thread_handle);
	}

	bool is_idle() noexcept
	{
		return head.load(std::memory_order_acquire) == tail.load(std::memory_order_acquire);
	}

	void await(uint32_t timeout_ms) noexcept
	{
		if (is_idle())
			return;
		auto desired = head.load(std::memory_order_acquire);
		(void)WaitOnAddress(&tail, &desired, sizeof(desired), timeout_ms == UINT32_MAX ? INFINITE : timeout_ms);
	}
}