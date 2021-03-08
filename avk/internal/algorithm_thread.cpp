#include "algorithm_thread.h"
#include "common.h"
#include <Windows.h>
#include <atomic>
#include <new>
#pragma comment(lib, "Synchronization.lib")

// TODO: This synchronization is probably overkill: Maybe remove atomics or reduce the number of generation counters...

extern std::atomic<bool> should_continue_global;
static std::atomic<bool> should_exit_algorithm;

static sort_function_pointer sort_function;
static HANDLE thread_handle;

static bool is_run_all;
static std::atomic<bool> paused;
static std::atomic<uint32_t> head;
static std::atomic<uint32_t> tail;

static DWORD WINAPI thread_entry_point(void* unused)
{
	while (should_continue_global.load(std::memory_order_acquire))
	{
		auto h = head.load(std::memory_order_acquire);
		(void)WaitOnAddress(&head, &h, sizeof(head), INFINITE);
		if (should_exit_algorithm.load(std::memory_order_acquire))
			break;
		if (sort_function == nullptr)
			continue;
		sort_function({});
		sort_stats::clear();
		sort_function = nullptr;
		(void)tail.fetch_add(1, std::memory_order_release);
		(void)WakeByAddressSingle(&tail);
	}
	return 0;
}

namespace algorithm_thread
{
	void assign_body(sort_function_pointer sort)
	{
		await();
		sort_function = sort;
		signal();
	}

	void launch()
	{
		thread_handle = CreateThread(nullptr, 1 << 21, thread_entry_point, nullptr, 0, nullptr);
		AVK_ASSERT(thread_handle != nullptr);
	}

	bool is_paused()
	{
		return paused.load(std::memory_order_acquire);
	}

	bool is_idle()
	{
		return head.load(std::memory_order_acquire) == tail.load(std::memory_order_acquire);
	}

	void pause()
	{
		paused.store(true, std::memory_order_release);
		SuspendThread(thread_handle);
		if (cmts_lib_is_initialized())
			cmts_lib_pause();
	}

	void resume()
	{
		if (cmts_lib_is_initialized())
			AVK_ASSERT(cmts_lib_resume() == CMTS_OK);
		ResumeThread(thread_handle);
#ifdef DEBUG
		bool prior = paused.exchange(false, std::memory_order_release);
		AVK_ASSERT(prior);
#else
		paused.store(false, std::memory_order_release);
#endif
	}

	void abort_sort()
	{
		if (cmts_lib_is_initialized())
			cmts_lib_terminate(nullptr);
		should_exit_algorithm.store(true, std::memory_order_release);
		(void)head.fetch_add(1, std::memory_order_release);
		(void)WakeByAddressSingle(&head);
		if (WaitForSingleObject(thread_handle, 10) == WAIT_TIMEOUT)
			TerminateThread(thread_handle, MAXDWORD);
		sort_function = nullptr;
		non_atomic_store(should_exit_algorithm, false);
		non_atomic_store(paused, false);
		non_atomic_store(head, 0);
		non_atomic_store(tail, 0);
		launch();
	}

	void signal()
	{
		(void)head.fetch_add(1, std::memory_order_acquire);
		WakeByAddressSingle(&head);
	}

	void await(uint32_t timeout_ms)
	{
		if (is_idle())
			return;
		auto desired = head.load(std::memory_order_acquire);
		(void)WaitOnAddress(&tail, &desired, sizeof(desired), timeout_ms == UINT32_MAX ? INFINITE : timeout_ms);
	}

	void terminate()
	{
		(void)TerminateThread(thread_handle, 0);
	}
}