/*
	Copyright 2021 Marcel Pi Nacy

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



// ================================================================
// CMTS Macros:
// ================================================================

#ifndef CMTS_INCLUDED
#define CMTS_INCLUDED

#ifdef _WIN32
#define CMTS_WINDOWS
#define CMTS_REQUIRES_TASK_ALLOCATOR 0
#else
#error "CMTS: UNSUPPORTED OPERATING SYSTEM."
#endif

#include <stdint.h>
#include <stddef.h>

#ifndef CMTS_CALL
#define CMTS_CALL
#endif

#ifndef CMTS_ATTR
#define CMTS_ATTR
#endif

#ifndef CMTS_PTR
#define CMTS_PTR
#endif

#ifndef CMTS_NODISCARD
#define CMTS_NODISCARD
#ifdef __cplusplus
#if __has_cpp_attribute(nodiscard)
#undef CMTS_NODISCARD
#define CMTS_NODISCARD [[nodiscard]]
#endif
#endif
#endif

#ifndef CMTS_NORETURN
#define CMTS_NORETURN
#ifdef __cplusplus
#if __has_cpp_attribute(noreturn)
#undef CMTS_NORETURN
#define CMTS_NORETURN [[noreturn]]
#endif
#endif
#endif

#define CMTS_MAX_TASKS UINT32_MAX

#ifndef CMTS_MAX_PRIORITY
#define CMTS_MAX_PRIORITY 3
#endif

#if CMTS_MAX_PRIORITY >= 256
#error "Error, CMTS_MAX_PRIORITY must not exceed 256"
#endif

#ifndef CMTS_DEFAULT_TASKS_PER_PROCESSOR
#define CMTS_DEFAULT_TASKS_PER_PROCESSOR 256
#endif

#ifndef CMTS_SPIN_THRESHOLD
#define CMTS_SPIN_THRESHOLD 8
#endif

#define CMTS_FENCE_DATA_SIZE 4
#define CMTS_EVENT_DATA_SIZE 8
#define CMTS_COUNTER_DATA_SIZE 16
#define CMTS_MUTEX_DATA_SIZE 8

#define CMTS_FENCE_INIT { UINT32_MAX }
#define CMTS_EVENT_INIT { UINT64_MAX }
#define CMTS_COUNTER_INIT(VALUE) { UINT64_MAX, (VALUE), UINT32_MAX }
#define CMTS_MUTEX_INIT { UINT64_MAX }

#ifndef CMTS_CHAR
#define CMTS_CHAR char
#endif

#ifndef CMTS_TEXT
#define CMTS_TEXT(string_literal) string_literal
#endif

#ifndef CMTS_EXTERN_C_BEGIN
#define CMTS_EXTERN_C_BEGIN extern "C" {
#endif

#ifndef CMTS_EXTERN_C_END
#define CMTS_EXTERN_C_END }
#endif

// ================================================================
// CMTS Types:
// ================================================================

CMTS_EXTERN_C_BEGIN

#ifdef __cplusplus
typedef bool cmts_bool_t;
#else
typedef _Bool cmts_boolean_t;
#endif

typedef uint64_t cmts_task_id_t;

typedef void(CMTS_PTR* cmts_task_function_pointer_t)(void* parameter);
typedef void* (CMTS_PTR* cmts_allocate_function_pointer_t)(size_t size);
typedef cmts_bool_t(CMTS_PTR* cmts_deallocate_function_pointer_t)(void* memory, size_t size);
typedef void(CMTS_PTR* cmts_destructor_function_pointer_t)(void* object);

typedef enum cmts_result_t
{
	CMTS_OK = 0,
	CMTS_SYNC_OBJECT_EXPIRED = 1,
	CMTS_NOT_READY = 2,

	CMTS_ERROR_MEMORY_ALLOCATION = -1,
	CMTS_ERROR_MEMORY_DEALLOCATION = -2,
	CMTS_ERROR_WORKER_THREAD_CREATION = -3,
	CMTS_ERROR_THREAD_AFFINITY_FAILURE = -4,
	CMTS_ERROR_RESUME_WORKER_THREAD = -5,
	CMTS_ERROR_SUSPEND_WORKER_THREAD = -6,
	CMTS_ERROR_WORKER_THREAD_TERMINATION = -7,
	CMTS_ERROR_AWAIT_WORKER_THREAD = -8,
	CMTS_ERROR_TASK_POOL_CAPACITY = -9,
	CMTS_ERROR_AFFINITY = -10,
	CMTS_ERROR_TASK_ALLOCATION = -11,
	CMTS_ERROR_FUTEX = -12,
	CMTS_ERROR_LIBRARY_UNINITIALIZED = -13,
	CMTS_ERROR_OS_INIT = -14,

	CMTS_RESULT_MIN_ENUM = CMTS_ERROR_LIBRARY_UNINITIALIZED,
	CMTS_RESULT_MAX_ENUM = CMTS_NOT_READY,
} cmts_result_t;

typedef enum cmts_sync_type_t
{
	CMTS_SYNC_TYPE_NONE,
	CMTS_SYNC_TYPE_EVENT,
	CMTS_SYNC_TYPE_COUNTER,

	CMTS_SYNC_TYPE_MIN_ENUM = CMTS_SYNC_TYPE_NONE,
	CMTS_SYNC_TYPE_MAX_ENUM = CMTS_SYNC_TYPE_COUNTER,
} cmts_sync_type_t;

typedef enum cmts_dispatch_flag_bits_t
{
	CMTS_DISPATCH_FLAGS_FORCE = 1,
} cmts_dispatch_flag_bits_t;
typedef uint64_t cmts_dispatch_flags_t;

typedef enum cmts_init_flag_bits_t
{
	CMTS_INIT_FLAGS_USE_AFFINITY = 1,
} cmts_init_flag_bits_t;
typedef uint64_t cmts_init_flags_t;

typedef enum cmts_ext_type_t
{
	CMTS_EXT_TYPE_DEBUGGER,
	CMTS_EXT_TYPE_TASK_NAME,

	CMTS_EXT_TYPE_MIN_ENUM = CMTS_EXT_TYPE_DEBUGGER,
	CMTS_EXT_TYPE_MAX_ENUM = CMTS_EXT_TYPE_TASK_NAME,
} cmts_ext_type_t;

typedef struct cmts_fence_t { uint32_t x; } cmts_fence_t;
typedef struct cmts_event_t { uint64_t x; } cmts_event_t;
typedef struct cmts_counter_t { uint64_t x; uint32_t y, z; } cmts_counter_t;
typedef struct cmts_mutex_t { uint64_t x; } cmts_mutex_t;
typedef struct cmts_hptr_t { void* impl; } cmts_hptr_t;

typedef struct cmts_init_options_t
{
	cmts_allocate_function_pointer_t allocate_function;
	size_t task_stack_size;
	uint32_t thread_count;
	cmts_init_flags_t flags;
	uint32_t max_tasks;
	uint32_t enabled_extension_count;
	const cmts_ext_type_t* enabled_extensions;
	const void* ext;
} cmts_init_options_t;

typedef struct cmts_dispatch_options_t
{
	cmts_dispatch_flags_t flags;
	cmts_task_id_t* out_task_id;
	void* parameter;
	void* sync_object;
	cmts_sync_type_t sync_type;
	uint8_t priority;
	const void* ext;
} cmts_dispatch_options_t;

typedef struct cmts_memory_requirements_t
{
	size_t size;
	size_t alignment;
} cmts_memory_requirements_t;

typedef enum cmts_ext_debugger_message_severity_t
{
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_INFO,
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_WARNING,
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_ERROR,
} cmts_ext_debugger_message_severity_t;

typedef struct cmts_ext_debugger_message_t
{
	const CMTS_CHAR* message;
	size_t message_length;
	cmts_ext_debugger_message_severity_t severity;
	const void* ext;
} cmts_ext_debugger_message_t;

typedef void(CMTS_CALL* cmts_ext_debugger_message_callback_t)(void* context, const cmts_ext_debugger_message_t* message);

typedef struct cmts_ext_debugger_init_options_t
{
	const void* next;
	cmts_ext_type_t ext_type; // Must be CMTS_EXT_TYPE_DEBUGGER.
	void* context;
	cmts_ext_debugger_message_callback_t message_callback;
} cmts_ext_debugger_init_options_t;

typedef struct cmts_ext_task_name_init_options_t
{
	const void* next;
	cmts_ext_type_t ext_type; // Must be CMTS_EXT_TYPE_TASK_NAME.
} cmts_ext_task_name_init_options_t;

// ================================================================
// CMTS Functions:
// ================================================================

CMTS_ATTR cmts_result_t	CMTS_CALL cmts_init(const cmts_init_options_t* options);
CMTS_ATTR cmts_result_t	CMTS_CALL cmts_pause();
CMTS_ATTR cmts_result_t CMTS_CALL cmts_resume();
CMTS_ATTR void CMTS_CALL cmts_finalize_signal();
CMTS_ATTR cmts_result_t CMTS_CALL cmts_finalize_await(cmts_deallocate_function_pointer_t deallocate);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_terminate(cmts_deallocate_function_pointer_t deallocate);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_initialized();
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_online();
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_paused();
CMTS_ATTR uint32_t CMTS_CALL cmts_purge(uint32_t max_trimmed_tasks);
CMTS_ATTR uint32_t CMTS_CALL cmts_purge_all();
CMTS_ATTR uint32_t CMTS_CALL cmts_worker_thread_count();
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_requires_task_allocator();
CMTS_ATTR void CMTS_CALL cmts_assign_task_allocator(cmts_allocate_function_pointer_t allocate, cmts_deallocate_function_pointer_t deallocate);

CMTS_ATTR cmts_result_t CMTS_CALL cmts_dispatch(cmts_task_function_pointer_t entry_point, cmts_dispatch_options_t* options);
CMTS_ATTR void CMTS_CALL cmts_yield();
CMTS_NORETURN CMTS_ATTR void CMTS_CALL cmts_exit();
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_task();
CMTS_ATTR cmts_task_id_t CMTS_CALL cmts_this_task_id();

CMTS_NODISCARD CMTS_ATTR cmts_task_id_t CMTS_CALL cmts_task_allocate();
CMTS_ATTR uint8_t CMTS_CALL cmts_task_get_priority(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_set_priority(cmts_task_id_t task_id, uint8_t priority);
CMTS_ATTR void CMTS_CALL cmts_task_set_parameter(cmts_task_id_t task_id, void* parameter);
CMTS_ATTR void* CMTS_CALL cmts_task_get_parameter(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_set_function(cmts_task_id_t task_id, cmts_task_function_pointer_t function);
CMTS_ATTR cmts_task_function_pointer_t CMTS_CALL cmts_task_get_function(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_attach_event(cmts_task_id_t task_id, cmts_event_t* event);
CMTS_ATTR void CMTS_CALL cmts_task_attach_counter(cmts_task_id_t task_id, cmts_counter_t* counter);
CMTS_ATTR void CMTS_CALL cmts_task_sleep(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_wake(cmts_task_id_t task_id);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_valid(cmts_task_id_t task_id);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_sleeping(cmts_task_id_t task_id);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_running(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_dispatch(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_deallocate(cmts_task_id_t task_id);

CMTS_ATTR void CMTS_CALL cmts_fence_init(cmts_fence_t* fence);
CMTS_ATTR void CMTS_CALL cmts_fence_signal(cmts_fence_t* fence);
CMTS_ATTR void CMTS_CALL cmts_fence_await(cmts_fence_t* fence);

CMTS_ATTR void CMTS_CALL cmts_event_init(cmts_event_t* event);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_state(const cmts_event_t* event);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_signal(cmts_event_t* event);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_await(cmts_event_t* event);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_reset(cmts_event_t* event);

CMTS_ATTR void CMTS_CALL cmts_counter_init(cmts_counter_t* counter, uint32_t start_value);
CMTS_ATTR uint32_t CMTS_CALL cmts_counter_value(const cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_state(const cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_increment(cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_decrement(cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_await(cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_reset(cmts_counter_t* counter, uint32_t new_start_value);

CMTS_ATTR void CMTS_CALL cmts_mutex_init(cmts_mutex_t* mutex);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_mutex_is_locked(const cmts_mutex_t* mutex);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_mutex_try_lock(cmts_mutex_t* mutex);
CMTS_ATTR void CMTS_CALL cmts_mutex_lock(cmts_mutex_t* mutex);
CMTS_ATTR void CMTS_CALL cmts_mutex_unlock(cmts_mutex_t* mutex);

CMTS_ATTR void CMTS_CALL cmts_rcu_read_begin();
CMTS_ATTR void CMTS_CALL cmts_rcu_read_end();
CMTS_ATTR void CMTS_CALL cmts_rcu_sync();
CMTS_ATTR size_t CMTS_CALL cmts_rcu_snapshot_size();
CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot(void* snapshot);
CMTS_ATTR uint32_t CMTS_CALL cmts_rcu_try_snapshot_sync(void* snapshot, uint32_t prior_result);
CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot_sync(void* snapshot);

CMTS_ATTR size_t CMTS_CALL cmts_hptr_required_size();
CMTS_ATTR void CMTS_CALL cmts_hptr_init(cmts_hptr_t* hptr, cmts_allocate_function_pointer_t allocate);
CMTS_ATTR void CMTS_CALL cmts_hptr_delete(cmts_hptr_t* hptr, cmts_deallocate_function_pointer_t deallocate);
CMTS_ATTR void CMTS_CALL cmts_hptr_protect(cmts_hptr_t* hptr, void* ptr);
CMTS_ATTR void CMTS_CALL cmts_hptr_release(cmts_hptr_t* hptr);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_hptr_is_unreachable(const cmts_hptr_t* hptr, const void* ptr);

CMTS_ATTR uint32_t CMTS_CALL cmts_this_worker_thread_index();
CMTS_ATTR uint32_t CMTS_CALL cmts_processor_count();
CMTS_ATTR uint32_t CMTS_CALL cmts_this_processor_index();
CMTS_ATTR size_t CMTS_CALL cmts_default_task_stack_size();

#ifdef CMTS_EXT_LOCAL_YIELD_COUNT
CMTS_ATTR uint32_t CMTS_CALL cmts_local_yield_count();
#endif
CMTS_ATTR void CMTS_CALL cmts_debug_enable_yield_trap();
CMTS_ATTR void CMTS_CALL cmts_debug_disable_yield_trap();

CMTS_ATTR cmts_bool_t CMTS_CALL cmts_ext_debugger_enabled();
CMTS_ATTR void CMTS_CALL cmts_ext_debugger_write(const cmts_ext_debugger_message_t* message);

CMTS_ATTR cmts_bool_t CMTS_CALL cmts_ext_task_name_enabled();
CMTS_ATTR void CMTS_CALL cmts_ext_task_name_set(cmts_task_id_t id, const CMTS_CHAR* name, size_t length);
CMTS_ATTR void CMTS_CALL cmts_ext_task_name_swap(cmts_task_id_t id, const CMTS_CHAR* name, size_t length, const CMTS_CHAR** out_old_name, size_t* out_old_length);
CMTS_ATTR void CMTS_CALL cmts_ext_task_name_get(cmts_task_id_t id, const CMTS_CHAR** out_name, size_t* out_length);
CMTS_ATTR void CMTS_CALL cmts_ext_task_name_clear(cmts_task_id_t id);

CMTS_EXTERN_C_END
#endif

// ================================================================
// CMTS Implementation:
// ================================================================

#ifdef CMTS_IMPLEMENTATION
#define CMTS_NAMESPACE_BEGIN namespace detail::cmts {
#define CMTS_NAMESPACE_END }
#include <atomic>
#include <new>

CMTS_NAMESPACE_BEGIN
using uint8 = uint8_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using ufast8 = uint_fast8_t;
using ufast32 = uint_fast32_t;
using ufast64 = uint_fast64_t;
using uintptr = size_t;
#if UINTPTR_MAX == UINT32_MAX
using ufastptr = ufast32;
#else
using ufastptr = ufast64;
#endif

#ifdef __has_cpp_attribute
#if __has_cpp_attribute(likely)
#define CMTS_LIKELY_IF(condition) if ((condition)) [[likely]]
#else
#define CMTS_LIKELY_IF(condition) if ((condition))
#endif

#if __has_cpp_attribute(unlikely)
#define CMTS_UNLIKELY_IF(condition) if ((condition)) [[unlikely]]
#else
#define CMTS_UNLIKELY_IF(condition) if ((condition))
#endif
#else
#define CMTS_LIKELY_IF(condition) if ((condition))
#define CMTS_UNLIKELY_IF(condition) if ((condition))
#endif

#if defined(_MSVC_LANG) || defined(_MSC_VER)
#define CMTS_ASSUME(expression) __assume((expression))
#define CMTS_INLINE_NEVER __declspec(noinline)
#define CMTS_INLINE_ALWAYS __forceinline
#define CMTS_ROR32(mask, count) _rotr((mask), (count))
#define CMTS_ROR64(mask, count) _rotr64((mask), (count))
#define CMTS_ROL32(mask, count) _rotl((mask), (count))
#define CMTS_ROL64(mask, count) _rotl64((mask), (count))
#if UINTPTR_MAX == UINT32_MAX
#define CMTS_POPCOUNT(value) ((ufast8)__popcnt((unsigned int)(value)))
#else
#define CMTS_POPCOUNT(value) ((ufast8)__popcnt64((unsigned long long)(value)))
#endif
#if UINTPTR_MAX == UINT32_MAX
#define CMTS_FLS(value) ((ufast8)__lzcnt((value)))
#else
#define CMTS_FLS(value) ((ufast8)__lzcnt64((value)))
#endif
#endif

#if defined(_DEBUG) && !defined(NDEBUG)
[[noreturn]] void cmts_debug_assertion_handler(const char* expression);
#define CMTS_DEBUG
#define CMTS_INVARIANT(expression) CMTS_UNLIKELY_IF(!(expression)) { CMTS_DEBUG_TRAP(); cmts_debug_assertion_handler(#expression); }
#define CMTS_ASSERT(expression) CMTS_INVARIANT((expression))
#else
#define CMTS_INVARIANT(expression) CMTS_ASSUME((expression))
#define CMTS_ASSERT(expression) ((expression))
#endif

template <typename T>
CMTS_INLINE_ALWAYS static T non_atomic_load(const std::atomic<T>& from)
{
	static_assert(std::atomic<T>::is_always_lock_free);
	return *(const T*)&from;
}

template <typename T>
CMTS_INLINE_ALWAYS static T& non_atomic_ref(std::atomic<T>& from)
{
	static_assert(std::atomic<T>::is_always_lock_free);
	return *(T*)&from;
}

template <typename T, typename U = T>
CMTS_INLINE_ALWAYS static void non_atomic_store(std::atomic<T>& where, U value)
{
	static_assert(std::atomic<T>::is_always_lock_free);
	*(T*)&where = value;
}

static constexpr ufastptr constexpr_log2(ufastptr value)
{
#if UINT32_MAX == UINTPTR_MAX
	constexpr uint8_t lookup[] = { 0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30, 8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31 };
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	return lookup[(uint32_t)(value * 0x07C4ACDD) >> 27];
#else
	constexpr uint8_t lookup[] =
	{
		63, 0, 58, 1, 59, 47, 53, 2, 60, 39, 48, 27, 54, 33, 42, 3, 61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4,
		62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21, 56, 45, 25, 31, 35, 16, 9, 12, 44, 24, 15, 8, 23, 7, 6, 5
	};
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;
	return lookup[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
#endif
}

static constexpr ufastptr FALSE_SHARING_THRESHOLD_LOG2 = constexpr_log2(std::hardware_destructive_interference_size);
static constexpr ufastptr SHARED_QUEUE_BASE_SHIFT = constexpr_log2(std::hardware_destructive_interference_size / sizeof(std::atomic<uint32>));
static_assert(FALSE_SHARING_THRESHOLD_LOG2 != 0);

#define CMTS_SHARED_ATTR alignas (std::hardware_destructive_interference_size)

#ifdef CMTS_DEBUG
namespace debugger
{
	static void* context;
	static cmts_ext_debugger_message_callback_t callback;

	CMTS_INLINE_ALWAYS static void init(const cmts_ext_debugger_init_options_t& options)
	{
		context = options.context;
		callback = options.message_callback;
	}

	template <typename String>
	static constexpr size_t constexpr_strlen(String text)
	{
		size_t k = 0;
		while (text[k] != '\0')
			++k;
		return k;
	}

	template <cmts_ext_debugger_message_severity_t S, typename String>
	CMTS_INLINE_ALWAYS static void message(String text)
	{
		if (callback == nullptr)
			return;
		cmts_ext_debugger_message_t message;
		message.ext = nullptr;
		message.message = text;
		message.message_length = constexpr_strlen(text);
		message.severity = S;
		callback(context, &message);
	}
}

#define CMTS_REPORT_INFO(msg) debugger::message<CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_INFO>(CMTS_TEXT(msg))
#define CMTS_REPORT_WARNING(msg) debugger::message<CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_WARNING>(CMTS_TEXT(msg))
#define CMTS_REPORT_ERROR(msg) debugger::message<CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_ERROR>(CMTS_TEXT(msg))
#else
#define CMTS_REPORT_INFO(msg)
#define CMTS_REPORT_WARNING(msg)
#define CMTS_REPORT_ERROR(msg)
#endif

#ifdef CMTS_WINDOWS

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
CMTS_NAMESPACE_END
#include <Windows.h>
CMTS_NAMESPACE_BEGIN

#define CMTS_WORKER_TASK_CALLING_CONVENTION WINAPI
#define CMTS_WORKER_THREAD_CALLING_CONVENTION WINAPI
#define CMTS_DEBUG_TRAP() DebugBreak()
#define CMTS_SPIN_WAIT() YieldProcessor()

namespace os
{
	using thread_return_type = DWORD;
	using thread_parameter_type = void*;
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
	using WaitOnAddress_t = decltype(WaitOnAddress)*;
	using WakeByAddressSingle_t = decltype(WakeByAddressSingle)*;

	static HMODULE sync_library;
	static WaitOnAddress_t wait_on_address;
	static WakeByAddressSingle_t wake_by_address_single;
#endif

	static uint64_t qpc_frequency;

	CMTS_INLINE_ALWAYS static bool initialize()
	{
		LARGE_INTEGER k;
		(void)QueryPerformanceFrequency(&k);
		qpc_frequency = k.QuadPart;
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
		sync_library = GetModuleHandle(TEXT("Synchronization.lib"));
		CMTS_UNLIKELY_IF(sync_library == nullptr)
		{
			sync_library = GetModuleHandle(TEXT("API-MS-Win-Core-Synch-l1-2-0.dll"));
			CMTS_REPORT_WARNING("\"GetModuleHandle(\"Synchronization.lib\")\" returned NULL. Attempting the same with \"API-MS-Win-Core-Synch-l1-2-0.dll\".");
			CMTS_UNLIKELY_IF(sync_library == nullptr)
			{
				CMTS_REPORT_ERROR("\"GetModuleHandle(\"API-MS-Win-Core-Synch-l1-2-0.dll\")\" returned NULL. Library initialization failed.");
				return false;
			}
			wait_on_address = (WaitOnAddress_t)GetProcAddress(sync_library, "WaitOnAddress");
			CMTS_UNLIKELY_IF(wait_on_address == nullptr)
				return false;
			wake_by_address_single = (WakeByAddressSingle_t)GetProcAddress(sync_library, "WakeByAddressSingle");
			CMTS_UNLIKELY_IF(wake_by_address_single == nullptr)
				return false;
		}
#endif
		return true;
	}

	CMTS_INLINE_ALWAYS static void* allocate(ufastptr size)
	{
		return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	}

	CMTS_INLINE_ALWAYS static bool deallocate(void* ptr, ufastptr size)
	{
		return VirtualFree(ptr, 0, MEM_RELEASE);
	}

	CMTS_INLINE_ALWAYS static HANDLE new_thread(LPTHREAD_START_ROUTINE entry_point, void* param, ufastptr stack_size, bool suspended)
	{
		return CreateThread(nullptr, stack_size, entry_point, param, suspended ? CREATE_SUSPENDED : 0, nullptr);
	}

	CMTS_INLINE_ALWAYS static bool set_thread_affinities(HANDLE* threads, DWORD count, DWORD first)
	{
		GROUP_AFFINITY prior = {};
		GROUP_AFFINITY group = {};
		for (ufast32 i = 0; i != count; ++i)
		{
			group.Group = (WORD)(first >> 6);
			group.Mask = (DWORD_PTR)1 << (DWORD_PTR)(first & 63U);
			CMTS_UNLIKELY_IF(!SetThreadGroupAffinity(threads[i], &group, &prior))
			{
				CMTS_REPORT_ERROR("SetThreadGroupAffinity returned FALSE. Library initialization failed.");
				return false;
			}
			++first;
		}
		return true;
	}

	CMTS_INLINE_ALWAYS static bool await_threads(HANDLE* threads, DWORD count)
	{
		return WaitForMultipleObjects(count, threads, true, INFINITE) == WAIT_OBJECT_0;
	}

	CMTS_INLINE_ALWAYS static bool suspend_threads(HANDLE* threads, DWORD count)
	{
		for (ufast32 i = 0; i != count; ++i)
		{
			CMTS_UNLIKELY_IF(SuspendThread(threads[i]) == MAXDWORD)
			{
				CMTS_REPORT_ERROR("SuspendThread returned FALSE.");
				return false;
			}
		}
		return true;
	}

	CMTS_INLINE_ALWAYS static bool resume_threads(HANDLE* threads, DWORD count)
	{
		for (ufast32 i = 0; i != count; ++i)
		{
			CMTS_UNLIKELY_IF(ResumeThread(threads[i]) == MAXDWORD)
			{
				CMTS_REPORT_ERROR("ResumeThread returned FALSE.");
				return false;
			}
		}
		return true;
	}

	CMTS_INLINE_ALWAYS static void exit_thread()
	{
		ExitThread(0);
	}

	CMTS_INLINE_ALWAYS static bool terminate_threads(HANDLE* threads, DWORD count)
	{
		for (ufast32 i = 0; i != count; ++i)
		{
			CMTS_UNLIKELY_IF(!TerminateThread(threads[i], MAXDWORD))
			{
				CMTS_REPORT_ERROR("TerminateThread returned FALSE.");
				return false;
			}
		}
		return true;
	}

	CMTS_INLINE_ALWAYS static bool is_thread_live(HANDLE thread)
	{
		return WaitForSingleObject(thread, 0) == WAIT_TIMEOUT;
	}

	CMTS_INLINE_ALWAYS static void yield_thread()
	{
		(void)SwitchToThread();
	}

	template <typename T>
	CMTS_INLINE_ALWAYS static void futex_signal(std::atomic<T>& target)
	{
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
		wake_by_address_single(&target);
#endif
	}

	template <typename T>
	CMTS_INLINE_ALWAYS static void futex_await(std::atomic<T>& target, T& last_value)
	{
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
		(void)wait_on_address((volatile void*)&target, &last_value, sizeof(T), INFINITE);
#endif
	}

	CMTS_INLINE_ALWAYS static uint64_t timestamp_us()
	{
		LARGE_INTEGER k;
		(void)QueryPerformanceCounter(&k);
		return ((uint64_t)k.QuadPart * 1000000000UI64) / qpc_frequency;
	}
}
#endif

#ifdef CMTS_WINDOWS
static HANDLE* worker_threads;
#endif

static uint32 worker_thread_count;

#ifdef CMTS_DEBUG
CMTS_NAMESPACE_END
#include <cstdio>
CMTS_NAMESPACE_BEGIN

[[noreturn]] void cmts_debug_assertion_handler(const char* expression)
{
	(void)os::suspend_threads(worker_threads, worker_thread_count);
	char buffer[4096];
	(void)sprintf_s(buffer, "CMTS: Assertion failed! Expression: \"%s\"", expression);
	CMTS_REPORT_ERROR(buffer);
	abort();
}
#endif

static uint32 queue_capacity;
static uint32 queue_capacity_mask;
static uint32 adjust_queue_index_shift;
static uint32 max_tasks;
static uintptr task_stack_size;

#ifdef CMTS_WINDOWS
thread_local static HANDLE root_fiber;
#endif
thread_local static uint32 this_task_index;
thread_local static uint32 worker_thread_index;
#ifdef CMTS_DEBUG
thread_local static bool yield_trap_enabled;
#endif
#ifdef CMTS_EXT_LOCAL_YIELD_COUNT
thread_local static ufast32 this_thread_yield_count;
#endif

#ifdef CMTS_LOCK_LIBRARY
CMTS_NAMESPACE_END
#include <mutex>
CMTS_NAMESPACE_BEGIN
CMTS_SHARED_ATTR static std::mutex library_lock;
#define CMTS_LIBRARY_GUARD std::scoped_lock guard(library_lock)
#else
#define CMTS_LIBRARY_GUARD
#endif

#if CMTS_REQUIRES_TASK_ALLOCATOR
static cmts_allocate_function_pointer_t task_allocate_function;
static cmts_deallocate_function_pointer_t task_deallocate_function;
#endif

CMTS_SHARED_ATTR static std::atomic_bool library_exit_flag;
CMTS_SHARED_ATTR static std::atomic_bool library_is_initialized;
CMTS_SHARED_ATTR static std::atomic_bool library_is_paused;

struct alignas(uint64) index_generation_pair
{
	uint32 index, generation;
};

struct CMTS_SHARED_ATTR shared_queue
{
	using value_type = std::atomic<uint32>;

	value_type* values;
	ufast32 tail;
	CMTS_SHARED_ATTR std::atomic<ufast32> head;
	CMTS_SHARED_ATTR std::atomic<ufast32> size;

	CMTS_INLINE_ALWAYS void initialize(void* buffer)
	{
		values = (value_type*)buffer;
		tail = 0;
		non_atomic_store(head, 0);
		non_atomic_store(size, 0);
		(void)memset(values, 0xff, queue_capacity * sizeof(value_type));
	}
};

enum class task_state : uint8
{
	INACTIVE,
	RUNNING,
	GOING_TO_SLEEP,
	SLEEPING,
};

struct CMTS_SHARED_ATTR task_data
{
#ifdef CMTS_WINDOWS
	HANDLE handle;
#endif
	cmts_task_function_pointer_t function;
	void* parameter;
	void* sync_object;
	uint32 next;
	uint32 generation;
	uint32 assigned_thread;
	uint8 priority;
	uint8 sync_type;
	std::atomic<task_state> state;

	CMTS_INLINE_ALWAYS bool has_stack() const
	{
#ifdef CMTS_WINDOWS
		return handle != nullptr;
#endif
	}
};

struct CMTS_SHARED_ATTR object_pool_header
{
	CMTS_SHARED_ATTR std::atomic<index_generation_pair> freelist;
	CMTS_SHARED_ATTR std::atomic<ufast32> bump;

	CMTS_INLINE_ALWAYS void initialize()
	{
		non_atomic_store(freelist, { UINT32_MAX, 0 });
		non_atomic_store(bump, 0);
	}
};

CMTS_SHARED_ATTR static object_pool_header task_pool_ctrl;
CMTS_SHARED_ATTR static object_pool_header event_pool_header;
CMTS_SHARED_ATTR static object_pool_header counter_pool_header;
CMTS_SHARED_ATTR static shared_queue* worker_thread_queues[CMTS_MAX_PRIORITY];
static task_data* task_pool;

template <typename T>
struct CMTS_SHARED_ATTR cache_aligned
{
	T value;
};

static cache_aligned<std::atomic<ufast32>>* worker_thread_generation_counters;

CMTS_INLINE_ALWAYS static bool is_valid_task(ufast32 index)
{
	CMTS_UNLIKELY_IF(index >= max_tasks)
		return false;
	CMTS_UNLIKELY_IF(index >= task_pool_ctrl.bump.load(std::memory_order_acquire))
		return false;
	return true;
}

CMTS_INLINE_ALWAYS static task_data& get_current_task()
{
	CMTS_INVARIANT(cmts_is_task());
	CMTS_INVARIANT(is_valid_task(this_task_index));
	return task_pool[this_task_index];
}

CMTS_INLINE_ALWAYS static void exit_current_worker_thread()
{
#ifdef CMTS_WINDOWS
	CMTS_LIKELY_IF(is_valid_task(this_task_index))
		get_current_task().handle = nullptr;
#endif
	os::exit_thread();
}

CMTS_INLINE_NEVER static void finalize_check_inner()
{
	exit_current_worker_thread();
}

CMTS_INLINE_ALWAYS static void finalize_check()
{
	CMTS_UNLIKELY_IF(library_exit_flag.load(std::memory_order_acquire))
		finalize_check_inner();
}

struct ext_task_name_view
{
	const CMTS_CHAR* begin;
	size_t length;
};

static std::atomic<ext_task_name_view>* ext_task_names;

template <typename T>
CMTS_INLINE_ALWAYS static bool enum_range_check(T value, T min, T max)
{
	return value >= min && value <= max;
}

CMTS_INLINE_ALWAYS static ufastptr round_to_cache_alignment(ufastptr value)
{
	constexpr ufastptr mask = std::hardware_destructive_interference_size - 1;
	return (value + mask) & (~mask);
}

CMTS_INLINE_ALWAYS static ufastptr round_pow2(ufastptr value)
{
	ufast8 count = CMTS_POPCOUNT(value);
	ufast8 lz = sizeof(uintptr) * 8 - CMTS_FLS(value - 1);
	if (count != 1)
		value = 1UI64 << lz;
	return value;
}

CMTS_INLINE_ALWAYS static ufast64 make_handle(ufast32 index, ufast32 generation)
{
	uint64 r = generation;
	r <<= 32;
	r |= index;
	return r;
}

CMTS_INLINE_ALWAYS static void split_handle(ufast64 handle, ufast32& out_index, ufast32& out_generation)
{
	out_index = (uint32)handle;
	handle >>= 32;
	out_generation = (uint32)handle;
}

namespace romu
{
	namespace duo_jr
	{
		thread_local static uint64 x, y;

		CMTS_INLINE_ALWAYS static void init()
		{
			uintptr seed = (uintptr)&worker_threads;
			seed >>= sizeof(uintptr) > 4 ? 3 : 2;
			seed ^= (uintptr)worker_threads;
			seed += (uintptr)worker_thread_index;

			x = seed ^ 0x9e3779b97f4a7c15;
			y = seed ^ 0xd1b54a32d192ed03;
		}

		CMTS_INLINE_ALWAYS static ufast64 get()
		{
			ufast64 result = x;
			x = 15241094284759029579u * y;
			y = y - result;
			y = CMTS_ROL64(y, 27);
			return result;
		}
	}
}

namespace default_prng = romu::duo_jr;

CMTS_INLINE_ALWAYS static void initialize_thread_local_state(os::thread_parameter_type param)
{
#ifdef CMTS_WINDOWS
	worker_thread_index = (uint32)(ufastptr)param;
	CMTS_INVARIANT(worker_thread_index < worker_thread_count);
	root_fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
	CMTS_INVARIANT(root_fiber != nullptr);
#endif
	default_prng::init();
}

CMTS_INLINE_ALWAYS static ufast32 adjust_queue_index(ufast32 index)
{
	index &= queue_capacity_mask;
	return index;
}

static ufast32(*thread_modulo)(ufast32 value);

CMTS_INLINE_ALWAYS static void submit_task(ufast32 task_index, ufast8 priority)
{
	CMTS_INVARIANT(priority < CMTS_MAX_PRIORITY);
	CMTS_ASSERT(is_valid_task(task_index));
	CMTS_ASSERT(task_pool[task_index].function != nullptr);
	ufast32 thread_index;
	shared_queue* queue;
	for (;; finalize_check())
	{
		thread_index = (ufast32)default_prng::get();
		thread_index = thread_modulo(thread_index);
		queue = &(worker_thread_queues[priority][thread_index]);
		ufast32 prior_size = queue->size.load(std::memory_order_acquire);
		CMTS_UNLIKELY_IF(prior_size >= queue_capacity)
			continue;
#ifdef CMTS_FULLY_WAIT_FREE_QUEUE
		CMTS_LIKELY_IF(queue->size.fetch_add(1, std::memory_order_acquire) < queue_capacity)
			break;
		(void)queue->size.fetch_sub(1, std::memory_order_release);
#else
		CMTS_LIKELY_IF(queue->size.compare_exchange_weak(prior_size, prior_size + 1, std::memory_order_acquire, std::memory_order_relaxed))
			break;
#endif
	}
	ufast32 index = queue->head.fetch_add(1, std::memory_order_acquire);
	index = adjust_queue_index(index);
#ifdef CMTS_DEBUG
	ufast32 n = queue->values[index].exchange(task_index, std::memory_order_release);
	CMTS_INVARIANT(n == UINT32_MAX);
#else
	queue->values[index].store(task_index, std::memory_order_release);
#endif

	(void)worker_thread_generation_counters[thread_index].value.fetch_add(1, std::memory_order_relaxed);
	os::futex_signal(worker_thread_generation_counters[thread_index].value);
}

CMTS_INLINE_NEVER static void submit_task_on(ufast32 task_index, ufast8 priority, ufast32 thread_index)
{
	CMTS_INVARIANT(priority < CMTS_MAX_PRIORITY);
	CMTS_ASSERT(is_valid_task(task_index));
	CMTS_ASSERT(task_pool[task_index].function != nullptr);
	shared_queue& queue = worker_thread_queues[priority][thread_index];
	for (;; finalize_check())
	{
		ufast32 prior_size = queue.size.load(std::memory_order_acquire);
		CMTS_UNLIKELY_IF(prior_size >= queue_capacity)
			continue;
#ifdef CMTS_FULLY_WAIT_FREE_QUEUE
		CMTS_LIKELY_IF(queue.size.fetch_add(1, std::memory_order_acquire) < queue_capacity)
			break;
		(void)queue.size.fetch_sub(1, std::memory_order_release);
#else
		CMTS_LIKELY_IF(queue.size.compare_exchange_weak(prior_size, prior_size + 1, std::memory_order_acquire, std::memory_order_relaxed))
			break;
#endif
	}
	ufast32 index = queue.head.fetch_add(1, std::memory_order_acquire);
	index = adjust_queue_index(index);
#ifdef CMTS_DEBUG
	ufast32 n = queue.values[index].exchange(task_index, std::memory_order_release);
	CMTS_INVARIANT(n == UINT32_MAX);
#else
	queue.values[index].store(task_index, std::memory_order_release);
#endif

	(void)worker_thread_generation_counters[thread_index].value.fetch_add(1, std::memory_order_relaxed);
	os::futex_signal(worker_thread_generation_counters[thread_index].value);
}

CMTS_INLINE_ALWAYS static ufast32 fetch_task()
{
#ifdef CMTS_NO_BUSY_WAIT
	std::atomic<ufast32>& busy_wait_counter = worker_thread_generation_counters[worker_thread_index].value;
#endif
	for (;; finalize_check())
	{
#ifdef CMTS_NO_BUSY_WAIT
		ufast32 last_index = busy_wait_counter.load(std::memory_order_acquire);
#endif
		for (ufast8 priority = 0; priority != CMTS_MAX_PRIORITY; ++priority)
		{
			shared_queue& queue = worker_thread_queues[priority][worker_thread_index];
			if (queue.size.load(std::memory_order_acquire) == 0)
				continue;
			std::atomic<uint32>& target = queue.values[queue.tail];
			CMTS_UNLIKELY_IF(target.load(std::memory_order_acquire) == UINT32_MAX)
				continue;
			ufast32 task_index = target.exchange(UINT32_MAX, std::memory_order_acquire);
			CMTS_UNLIKELY_IF(task_index == UINT32_MAX)
				continue;
			++queue.tail;
			queue.tail = adjust_queue_index(queue.tail);
			(void)queue.size.fetch_sub(1, std::memory_order_release);
			return task_index;
		}
#ifdef CMTS_NO_BUSY_WAIT
		os::futex_await(busy_wait_counter, last_index);
#endif
	}
}

CMTS_INLINE_ALWAYS static void yield_impl()
{
#ifdef CMTS_WINDOWS
	CMTS_INVARIANT(root_fiber != nullptr);
	SwitchToFiber(root_fiber);
	CMTS_INVARIANT(root_fiber != nullptr);
#endif
}

CMTS_INLINE_ALWAYS static void sleep_impl()
{
	get_current_task().state.store(task_state::GOING_TO_SLEEP, std::memory_order_release);
	yield_impl();
}

CMTS_INLINE_ALWAYS static void wake_task(ufast32 index)
{
	CMTS_ASSERT(is_valid_task(index));
	while (task_pool[index].state.load(std::memory_order_acquire) != task_state::SLEEPING)
		CMTS_SPIN_WAIT();
	task_pool[index].state.store(task_state::INACTIVE, std::memory_order_release);
	submit_task(index, task_pool[index].priority);
}

/*
Queue states:
- (NIL, NIL) => READY
- (IDX, IDX) => USED
- (IDX, NIL) => CLOSED
*/
struct wait_queue
{
	struct queue_state
	{
		uint32 head, tail;
	};

	std::atomic<queue_state> ctrl;

	CMTS_INLINE_ALWAYS void init()
	{
		non_atomic_store(ctrl, { UINT32_MAX, UINT32_MAX });
	}

	CMTS_INLINE_ALWAYS bool reset()
	{
		queue_state prior = ctrl.load(std::memory_order_acquire);
		if (prior.head != UINT32_MAX && prior.tail == UINT32_MAX)
			return false;
		init();
		return true;
	}

	CMTS_INLINE_ALWAYS bool is_closed() const
	{
		queue_state prior = ctrl.load(std::memory_order_acquire);
		return prior.head != UINT32_MAX && prior.tail == UINT32_MAX;
	}

	CMTS_INLINE_ALWAYS cmts_result_t get_state() const
	{
		queue_state prior = ctrl.load(std::memory_order_acquire);
		if (prior.head == UINT32_MAX)
			return CMTS_NOT_READY;
		return prior.tail != UINT32_MAX ? CMTS_OK : CMTS_SYNC_OBJECT_EXPIRED;
	}

	CMTS_INLINE_ALWAYS bool push_current_task()
	{
		ufast32 task_index = this_task_index;
		queue_state prior, desired;
		for (;; CMTS_SPIN_WAIT())
		{
			prior = ctrl.load(std::memory_order_acquire);
			if (prior.head != UINT32_MAX && prior.tail == UINT32_MAX)
				return false;
			desired.head = prior.head;
			if (prior.head == UINT32_MAX)
				desired.head = task_index;
			desired.tail = task_index;
			CMTS_LIKELY_IF(ctrl.compare_exchange_weak(prior, desired, std::memory_order_release, std::memory_order_relaxed))
				break;
		}
		CMTS_LIKELY_IF(prior.tail != UINT32_MAX)
		{
			CMTS_INVARIANT(task_pool[prior.tail].next == UINT32_MAX);
			task_pool[prior.tail].next = task_index;
		}
		sleep_impl();
		return true;
	}

	CMTS_INLINE_ALWAYS bool pop()
	{
		queue_state prior, desired;
		for (;; CMTS_SPIN_WAIT())
		{
			prior = ctrl.load(std::memory_order_acquire);
			CMTS_LIKELY_IF(prior.head == UINT32_MAX)
				return false;
			desired.head = task_pool[prior.head].next;
			desired.tail = prior.tail;
			CMTS_LIKELY_IF(ctrl.compare_exchange_weak(prior, desired, std::memory_order_acquire, std::memory_order_relaxed))
				break;
		}
		task_pool[prior.head].next = UINT32_MAX;
		wake_task(prior.head);
	}

	CMTS_INLINE_ALWAYS bool pop_all()
	{
		queue_state prior = ctrl.exchange({ 0, UINT32_MAX }, std::memory_order_acquire);
		if (prior.head != UINT32_MAX && prior.tail == UINT32_MAX)
			return false;
		if (prior.head == UINT32_MAX)
			return true;
		ufast32 n = prior.head;
		while (true)
		{
			while (task_pool[n].state.load(std::memory_order_acquire) != task_state::SLEEPING)
				CMTS_SPIN_WAIT();
			ufast32 next = task_pool[n].next;
			task_pool[n].next = UINT32_MAX;
			wake_task(n);
			CMTS_UNLIKELY_IF(n == prior.tail)
				return true;
			n = next;
		}
	}
};

using fence_data = std::atomic<uint32>;

struct event_data
{
	wait_queue queue;
};

struct counter_data
{
	wait_queue queue;
	std::atomic<uint32_t> value;
	uint32_t unused;
};

struct mutex_data
{
	uint32 owner;
	uint32 tail;
};

static_assert(sizeof(cmts_event_t) >= sizeof(event_data));
static_assert(sizeof(cmts_counter_t) >= sizeof(counter_data));
static_assert(sizeof(cmts_mutex_t) >= sizeof(mutex_data));

CMTS_INLINE_ALWAYS static ufast32 try_acquire_task_dirty()
{
	index_generation_pair prior, desired;
	prior = task_pool_ctrl.freelist.load(std::memory_order_acquire);
	CMTS_LIKELY_IF(prior.index != UINT32_MAX)
	{
		desired.index = task_pool[prior.index].next;
		desired.generation = prior.generation + 1;
		CMTS_LIKELY_IF(task_pool_ctrl.freelist.compare_exchange_weak(prior, desired, std::memory_order_acquire, std::memory_order_relaxed))
			return prior.index;
	}
	return UINT32_MAX;
}

CMTS_INLINE_ALWAYS static ufast32 try_acquire_task()
{
	ufast32 r = try_acquire_task_dirty();
	CMTS_UNLIKELY_IF(r == UINT32_MAX)
	{
		CMTS_LIKELY_IF(task_pool_ctrl.bump.load(std::memory_order_acquire) < max_tasks)
		{
			ufast32 r = task_pool_ctrl.bump.fetch_add(1, std::memory_order_acquire);
			CMTS_LIKELY_IF(r < max_tasks)
				return r;
			ufast32 expected_bump = task_pool_ctrl.bump.load(std::memory_order_acquire);
			CMTS_UNLIKELY_IF(expected_bump > max_tasks)
				(void)task_pool_ctrl.bump.compare_exchange_weak(expected_bump, max_tasks, std::memory_order_release, std::memory_order_relaxed);
		}
	}
	return r;
}

CMTS_INLINE_ALWAYS static ufast32 acquire_task_blocking()
{
	void(*yield_fn)() = cmts_is_task() ? cmts_yield : os::yield_thread;
	ufast32 r;
	for (;; yield_fn())
	{
		for (ufast8 i = 0; i != CMTS_SPIN_THRESHOLD; ++i)
		{
			r = try_acquire_task();
			CMTS_LIKELY_IF(r != UINT32_MAX)
				return r;
			CMTS_SPIN_WAIT();
		}
	}
}

CMTS_INLINE_ALWAYS static void release_task(ufast32 index)
{
	CMTS_INVARIANT(is_valid_task(index));
	task_data& task = task_pool[index];
	++task.generation;
	task.assigned_thread = worker_thread_count;
	task.function = nullptr;
	task.parameter = nullptr;
	task.priority = 0;
	task.sync_type = CMTS_SYNC_TYPE_NONE;
	task.sync_object = nullptr;
	task.next = UINT32_MAX;
	index_generation_pair prior, desired;
	for (;; CMTS_SPIN_WAIT())
	{
		prior = task_pool_ctrl.freelist.load(std::memory_order_acquire);
		task.next = prior.index;
		desired.index = index;
		desired.generation = prior.generation + 1;
		CMTS_LIKELY_IF(task_pool_ctrl.freelist.compare_exchange_weak(prior, desired, std::memory_order_release, std::memory_order_relaxed))
			break;
	}
}

static os::thread_return_type CMTS_WORKER_THREAD_CALLING_CONVENTION cmts_worker_thread_entry_point(os::thread_parameter_type param)
{
	initialize_thread_local_state(param);
	for (;; finalize_check())
	{
		ufast32 index = fetch_task();
		CMTS_INVARIANT(is_valid_task(index));
		this_task_index = index;
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.function != nullptr);
		CMTS_INVARIANT(task.has_stack());
		task.state.store(task_state::RUNNING, std::memory_order_release);
#ifdef CMTS_WINDOWS
		CMTS_INVARIANT(GetCurrentFiber() == root_fiber);
		SwitchToFiber(task.handle);
		CMTS_INVARIANT(GetCurrentFiber() == root_fiber);
#endif
		CMTS_INVARIANT(index == this_task_index);
		bool to_sleep = non_atomic_load(task.state) == task_state::GOING_TO_SLEEP;
		CMTS_UNLIKELY_IF(to_sleep)
			task.state.store(task_state::SLEEPING, std::memory_order_release);
		if (task.function != nullptr)
		{
			if (!to_sleep)
			{
				if (task.assigned_thread == worker_thread_count)
					submit_task(index, task.priority);
				else
					submit_task_on(index, task.priority, task.assigned_thread);
			}
		}
		else
		{
			CMTS_INVARIANT(!to_sleep);
			ufast8 sync_type = task.sync_type;
			void* sync_object = task.sync_object;
			release_task(index);
			if (sync_type == 0)
				continue;
			CMTS_INVARIANT((uint32)sync_type <= 2);
			CMTS_INVARIANT(sync_object != nullptr);
			using signal_ptr_type = void(*)(void*);
			signal_ptr_type signal = (signal_ptr_type)cmts_event_signal;
			if (sync_type == CMTS_SYNC_TYPE_COUNTER)
				signal = (signal_ptr_type)cmts_counter_decrement;
			signal(sync_object);
		}
	}
	return 0;
}

static void CMTS_WORKER_TASK_CALLING_CONVENTION cmts_task_entry_point(void* param)
{
	CMTS_INVARIANT(param != nullptr);
	for (;; finalize_check())
	{
		CMTS_INVARIANT(this_task_index < max_tasks);
		task_data& task = task_pool[this_task_index];
		CMTS_INVARIANT(task.function != nullptr);
		task.function(task.parameter);
		task.function = nullptr;
		cmts_yield();
	}
}

struct ext_header
{
	const ext_header* next;
	cmts_ext_type_t type;
};

CMTS_INLINE_ALWAYS static void handle_extension(const cmts_init_options_t& options, const ext_header* node)
{
	CMTS_INVARIANT(enum_range_check(node->type, CMTS_EXT_TYPE_MIN_ENUM, CMTS_EXT_TYPE_MAX_ENUM));
	switch (node->type)
	{
	case CMTS_EXT_TYPE_DEBUGGER:
#ifdef CMTS_DEBUG
		debugger::init(*(const cmts_ext_debugger_init_options_t*)node);
#endif
		break;
	default:
		abort();
	}
}

CMTS_INLINE_ALWAYS static ufastptr required_library_buffer_size()
{
	ufastptr r = 0;
	r += sizeof(*worker_threads) * worker_thread_count;
	r += sizeof(task_data) * max_tasks;
	ufastptr queues_per_thread = (ufastptr)worker_thread_count * CMTS_MAX_PRIORITY;
	r += sizeof(shared_queue) * queues_per_thread;
	r += sizeof(shared_queue::value_type) * round_pow2(max_tasks / worker_thread_count) * queues_per_thread;
#ifdef CMTS_NO_BUSY_WAIT
	r += sizeof(cache_aligned<std::atomic<ufast32>>) * worker_thread_count;
#endif
	return r;
}

CMTS_INLINE_ALWAYS static ufastptr get_extension_sizes(const cmts_ext_type_t* extensions, uint32_t count)
{
	ufastptr r = 0;
	const cmts_ext_type_t* i = extensions;
	const cmts_ext_type_t* end = i + count;
#ifdef CMTS_DEBUG
	for (; i != end; ++i)
		for (const cmts_ext_type_t* j = i + 1; j != end; ++j)
			CMTS_INVARIANT(*j != *i);
	i = extensions;
#endif
	for (; i != end; ++i)
	{
		CMTS_INVARIANT(enum_range_check(*i, CMTS_EXT_TYPE_MIN_ENUM, CMTS_EXT_TYPE_MAX_ENUM));
		switch (*i)
		{
		case CMTS_EXT_TYPE_TASK_NAME:
			r += max_tasks * sizeof(ext_task_name_view);
			break;
		default:
			CMTS_INVARIANT(false);
		}
	}
	return r;
}

static ufast32 thread_modulo_default(ufast32 value)
{
	if (value < worker_thread_count)
		return value;
	if (value == worker_thread_count)
		return 0;
	return value % worker_thread_count;
}

static ufast32 thread_modulo_fast_mask;
static ufast32 thread_modulo_fast(ufast32 value)
{
	return value & thread_modulo_fast_mask;
}

CMTS_INLINE_ALWAYS static void library_common_init(uint8* buffer)
{
	ufastptr thread_size = round_to_cache_alignment(sizeof(*worker_threads) * worker_thread_count);
	ufastptr task_size = sizeof(task_data) * max_tasks;
	ufastptr queue_size = sizeof(shared_queue::value_type) * queue_capacity;
	ufastptr worker_thread_generations_size = worker_thread_count * sizeof(cache_aligned<std::atomic<ufast32>>);
	worker_threads = (decltype(worker_threads))buffer;
	buffer += thread_size;
	task_pool = (task_data*)buffer;
	buffer += task_size;
	worker_thread_generation_counters = (cache_aligned<std::atomic<ufast32>>*)buffer;
	buffer += worker_thread_generations_size;
	for (ufast32 i = 0; i != worker_thread_count; ++i)
		non_atomic_store(worker_thread_generation_counters[i].value, 0);
	for (ufast8 i = 0; i != CMTS_MAX_PRIORITY; ++i)
	{
		worker_thread_queues[i] = (shared_queue*)buffer;
		buffer += sizeof(shared_queue) * worker_thread_count;
	}
	for (ufast8 i = 0; i != CMTS_MAX_PRIORITY; ++i)
	{
		for (ufast32 j = 0; j != worker_thread_count; ++j)
		{
			shared_queue& queue = worker_thread_queues[i][j];
			queue.initialize(buffer);
			buffer += queue_size;
		}
	}
	task_pool_ctrl.initialize();
	event_pool_header.initialize();
	counter_pool_header.initialize();
	for (ufast32 i = 0; i != max_tasks; ++i)
	{
		(void)memset(task_pool + i, 0, sizeof(task_data));
		task_pool[i].assigned_thread = worker_thread_count;
		task_pool[i].next = UINT32_MAX;
	}
	bool pow2 = CMTS_POPCOUNT(worker_thread_count) == 1;
	if (pow2)
		thread_modulo_fast_mask = worker_thread_count - 1;
	thread_modulo = pow2 ? thread_modulo_fast : thread_modulo_default;
}

static cmts_result_t default_library_init()
{
	ufast32 cpu_count = (ufast32)cmts_processor_count();
	worker_thread_count = cpu_count;
	max_tasks = CMTS_DEFAULT_TASKS_PER_PROCESSOR * cpu_count;
	queue_capacity = (uint32)round_pow2(max_tasks / worker_thread_count);
	queue_capacity_mask = queue_capacity - 1;
	adjust_queue_index_shift = CMTS_FLS(queue_capacity) - SHARED_QUEUE_BASE_SHIFT;
	task_stack_size = cmts_default_task_stack_size();
	ufastptr buffer_size = required_library_buffer_size();
	uint8* buffer = (uint8*)os::allocate(buffer_size);
	CMTS_UNLIKELY_IF(buffer == nullptr)
		return CMTS_ERROR_MEMORY_ALLOCATION;
	library_common_init(buffer);
#ifdef CMTS_WINDOWS
	for (ufast32 i = 0; i != worker_thread_count; ++i)
	{
		worker_threads[i] = os::new_thread(cmts_worker_thread_entry_point, (void*)(uintptr)i, task_stack_size, true);
		CMTS_UNLIKELY_IF(worker_threads[i] == nullptr)
			return CMTS_ERROR_WORKER_THREAD_CREATION;
	}
	CMTS_UNLIKELY_IF(!os::set_thread_affinities(worker_threads, worker_thread_count, 0))
		return CMTS_ERROR_THREAD_AFFINITY_FAILURE;
	CMTS_UNLIKELY_IF(!os::resume_threads(worker_threads, worker_thread_count))
		return CMTS_ERROR_RESUME_WORKER_THREAD;
#endif
	return CMTS_OK;
}

static cmts_result_t custom_library_init(const cmts_init_options_t& options)
{
	CMTS_INVARIANT(options.thread_count <= UINT32_MAX);
	worker_thread_count = (uint32_t)options.thread_count;
	max_tasks = options.max_tasks;
	queue_capacity = (uint32)round_pow2(max_tasks / worker_thread_count);
	queue_capacity_mask = queue_capacity - 1;
	adjust_queue_index_shift = CMTS_FLS(queue_capacity) - SHARED_QUEUE_BASE_SHIFT;
	task_stack_size = options.task_stack_size;
	ufastptr buffer_size = required_library_buffer_size();
	uint8* buffer;
	if (options.allocate_function == nullptr)
		buffer = (uint8*)os::allocate(buffer_size);
	else
		buffer = (uint8*)options.allocate_function(buffer_size);
	CMTS_UNLIKELY_IF(buffer == nullptr)
		return CMTS_ERROR_MEMORY_ALLOCATION;
	library_common_init(buffer);
#ifdef CMTS_WINDOWS
	bool use_affinity = (options.flags & CMTS_INIT_FLAGS_USE_AFFINITY) != 0;
	for (ufast32 i = 0; i != worker_thread_count; ++i)
	{
		worker_threads[i] = os::new_thread(cmts_worker_thread_entry_point, (void*)(uintptr)i, options.task_stack_size, use_affinity);
		CMTS_UNLIKELY_IF(worker_threads[i] == nullptr)
			return CMTS_ERROR_WORKER_THREAD_CREATION;
	}
	if (use_affinity)
	{
		CMTS_UNLIKELY_IF(!os::set_thread_affinities(worker_threads, worker_thread_count, 0))
			return CMTS_ERROR_THREAD_AFFINITY_FAILURE;
		CMTS_UNLIKELY_IF(!os::resume_threads(worker_threads, worker_thread_count))
			return CMTS_ERROR_RESUME_WORKER_THREAD;
	}
#endif
	for (const ext_header* node = (const ext_header*)options.ext; node != nullptr; node = node->next)
		handle_extension(options, node);
	return CMTS_OK;
}

static cmts_result_t dispatch_default_options(cmts_task_function_pointer_t entry_point)
{
	ufast32 index = try_acquire_task();
	CMTS_UNLIKELY_IF(index == UINT32_MAX)
		return CMTS_ERROR_TASK_POOL_CAPACITY;
	CMTS_INVARIANT(is_valid_task(index));
	task_data& task = task_pool[index];
	++task.generation;
	task.function = entry_point;
	task.parameter = nullptr;
	task.sync_object = nullptr;
	task.next = UINT32_MAX;
	task.priority = 0;
	task.sync_type = CMTS_SYNC_TYPE_NONE;
	CMTS_INVARIANT(non_atomic_load(task.state) == task_state::INACTIVE);
#ifdef CMTS_WINDOWS
	CMTS_UNLIKELY_IF(task.handle == nullptr)
		task.handle = CreateFiberEx(task_stack_size, task_stack_size, FIBER_FLAG_FLOAT_SWITCH, cmts_task_entry_point, &task);
#endif
	CMTS_UNLIKELY_IF(!task.has_stack())
	{
		release_task(index);
		return CMTS_ERROR_TASK_ALLOCATION;
	}
	submit_task(index, task.priority);
	return CMTS_OK;
}

#ifdef CMTS_DEBUG
static uint32_t rcu_depth;
#endif

CMTS_NAMESPACE_END

extern "C"
{
	CMTS_ATTR cmts_result_t CMTS_CALL cmts_init(const cmts_init_options_t* options)
	{
		using namespace detail::cmts;
		cmts_result_t result;
		CMTS_INVARIANT(!library_is_initialized.load(std::memory_order_acquire));
		{
			CMTS_LIBRARY_GUARD;
			CMTS_UNLIKELY_IF(!os::initialize())
				return CMTS_ERROR_OS_INIT;
			if (options == nullptr)
				result = default_library_init();
			else
				result = custom_library_init(*options);
			CMTS_UNLIKELY_IF(result < 0)
				return result;
			non_atomic_store(library_exit_flag, false);
			non_atomic_store(library_is_initialized, true);
		}
		return result;
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_pause()
	{
		using namespace detail::cmts;
		CMTS_UNLIKELY_IF(library_is_paused.exchange(true, std::memory_order_acquire))
			return CMTS_OK;
		CMTS_UNLIKELY_IF(!library_is_initialized.load(std::memory_order_acquire))
			return CMTS_ERROR_LIBRARY_UNINITIALIZED;
		{
			CMTS_LIBRARY_GUARD;
			CMTS_UNLIKELY_IF(!os::suspend_threads(worker_threads, worker_thread_count))
				return CMTS_ERROR_SUSPEND_WORKER_THREAD;
		}
		return CMTS_OK;
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_resume()
	{
		using namespace detail::cmts;
		CMTS_UNLIKELY_IF(!library_is_paused.load(std::memory_order_acquire))
			return CMTS_OK;
		CMTS_UNLIKELY_IF(!library_is_initialized.load(std::memory_order_acquire))
			return CMTS_ERROR_LIBRARY_UNINITIALIZED;
		{
			CMTS_LIBRARY_GUARD;
			CMTS_UNLIKELY_IF(!os::resume_threads(worker_threads, worker_thread_count))
				return CMTS_ERROR_RESUME_WORKER_THREAD;
			non_atomic_store(library_is_paused, false);
		}
		return CMTS_OK;
	}

	CMTS_ATTR void CMTS_CALL cmts_finalize_signal()
	{
		using namespace detail::cmts;
		if (library_exit_flag.exchange(true, std::memory_order_acquire))
			return;
#ifdef CMTS_NO_BUSY_WAIT
		for (ufast32 i = 0; i != worker_thread_count; ++i)
			os::futex_signal(worker_thread_generation_counters[i].value);
#endif
		CMTS_LIKELY_IF(cmts_is_task())
			exit_current_worker_thread();
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_finalize_await(cmts_deallocate_function_pointer_t deallocate)
	{
		using namespace detail::cmts;
		CMTS_UNLIKELY_IF(!os::await_threads(worker_threads, worker_thread_count))
			return CMTS_ERROR_AWAIT_WORKER_THREAD;
		CMTS_UNLIKELY_IF(!library_is_initialized.exchange(false, std::memory_order_acquire))
			return CMTS_ERROR_LIBRARY_UNINITIALIZED;
		{
			CMTS_LIBRARY_GUARD;
			ufast32 n = non_atomic_load(task_pool_ctrl.bump);
			for (ufast32 i = 0; i != n; ++i)
			{
				task_data& task = task_pool[i];
#ifdef CMTS_WINDOWS
				CMTS_LIKELY_IF(task.handle != nullptr)
				{
					DeleteFiber(task.handle);
					task.handle = nullptr;
				}
#endif
			}
			ufastptr buffer_size = required_library_buffer_size();
			cmts_deallocate_function_pointer_t fn = os::deallocate;
			if (deallocate != nullptr)
				fn = deallocate;
			CMTS_INVARIANT(fn != nullptr);
			CMTS_UNLIKELY_IF(!fn(worker_threads, buffer_size))
				return CMTS_ERROR_MEMORY_DEALLOCATION;
			worker_threads = nullptr;
		}
		return CMTS_OK;
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_terminate(cmts_deallocate_function_pointer_t deallocate)
	{
		using namespace detail::cmts;
		cmts_finalize_signal();
		CMTS_UNLIKELY_IF(!library_is_initialized.exchange(false, std::memory_order_acquire))
			return CMTS_ERROR_LIBRARY_UNINITIALIZED;
		{
			CMTS_LIBRARY_GUARD;
			CMTS_UNLIKELY_IF(!os::terminate_threads(worker_threads, worker_thread_count))
				return CMTS_ERROR_WORKER_THREAD_TERMINATION;
			ufast32 n = non_atomic_load(task_pool_ctrl.bump);
			for (ufast32 i = 0; i != n; ++i)
			{
				task_data& task = task_pool[i];
#ifdef CMTS_WINDOWS
				CMTS_LIKELY_IF(task.handle != nullptr)
				{
					DeleteFiber(task.handle);
					task.handle = nullptr;
				}
#endif
			}
			ufastptr buffer_size = required_library_buffer_size();
			bool flag;
			if (deallocate != nullptr)
				flag = deallocate(worker_threads, buffer_size);
			else
				flag = os::deallocate(worker_threads, buffer_size);
			CMTS_UNLIKELY_IF(!flag)
				return CMTS_ERROR_MEMORY_DEALLOCATION;
			worker_threads = nullptr;
		}
		return CMTS_OK;
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_initialized()
	{
		using namespace detail::cmts;
		return library_is_initialized.load(std::memory_order_acquire);
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_online()
	{
		using namespace detail::cmts;
		CMTS_UNLIKELY_IF(!cmts_is_initialized())
			return false;
		return !library_exit_flag.load(std::memory_order_acquire);
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_paused()
	{
		using namespace detail::cmts;
		return library_is_paused.load(std::memory_order_acquire);
	}

	CMTS_ATTR uint32_t CMTS_CALL cmts_purge(uint32_t max_trimmed_tasks)
	{
		using namespace detail::cmts;
		return 0;
	}

	CMTS_ATTR uint32_t CMTS_CALL cmts_purge_all()
	{
		using namespace detail::cmts;
		return 0;
	}

	CMTS_ATTR uint32_t CMTS_CALL cmts_worker_thread_count()
	{
		using namespace detail::cmts;
		return worker_thread_count;
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_requires_task_allocator()
	{
		return CMTS_REQUIRES_TASK_ALLOCATOR;
	}

	CMTS_ATTR void CMTS_CALL cmts_assign_task_allocator(cmts_allocate_function_pointer_t allocate, cmts_deallocate_function_pointer_t deallocate)
	{
		using namespace detail::cmts;
#if CMTS_REQUIRES_TASK_ALLOCATOR
		task_allocate_function = allocate;
		task_deallocate_function = deallocate;
#else
		CMTS_REPORT_WARNING("Invoked cmts_assign_task_allocator, but the current platform doesn't allow custom task stack allocation.");
#endif
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_dispatch(cmts_task_function_pointer_t entry_point, cmts_dispatch_options_t* options)
	{
		using namespace detail::cmts;
		CMTS_UNLIKELY_IF(options == nullptr)
			return dispatch_default_options(entry_point);
		CMTS_INVARIANT(options->ext == nullptr);
		CMTS_INVARIANT(options->sync_type <= CMTS_SYNC_TYPE_MAX_ENUM);
		bool blocking_acquire = (options->flags & CMTS_DISPATCH_FLAGS_FORCE) != 0;
		ufast32 index;
		if (blocking_acquire)
			index = acquire_task_blocking();
		else
			index = try_acquire_task();
		CMTS_INVARIANT(!blocking_acquire || index != UINT32_MAX);
		CMTS_UNLIKELY_IF(index == UINT32_MAX)
			return CMTS_ERROR_TASK_POOL_CAPACITY;
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.function == nullptr);
		++task.generation;
		task.function = entry_point;
		task.parameter = options->parameter;
		task.sync_object = options->sync_object;
		task.next = UINT32_MAX;
		task.priority = options->priority;
		task.sync_type = options->sync_type;
		CMTS_INVARIANT(task.state.load(std::memory_order_acquire) == task_state::INACTIVE);
#ifdef CMTS_WINDOWS
		CMTS_UNLIKELY_IF(task.handle == nullptr)
			task.handle = CreateFiberEx(task_stack_size, task_stack_size, FIBER_FLAG_FLOAT_SWITCH, cmts_task_entry_point, &task);
#endif
		CMTS_UNLIKELY_IF(!task.has_stack())
		{
			release_task(index);
			return CMTS_ERROR_TASK_POOL_CAPACITY;
		}
		if (options->out_task_id != nullptr)
			*options->out_task_id = make_handle(index, task.generation);
		submit_task(index, task.priority);
		return CMTS_OK;
	}

	CMTS_ATTR void CMTS_CALL cmts_yield()
	{
		using namespace detail::cmts;
#ifdef CMTS_DEBUG
		CMTS_INVARIANT(!yield_trap_enabled);
#endif
#ifdef CMTS_EXT_LOCAL_YIELD_COUNT
		++this_thread_yield_count;
#endif
		CMTS_INVARIANT(cmts_is_task());
		task_pool[this_task_index].state.store(task_state::INACTIVE, std::memory_order_release);
		yield_impl();
	}

	CMTS_ATTR void CMTS_CALL cmts_exit()
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(cmts_is_task());
		task_pool[this_task_index].function = nullptr;
		task_pool[this_task_index].state.store(task_state::INACTIVE, std::memory_order_release);
		yield_impl();
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_task()
	{
		using namespace detail::cmts;
#ifdef CMTS_WINDOWS
		return root_fiber != nullptr;
#endif
	}

	CMTS_ATTR cmts_task_id_t CMTS_CALL cmts_this_task_id()
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(cmts_is_task());
		CMTS_INVARIANT(is_valid_task(this_task_index));
		return make_handle(this_task_index, task_pool[this_task_index].generation);
	}

	CMTS_NODISCARD CMTS_ATTR cmts_task_id_t CMTS_CALL cmts_task_allocate()
	{
		using namespace detail::cmts;
		ufast32 index = try_acquire_task();
		CMTS_UNLIKELY_IF(index == UINT32_MAX)
			return UINT32_MAX;
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.function == nullptr);
		++task.generation;
		return make_handle(index, task.generation);
	}

	CMTS_ATTR uint8_t CMTS_CALL cmts_task_get_priority(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		CMTS_INVARIANT(task.priority < CMTS_MAX_PRIORITY);
		return task.priority;
	}

	CMTS_ATTR void CMTS_CALL cmts_task_set_priority(cmts_task_id_t task_id, uint8_t priority)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(priority < CMTS_MAX_PRIORITY);
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		task.priority = priority;
	}

	CMTS_ATTR void* CMTS_CALL cmts_task_get_parameter(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		return task.parameter;
	}

	CMTS_ATTR void CMTS_CALL cmts_task_set_parameter(cmts_task_id_t task_id, void* parameter)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		task.parameter = parameter;
	}

#ifdef CMTS_EXT_LOCAL_YIELD_COUNT
	CMTS_ATTR uint32_t CMTS_CALL cmts_local_yield_count()
	{
		using namespace detail::cmts;
		return this_thread_yield_count;
	}
#endif

	CMTS_ATTR void CMTS_CALL cmts_debug_enable_yield_trap()
	{
		using namespace detail::cmts;
#ifdef CMTS_DEBUG
		yield_trap_enabled = true;
#endif
	}

	CMTS_ATTR void CMTS_CALL cmts_debug_disable_yield_trap()
	{
		using namespace detail::cmts;
#ifdef CMTS_DEBUG
		yield_trap_enabled = false;
#endif
	}

	CMTS_ATTR cmts_task_function_pointer_t CMTS_CALL cmts_task_get_function(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		return task.function;
	}

	CMTS_ATTR void CMTS_CALL cmts_task_set_function(cmts_task_id_t task_id, cmts_task_function_pointer_t function)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		task.function = function;
	}

	CMTS_ATTR void CMTS_CALL cmts_task_attach_event(cmts_task_id_t task_id, cmts_event_t* event)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.next == UINT32_MAX);
		CMTS_INVARIANT(task.generation == generation);
		task.sync_type = CMTS_SYNC_TYPE_EVENT;
		task.sync_object = event;
	}

	CMTS_ATTR void CMTS_CALL cmts_task_attach_counter(cmts_task_id_t task_id, cmts_counter_t* counter)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.next == UINT32_MAX);
		CMTS_INVARIANT(task.generation == generation);
		task.sync_type = CMTS_SYNC_TYPE_COUNTER;
		task.sync_object = counter;
	}

	CMTS_ATTR void CMTS_CALL cmts_task_sleep(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		sleep_impl();
	}

	CMTS_ATTR void CMTS_CALL cmts_task_wake(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		wake_task(index);
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_valid(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_UNLIKELY_IF(!is_valid_task(index))
			return false;
		task_data& task = task_pool[index];
		return task.generation == generation;
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_sleeping(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		return task.state.load(std::memory_order_acquire) == task_state::SLEEPING;
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_running(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		return task.state.load(std::memory_order_acquire) == task_state::RUNNING;
	}

	CMTS_ATTR void CMTS_CALL cmts_task_dispatch(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		CMTS_INVARIANT(task.priority < CMTS_MAX_PRIORITY);
		submit_task(index, task.priority);
	}

	CMTS_ATTR void CMTS_CALL cmts_task_deallocate(cmts_task_id_t task_id)
	{
		using namespace detail::cmts;
		ufast32 index, generation;
		split_handle(task_id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		task_data& task = task_pool[index];
		CMTS_INVARIANT(task.generation == generation);
		release_task(index);
	}

	CMTS_ATTR void CMTS_CALL cmts_fence_init(cmts_fence_t* fence_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(fence_ptr != nullptr);
		fence_data& fence = *(fence_data*)fence_ptr;
		non_atomic_store(fence, UINT32_MAX);
	}

	CMTS_ATTR void CMTS_CALL cmts_fence_signal(cmts_fence_t* fence_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(fence_ptr != nullptr);
		fence_data& fence = *(fence_data*)fence_ptr;
		for (;; CMTS_SPIN_WAIT())
		{
			ufast32 task_index = fence.load(std::memory_order_acquire);
			CMTS_LIKELY_IF(task_index != UINT32_MAX)
			{
				wake_task(task_index);
				break;
			}
		}
	}

	CMTS_ATTR void CMTS_CALL cmts_fence_await(cmts_fence_t* fence_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(fence_ptr != nullptr);
		fence_data& fence = *(fence_data*)fence_ptr;
#ifdef CMTS_DEBUG
		ufast32 n = fence.exchange(this_task_index, std::memory_order_release);
		CMTS_INVARIANT(n == UINT32_MAX);
#else
		fence.store(this_task_index, std::memory_order_release);
#endif
		sleep_impl();
	}

	CMTS_ATTR void CMTS_CALL cmts_event_init(cmts_event_t* event_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(event_ptr != nullptr);
		event_data& event = *(event_data*)event_ptr;
		event.queue.init();
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_state(const cmts_event_t* event_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(event_ptr != nullptr);
		const event_data& event = *(const event_data*)event_ptr;
		return event.queue.get_state();
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_signal(cmts_event_t* event_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(event_ptr != nullptr);
		event_data& event = *(event_data*)event_ptr;
		return event.queue.pop_all() ? CMTS_OK : CMTS_SYNC_OBJECT_EXPIRED;
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_await(cmts_event_t* event_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(cmts_is_task());
		CMTS_INVARIANT(event_ptr != nullptr);
		event_data& event = *(event_data*)event_ptr;
		return event.queue.push_current_task() ? CMTS_OK : CMTS_SYNC_OBJECT_EXPIRED;
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_reset(cmts_event_t* event_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(event_ptr != nullptr);
		event_data& event = *(event_data*)event_ptr;
		return event.queue.reset() ? CMTS_OK : CMTS_NOT_READY;
	}

	CMTS_ATTR void CMTS_CALL cmts_counter_init(cmts_counter_t* counter_ptr, uint32_t start_value)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(counter_ptr != nullptr);
		counter_data& counter = *(counter_data*)counter_ptr;
		(void)memset(counter_ptr, 0, sizeof(cmts_counter_t));
		non_atomic_store(counter.value, start_value);
		counter.queue.init();
	}

	CMTS_ATTR uint32_t CMTS_CALL cmts_counter_value(const cmts_counter_t* counter_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(counter_ptr != nullptr);
		const counter_data& counter = *(const counter_data*)counter_ptr;
		return counter.value.load(std::memory_order_acquire);
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_state(const cmts_counter_t* counter_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(counter_ptr != nullptr);
		const counter_data& counter = *(const counter_data*)counter_ptr;
		return counter.queue.get_state();
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_increment(cmts_counter_t* counter_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(counter_ptr != nullptr);
		counter_data& counter = *(counter_data*)counter_ptr;
		CMTS_UNLIKELY_IF(counter.queue.is_closed())
			return CMTS_SYNC_OBJECT_EXPIRED;
		(void)counter.value.fetch_add(1, std::memory_order_release);
		return CMTS_OK;
	}

	CMTS_ATTR cmts_result_t CMTS_CALL CMTS_CALL cmts_counter_decrement(cmts_counter_t* counter_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(counter_ptr != nullptr);
		counter_data& counter = *(counter_data*)counter_ptr;
		CMTS_UNLIKELY_IF(counter.queue.is_closed())
			return CMTS_SYNC_OBJECT_EXPIRED;
		ufast32 current_value = counter.value.fetch_sub(1UI32, std::memory_order_acquire) - 1UI32;
		CMTS_LIKELY_IF(current_value != 0)
			return CMTS_NOT_READY;
		return counter.queue.pop_all() ? CMTS_OK : CMTS_SYNC_OBJECT_EXPIRED;
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_await(cmts_counter_t* counter_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(cmts_is_task());
		CMTS_INVARIANT(counter_ptr != nullptr);
		counter_data& counter = *(counter_data*)counter_ptr;
		return counter.queue.push_current_task() ? CMTS_OK : CMTS_SYNC_OBJECT_EXPIRED;
	}

	CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_reset(cmts_counter_t* counter_ptr, uint32_t new_start_value)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(counter_ptr != nullptr);
		counter_data& counter = *(counter_data*)counter_ptr;
		CMTS_UNLIKELY_IF(!counter.queue.reset())
			return CMTS_NOT_READY;
		non_atomic_store(counter.value, new_start_value);
		return CMTS_OK;
	}

	CMTS_ATTR void CMTS_CALL cmts_mutex_init(cmts_mutex_t* mutex_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(mutex_ptr != nullptr);
		std::atomic<mutex_data>& mutex = *(std::atomic<mutex_data>*)mutex_ptr;
		non_atomic_store(mutex, { UINT32_MAX, UINT32_MAX });
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_mutex_is_locked(const cmts_mutex_t* mutex_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(mutex_ptr != nullptr);
		const std::atomic<mutex_data>& mutex = *(const std::atomic<mutex_data>*)mutex_ptr;
		return mutex.load(std::memory_order_acquire).owner != UINT32_MAX;
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_mutex_try_lock(cmts_mutex_t* mutex_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(mutex_ptr != nullptr);
		std::atomic<mutex_data>& mutex = *(std::atomic<mutex_data>*)mutex_ptr;
		mutex_data expected = { UINT32_MAX, UINT32_MAX };
		return mutex.compare_exchange_strong(expected, { this_task_index, UINT32_MAX }, std::memory_order_acquire, std::memory_order_relaxed);
	}

	CMTS_ATTR void CMTS_CALL cmts_mutex_lock(cmts_mutex_t* mutex_ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(mutex_ptr != nullptr);
		std::atomic<mutex_data>& mutex = *(std::atomic<mutex_data>*)mutex_ptr;
		mutex_data prior, desired;
#ifndef CMTS_HYBRID_MUTEX
		CMTS_INVARIANT(cmts_is_task());
		CMTS_INVARIANT(get_current_task().next == UINT32_MAX);
#else
		CMTS_UNLIKELY_IF(!cmts_is_task())
		{
			for (;; os::futex_await(mutex, prior))
			{
				for (ufast32 i = 0; i != CMTS_SPIN_THRESHOLD; ++i)
					CMTS_LIKELY_IF(cmts_mutex_try_lock(mutex_ptr))
					return;
			}
		}
#endif
		for (;; CMTS_SPIN_WAIT())
		{
			prior = mutex.load(std::memory_order_acquire);
			desired = prior.owner == UINT32_MAX ?
				mutex_data{ this_task_index, UINT32_MAX } :
				mutex_data{ prior.owner, this_task_index };
			CMTS_LIKELY_IF(mutex.compare_exchange_weak(prior, desired, std::memory_order_acquire, std::memory_order_relaxed))
				break;
		}
		CMTS_LIKELY_IF(prior.owner == UINT32_MAX)
			return;
		CMTS_LIKELY_IF(prior.tail != UINT32_MAX)
			task_pool[prior.tail].next = this_task_index;
		sleep_impl();
		get_current_task().next = UINT32_MAX;
	}

	CMTS_ATTR void CMTS_CALL cmts_mutex_unlock(cmts_mutex_t* mutex_ptr)
	{
		using namespace detail::cmts;
#ifndef CMTS_HYBRID_MUTEX
		CMTS_INVARIANT(cmts_is_task());
#endif
		CMTS_INVARIANT(mutex_ptr != nullptr);
		std::atomic<mutex_data>& mutex = *(std::atomic<mutex_data>*)mutex_ptr;
		mutex_data prior = { this_task_index, UINT32_MAX };
		mutex_data desired = { UINT32_MAX, UINT32_MAX };
		CMTS_LIKELY_IF(mutex.compare_exchange_strong(prior, desired, std::memory_order_release, std::memory_order_relaxed))
			return;
		for (;; CMTS_SPIN_WAIT())
		{
			prior = mutex.load(std::memory_order_acquire);
			desired.owner = prior.tail;
			CMTS_LIKELY_IF(prior.tail != UINT32_MAX)
			{
				CMTS_UNLIKELY_IF(task_pool[prior.tail].state.load(std::memory_order_acquire) != task_state::SLEEPING)
					continue;
				desired.tail = task_pool[prior.tail].next;
			}
			CMTS_LIKELY_IF(mutex.compare_exchange_weak(prior, desired, std::memory_order_acquire, std::memory_order_relaxed))
				break;
		}
		CMTS_UNLIKELY_IF(prior.tail == UINT32_MAX)
			wake_task(prior.tail);
#ifdef CMTS_HYBRID_MUTEX
		os::futex_signal(mutex);
#endif
	}

	CMTS_ATTR void CMTS_CALL cmts_rcu_read_begin()
	{
#ifdef CMTS_DEBUG
		using namespace detail::cmts;
		CMTS_INVARIANT(cmts_is_task());
		if (rcu_depth == 0)
			cmts_debug_enable_yield_trap();
		++rcu_depth;
#endif
	}

	CMTS_ATTR void CMTS_CALL cmts_rcu_read_end()
	{
#ifdef CMTS_DEBUG
		using namespace detail::cmts;
		CMTS_INVARIANT(cmts_is_task());
		--rcu_depth;
		if (rcu_depth == 0)
			cmts_debug_disable_yield_trap();
#endif
	}

	CMTS_ATTR void CMTS_CALL cmts_rcu_sync()
	{
		using namespace detail::cmts;

#ifdef CMTS_RCU_SYNC_BASIC

		ufast32 initial_index = worker_thread_index;
		for (ufast32 i = 0; i != worker_thread_count; ++i)
		{
			if (i == initial_index)
				continue;
			get_current_task().assigned_thread = i;
			cmts_yield();
		}
		get_current_task().assigned_thread = worker_thread_count;

#else
		constexpr ufast32 BLOCK_SIZE = 256;
		uint32 block[BLOCK_SIZE];

		ufast32 next;
		for (ufast32 i = 0; i < worker_thread_count; i = next)
		{
			next = i + BLOCK_SIZE;
			ufast32 max = BLOCK_SIZE;

			CMTS_UNLIKELY_IF(next > worker_thread_count)
				max = worker_thread_count - i;

			for (ufast32 j = 0; j != BLOCK_SIZE; ++j)
				block[j] = worker_thread_generation_counters[i + j].value.load(std::memory_order_relaxed);

			cmts_yield();

			for (ufast32 j = 0; j != BLOCK_SIZE; ++j)
			{
				for (ufast32 k = 0;;)
				{
					if (block[j] != worker_thread_generation_counters[i + j].value.load(std::memory_order_acquire))
						break;
					++k;
					if (k == CMTS_SPIN_THRESHOLD)
					{
						cmts_yield();
						k = 0;
					}
				}
			}
		}
#endif
	}

	CMTS_ATTR size_t CMTS_CALL cmts_rcu_snapshot_size()
	{
		using namespace detail::cmts;
		return worker_thread_count * sizeof(uint32);
	}

	CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot(void* snapshot)
	{
		using namespace detail::cmts;
		uint32* state = (uint32*)snapshot;
		for (ufast32 i = 0; i != worker_thread_count; ++i)
			state[i] = worker_thread_generation_counters[i].value.load(std::memory_order_acquire);
	}

	CMTS_ATTR uint32_t CMTS_CALL cmts_rcu_try_snapshot_sync(void* snapshot, uint32_t prior_result)
	{
		using namespace detail::cmts;
		uint32* state = (uint32*)snapshot;
		ufast32 i = prior_result;
		for (; i != worker_thread_count; ++i)
		{
			if (i == worker_thread_index)
				continue;
			if (worker_thread_generation_counters[i].value.load(std::memory_order_acquire) == state[i])
				break;
		}
		return i;
	}

	CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot_sync(void* snapshot)
	{
		using namespace detail::cmts;
		uint32* state = (uint32*)snapshot;
		for (ufast32 i = 0; i != worker_thread_count; ++i)
		{
			if (i == worker_thread_index)
				continue;
			while (worker_thread_generation_counters[i].value.load(std::memory_order_acquire) == state[i])
				cmts_yield();
		}
	}

	CMTS_ATTR size_t CMTS_CALL cmts_hptr_required_size()
	{
		using namespace detail::cmts;
		return worker_thread_count * sizeof(cache_aligned<std::atomic<void*>>);
	}

	CMTS_ATTR void CMTS_CALL cmts_hptr_init(cmts_hptr_t* hptr, cmts_allocate_function_pointer_t allocate)
	{
		using namespace detail::cmts;
		size_t k = cmts_hptr_required_size();
		hptr->impl = allocate(k);
		(void)memset(hptr->impl, 0, k);
	}

	CMTS_ATTR void CMTS_CALL cmts_hptr_delete(cmts_hptr_t* hptr, cmts_deallocate_function_pointer_t deallocate)
	{
		using namespace detail::cmts;
		deallocate(hptr->impl, cmts_hptr_required_size());
	}

	CMTS_ATTR void CMTS_CALL cmts_hptr_protect(cmts_hptr_t* hptr, void* ptr)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(ptr != nullptr);
#ifdef CMTS_DEBUG
		CMTS_ASSERT(((cache_aligned<std::atomic<void*>>*)hptr->impl)[worker_thread_index].value.exchange(ptr, std::memory_order_release) == nullptr);
#else
		((cache_aligned<std::atomic<void*>>*)hptr->impl)[worker_thread_index].value.store(ptr, std::memory_order_release);
#endif
	}

	CMTS_ATTR void CMTS_CALL cmts_hptr_release(cmts_hptr_t* hptr)
	{
		using namespace detail::cmts;
		((cache_aligned<std::atomic<void*>>*)hptr->impl)[worker_thread_index].value.store(nullptr, std::memory_order_release);
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_hptr_is_unreachable(const cmts_hptr_t* hptr, const void* ptr)
	{
		using namespace detail::cmts;
		for (uint_fast32_t i = 0; i != worker_thread_index; ++i)
			CMTS_UNLIKELY_IF(((const cache_aligned<std::atomic<void*>>*)hptr->impl)[i].value.load(std::memory_order_acquire) == ptr)
			return false;
		return true;
	}

	CMTS_ATTR uint32_t CMTS_CALL cmts_this_worker_thread_index()
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(cmts_is_task());
		CMTS_INVARIANT(worker_thread_index < worker_thread_count);
		return worker_thread_index;
	}

	CMTS_ATTR uint32_t CMTS_CALL cmts_processor_count()
	{
		uint32_t cpu_count;
#ifdef CMTS_WINDOWS
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		cpu_count = info.dwNumberOfProcessors;
#endif
		return cpu_count;
	}

	CMTS_ATTR uint32_t CMTS_CALL cmts_this_processor_index()
	{
#ifdef CMTS_WINDOWS
		PROCESSOR_NUMBER k;
		GetCurrentProcessorNumberEx(&k);
		return ((size_t)k.Group << 6U) | (size_t)k.Number;
#endif
	}

	CMTS_ATTR size_t CMTS_CALL cmts_default_task_stack_size()
	{
		size_t size;
#ifdef CMTS_WINDOWS
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		size = info.dwAllocationGranularity;
#endif
		return size;
	}

	CMTS_ATTR cmts_bool_t CMTS_ATTR cmts_ext_debugger_enabled()
	{
		using namespace detail::cmts;
#ifdef CMTS_DEBUG
		return debugger::callback != nullptr;
#else
		return false;
#endif
	}

	CMTS_ATTR void CMTS_CALL cmts_ext_debugger_write(const cmts_ext_debugger_message_t* message)
	{
		using namespace detail::cmts;
#ifdef CMTS_DEBUG
		debugger::callback(debugger::context, message);
#endif
	}

	CMTS_ATTR cmts_bool_t CMTS_CALL cmts_ext_task_name_enabled()
	{
		using namespace detail::cmts;
		return ext_task_names != nullptr;
	}

	CMTS_ATTR void CMTS_CALL cmts_ext_task_name_set(cmts_task_id_t id, const CMTS_CHAR* name, size_t length)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(ext_task_names != nullptr);
		ufast32 index, generation;
		split_handle(id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		CMTS_INVARIANT(task_pool[index].generation == generation);
		std::atomic<ext_task_name_view>& data = ext_task_names[index];
		ext_task_name_view new_value = { name, length };
		data.store(new_value, std::memory_order_release);
	}

	CMTS_ATTR void CMTS_CALL cmts_ext_task_name_swap(cmts_task_id_t id, const CMTS_CHAR* name, size_t length, const CMTS_CHAR** out_old_name, size_t* out_old_length)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(ext_task_names != nullptr);
		ufast32 index, generation;
		split_handle(id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		CMTS_INVARIANT(task_pool[index].generation == generation);
		std::atomic<ext_task_name_view>& data = ext_task_names[index];
		ext_task_name_view new_value = { name, length };
		ext_task_name_view prior = data.exchange(new_value, std::memory_order_release);
		*out_old_name = prior.begin;
		*out_old_length = prior.length;
	}

	CMTS_ATTR void CMTS_CALL cmts_ext_task_name_get(cmts_task_id_t id, const CMTS_CHAR** out_name, size_t* out_length)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(ext_task_names != nullptr);
		ufast32 index, generation;
		split_handle(id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		CMTS_INVARIANT(task_pool[index].generation == generation);
		const ext_task_name_view value = ext_task_names[index].load(std::memory_order_acquire);
		*out_name = value.begin;
		*out_length = value.length;
	}

	CMTS_ATTR void CMTS_CALL cmts_ext_task_name_clear(cmts_task_id_t id)
	{
		using namespace detail::cmts;
		CMTS_INVARIANT(ext_task_names != nullptr);
		ufast32 index, generation;
		split_handle(id, index, generation);
		CMTS_INVARIANT(is_valid_task(index));
		CMTS_INVARIANT(task_pool[index].generation == generation);
		ext_task_names[index].store({}, std::memory_order_release);
	}

};
#endif