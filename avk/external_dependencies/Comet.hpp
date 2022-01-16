/*
	Copyright 2022 Marcel Pi Nacy
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at
		http://www.apache.org/licenses/LICENSE-2.0
	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#ifndef COMET_INCLUDED
#define COMET_INCLUDED
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>

#undef Yield

#if ((defined(__clang__) || defined(__GNUC__)) && !defined(NDEBUG)) || (defined(_MSVC_LANG) && defined(_DEBUG))
#define COMET_DEBUG
#endif

#ifndef COMET_MAX_PRIORITY
#define COMET_MAX_PRIORITY 4
#endif



namespace Comet
{
	enum class SchedulerState : uint8_t
	{
		Uninitialized,
		Initializing,
		Running,
		Pausing,
		Paused,
		Resuming,
		ShuttingDown,
		Finalizing,

		MaxEnum
	};

	enum class DispatchResult : uint8_t
	{
		Success,
		Sequential,
		Failure
	};

	enum class DispatchErrorPolicy : uint8_t
	{
		RunSequentially,
		Spin,
		Return,
		Abort
	};

#define COMET_DEFINE_COMMON_FUNCTIONS(TYPE)	\
	TYPE(const TYPE&) = delete;				\
	TYPE& operator=(const TYPE&) = delete;	\
	~TYPE() = default

	struct Fence
	{
		uint32_t state;

		constexpr Fence() : state(UINT32_MAX) { }
		COMET_DEFINE_COMMON_FUNCTIONS(Fence);

		void		Signal();
		void		Await();

		constexpr void Reset()
		{
			state = UINT32_MAX;
		}
	};

	struct alignas(16) Counter
	{
		uint64_t state[2];

		constexpr Counter(uint64_t value) : state{ value, UINT64_MAX } { }
		COMET_DEFINE_COMMON_FUNCTIONS(Counter);

		bool		Decrement();
		bool		WakeAll();
		bool		Await();
		void		Reset(uint64_t new_value);
		uint64_t	Value() const;
	};

	struct SpinLock
	{
		uint32_t state;

		constexpr SpinLock() : state(UINT32_MAX) { }
		COMET_DEFINE_COMMON_FUNCTIONS(SpinLock);

		void		Lock();
		bool		IsLocked() const;
		bool		TryLock();
		void		Unlock();
	};

	/* @TODO
	struct Mutex
	{
		uint64_t state[2];

		constexpr Mutex() : state{ UINT64_MAX, UINT64_MAX } { }
		COMET_DEFINE_COMMON_FUNCTIONS(Mutex);

		void		Lock();
		bool		IsLocked() const;
		bool		TryLock();
		void		Unlock();
	};
	*/

	struct MCSMutex
	{
		struct Node
		{
			Node* next = nullptr;
			uint32_t index = UINT32_MAX;
		};

		size_t state;

		constexpr MCSMutex() : state(0) { }
		COMET_DEFINE_COMMON_FUNCTIONS(MCSMutex);

		void		Lock(Node& node);
		bool		IsLocked() const;
		bool		TryLock(Node& node);
		void		Unlock(Node& node);
	};

#undef COMET_DEFINE_COMMON_FUNCTIONS

	struct InitOptions
	{
		uint32_t thread_stack_size;
		uint32_t task_stack_size;
#ifndef COMET_FIXED_PROCESSOR_COUNT
		uint32_t max_threads;
#endif
		uint32_t max_tasks;
		uint64_t reseed_threshold_ns;
		const uint32_t* affinity_indices;

		static InitOptions Default();
	};

	struct TaskOptions
	{
		const uint32_t* preferred_thread;
		Counter* counter;
		DispatchErrorPolicy error_policy;
		uint8_t priority;

		static constexpr TaskOptions Default() { return {}; }
	};

	bool			Init();
	bool			Init(InitOptions options);
	bool			Pause();
	bool			Resume();
	bool			Shutdown();
	bool			TryFinalize();
	bool			Finalize();
	void			Terminate();
	SchedulerState	GetSchedulerState();
	bool			IsTask();
	DispatchResult	Dispatch(void(*fn)(void* param), void* param);
	DispatchResult	Dispatch(void(*fn)(void* param), void* param, TaskOptions options);
	void			Yield();
	void			Exit();
	uint64_t		ThisTaskID();
	uint32_t		WorkerThreadIndex();
	uint32_t		MaxTasks();

#ifndef COMET_FIXED_PROCESSOR_COUNT
	uint32_t		WorkerThreadCount();
#else
	constexpr uint32_t WorkerThreadCount()
	{
		return COMET_FIXED_PROCESSOR_COUNT;
	}
#endif

	namespace Debug
	{
		using		MessageFn = void(*)(void* context, const char* message, size_t size);

#ifdef COMET_DEBUG
		void		SetYieldTrap(bool k);
#else
		constexpr
		void		SetYieldTrap(bool value) {}
#endif
		void		SetInfoCallback(MessageFn callback, void* context = nullptr);
		void		SetWarningCallback(MessageFn callback, void* context = nullptr);
		void		SetErrorCallback(MessageFn callback, void* context = nullptr);
	}

	namespace RCU
	{
		constexpr size_t GetSchedulerSnapshotSize(size_t thread_count)
		{
			return thread_count * 4;
		}

		size_t		GetSchedulerSnapshotSize();
		void		GetSchedulerSnapshot(void* out);
		uint32_t	TrySync(void* snapshot, uint32_t prior_result = 0);

		template <typename F>
		void CriticalSection(F&& body)
		{
			Debug::SetYieldTrap(true);
			body();
			Debug::SetYieldTrap(false);
		}
	}




	template <typename F>
	void CriticalSection(SpinLock& lock, F&& body)
	{
		lock.Lock();
		body();
		lock.Unlock();
	}

	/* @TODO
	template <typename F>
	void CriticalSection(Mutex& lock, F&& body)
	{
		lock.Lock();
		body();
		lock.Unlock();
	}
	*/

	template <typename F>
	void CriticalSection(MCSMutex& lock, F&& body)
	{
		MCSMutex::Node node;
		lock.Lock(node);
		body();
		lock.Unlock(node);
	}

	template <typename F>
	auto Dispatch(F&& fn, TaskOptions options = {})
	{
		struct Context { F fn; Fence fence; };
		Context c = { std::forward<F>(fn), Fence() };
		auto code = Dispatch([](void* ptr)
		{
			auto& ctx_ref = *(Context*)ptr;
			F fn(std::move(ctx_ref.fn));
			ctx_ref.fence.Signal();
			fn();
		}, &c, options);
		switch (code)
		{
		case DispatchResult::Success:
			c.fence.Await();
			c.fence.Reset();
			break;
		case DispatchResult::Sequential:
		case DispatchResult::Failure:
			break;
		default:
			assert(0);
		}
		return code;
	}

	template <typename I, typename J>
	bool ForEach(I begin, J end, void(*body)(I value), TaskOptions options = {})
	{
		using F = decltype(body);
		if (begin == end)
			return true;
		assert(end > begin);
		struct Context { F fn; I it; Fence fence; };
		Context c = { body, begin, Fence() };
		for (; c.it < end; ++c.it)
		{
			auto code = Dispatch([](void* ptr)
			{
				auto& ctx_ref = *(Context*)ptr;
				auto fn = ctx_ref.fn;
				auto it = ctx_ref.it;
				ctx_ref.fence.Signal();
				fn(it);
			}, &c, options);
			switch (code)
			{
			case DispatchResult::Success:
				c.fence.Await();
				c.fence.Reset();
				break;
			case DispatchResult::Sequential:
				break;
			case DispatchResult::Failure:
				return false;
			default:
				assert(0);
			}
		}
		return true;
	}

	namespace Impl
	{
		template <typename T>
		constexpr bool is_lambda = !(
			std::is_trivially_constructible_v<T> &&
			std::is_trivially_copyable_v<T> &&
			std::is_trivially_destructible_v<T> &&
			std::is_class_v<T> &&
			std::is_empty_v<T>);

		template <typename T>
		using enable_if_lambda = std::enable_if_t<is_lambda<T>>;
	}

	template <typename I, typename J, typename F, typename = Impl::enable_if_lambda<F>>
	bool ForEach(I begin, J end, F&& body, TaskOptions options = {})
	{
		if (begin == end)
			return true;
		assert(end > begin);
		struct Context { F fn; I it; Fence fence; };
		Context c = { std::forward<F>(body), begin, Fence() };
		assert(options.counter == nullptr);
		Counter ctr = end - begin;
		options.counter = &ctr;
		for (; c.it < end; ++c.it)
		{
			auto code = Dispatch([](void* ptr)
			{
				auto& ctx = *(Context*)ptr;
				I it = ctx.it;
				ctx.fence.Signal();
				ctx.fn(it);
			}, &c, options);

			switch (code)
			{
			case DispatchResult::Success:
				c.fence.Await();
				c.fence.Reset();
				break;
			case DispatchResult::Sequential:
				break;
			case DispatchResult::Failure:
				return false;
			default:
				assert(0);
			}
		}
		ctr.Await();
		return true;
	}
}
#endif



#ifdef COMET_IMPLEMENTATION
#if defined(__clang__) || defined(__GNUC__)
#include <immintrin.h>
#ifdef COMET_DEBUG
#define COMET_NOINLINE
#define COMET_INLINE
#define COMET_FLATTEN
#define COMET_DEBUGTRAP __builtin_debugtrap()
#else
#define COMET_NOINLINE __attribute__((noinline))
#define COMET_INLINE __attribute__((always_inline))
#define COMET_FLATTEN __attribute__((flatten))
#define COMET_DEBUGTRAP
#endif
#define COMET_CLZ32 __builtin_clz
#define COMET_CTZ32 __builtin_ctz
#ifdef __clang__
#define COMET_ROL64 __builtin_rotateleft64
#else
#define COMET_ROL64(V, K) ((V << K) | (V >> (64 - K)))
#endif
#define COMET_UNREACHABLE __builtin_unreachable()
#define COMET_ASSUME __builtin_assume
#if defined(__x86_64__) || defined(__i386__)
#define COMET_SPIN __builtin_ia32_pause()
#elif defined(__ARM__)
#define COMET_SPIN __yield()
#define COMET_NO_RDRAND
#else
#define COMET_SPIN std::atomic_thread_fence(std::memory_order_acq_rel)
#endif
#else
#include <intrin.h>
#ifdef COMET_DEBUG
#define COMET_NOINLINE
#define COMET_INLINE
#define COMET_FLATTEN
#define COMET_DEBUGTRAP __debugbreak()
#else
#define COMET_NOINLINE __declspec(noinline)
#define COMET_INLINE __forceinline
#define COMET_FLATTEN COMET_INLINE
#define COMET_DEBUGTRAP
#endif
#if defined(_M_X86) || defined(_M_X64)
#define COMET_CLZ32 __lzcnt
#define COMET_CTZ32 _tzcnt_u32
#define COMET_SPIN _mm_pause()
#elif defined(_M_ARM64) || defined(_M_ARM)
#define COMET_CLZ32 _arm_clz
#define COMET_CTZ32(M) _arm_clz(_arm_rbit(M))
#define COMET_SPIN __yield()
#define COMET_NO_RDRAND
#endif
#define COMET_ROL64 _rotl64
#define COMET_UNREACHABLE __assume(0)
#define COMET_ASSUME __assume
#endif

#ifdef _WIN32
#include <Windows.h>
#undef Yield
#ifdef COMET_NO_BUSY_WAIT
#pragma comment(lib, "Synchronization.lib")
#endif
#define COMET_TASK_CALL __stdcall
#define COMET_THREAD_CALL __stdcall
#else
#define COMET_TASK_CALL
#define COMET_THREAD_CALL
#endif

#include <string_view>
#include <utility>
#include <atomic>
#include <new>

#define COMET_SHARED_ATTR alignas(std::hardware_destructive_interference_size)


namespace Comet
{
	using std::atomic;

	struct TaskContext;
	struct ThreadContext;

#ifdef _WIN32
	using ThreadHandle = HANDLE;
	using ThreadParam = PVOID;
	using ThreadReturn = DWORD;
	using ThreadEntryPoint = LPTHREAD_START_ROUTINE;
	using TaskHandle = PVOID;
	using TaskEntryPoint = LPFIBER_START_ROUTINE;
#endif

	struct TaskContext;

	struct MPSCQueue
	{
		COMET_SHARED_ATTR atomic<uint32_t> head;
		COMET_SHARED_ATTR atomic<uint32_t> size;
		COMET_SHARED_ATTR atomic<uint32_t>* values;
		uint32_t tail;
	};

	struct ThreadContext
	{
		TaskContext* this_task;
		TaskHandle root_task_handle;
		uint64_t romu2jr[2];
		uint64_t next_reseed;
		uint32_t yield_counter;
		uint32_t reseed_counter;
		uint32_t spinlock_next;
		uint32_t local_accumulator;
		uint8_t quit_flag;
#ifdef COMET_NO_BUSY_WAIT
		uint32_t last_counter;
#endif
		bool spinlock_flag;
#ifdef COMET_DEBUG
		bool yield_trap;
#endif
#ifdef COMET_NO_BUSY_WAIT
		COMET_SHARED_ATTR atomic<uint32_t> counter;
#endif
		COMET_SHARED_ATTR MPSCQueue queues[COMET_MAX_PRIORITY];
		ThreadHandle handle;
	};

	struct TaskContext
	{
		TaskHandle handle;
		void(*fn)(void* param);
		void* param;
		Counter* counter;
		uint32_t generation;
		uint32_t next;
		uint8_t priority;
		uint8_t sleeping;
	};

	struct alignas(8) IndexPair
	{
		uint32_t first, second;
	};

	struct COMET_SHARED_ATTR TaskFreeList
	{
		atomic<IndexPair> head;
	};

	struct CounterState
	{
		std::atomic<uint64_t> counter;
		std::atomic<IndexPair> queue;
	};

	static thread_local ThreadContext* this_thread;

	namespace Debug
	{
		static MessageFn info_fn, warning_fn, error_fn;
		static void* info_ctx;
		static void* warning_ctx;
		static void* error_ctx;

		COMET_INLINE static void Info(std::string_view message)
		{
			info_fn(info_ctx, message.data(), message.size());
		}

		COMET_INLINE static void Warning(std::string_view message)
		{
			warning_fn(warning_ctx, message.data(), message.size());
		}

		COMET_INLINE static void Error(std::string_view message)
		{
			error_fn(error_ctx, message.data(), message.size());
		}
	}

	enum class TaskState : uint8_t
	{
		Uninitialized,
		Running,
		Pausing,
		Paused,
		Resuming,
	};

#ifdef COMET_DEBUG
	COMET_NOINLINE static
	void CustomAssertHandler(std::string_view text)
	{
		static atomic<bool> flag;
		COMET_DEBUGTRAP;
		if (!flag.exchange(true, std::memory_order_acquire))
			Debug::Error(text);
		abort();
	}
#define COMET_SYMBOL_TO_STRING(E) #E
#define COMET_ASSERT(E) if (!(E)) CustomAssertHandler("Debug assertion failed. Expression: \"" COMET_SYMBOL_TO_STRING(E) "\".")
#define COMET_INVARIANT COMET_ASSERT
#else
#define COMET_ASSERT(E)
#define COMET_INVARIANT COMET_ASSUME
#endif

	namespace OS
	{
		COMET_INLINE static void* Malloc(size_t n)
		{
			return VirtualAlloc(nullptr, n, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		}

		COMET_INLINE static void Free(void* p, size_t n)
		{
			(void)VirtualFree(p, 0, MEM_RELEASE);
		}

		COMET_INLINE static ThreadHandle NewThread(ThreadEntryPoint fn, ThreadParam param, size_t stack_size, size_t affinity)
		{
			ThreadHandle r = CreateThread(nullptr, stack_size, fn, param, CREATE_SUSPENDED, nullptr);
			GROUP_AFFINITY prior, desired = {};
			desired.Group = (WORD)(affinity >> 6);
			desired.Mask = (ULONG_PTR)1 << (affinity & 63);
			if (r != nullptr)
			{
				if (!SetThreadGroupAffinity(r, &desired, &prior))
				{
					(void)TerminateThread(r, MAXDWORD);
					return nullptr;
				}
				::ResumeThread(r);
			}
			return r;
		}

		COMET_INLINE static bool PauseThread(ThreadHandle thread)
		{
			return SuspendThread(thread) != MAXDWORD;
		}

		COMET_INLINE static bool ResumeThread(ThreadHandle thread)
		{
			return ::ResumeThread(thread) != MAXDWORD;
		}

		COMET_INLINE static bool AwaitThread(ThreadHandle thread)
		{
			return WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0;
		}

		COMET_INLINE static void KillThread(ThreadContext& thread)
		{
			(void)TerminateThread(thread.handle, MAXDWORD);
			if (thread.this_task != nullptr)
				thread.this_task->handle = nullptr;
		}

		[[noreturn]]
		COMET_INLINE static void ExitThread()
		{
			if (this_thread->this_task != nullptr)
				this_thread->this_task->handle = nullptr;
			::ExitThread(0);
		}

		COMET_INLINE static uint64_t GetThreadID()
		{
			return ::GetCurrentThreadId();
		}

#ifdef COMET_NO_BUSY_WAIT
		COMET_INLINE static void FutexAwait(atomic<uint32_t>& futex, uint32_t& prior_value)
		{
			WaitOnAddress((volatile void*)&futex, (PVOID)&prior_value, 4, INFINITE);
		}

		COMET_INLINE static void FutexSignal(atomic<uint32_t>& futex)
		{
			(void)futex.fetch_add(1, std::memory_order_relaxed);
			WakeByAddressSingle(&futex);
		}
#endif
	}

	namespace Task
	{
		COMET_INLINE static TaskHandle New(TaskEntryPoint fn, TaskContext* context, size_t stack_size)
		{
			return CreateFiberEx(stack_size, stack_size, FIBER_FLAG_FLOAT_SWITCH, fn, context);
		}

		COMET_INLINE static void Delete(TaskHandle& handle)
		{
			if (handle != nullptr)
			{
				DeleteFiber(handle);
				handle = nullptr;
			}
		}

		COMET_INLINE static void Switch(TaskHandle from, TaskHandle to)
		{
			SwitchToFiber(to);
		}
	}

	static atomic<SchedulerState> lib_state;

	template <SchedulerState From, SchedulerState To>
	COMET_FLATTEN bool SwitchState()
	{
		static_assert((uint8_t)From < (uint8_t)SchedulerState::MaxEnum);
		SchedulerState prior = From;
		if constexpr (From == SchedulerState::Uninitialized)
		{
			static_assert(To == SchedulerState::Initializing);
			return lib_state.compare_exchange_strong(prior, To, std::memory_order_acquire, std::memory_order_relaxed);
		}
		else if constexpr (From == SchedulerState::Initializing)
		{
			static_assert(To == SchedulerState::Running);
			lib_state.store(To, std::memory_order_release);
			return true;
		}
		else if constexpr (From == SchedulerState::Running)
		{
			static_assert(
				To == SchedulerState::Pausing ||
				To == SchedulerState::ShuttingDown);
			return lib_state.compare_exchange_strong(prior, To, std::memory_order_acquire, std::memory_order_relaxed);
		}
		else if constexpr (From == SchedulerState::Pausing)
		{
			static_assert(To == SchedulerState::Paused);
			lib_state.store(To, std::memory_order_release);
			return true;
		}
		else if constexpr (From == SchedulerState::Paused)
		{
			static_assert(To == SchedulerState::Resuming);
			return lib_state.compare_exchange_strong(prior, To, std::memory_order_acquire, std::memory_order_relaxed);
		}
		else if constexpr (From == SchedulerState::Resuming)
		{
			static_assert(To == SchedulerState::Running);
			lib_state.store(To, std::memory_order_release);
			return true;
		}
		else if constexpr (From == SchedulerState::ShuttingDown)
		{
			static_assert(To == SchedulerState::Finalizing);
			return lib_state.compare_exchange_strong(prior, To, std::memory_order_acquire, std::memory_order_relaxed);
		}
		else if constexpr (From == SchedulerState::Finalizing)
		{
			static_assert(To == SchedulerState::Uninitialized);
			lib_state.store(To, std::memory_order_release);
			return true;
		}
	}

	constexpr uint8_t ConstexprLog2(uint32_t k)
	{
		constexpr uint8_t lookup[] = { 0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30, 8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31 };
		k |= k >> 1;
		k |= k >> 2;
		k |= k >> 4;
		k |= k >> 8;
		k |= k >> 16;
		return lookup[(uint32_t)(k * 0x07C4ACDD) >> 27];
	}

#ifndef COMET_FIXED_PROCESSOR_COUNT
	static ThreadContext* thread_contexts;
#else
	static ThreadContext thread_contexts[COMET_FIXED_PROCESSOR_COUNT];
#endif
	static TaskContext* task_contexts;
	static uint64_t reseed_threshold;
	static uint32_t thread_stack_size;
	static uint32_t task_stack_size;
#ifndef COMET_FIXED_PROCESSOR_COUNT
	static uint32_t max_threads;
#else
	static constexpr uint32_t max_threads = COMET_FIXED_PROCESSOR_COUNT;
#endif
	static uint32_t max_tasks;
	static uint32_t queue_capacity;
	static uint8_t max_threads_log2;
	static uint8_t queue_capacity_log2;
	COMET_SHARED_ATTR static atomic<uint32_t> task_pool_bump;
	COMET_SHARED_ATTR static atomic<IndexPair> task_pool_flist;

	template <typename T>
	COMET_FLATTEN static T& NonAtomicRef(atomic<T>& source)
	{
		static_assert(source.is_always_lock_free);
		return *(T*)&source;
	}

	namespace Time
	{
		static uint64_t Get()
		{
			LARGE_INTEGER k;
			QueryPerformanceCounter(&k);
			return k.QuadPart;
		}

		static uint64_t GetReseedThreshold(uint64_t ns)
		{
			LARGE_INTEGER k;
			QueryPerformanceFrequency(&k);
			ns *= k.QuadPart;
			ns /= 1000000000;
			return ns;
		}
	}

	namespace RNG
	{
		static constexpr uint8_t POOL_COUNT = 32;
		static constexpr uint8_t POOL_MASK = POOL_COUNT - 1;

		struct COMET_SHARED_ATTR Pool { atomic<uint64_t> hash; };
		static Pool pools[POOL_COUNT];

		COMET_FLATTEN static uint64_t IntHash64(uint64_t n)
		{
			n ^= n >> 32;
			n *= 0xd6e8feb86659fd93U;
			n ^= n >> 32;
			n *= 0xd6e8feb86659fd93U;
			n ^= n >> 32;
			return n;
		}

		COMET_FLATTEN static uint64_t Romu2Jr(uint64_t* state)
		{
			auto r = state[0];
			state[0] = state[1] * UINT64_C(15241094284759029579);
			state[1] = COMET_ROL64(state[1] - r, 27);
			return r;
		}

		template <typename T>
		COMET_FLATTEN static uint64_t Mix(T value)
		{
			return (size_t)value;
		}

		template <typename T, typename... U>
		COMET_FLATTEN static uint64_t Mix(T value, U... remaining)
		{
			return (size_t)value + Mix(remaining...);
		}

		template <typename... T>
		COMET_FLATTEN static void AddEntropy(T... data)
		{
			pools[(this_thread - thread_contexts) & POOL_MASK].hash.fetch_xor(IntHash64(Mix(data...)), std::memory_order_relaxed);
		}

		COMET_NOINLINE static void ReseedThread(ThreadContext& here)
		{
			++here.reseed_counter;
			auto x = here.romu2jr[0] ^ here.romu2jr[1];
#ifndef COMET_NO_RDRAND
			uint64_t y = 0;
			(void)_rdrand64_step(&y);
			x ^= y;
#endif
			for (uint8_t i = 0; i != (COMET_CTZ32(here.reseed_counter) & POOL_MASK); ++i)
				x += NonAtomicRef(pools[i].hash);
			if (x == 0)
			{
				x = Time::Get();
				here.next_reseed = Time::Get() + reseed_threshold;
			}
			here.romu2jr[0] = x;
			here.romu2jr[1] = IntHash64(x);
		}

		COMET_INLINE static uint32_t Get()
		{
			uint64_t x;
			if (this_thread == nullptr)
			{
				x = IntHash64(Time::Get());
			}
			else
			{
				auto& here = *this_thread;
				if (Time::Get() >= here.next_reseed)
					ReseedThread(here);
				x = Romu2Jr(here.romu2jr);
			}
			return (uint32_t)x;
		}

		COMET_INLINE static uint32_t GetRandomThreadIndex()
		{
			auto n = Get();
			n *= max_threads;
			n >>= (32 - max_threads_log2);
			COMET_INVARIANT(n < max_threads);
			return (uint32_t)n;
		}
	}

	COMET_FLATTEN static uint32_t AcquireTask()
	{
		for (;; COMET_SPIN)
		{
			auto prior = task_pool_flist.load(std::memory_order_acquire);
			if (prior.first == UINT32_MAX)
				break;
			if (task_pool_flist.compare_exchange_weak(prior, { task_contexts[prior.first].next, prior.second + 1 }, std::memory_order_acquire, std::memory_order_relaxed))
				return prior.first;
		}
		auto n = task_pool_bump.fetch_add(1, std::memory_order_acquire);
		if (n >= max_tasks)
			return UINT32_MAX;
		return n;
	}

	COMET_FLATTEN static void ReleaseTask(uint32_t index)
	{
		auto& task = task_contexts[index];
		for (;; COMET_SPIN)
		{
			auto prior = task_pool_flist.load(std::memory_order_acquire);
			task.next = prior.first;
			if (task_pool_flist.compare_exchange_weak(prior, { index, prior.second + 1 }, std::memory_order_release, std::memory_order_relaxed))
				break;
		}
	}

	COMET_FLATTEN static uint32_t AdjustQueueIndex(uint32_t index)
	{
		return index & (queue_capacity - 1);
	}

	COMET_FLATTEN static uint32_t PopTask(ThreadContext& thread)
	{
		uint32_t m = 16;
		while (true)
		{
#ifdef COMET_NO_BUSY_WAIT
			thread.last_counter = thread.counter.load(std::memory_order_acquire);
#endif
			uint8_t n = 0;
			do
			{
				for (auto& q : thread.queues)
				{
					if (thread.quit_flag)
						return UINT32_MAX;
					if (q.size.load(std::memory_order_acquire) == 0)
						continue;
					auto& e = q.values[q.tail];
					if (NonAtomicRef(e) == UINT32_MAX)
						continue;
					auto r = e.exchange(UINT32_MAX, std::memory_order_acquire);
					if (r == UINT32_MAX)
						continue;
					++q.tail;
					q.tail = AdjustQueueIndex(q.tail);
					(void)q.size.fetch_sub(1, std::memory_order_release);
					return r;
				}
				++n;
			} while (n < m);
			m /= 2;
#ifdef COMET_NO_BUSY_WAIT
			OS::FutexAwait(thread.counter, thread.last_counter);
#endif
		}
	}

	COMET_FLATTEN static bool PushTask(ThreadContext& thread, TaskContext& task, uint32_t index)
	{
		auto& q = thread.queues[task.priority];
		auto n = NonAtomicRef(q.size);
		if (n >= queue_capacity)
			return false;
		n = q.size.fetch_add(1, std::memory_order_acquire);
		if (n >= queue_capacity)
		{
			(void)q.size.fetch_sub(1, std::memory_order_release);
			return false;
		}
		n = q.head.fetch_add(1, std::memory_order_acquire);
		n = AdjustQueueIndex(n);
		q.values[n].store(index, std::memory_order_release);
#ifdef COMET_NO_BUSY_WAIT
		(void)thread.counter.fetch_add(1, std::memory_order_relaxed);
		OS::FutexSignal(thread.counter);
#endif
		return true;
	}

	static ThreadReturn COMET_THREAD_CALL ThreadMain(ThreadParam param)
	{
		auto& here = thread_contexts[(size_t)param];
#ifdef _WIN32
		here.root_task_handle = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
#endif
		this_thread = &here;
		RNG::AddEntropy(OS::GetThreadID(), Time::Get(), &here);
		while (true)
		{
			auto index = PopTask(here);
			if (here.quit_flag)
				OS::ExitThread();
			COMET_INVARIANT(index < max_tasks);
			here.this_task = task_contexts + index;
			Task::Switch(here.root_task_handle, here.this_task->handle);
			++here.yield_counter;
			if (here.this_task->fn != nullptr)
			{
				COMET_INVARIANT(here.this_task->sleeping != 2);
				if (here.this_task->sleeping == 1)
				{
					here.this_task->sleeping = 2;
					std::atomic_thread_fence(std::memory_order_release);
					continue;
				}
				auto target = this_thread;
				while (!PushTask(*target, *here.this_task, index))
					target = thread_contexts + RNG::GetRandomThreadIndex();
			}
			else
			{
				RNG::AddEntropy(here.local_accumulator, here.this_task, here.yield_counter);
				if (here.this_task->counter != nullptr)
					here.this_task->counter->Decrement();
				ReleaseTask(index);
			}
		}
		COMET_UNREACHABLE;
	}

	static void COMET_TASK_CALL TaskMain(void* param)
	{
		while (true)
		{
			if (this_thread->quit_flag)
				OS::ExitThread();
			auto& task = *(TaskContext*)param;
			COMET_INVARIANT(task.fn != nullptr);
			task.fn(task.param);
			task.fn = nullptr;
			if (this_thread->quit_flag)
				OS::ExitThread();
			Task::Switch(task.handle, this_thread->root_task_handle);
		}
	}

	COMET_FLATTEN static void FinalizeInner()
	{
		for (uint32_t i = 0; i != NonAtomicRef(task_pool_bump); ++i)
			Task::Delete(task_contexts[i].handle);
		size_t buffer_size =
			sizeof(ThreadContext) * max_threads +
			sizeof(TaskContext) * max_tasks;
		OS::Free(thread_contexts, buffer_size);
	}

	InitOptions InitOptions::Default()
	{
		InitOptions r = {};
		r.thread_stack_size = 1 << 16;
		r.task_stack_size = r.thread_stack_size;
#if defined(_WIN32) && !defined(COMET_FIXED_PROCESSOR_COUNT)
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		r.max_threads = info.dwNumberOfProcessors;
		r.max_tasks = r.max_threads * 256;
#else
		r.max_tasks = COMET_FIXED_PROCESSOR_COUNT * 256;
#endif
		r.reseed_threshold_ns = UINT32_MAX; // ~4s
		return r;
	}

	bool Init()
	{
		return Init(InitOptions::Default());
	}

	bool Init(InitOptions options)
	{
		if (!SwitchState<SchedulerState::Uninitialized, SchedulerState::Initializing>())
			return false;
		COMET_INVARIANT(options.max_tasks != 0);
#ifndef COMET_FIXED_PROCESSOR_COUNT
		COMET_INVARIANT(options.max_threads != 0);
#endif
		COMET_INVARIANT(options.reseed_threshold_ns != 0);
		COMET_INVARIANT(options.task_stack_size != 0);
		COMET_INVARIANT(options.thread_stack_size != 0);
#ifndef COMET_FIXED_PROCESSOR_COUNT
		max_threads = options.max_threads;
#endif
		max_tasks = options.max_tasks;
		queue_capacity = 1U << (31 - COMET_CLZ32((max_tasks / max_threads) - 1));
		queue_capacity_log2 = 31 - COMET_CLZ32(queue_capacity);
		max_threads_log2 = 31 - COMET_CLZ32(max_threads);
#ifndef COMET_FIXED_PROCESSOR_COUNT
		size_t buffer_size =
			sizeof(ThreadContext) * max_threads +
			sizeof(atomic<uint32_t>) * queue_capacity * max_threads * MAX_PRIORITY +
			sizeof(TaskContext) * max_tasks;
		thread_contexts = (ThreadContext*)OS::Malloc(buffer_size);
		task_contexts = (TaskContext*)(thread_contexts + max_threads);
#else
		size_t buffer_size =
			sizeof(atomic<uint32_t>) * queue_capacity * max_threads * MAX_PRIORITY +
			sizeof(TaskContext) * max_tasks;
		task_contexts = (TaskContext*)OS::Malloc(buffer_size);
#endif
		task_stack_size = options.task_stack_size;
		uint8_t* bump = (uint8_t*)(task_contexts + max_tasks);
		auto qn = sizeof(atomic<uint32_t>) * queue_capacity;
		for (uint32_t i = 0; i != max_threads; ++i)
		{
			auto& e = thread_contexts[i];
			e.handle = OS::NewThread(ThreadMain, (ThreadParam)(size_t)i, thread_stack_size, i);
			for (auto& q : e.queues)
			{
				q.values = (atomic<uint32_t>*)bump;
				(void)memset(bump, 0xff, qn);
				bump += qn;
			}
		}
		NonAtomicRef(task_pool_flist) = { UINT32_MAX, 0 };
		NonAtomicRef(task_pool_bump) = 0;
		reseed_threshold = Time::GetReseedThreshold(options.reseed_threshold_ns);
		return SwitchState<SchedulerState::Initializing, SchedulerState::Running>();
	}

	bool Pause()
	{
		if (!SwitchState<SchedulerState::Running, SchedulerState::Pausing>())
			return false;
		for (uint32_t i = 0; i != max_threads; ++i)
			if (!OS::PauseThread(thread_contexts[i].handle))
				return false;
		return SwitchState<SchedulerState::Pausing, SchedulerState::Paused>();
	}

	bool Resume()
	{
		if (!SwitchState<SchedulerState::Paused, SchedulerState::Resuming>())
			return false;
		for (uint32_t i = 0; i != max_threads; ++i)
			if (!OS::ResumeThread(thread_contexts[i].handle))
				return false;
		return SwitchState<SchedulerState::Resuming, SchedulerState::Running>();
	}

	bool Shutdown()
	{
		if (!SwitchState<SchedulerState::Running, SchedulerState::ShuttingDown>())
			return false;
		for (uint32_t i = 0; i != max_threads; ++i)
		{
			auto& thread = thread_contexts[i];
			thread.quit_flag = true;
#ifdef COMET_NO_BUSY_WAIT
			(void)thread.counter.fetch_add(1, std::memory_order_relaxed);
			OS::FutexSignal(thread.counter);
#endif
		}
		return true;
	}

	bool TryFinalize()
	{
		if (!SwitchState<SchedulerState::ShuttingDown, SchedulerState::Finalizing>())
			return false;
		return Finalize();
	}

	bool Finalize()
	{
		for (uint32_t i = 0; i != max_threads; ++i)
		{
			auto& thread = thread_contexts[i];
#ifdef COMET_NO_BUSY_WAIT
			(void)thread.counter.fetch_add(1, std::memory_order_relaxed);
			OS::FutexSignal(thread.counter);
#endif
			if (!OS::AwaitThread(thread.handle))
				return false;
		}
		FinalizeInner();
		return SwitchState<SchedulerState::Finalizing, SchedulerState::Uninitialized>();
	}

	void Terminate()
	{
		lib_state.store(SchedulerState::Uninitialized, std::memory_order_release);
		for (uint32_t i = 0; i != max_threads; ++i)
			OS::KillThread(thread_contexts[i]);
	}

	SchedulerState GetSchedulerState()
	{
		return lib_state.load(std::memory_order_acquire);
	}

	bool IsTask()
	{
		return this_thread != nullptr;
	}

	DispatchResult Dispatch(void(*fn)(void* param), void* param)
	{
		return Dispatch(fn, param, {});
	}

	DispatchResult Dispatch(void(*fn)(void* param), void* param, TaskOptions options)
	{
		auto index = AcquireTask();
		if (index == UINT32_MAX)
		{
			switch (options.error_policy)
			{
			case DispatchErrorPolicy::RunSequentially:
				fn(param);
				if (options.counter != nullptr)
					options.counter->Decrement();
				return DispatchResult::Sequential;
			case DispatchErrorPolicy::Spin:
				for (;; COMET_SPIN)
					if (index = AcquireTask(); index == UINT32_MAX)
						break;
				break;
			case DispatchErrorPolicy::Return:
				return DispatchResult::Failure;
			default:
				COMET_UNREACHABLE;
			}
		}
		auto& task = task_contexts[index];
		if (task.handle == nullptr)
			task.handle = Task::New(TaskMain, &task, task_stack_size);
		task.fn = fn;
		task.param = param;
		task.priority = options.priority;
		task.counter = options.counter;
		task.next = UINT32_MAX;
		if (options.preferred_thread != nullptr)
		{
			if (!PushTask(thread_contexts[*options.preferred_thread], task, index))
			{
				switch (options.error_policy)
				{
				case DispatchErrorPolicy::RunSequentially:
					fn(param);
					if (options.counter != nullptr)
						options.counter->Decrement();
					return DispatchResult::Sequential;
				case DispatchErrorPolicy::Spin:
					while (!PushTask(thread_contexts[*options.preferred_thread], task, index))
						COMET_SPIN;
					break;
				case DispatchErrorPolicy::Return:
					return DispatchResult::Failure;
				default:
					COMET_UNREACHABLE;
				}
			}
		}
		else
		{
			auto target = this_thread;
			if (target == nullptr)
				target = thread_contexts + RNG::GetRandomThreadIndex();
			if (!PushTask(*target, task, index))
			{
				switch (options.error_policy)
				{
				case DispatchErrorPolicy::RunSequentially:
					fn(param);
					if (options.counter != nullptr)
						options.counter->Decrement();
					return DispatchResult::Sequential;
				case DispatchErrorPolicy::Spin:
					while (!PushTask(thread_contexts[RNG::GetRandomThreadIndex()], task, index))
						COMET_SPIN;
					break;
				case DispatchErrorPolicy::Return:
					return DispatchResult::Failure;
				default:
					COMET_UNREACHABLE;
				}
			}
		}
		return DispatchResult::Success;
	}

	void Yield()
	{
#ifdef COMET_DEBUG
		COMET_ASSERT(!this_thread->yield_trap);
#endif
		auto mask = 
#ifdef _MSVC_LANG
			(uint32_t)((size_t)_ReturnAddress()) >> 4;
#else
			(uint32_t)((size_t)__builtin_return_address(0)) >> 4;
#endif
		this_thread->local_accumulator += mask;
		Task::Switch(this_thread->this_task->handle, this_thread->root_task_handle);
	}

	void Exit()
	{
		this_thread->this_task->fn = nullptr;
		Yield();
	}

	uint64_t ThisTaskID()
	{
		auto task = this_thread->this_task;
		return ((uint64_t)task->generation << 32) | (task - task_contexts);
	}

	uint32_t WorkerThreadIndex()
	{
		return (uint32_t)(this_thread - thread_contexts);
	}

	uint32_t MaxTasks()
	{
		return max_tasks;
	}

#ifndef COMET_FIXED_PROCESSOR_COUNT
	uint32_t WorkerThreadCount()
	{
		return max_threads;
	}
#endif

	void Fence::Signal()
	{
		auto& self = *(atomic<uint32_t>*)this;
		auto index = self.load(std::memory_order_acquire);
		if (index == UINT32_MAX)
			return;
		auto& task = task_contexts[index];
		while (task.sleeping != 2)
			COMET_SPIN;
		task.sleeping = 0;
		auto target = this_thread;
		while (!PushTask(*target, task, index))
			target = thread_contexts + RNG::GetRandomThreadIndex();
	}

	void Fence::Await()
	{
		COMET_ASSERT(IsTask());
		auto& self = *(atomic<uint32_t>*)this;
		auto index = (uint32_t)(this_thread->this_task - task_contexts);
		NonAtomicRef(self) = index;
		this_thread->this_task->sleeping = 1;
		Yield();
	}

	bool Counter::Decrement()
	{
		auto& self = *(CounterState*)this;
		if (self.counter.fetch_sub(1, std::memory_order_acquire) != 1)
			return false;
		return WakeAll();
	}

	bool Counter::WakeAll()
	{
		auto& self = *(CounterState*)this;
		auto prior = self.queue.exchange({ 0, UINT32_MAX }, std::memory_order_acquire);
		if (prior.first == UINT32_MAX)
			return true;
		if (prior.second == UINT32_MAX)
			return false;
		auto i = prior.first;
		while (true)
		{
			auto& task = task_contexts[i];
			std::atomic_thread_fence(std::memory_order_acquire);
			while (task.sleeping != 2)
				COMET_SPIN;
			auto next = task.next;
			task.next = UINT32_MAX;
			task.sleeping = 0;
			auto target = this_thread;
			while (!PushTask(*target, task, i))
				target = thread_contexts + RNG::GetRandomThreadIndex();
			if (i == prior.second)
				return true;
			i = next;
		}
	}

	bool Counter::Await()
	{
		auto& self = *(CounterState*)this;
		IndexPair prior;
		auto index = (uint32_t)(this_thread->this_task - task_contexts);
		for (;; COMET_SPIN)
		{
			prior = self.queue.load(std::memory_order_acquire);
			if (prior.first != UINT32_MAX && prior.second == UINT32_MAX)
				return false;
			if (self.queue.compare_exchange_weak(prior, { prior.first == UINT32_MAX ? index : prior.first, index }, std::memory_order_acquire, std::memory_order_relaxed))
				break;
		}
		if (prior.second != UINT32_MAX)
		{
			COMET_INVARIANT(task_contexts[prior.first].next == UINT32_MAX);
			task_contexts[prior.second].next = index;
		}
		this_thread->this_task->sleeping = 1;
		Yield();
		return true;
	}

	void Counter::Reset(uint64_t new_value)
	{
		WakeAll();
		state[0] = new_value;
		state[1] = UINT64_MAX;
	}

	uint64_t Counter::Value() const
	{
		auto& self = *(CounterState*)this;
		return self.counter.load(std::memory_order_relaxed);
	}

	void SpinLock::Lock()
	{
		auto& self = *(atomic<uint32_t>*)this;
		Debug::SetYieldTrap(true);
		this_thread->spinlock_next = UINT32_MAX;
		auto here_index = (uint32_t)(this_thread - thread_contexts);
		auto prior = self.exchange(here_index, std::memory_order_acquire);
		if (prior == UINT32_MAX)
			return;
		this_thread->spinlock_flag = true;
		thread_contexts[prior].spinlock_next = here_index;
		while (this_thread->spinlock_flag)
			COMET_SPIN;
	}

	bool SpinLock::IsLocked() const
	{
		return state != UINT32_MAX;
	}

	bool SpinLock::TryLock()
	{
		auto& self = *(atomic<uint32_t>*)this;
		auto expected = UINT32_MAX;
		return self.compare_exchange_weak(expected, (uint32_t)(this_thread - thread_contexts), std::memory_order_acquire, std::memory_order_relaxed);
	}

	void SpinLock::Unlock()
	{
		auto& self = *(atomic<uint32_t>*)this;
		Debug::SetYieldTrap(false);
		auto here_index = (uint32_t)(this_thread - thread_contexts);
		auto prior = here_index;
		if (self.compare_exchange_strong(prior, UINT32_MAX, std::memory_order_release, std::memory_order_relaxed))
			return;
		while (this_thread->spinlock_next == UINT32_MAX)
			std::atomic_thread_fence(std::memory_order_acquire);
		thread_contexts[this_thread->spinlock_next].spinlock_flag = false;
	}

	void MCSMutex::Lock(Node& node)
	{
		auto& self = *(atomic<Node*>*)this;
		node.index = (uint32_t)(this_thread->this_task - task_contexts);
		auto prior = self.exchange(&node, std::memory_order_acquire);
		if (prior == nullptr)
			return;
		prior->next = &node;
		this_thread->this_task->sleeping = 1;
		Yield();
	}

	bool MCSMutex::IsLocked() const
	{
		return state != 0;
	}

	bool MCSMutex::TryLock(Node& node)
	{
		auto& self = *(atomic<Node*>*)this;
		node.index = (uint32_t)(this_thread->this_task - task_contexts);
		Node* expected = nullptr;
		return self.compare_exchange_weak(expected, &node, std::memory_order_acquire, std::memory_order_relaxed);
	}

	void MCSMutex::Unlock(Node& node)
	{
		auto& self = *(atomic<Node*>*)this;
		Node* expected = &node;
		if (self.compare_exchange_strong(expected, nullptr, std::memory_order_release, std::memory_order_relaxed))
			return;
		auto& task = task_contexts[node.next->index];
		std::atomic_thread_fence(std::memory_order_acquire);
		while (task.sleeping != 2)
			COMET_SPIN;
		task.sleeping = 0;
		auto target = this_thread;
		while (!PushTask(*target, task, node.next->index))
			target = thread_contexts + RNG::GetRandomThreadIndex();
	}

	namespace Debug
	{
#ifdef COMET_DEBUG
		void SetYieldTrap(bool value)
		{
			this_thread->yield_trap = value;
		}
#endif

		void SetInfoCallback(MessageFn callback, void* context)
		{
			info_fn = callback;
			info_ctx = context;
		}

		void SetWarningCallback(MessageFn callback, void* context)
		{
			warning_fn = callback;
			warning_ctx = context;
		}

		void SetErrorCallback(MessageFn callback, void* context)
		{
			error_fn = callback;
			error_ctx = context;
		}
	}

	namespace RCU
	{
		size_t GetSchedulerSnapshotSize()
		{
			return GetSchedulerSnapshotSize(max_threads);
		}

		void GetSchedulerSnapshot(void* out)
		{
			auto ptr = (uint32_t*)out;
			auto here_index = (uint32_t)(this_thread - thread_contexts);
			*ptr = here_index;
			++ptr;
			for (uint32_t i = 0; i != max_threads; ++i)
			{
				if (i == here_index)
					continue;
				std::atomic_thread_fence(std::memory_order_acquire);
				*ptr = thread_contexts[i].yield_counter;
				++ptr;
			}
		}

		uint32_t TrySync(void* snapshot, uint32_t prior_result)
		{
			auto ptr = (uint32_t*)snapshot;
			auto ignored = *ptr;
			++ptr;
			auto i = prior_result;
			for (; i != max_threads; ++i)
			{
				if (i == ignored)
					continue;
				std::atomic_thread_fence(std::memory_order_acquire);
				auto current = thread_contexts[i].yield_counter;
				auto prior = *ptr;
				if (current == prior)
					break;
			}
			return i;
		}
	}
}
#endif