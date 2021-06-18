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

/** @file cmts.h
* Contains the core C api and implementation of CMTS.
*/

#ifndef CMTS_INCLUDED
#define CMTS_INCLUDED
#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#define CMTS_WINDOWS
#define CMTS_REQUIRES_TASK_ALLOCATOR 0
#else
#error "CMTS: UNSUPPORTED OPERATING SYSTEM."
#endif

#ifndef CMTS_CALL
/**
* Used to override the calling convention of public CMTS functions.
*/
#define CMTS_CALL
#endif

#ifndef CMTS_ATTR
/**
* Used to add extra attributes to public CMTS functions.
*/
#define CMTS_ATTR
#endif

#ifndef CMTS_PTR
/**
* Used to override the calling convention of CMTS function pointers.
*/
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

#define CMTS_INVALID_TASK_ID UINT64_MAX
#define CMTS_MAX_TASKS UINT32_MAX

#ifndef CMTS_CACHE_LINE_SIZE
#define CMTS_CACHE_LINE_SIZE 64
#endif

#ifndef CMTS_MAX_PRIORITY
/**
* Used to override the number of queues per worker thread.
*/
#define CMTS_MAX_PRIORITY 3
#endif

#if CMTS_MAX_PRIORITY >= 256
#error "Error, CMTS_MAX_PRIORITY must not exceed 256"
#endif

#ifndef CMTS_DEFAULT_TASKS_PER_THREAD
/**
* Used to override the number allocated tasks per worker thread (if cmts_init is called with NULL).
*/
#define CMTS_DEFAULT_TASKS_PER_THREAD 256
#endif

#ifndef CMTS_SPIN_THRESHOLD
/**
* Used to override the maximum number of retries of a spin loop before switching to a long-term waiting strategy.
*/
#define CMTS_SPIN_THRESHOLD 8
#endif

#define CMTS_FENCE_DATA_SIZE 4
#define CMTS_EVENT_DATA_SIZE 8
#define CMTS_COUNTER_DATA_SIZE 16
#define CMTS_MUTEX_DATA_SIZE 8

#ifndef CMTS_CHAR
#define CMTS_CHAR char
#endif

#ifndef CMTS_TEXT
#define CMTS_TEXT(TEXT) TEXT
#endif

#define CMTS_FALSE ((cmts_bool)0)
#define CMTS_TRUE ((cmts_bool)1)

extern "C"
{

#ifdef __cplusplus
typedef bool cmts_bool;
#else
typedef _Bool cmts_bool;
#endif

typedef uint64_t cmts_task_id;
typedef void(CMTS_PTR* cmts_fn_task)(void* parameter);
typedef void* (CMTS_PTR* cmts_fn_allocate)(size_t size);
typedef cmts_bool(CMTS_PTR* cmts_fn_deallocate)(void* memory, size_t size);
typedef void(CMTS_PTR* cmts_fn_destructor)(void* object);

typedef enum cmts_result
{
	CMTS_OK = 0,
	CMTS_SYNC_OBJECT_EXPIRED = 1,
	CMTS_NOT_READY = 2,
	CMTS_ALREADY_INITIALIZED = 3,
	CMTS_INITIALIZATION_IN_PROGRESS = 4,

	CMTS_ERROR_MEMORY_ALLOCATION = -1,
	CMTS_ERROR_MEMORY_DEALLOCATION = -2,
	CMTS_ERROR_THREAD_CREATION = -3,
	CMTS_ERROR_THREAD_AFFINITY = -4,
	CMTS_ERROR_RESUME_THREAD = -5,
	CMTS_ERROR_SUSPEND_THREAD = -6,
	CMTS_ERROR_TERMINATE_THREAD = -7,
	CMTS_ERROR_AWAIT_THREAD = -8,
	CMTS_ERROR_TASK_POOL_CAPACITY = -9,
	CMTS_ERROR_AFFINITY = -10,
	CMTS_ERROR_TASK_ALLOCATION = -11,
	CMTS_ERROR_FUTEX = -12,
	CMTS_ERROR_LIBRARY_UNINITIALIZED = -13,
	CMTS_ERROR_OS_INIT = -14,
	CMTS_ERROR_INVALID_EXTENSION_TYPE = -15,
	CMTS_ERROR_UNSUPPORTED_EXTENSION = -16,

	CMTS_RESULT_BEGIN_ENUM = CMTS_ERROR_UNSUPPORTED_EXTENSION,
	CMTS_RESULT_END_ENUM = CMTS_INITIALIZATION_IN_PROGRESS + 1,
} cmts_result;

typedef enum cmts_sync_type
{
	CMTS_SYNC_TYPE_NONE,
	CMTS_SYNC_TYPE_EVENT,
	CMTS_SYNC_TYPE_COUNTER,

	CMTS_SYNC_TYPE_BEGIN_ENUM = CMTS_SYNC_TYPE_NONE,
	CMTS_SYNC_TYPE_END_ENUM = CMTS_SYNC_TYPE_COUNTER + 1,
} cmts_sync_type;

typedef enum cmts_dispatch_flag_bits
{
	CMTS_DISPATCH_FLAGS_FORCE = 1,
} cmts_dispatch_flag_bits;
typedef uint64_t cmts_dispatch_flags;

typedef enum cmts_init_flag_bits
{
} cmts_init_flag_bits;
typedef uint64_t cmts_init_flags;

typedef enum cmts_ext_type
{
	CMTS_EXT_TYPE_DEBUGGER,

	CMTS_EXT_TYPE_BEGIN_ENUM = CMTS_EXT_TYPE_DEBUGGER,
	CMTS_EXT_TYPE_END_ENUM = CMTS_EXT_TYPE_DEBUGGER + 1,
} cmts_ext_type;

typedef uint32_t cmts_fence;
typedef uint64_t cmts_event;
typedef struct cmts_counter { uint64_t low, high; } cmts_counter;
typedef uint32_t cmts_mutex;
typedef size_t cmts_hazard_context;

/** Macro alternative to cmts_fence_init.
* Mainly useful for compile-time initialization.
*/
#define CMTS_FENCE_INIT UINT32_MAX
/** Macro alternative to cmts_event_init.
* Mainly useful for compile-time initialization.
*/
#define CMTS_EVENT_INIT UINT64_MAX
/** Macro alternative to cmts_counter_init.
* Mainly useful for compile-time initialization.
*/
#define CMTS_COUNTER_INIT(VALUE) { UINT64_MAX, (VALUE) }
/** Macro alternative to cmts_mutex_init.
* Mainly useful for compile-time initialization.
*/
#define CMTS_MUTEX_INIT UINT32_MAX

typedef struct cmts_task_allocator
{
	// The memory allocation callback.
	cmts_fn_allocate allocate;
	// The memory deallocation callback.
	cmts_fn_deallocate deallocate;
} cmts_task_allocator;

typedef struct cmts_init_options
{
	// Reserved, must be 0.
	cmts_init_flags flags;
	// A callback that will be used when allocating memory for the global state of CMTS. This function is currently called only once.
	cmts_fn_allocate allocate_function;
	// Either NULL or a pointer to an array of indices to use when assigning worker threads to CPU cores.
	const uint32_t* thread_affinities;
	// Either NULL or a valid pointer to a cmts_task_allocator record. In Windows platforms this value is ignored.
	const cmts_task_allocator* task_allocator;
	// The default size of the stack of a tasks.
	uint32_t task_stack_size;
	// The number of worker threads to launch.
	uint32_t thread_count;
	// The capacity of the global task pool
	uint32_t max_tasks;
	// Either NULL or a valid pointer to a record of type cmts_ext_*_init_options. cmts_ext_debug_init_options, for example.
	const void* next_ext;
} cmts_init_options;

typedef struct cmts_dispatch_options
{
	// Flags that specify the behavior of cmts_dispatch. For example, CMTS_DISPATCH_FLAGS_FORCE makes this function block until a task is acquired.
	cmts_dispatch_flags flags;
	// Either NULL or a valid pointer to a cmts_task_id variable that receives the ID of the submitted task.
	cmts_task_id* out_task_id;
	// Either NULL or a valid pointer to a uint32_t variable that specifies the index of the worker thread to which the task will always run on.
	const uint32_t* locked_thread;
	// The parameter to pass to the task entry point.
	void* parameter;
	// Either NULL or a valid pointer to a synchronization object (only cmts_event or cmts_counter).
	void* sync_object;
	// The type of the object pointed to by sync_object. Ignored if it is NULL.
	cmts_sync_type sync_type;
	// The execution priority of the task.
	uint8_t priority;
	// Reserved, must be NULL.
	const void* next_ext;
} cmts_dispatch_options;

typedef struct cmts_memory_requirements
{
	// The required buffer size.
	size_t size;
	// The base-2 log of the alignment of the buffer.
	uint8_t alignment_log2;
} cmts_memory_requirements;

typedef enum cmts_ext_debug_message_severity
{
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_INFO,
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_WARNING,
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_ERROR,

	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_BEGIN_ENUM = CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_INFO,
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_END_ENUM = CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_ERROR + 1,
} cmts_ext_debug_message_severity;

typedef struct cmts_ext_debug_message
{
	// A pointer to a null-terminated string with the message text.
	const CMTS_CHAR* message;
	// The length of the message.
	size_t message_length;
	// The severity of the message.
	cmts_ext_debug_message_severity severity;
	// Reserved, must be NULL.
	const void* next_ext;
} cmts_ext_debug_message;

typedef void(CMTS_CALL* cmts_fn_debugger_message)(void* context, const cmts_ext_debug_message* message);

typedef struct cmts_ext_debug_init_options
{
	// Either NULL or a valid pointer to another record of type cmts_ext_*_init_options.
	const void* next;
	// Must be CMTS_EXT_TYPE_DEBUGGER.
	cmts_ext_type type;
	// Either NULL or a pointer that will be passed to cmts_ext_debug_write when invoked.
	void* context;
	// Either NULL or the callback to use when cmts_ext_debug_write is invoked.
	cmts_fn_debugger_message message_callback;
} cmts_ext_debug_init_options;

/** @brief Initializes the CMTS scheduler.
* @param options An optional pointer to a @ref cmts_init_options record.
* @return CMTS_OK on success. Possible error codes include: 
* @li CMTS_ALREADY_INITIALIZED
* @li CMTS_INITIALIZATION_IN_PROGRESS
* @li CMTS_ERROR_OS_INIT
* @li CMTS_ERROR_MEMORY_ALLOCATION
* @li CMTS_ERROR_INVALID_EXTENSION_TYPE
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_init(const cmts_init_options* options);
/** Pauses all worker threads.
* @return CMTS_OK on success. Otherwise CMTS_ERROR_LIBRARY_UNINITIALIZED or CMTS_ERROR_SUSPEND_THREAD.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_pause();
/** Resumes all worker threads.
* @return CMTS_OK on success. Otherwise CMTS_ERROR_LIBRARY_UNINITIALIZED or CMTS_ERROR_RESUME_THREAD.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_resume();
/** Begins the library cleanup process.
* After this function is called, the worker threads will begin to exit once they reach an idle state.
*/
CMTS_ATTR void CMTS_CALL cmts_finalize_signal();
/** Blocks until all worker threads have quit and then handles library cleanup.
* @param deallocate Either NULL or a callback to free the memory that was previously allocated by cmts_init_options::allocate_function at library startup.
* @return CMTS_OK on success. Otherwise CMTS_ERROR_LIBRARY_UNINITIALIZED, CMTS_ERROR_AWAIT_THREAD or CMTS_ERROR_MEMORY_DEALLOCATION;
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_finalize_await(cmts_fn_deallocate deallocate);
/** Forcefully quits all worker threads and then handles library cleanup.
* @param deallocate Either NULL or a callback to free the memory that was previously allocated by cmts_init_options::allocate_function at library startup.
* @return CMTS_OK on success. Otherwise CMTS_ERROR_LIBRARY_UNINITIALIZED, CMTS_ERROR_TERMINATE_THREAD or CMTS_ERROR_MEMORY_DEALLOCATION;
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_terminate(cmts_fn_deallocate deallocate);
/**
* @return Whether CMTS is initialized.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_is_initialized();
/**
* @return CMTS_TRUE if the library is initialized and cmts_finalize_signal has not yet been called. CMTS_FALSE otherwise.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_is_online();
/**
* @return Whether the worker threads are paused.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_is_paused();
/** Frees cached tasks.
* @params max_purged_tasks The maximum number of cached tasks to free.
* @return The number of freed tasks.
* @note This function is not thread-safe.
*/
CMTS_ATTR uint32_t CMTS_CALL cmts_purge(uint32_t max_purged_tasks);
/** Frees all cached tasks.
* @return The number of freed tasks.
* @note This function is not thread-safe.
*/
CMTS_ATTR uint32_t CMTS_CALL cmts_purge_all();
/**
* @return Whether the current thread is a CMTS worker thread.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_is_worker_thread();
/**
* @return If the current thread belongs to CMTS, the index of the worker thread. Otherwise returns the number of worker threads.
*/
CMTS_ATTR uint32_t CMTS_CALL cmts_worker_thread_index();
/**
* @return The number of worker threads.
*/
CMTS_ATTR uint32_t CMTS_CALL cmts_worker_thread_count();

/** Submits a task to the CMTS scheduler with the specified entry point.
* @param entry_point A callback to use as the entry point of the submitted task.
* @param options Either NULL or a valid pointer to a cmts_dispatch_options record.
* @return CMTS_OK on success. Otherwise CMTS_ERROR_TASK_POOL_CAPACITY or CMTS_ERROR_TASK_ALLOCATION.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_dispatch(cmts_fn_task entry_point, cmts_dispatch_options* options);
/** Pauses execution of the current task, allowing another one to run on the current worker thread.
*/
CMTS_ATTR void CMTS_CALL cmts_yield();
/** Finishes execution of the current task, returning it to the global pool.
* @remarks In C++ programs extra care must be taken to ensure that local objects have their resources freed, since calling cmts_exit does not invoke destructors automatically.
*/
CMTS_NORETURN CMTS_ATTR void CMTS_CALL cmts_exit();
/**
* @return Whether this function is being called from within a task.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_is_task();
/**
* @return The ID of the current task.
*/
CMTS_ATTR cmts_task_id CMTS_CALL cmts_this_task_id();

/** Manually allocates a task from the global pool.
* @return The ID of the allocated task. CMTS_INVALID_TASK_ID if the global pool has run out of tasks.
*/
CMTS_NODISCARD CMTS_ATTR cmts_task_id CMTS_CALL cmts_task_allocate();
/** Retrieves the priority of a task.
* @param task_id The ID of the task.
* @return The priority of the task.
*/
CMTS_ATTR uint8_t CMTS_CALL cmts_task_get_priority(cmts_task_id task_id);
/** Sets the priority of a task.
* @param task_id The ID of the task.
* @param new_priority The desired priority of the task.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR void CMTS_CALL cmts_task_set_priority(cmts_task_id task_id, uint8_t new_priority);
/** Retrieves the parameter of a task.
* @param task_id The ID of the task.
* @return The parameters of the task.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR void* CMTS_CALL cmts_task_get_parameter(cmts_task_id task_id);
/** Sets the parameter of a task.
* @param task_id The ID of the task.
* @param new_parameter The desired parameter of the task.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR void CMTS_CALL cmts_task_set_parameter(cmts_task_id task_id, void* new_parameter);
/** Retrieves the entry point of a task.
* @param task_id The ID of the task.
* @return The entry point of the task.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR cmts_fn_task CMTS_CALL cmts_task_get_function(cmts_task_id task_id);
/** Sets the entry point of a task.
* @param task_id The ID of the task.
* @param new_function The desired entry point of the task.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR void CMTS_CALL cmts_task_set_function(cmts_task_id task_id, cmts_fn_task new_function);
/** Attaches a cmts_event to a task.
* @param task_id The ID of the task.
* @param event A pointer to the event to attach.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR void CMTS_CALL cmts_task_attach_event(cmts_task_id task_id, cmts_event* event);
/** Attaches a cmts_counter to a task.
* @param task_id The ID of the task.
* @param counter A pointer to the counter to attach.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR void CMTS_CALL cmts_task_attach_counter(cmts_task_id task_id, cmts_counter* counter);
/** Pauses the execution of a task.
* @param task_id The ID of the task.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR void CMTS_CALL cmts_task_sleep(cmts_task_id task_id);
/** Resumes execution of a task.
* @param task_id The ID of the task.
* @note WARNING: Calling this function while the specified task is not currently sleeping is UB.
*/
CMTS_ATTR void CMTS_CALL cmts_task_resume(cmts_task_id task_id);
/** Checks whether a task ID is valid.
* @param task_id The ID of the task.
* @return Whether the specified ID is valid.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_is_valid_task_id(cmts_task_id task_id);
/** Checks whether a task is sleeping.
* @param task_id The ID of the task.
* @return Whether the specified task is sleeping.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_task_is_sleeping(cmts_task_id task_id);
/** Checks whether a task is running.
* @param task_id The ID of the task.
* @return Whether the specified task is running.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_task_is_running(cmts_task_id task_id);
/** Submits a task to the scheduler.
* @param task_id The ID of the task.
*/
CMTS_ATTR void CMTS_CALL cmts_task_dispatch(cmts_task_id task_id);
/** Returns the specified task to the global pool.
* @param task_id The ID of the task.
* @note WARNING: Reading or writing task attributes is not thread-safe and if the task has been submitted it is UB.
*/
CMTS_ATTR void CMTS_CALL cmts_task_deallocate(cmts_task_id task_id);

/** Initializes a fence.
* @param fence A valid pointer to a cmts_fence object.
* @note This function is not thread-safe.
*/
CMTS_ATTR void CMTS_CALL cmts_fence_init(cmts_fence* fence);
/** Busy waits until a task is waiting on the fence, then resumes it.
* @param fence A valid pointer to a cmts_fence object.
*/
CMTS_ATTR void CMTS_CALL cmts_fence_signal(cmts_fence* fence);
/** Attempts to suspend execution of the current task until the fence is signaled.
* @param fence A valid pointer to a cmts_fence object.
* @return CMTS_TRUE if the wait succeeded. CMTS_FALSE if the fence was already being awaited on.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_fence_try_await(cmts_fence* fence);
/** Suspends execution of the current task until the fence is signaled.
* [DEBUG ONLY] If the fence is being awaited on by another task, abort is called.
* @param fence A valid pointer to a cmts_fence object.
*/
CMTS_ATTR void CMTS_CALL cmts_fence_await(cmts_fence* fence);

/** Initializes an event object.
* @param event A valid pointer to a cmts_event object.
* @note This function is not thread-safe.
*/
CMTS_ATTR void CMTS_CALL cmts_event_init(cmts_event* event);
/** Retrieves the state of an event.
* @param event A valid pointer to a cmts_event object.
* @return The state of the event. Possible results:
* @li CMTS_OK if the event has already been signaled.
* @li CMTS_SYNC_OBJECT_EXPIRED if the event has already been signaled.
* @li CMTS_NOT_READY if no task is waiting for the event to become signaled.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_event_state(const cmts_event* event);
/** Signals the event, resuming any tasks awaiting on it.
* @param event A valid pointer to a cmts_event object.
* @return CMTS_OK if the operation succeeded, CMTS_SYNC_OBJECT_EXPIRED if the event has already been signaled.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_event_signal(cmts_event* event);
/** Waits for the event to become signaled, putting the current task to sleep.
* @param event A valid pointer to a cmts_event object.
* @return CMTS_OK if the operation succeeded, CMTS_SYNC_OBJECT_EXPIRED if the event had already been signaled.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_event_await(cmts_event* event);
/** Attempts to reset the event, allowing new tasks to await on it.
* @param event A valid pointer to a cmts_event object.
* @return CMTS_OK if the operation succeeded, CMTS_NOT_READY if the event still has tasks waiting for it to become signaled.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_event_reset(cmts_event* event);

/** Initializes a counter object.
* @param counter A valid pointer to a cmts_counter object.
* @param start_value The initial value of the counter.
* @note This function is not thread-safe.
*/
CMTS_ATTR void CMTS_CALL cmts_counter_init(cmts_counter* counter, uint64_t start_value);
/** Retrieves the value of a counter.
* @param counter A valid pointer to a cmts_counter object.
* @return The current value of the counter.
*/
CMTS_ATTR uint64_t CMTS_CALL cmts_counter_value(const cmts_counter* counter);
/** Retrieves the state of a counter.
* @param counter A valid pointer to a cmts_counter object.
* @return The state of the counter. Possible results:
* @li CMTS_OK if the counter has already reached zero (signaled).
* @li CMTS_SYNC_OBJECT_EXPIRED if the event has already been signaled.
* @li CMTS_NOT_READY if no task is waiting for the counter to become signaled.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_counter_state(const cmts_counter* counter);
/** Atomically increments the value of the counter.
* @param counter A valid pointer to a cmts_counter object.
* @return CMTS_OK if the operation succeeded, CMTS_SYNC_OBJECT_EXPIRED if the counter has already reached zero.
* @note WARNING: Incrementing a counter after it reached zero leads to UB.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_counter_increment(cmts_counter* counter);
/** Atomically decrements the value of the counter. If it reaches zero all waiting tasks are resumed.
* @param counter A valid pointer to a cmts_counter object.
* @return CMTS_OK if the operation succeeded, CMTS_SYNC_OBJECT_EXPIRED if the counter has already reached zero.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_counter_decrement(cmts_counter* counter);
/** Attempts to wait for the counter to reach zero, putting the current task to sleep.
* @param counter A valid pointer to a cmts_counter object.
* @return CMTS_OK if the operation succeeded, CMTS_SYNC_OBJECT_EXPIRED if the counter had already reached zero.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_counter_await(cmts_counter* counter);
/** Attempts to reset the counter, allowing new tasks to await on it.
* @param counter A valid pointer to a cmts_counter object.
* @return CMTS_OK if the operation succeeded, CMTS_NOT_READY if the counter still has tasks waiting for it to reach zero.
*/
CMTS_ATTR cmts_result CMTS_CALL cmts_counter_reset(cmts_counter* counter, uint64_t new_start_value);

/** Initializes a mutex object.
* @param mutex A valid pointer to a cmts_mutex object.
*/
CMTS_ATTR void CMTS_CALL cmts_mutex_init(cmts_mutex* mutex);
/** Checks whether a mutex is locked.
* @param mutex A valid pointer to a cmts_mutex object.
* @return Whether the mutex is locked.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_mutex_is_locked(const cmts_mutex* mutex);
/** Attempts to lock the mutex.
* @param mutex A valid pointer to a cmts_mutex object.
* @return Whether the mutex was successfully acquired.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_mutex_try_lock(cmts_mutex* mutex);
/** Attempts to lock the mutex, otherwise blocks until it becomes available.
* @param mutex A valid pointer to a cmts_mutex object.
*/
CMTS_ATTR void CMTS_CALL cmts_mutex_lock(cmts_mutex* mutex);
/** Releases a previously acquired mutex.
* @param mutex A valid pointer to a cmts_mutex object.
*/
CMTS_ATTR void CMTS_CALL cmts_mutex_unlock(cmts_mutex* mutex);

/** Begins a CMTS RCU reader critical section.
* Until cmts_rcu_read_end is invoked, suspending execution of the current task is forbidden.
* @note No-op in release mode.
*/
CMTS_ATTR void CMTS_CALL cmts_rcu_read_begin();
/** Ends a CMTS RCU reader critical section.
* @note No-op in release mode.
*/
CMTS_ATTR void CMTS_CALL cmts_rcu_read_end();
/** Performs a relatively naive RCU synchronize step.
* This function blocks until all RCU reader critical sections have ended.
*/
CMTS_ATTR void CMTS_CALL cmts_rcu_sync();
/** Retrieves the memory requirements of a CMTS RCU snapshot.
* @param out_requirements A valid pointer to a cmts_memory_requirements record, which will be filled with the required size and alignment of an RCU snapshot.
*/
CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot_requirements(cmts_memory_requirements* out_requirements);
/** Fills a buffer with some information of the current state of the scheduler.
* @param snapshot_buffer A pointer to the buffer to fill with snapshot information. This buffer must fulfill the requirements specified by cmts_rcu_snapshot_requirements.
*/
CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot(void* snapshot_buffer);
/** Attempts to synchronize using an RCU snaphost.
* @param snapshot_buffer A pointer to a valid snapshot buffer.
* @param prior_result 0 or the value that a prior invocation of cmts_rcu_try_snapshot_sync returned.
* @return The number of worker threads that have finished executing an RCU critical section. If this value matches the number of worker threads then synchronization has finished.
*/
CMTS_ATTR uint32_t CMTS_CALL cmts_rcu_try_snapshot_sync(const void* snapshot_buffer, uint32_t prior_result);
/** Synchronizes using an RCU snaphost.
* @param snapshot_buffer A pointer to a valid snapshot buffer.
*/
CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot_sync(const void* snapshot_buffer);

/** Retrieves the memory requirements of a cmts_hazard_context.
* @param out_requirements A valid pointer to a cmts_memory_requirements record, which will be filled with the required size and alignment of a cmts_hazard_context.
*/
CMTS_ATTR void CMTS_CALL cmts_hazard_ptr_requirements(cmts_memory_requirements* out_requirements);
/** Initializes a cmts_hazard_context.
* @param hctx A valid pointer to a cmts_hazard_context object.
* @param buffer A pointer to a block of memory with the required size and alignment.
*/
CMTS_ATTR void CMTS_CALL cmts_hazard_ptr_init(cmts_hazard_context* hctx, void* buffer);
/** Protects the specified pointer.
* Until cmts_hazard_ptr_release is invoked, suspending execution of the current task is forbidden.
* @param hctx A valid pointer to a cmts_hazard_context object.
* @param ptr The pointer to protect.
*/
CMTS_ATTR void CMTS_CALL cmts_hazard_ptr_protect(cmts_hazard_context* hctx, void* ptr);
/** Stops protecting the pointer that was previously passed to cmts_hazard_ptr_protect.
* @param hctx A valid pointer to a cmts_hazard_context object.
*/
CMTS_ATTR void CMTS_CALL cmts_hazard_ptr_release(cmts_hazard_context* hctx);
/** Retrieves the current hazard pointer.
* @param hctx A valid pointer to a cmts_hazard_context object.
*/
CMTS_ATTR void* CMTS_CALL cmts_hazard_ptr_get(cmts_hazard_context* hctx);
/** Checks whether a pointer is in use by other tasks.
* @param hctx A valid pointer to a cmts_hazard_context object.
* @param ptr The pointer to check.
* @return Whether the specified pointer is reachable by other tasks.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_hazard_ptr_is_unreachable(const cmts_hazard_context* hctx, const void* ptr);

/**
* @return The number of physical processors.
*/
CMTS_ATTR size_t CMTS_CALL cmts_processor_count();
/** 
* @return The index of the current physical processor.
*/
CMTS_ATTR size_t CMTS_CALL cmts_this_processor_index();
/**
* @return The default size of the stack of a task
*/
CMTS_ATTR size_t CMTS_CALL cmts_default_task_stack_size();

#ifdef CMTS_FORMAT_RESULT
/** Formats a CMTS error code.
* @param result The error code.
* @param out_size Either NULL or a valid pointer to a variable that receives the length of the returned string.
* @return A pointer to a read-only null-terminated string with error code as text.
*/
CMTS_ATTR const CMTS_CHAR* CMTS_CALL cmts_format_result(cmts_result result, size_t* out_size);
#endif

/** Enables or disables the yield trap.
* [DEBUG ONLY] If enabled, any operation that results in the current task being suspended will crash the program.
* @param enable Whether to enable the trap.
* @return The previous state of the trap.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_ext_debug_enable_yield_trap(cmts_bool enable);

/**
* @return Whether the CMTS debugger is enabled. Always CMTS_FALSE in release mode.
*/
CMTS_ATTR cmts_bool CMTS_CALL cmts_ext_debug_enabled();
/** Prints a message to the debugger.
* @param message The message information (text, length, severity, etc).
*/
CMTS_ATTR void CMTS_CALL cmts_ext_debug_write(const cmts_ext_debug_message* message);

}
#endif



#ifdef CMTS_IMPLEMENTATION
#define CMTS_ROUND_CACHE_LINE_SIZE(VALUE) ((VALUE) + (CMTS_CACHE_LINE_SIZE - 1)) & (~(CMTS_CACHE_LINE_SIZE - 1))
#define CMTS_MAKE_HANDLE(INDEX, GENERATION) ((uint64_t)(INDEX) | ((uint64_t)(GENERATION) << 32))
#define CMTS_BREAK_HANDLE(H, OUT_INDEX, OUT_GENERATION) OUT_INDEX = (uint32_t)(H); OUT_GENERATION = (uint32_t)((H) >> 32)
#define CMTS_ARRAY_SIZE(ARRAY) (sizeof((ARRAY)) / sizeof(CMTS_CHAR))
#define CMTS_STRING_SIZE(STRING) (CMTS_ARRAY_SIZE(STRING) - 1)

#if defined(__GNUC__) || defined(__clang__)
#define CMTS_GCC_OR_CLANG
#ifdef __alpha__
#define CMTS_ARCH_ALPHA
#elif defined(__x86_64__)
#define CMTS_ARCH_X64
#elif defined(__arm__)
#define CMTS_ARCH_ARM
#ifdef __thumb__
#define CMTS_ARCH_ARM_THUMB
#endif
#elif defined(__i386__)
#define CMTS_ARCH_X86
#elif defined(__ia64__)
#define CMTS_ARCH_ITANIUM
#elif defined(__powerpc__)
#define CMTS_ARCH_POWERPC
#endif
#define CMTS_THREAD_LOCAL(TYPE) __thread TYPE
#define CMTS_ALIGNAS(K) __attribute__((aligned(K)))
#define CMTS_INLINE_ALWAYS __attribute__((always_inline))
#define CMTS_INLINE_NEVER __attribute__((noinline))
#define CMTS_LIKELY_IF(CONDITION) if (__builtin_expect((CONDITION), 1))
#define CMTS_UNLIKELY_IF(CONDITION) if (__builtin_expect((CONDITION), 0))
#define CMTS_ASSUME(CONDITION) __builtin_assume((CONDITION))
#ifdef CMTS_ARCH_ARM
#define CMTS_SPIN_WAIT __yield()
#elif defined(CMTS_ARCH_X64)
#define CMTS_SPIN_WAIT __builtin_ia32_pause()
#else
#define CMTS_SPIN_WAIT
#endif
#define CMTS_POPCNT32(MASK) ((uint8_t)__builtin_popcount((MASK)))
#define CMTS_POPCNT64(MASK) ((uint8_t)__builtin_popcountll((MASK)))
#define CMTS_CLZ32(MASK) ((uint8_t)__builtin_clz((MASK)))
#define CMTS_CLZ64(MASK) ((uint8_t)__builtin_clzll((MASK)))
#define CMTS_ROL32(MASK, COUNT) (((MASK) << (COUNT)) | ((MASK) >> (32 - (COUNT))))
#define CMTS_ROL64(MASK, COUNT) (((MASK) >> (COUNT)) | ((MASK) << (64 - (COUNT))))
#define CMTS_ROR32(MASK, COUNT) (((MASK) << (COUNT)) | ((MASK) >> (64 - (COUNT))))
#define CMTS_ROR64(MASK, COUNT) (((MASK) >> (COUNT)) | ((MASK) << (32 - (COUNT))))
#define CMTS_ATOMIC(TYPE) TYPE
#define CMTS_ATOMIC_LOAD_ACQ_U8(TARGET) (uint8_t)__atomic_load_n(((CMTS_ATOMIC(uint8_t)*)(TARGET)), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_LOAD_ACQ_U32(TARGET) (uint32_t)__atomic_load_n(((CMTS_ATOMIC(uint32_t)*)(TARGET)), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_LOAD_ACQ_U64(TARGET) (uint64_t)__atomic_load_n(((CMTS_ATOMIC(uint64_t)*)(TARGET)), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_STORE_REL_U8(TARGET, VALUE) __atomic_store_n(((CMTS_ATOMIC(uint8_t)*)(TARGET)), (VALUE), __ATOMIC_RELEASE)
#define CMTS_ATOMIC_STORE_REL_U32(TARGET, VALUE) __atomic_store_n(((CMTS_ATOMIC(uint32_t)*)(TARGET)), (VALUE), __ATOMIC_RELEASE)
#define CMTS_ATOMIC_STORE_REL_U64(TARGET, VALUE) __atomic_store_n(((CMTS_ATOMIC(uint64_t)*)(TARGET)), (VALUE), __ATOMIC_RELEASE)
#define CMTS_ATOMIC_XCHG_ACQ_U32(TARGET, VALUE) (uint32_t)__atomic_exchange_n(((CMTS_ATOMIC(uint32_t)*)(TARGET)), (VALUE), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_XCHG_ACQ_U64(TARGET, VALUE) (uint64_t)__atomic_exchange_n(((CMTS_ATOMIC(uint64_t)*)(TARGET)), (VALUE), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_XCHG_REL_U32(TARGET, VALUE) (uint32_t)__atomic_exchange_n(((CMTS_ATOMIC(uint32_t)*)(TARGET)), (VALUE), __ATOMIC_RELEASE)
#define CMTS_ATOMIC_XCHG_REL_U64(TARGET, VALUE) (uint64_t)__atomic_exchange_n(((CMTS_ATOMIC(uint64_t)*)(TARGET)), (VALUE), __ATOMIC_RELEASE)
#define CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U8(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint8_t)*)(TARGET)), (EXPECTED), (VALUE), 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U32(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint32_t)*)(TARGET)), (EXPECTED), (VALUE), 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U64(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint64_t)*)(TARGET)), (EXPECTED), (VALUE), 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_STRONG_REL_U8(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint8_t)*)(TARGET)), (EXPECTED), (VALUE), 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_STRONG_REL_U32(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint32_t)*)(TARGET)), (EXPECTED), (VALUE), 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_STRONG_REL_U64(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint64_t)*)(TARGET)), (EXPECTED), (VALUE), 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_ACQ_U8(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint8_t)*)(TARGET)), (EXPECTED), (VALUE), 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_ACQ_U32(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint32_t)*)(TARGET)), (EXPECTED), (VALUE), 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_ACQ_U64(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint64_t)*)(TARGET)), (EXPECTED), (VALUE), 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_REL_U8(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint8_t)*)(TARGET)), (EXPECTED), (VALUE), 1, __ATOMIC_RELEASE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_REL_U32(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint32_t)*)(TARGET)), (EXPECTED), (VALUE), 1, __ATOMIC_RELEASE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_CMPXCHG_REL_U64(TARGET, EXPECTED, VALUE) __atomic_compare_exchange_n(((CMTS_ATOMIC(uint64_t)*)(TARGET)), (EXPECTED), (VALUE), 1, __ATOMIC_RELEASE, __ATOMIC_RELAXED)
#define CMTS_ATOMIC_INCREMENT_ACQ_U32(TARGET) __atomic_fetch_add(((CMTS_ATOMIC(uint32_t)*)(TARGET)), UINT32_C(1), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_INCREMENT_ACQ_U64(TARGET) __atomic_fetch_add(((CMTS_ATOMIC(uint64_t)*)(TARGET)), UINT64_C(1), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_INCREMENT_REL_U32(TARGET) __atomic_fetch_add(((CMTS_ATOMIC(uint32_t)*)(TARGET)), UINT32_C(1), __ATOMIC_RELEASE)
#define CMTS_ATOMIC_INCREMENT_REL_U64(TARGET) __atomic_fetch_add(((CMTS_ATOMIC(uint64_t)*)(TARGET)), UINT64_C(1), __ATOMIC_RELEASE)
#define CMTS_ATOMIC_DECREMENT_ACQ_U32(TARGET) __atomic_fetch_sub(((CMTS_ATOMIC(uint32_t)*)(TARGET)), UINT32_C(1), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_DECREMENT_ACQ_U64(TARGET) __atomic_fetch_sub(((CMTS_ATOMIC(uint64_t)*)(TARGET)), UINT64_C(1), __ATOMIC_ACQUIRE)
#define CMTS_ATOMIC_DECREMENT_REL_U32(TARGET) __atomic_fetch_sub(((CMTS_ATOMIC(uint32_t)*)(TARGET)), UINT32_C(1), __ATOMIC_RELEASE)
#define CMTS_ATOMIC_DECREMENT_REL_U64(TARGET) __atomic_fetch_sub(((CMTS_ATOMIC(uint64_t)*)(TARGET)), UINT64_C(1), __ATOMIC_RELEASE)
#elif defined(_MSC_VER) || defined(_MSVC_LANG)
#include <intrin.h>
#define CMTS_MSVC
#ifdef _M_ALPHA
#define CMTS_ARCH_ALPHA
#elif defined(_M_AMD64)
#define CMTS_ARCH_X64
#elif defined(_M_ARM)
#define CMTS_ARCH_ARM
#ifdef _M_ARMT
#define CMTS_ARCH_ARM_THUMB
#endif
#elif defined(_M_IX86)
#define CMTS_ARCH_X86
#elif defined(_M_IA64)
#define CMTS_ARCH_ITANIUM
#elif defined(_M_PPC)
#define CMTS_ARCH_POWERPC
#endif
#define CMTS_THREAD_LOCAL(TYPE) __declspec(thread) TYPE
#define CMTS_ALIGNAS(K) __declspec(align(K))
#define CMTS_INLINE_ALWAYS __forceinline
#define CMTS_INLINE_NEVER __declspec(noinline)
#define CMTS_LIKELY_IF(CONDITION) if ((CONDITION))
#define CMTS_UNLIKELY_IF(CONDITION) if ((CONDITION))
#define CMTS_ASSUME(CONDITION) __assume((CONDITION))
#define CMTS_POPCNT32(MASK) ((uint8_t)__popcnt((MASK)))
#define CMTS_POPCNT64(MASK) ((uint8_t)__popcnt64((MASK)))
#define CMTS_CLZ32(MASK) ((uint8_t)__lzcnt((MASK)))
#define CMTS_CLZ64(MASK) ((uint8_t)__lzcnt64((MASK)))
#define CMTS_ROL32(MASK, COUNT) _rotl((MASK), (COUNT))
#define CMTS_ROL64(MASK, COUNT) _rotl64((MASK), (COUNT))
#define CMTS_ROR32(MASK, COUNT) _rotr((MASK), (COUNT))
#define CMTS_ROR64(MASK, COUNT) _rotr64((MASK), (COUNT))
#ifdef CMTS_ARCH_ARM
#define CMTS_SPIN_WAIT __yield()
#define CMTS_MSVC_ATOMIC_ACQ_SUFFIX(NAME) NAME##_acq
#define CMTS_MSVC_ATOMIC_REL_SUFFIX(NAME) NAME##_acq
#elif defined(CMTS_ARCH_X86) || defined(CMTS_ARCH_X64)
#define CMTS_SPIN_WAIT _mm_pause()
#define CMTS_MSVC_ATOMIC_ACQ_SUFFIX(NAME) NAME
#define CMTS_MSVC_ATOMIC_REL_SUFFIX(NAME) NAME
#else
#define CMTS_SPIN_WAIT
#define CMTS_MSVC_ATOMIC_ACQ_SUFFIX(NAME) NAME
#define CMTS_MSVC_ATOMIC_REL_SUFFIX(NAME) NAME
#endif
#define CMTS_ATOMIC(TYPE) volatile TYPE
#define CMTS_ATOMIC_LOAD_ACQ_U8(TARGET) (uint8_t)CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedOr8)((volatile char*)(TARGET), '\0')
#define CMTS_ATOMIC_LOAD_ACQ_U32(TARGET) (uint32_t)CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedOr)((volatile long*)(TARGET), (long)0)
#define CMTS_ATOMIC_LOAD_ACQ_U64(TARGET) (uint64_t)CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedOr64)((volatile long long*)(TARGET), (long long)0)
#define CMTS_ATOMIC_STORE_REL_U8(TARGET, VALUE) (void)CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedExchange8)((volatile char*)(TARGET), (char)(VALUE))
#define CMTS_ATOMIC_STORE_REL_U32(TARGET, VALUE) (void)CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedExchange)((volatile long*)(TARGET), (long)(VALUE))
#define CMTS_ATOMIC_STORE_REL_U64(TARGET, VALUE) (void)CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedExchange64)((volatile long long*)(TARGET), (long long)(VALUE))
#define CMTS_ATOMIC_XCHG_ACQ_U32(TARGET, VALUE) (uint32_t)CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedExchange)((volatile long*)(TARGET), (long)(VALUE))
#define CMTS_ATOMIC_XCHG_ACQ_U64(TARGET, VALUE) (uint64_t)CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedExchange64)((volatile long long*)(TARGET), (long long)(VALUE))
#define CMTS_ATOMIC_XCHG_REL_U32(TARGET, VALUE) (uint32_t)CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedExchange)((volatile long*)(TARGET), (long)(VALUE))
#define CMTS_ATOMIC_XCHG_REL_U64(TARGET, VALUE) (uint64_t)CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedExchange64)((volatile long long*)(TARGET), (long long)(VALUE))
#define CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U8(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedCompareExchange8)((volatile char*)(TARGET), (char)(VALUE), *(char*)(EXPECTED)) == *(char*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U32(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedCompareExchange)((volatile long*)(TARGET), (long)(VALUE), *(long*)(EXPECTED)) == *(long*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U64(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedCompareExchange64)((volatile long long*)(TARGET), (long long)(VALUE), *(long long*)(EXPECTED)) == *(long long*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_STRONG_REL_U8(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedCompareExchange8)((volatile char*)(TARGET), (char)(VALUE), *(char*)(EXPECTED)) == *(char*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_STRONG_REL_U32(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedCompareExchange)((volatile long*)(TARGET), (long)(VALUE), *(long*)(EXPECTED)) == *(long*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_STRONG_REL_U64(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedCompareExchange64)((volatile long long*)(TARGET), (long long)(VALUE), *(long long*)(EXPECTED)) == *(long long*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_ACQ_U8(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedCompareExchange8)((volatile char*)(TARGET), (char)(VALUE), *(char*)(EXPECTED)) == *(char*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_ACQ_U32(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedCompareExchange)((volatile long*)(TARGET), (long)(VALUE), *(long*)(EXPECTED)) == *(long*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_ACQ_U64(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedCompareExchange64)((volatile long long*)(TARGET), (long long)(VALUE), *(long long*)(EXPECTED)) == *(long long*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_REL_U8(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedCompareExchange8)((volatile char*)(TARGET), (char)(VALUE), *(char*)(EXPECTED)) == *(char*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_REL_U32(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedCompareExchange)((volatile long*)(TARGET), (long)(VALUE), *(long*)(EXPECTED)) == *(long*)(EXPECTED))
#define CMTS_ATOMIC_CMPXCHG_REL_U64(TARGET, EXPECTED, VALUE) (CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedCompareExchange64)((volatile long long*)(TARGET), (long long)(VALUE), *(long long*)(EXPECTED)) == *(long long*)(EXPECTED))
#define CMTS_ATOMIC_INCREMENT_ACQ_U32(TARGET) ((uint32_t)(CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedIncrement)((volatile long*)(TARGET))) - 1)
#define CMTS_ATOMIC_INCREMENT_ACQ_U64(TARGET) ((uint64_t)(CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedIncrement64)((volatile long long*)(TARGET))) - 1)
#define CMTS_ATOMIC_INCREMENT_REL_U32(TARGET) ((uint32_t)(CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedIncrement)((volatile long*)(TARGET))) - 1)
#define CMTS_ATOMIC_INCREMENT_REL_U64(TARGET) ((uint64_t)(CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedIncrement64)((volatile long long*)(TARGET))) - 1)
#define CMTS_ATOMIC_DECREMENT_ACQ_U32(TARGET) ((uint32_t)(CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedDecrement)((volatile long*)(TARGET))) + 1)
#define CMTS_ATOMIC_DECREMENT_ACQ_U64(TARGET) ((uint64_t)(CMTS_MSVC_ATOMIC_ACQ_SUFFIX(_InterlockedDecrement64)((volatile long long*)(TARGET))) + 1)
#define CMTS_ATOMIC_DECREMENT_REL_U32(TARGET) ((uint32_t)(CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedDecrement)((volatile long*)(TARGET))) + 1)
#define CMTS_ATOMIC_DECREMENT_REL_U64(TARGET) ((uint64_t)(CMTS_MSVC_ATOMIC_REL_SUFFIX(_InterlockedDecrement64)((volatile long long*)(TARGET))) + 1)
#endif

#if UINTPTR_MAX == UINT32_MAX
#define CMTS_ATOMIC_LOAD_ACQ_UPTR			CMTS_ATOMIC_LOAD_ACQ_U32
#define CMTS_ATOMIC_STORE_REL_UPTR			CMTS_ATOMIC_STORE_REL_U32
#define CMTS_ATOMIC_XCHG_ACQ_UPTR			CMTS_ATOMIC_XCHG_ACQ_U32
#define CMTS_ATOMIC_XCHG_REL_UPTR			CMTS_ATOMIC_XCHG_REL_U32
#define CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_UPTR	CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U32
#define CMTS_ATOMIC_CMPXCHG_STRONG_REL_UPTR	CMTS_ATOMIC_CMPXCHG_STRONG_REL_U32
#else
#define CMTS_ATOMIC_LOAD_ACQ_UPTR			CMTS_ATOMIC_LOAD_ACQ_U64
#define CMTS_ATOMIC_STORE_REL_UPTR			CMTS_ATOMIC_STORE_REL_U64
#define CMTS_ATOMIC_XCHG_ACQ_UPTR			CMTS_ATOMIC_XCHG_ACQ_U64
#define CMTS_ATOMIC_XCHG_REL_UPTR			CMTS_ATOMIC_XCHG_REL_U64
#define CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_UPTR	CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U64
#define CMTS_ATOMIC_CMPXCHG_STRONG_REL_UPTR	CMTS_ATOMIC_CMPXCHG_STRONG_REL_U64
#endif

#define CMTS_SHARED_ATTR CMTS_ALIGNAS(CMTS_CACHE_LINE_SIZE)
#define CMTS_SPIN_LOOP for (;; CMTS_SPIN_WAIT)

#if defined(_DEBUG) || defined(NDEBUG)
#define CMTS_DEBUG
#include <assert.h>
#define CMTS_ASSERT(CONDITION) assert(CONDITION)
#define CMTS_INVARIANT(CONDITION) CMTS_ASSERT(CONDITION)
#else
#define CMTS_ASSERT(CONDITION)
#define CMTS_INVARIANT(CONDITION) CMTS_ASSUME(CONDITION)
#endif

#ifdef CMTS_DEBUG
static void* debugger_context;
static cmts_fn_debugger_message debugger_callback;

CMTS_INLINE_NEVER static void cmts_debug_message(cmts_ext_debug_message_severity severity, const CMTS_CHAR * text, size_t size)
{
	CMTS_UNLIKELY_IF(debugger_callback == NULL)
		return;
	cmts_ext_debug_message message;
	message.next_ext = NULL;
	message.message = text;
	message.message_length = size;
	message.severity = severity;
	debugger_callback(debugger_context, &message);
}

#define CMTS_REPORT_INFO(MESSAGE) cmts_debug_message(CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_INFO, MESSAGE, CMTS_STRING_SIZE(MESSAGE))
#define CMTS_REPORT_WARNING(MESSAGE) cmts_debug_message(CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_WARNING, MESSAGE, CMTS_STRING_SIZE(MESSAGE))
#define CMTS_REPORT_ERROR(MESSAGE) cmts_debug_message(CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_ERROR, MESSAGE, CMTS_STRING_SIZE(MESSAGE))
#endif

#ifdef _WIN32
#define CMTS_TARGET_WINDOWS
#include <Windows.h>
#include <bcrypt.h>
typedef HANDLE thread_type;
typedef DWORD thread_return_type;
#define CMTS_THREAD_CALLING_CONVENTION __stdcall

#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
typedef BOOL (WINAPI *WaitOnAddress_t)(volatile VOID* Address, PVOID CompareAddress, SIZE_T AddressSize, DWORD dwMilliseconds);
typedef VOID (WINAPI *WakeByAddressSingle_t)(PVOID Address);
static HMODULE sync_library;
static WaitOnAddress_t wait_on_address;
static WakeByAddressSingle_t wake_by_address_single;
#endif
static uint64_t qpc_frequency;

CMTS_INLINE_ALWAYS static cmts_bool cmts_os_init()
{
	LARGE_INTEGER k;
	(void)QueryPerformanceFrequency(&k);
	qpc_frequency = k.QuadPart;
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
	sync_library = GetModuleHandle(TEXT("Synchronization.lib"));
	CMTS_UNLIKELY_IF(sync_library == NULL)
	{
		sync_library = GetModuleHandle(TEXT("API-MS-Win-Core-Synch-l1-2-0.dll"));
		CMTS_REPORT_ERROR("\"GetModuleHandle(\"Synchronization.lib\")\" returned NULL. Attempting the same with \"API-MS-Win-Core-Synch-l1-2-0.dll\".");
		CMTS_UNLIKELY_IF(sync_library == NULL)
		{
			CMTS_REPORT_ERROR("\"GetModuleHandle(\"API-MS-Win-Core-Synch-l1-2-0.dll\")\" returned NULL. Library initialization failed.");
			return CMTS_FALSE;
		}
		wait_on_address = (WaitOnAddress_t)GetProcAddress(sync_library, "WaitOnAddress");
		CMTS_UNLIKELY_IF(wait_on_address == NULL)
			return CMTS_FALSE;
		wake_by_address_single = (WakeByAddressSingle_t)GetProcAddress(sync_library, "WakeByAddressSingle");
		CMTS_UNLIKELY_IF(wake_by_address_single == NULL)
			return CMTS_FALSE;
	}
#endif
	return CMTS_TRUE;
}

CMTS_INLINE_ALWAYS static void* cmts_os_malloc(size_t size)
{
	return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

CMTS_INLINE_ALWAYS static cmts_bool cmts_os_free(void* ptr, size_t size)
{
	return VirtualFree(ptr, 0, MEM_RELEASE);
}

CMTS_INLINE_ALWAYS static cmts_result cmts_os_init_threads(thread_type* threads, size_t count, size_t stack_size, LPTHREAD_START_ROUTINE function)
{
	GROUP_AFFINITY desired, prior;
	size_t i;
	(void)memset(desired.Reserved, 0, sizeof(desired.Reserved));
	for (i = 0; i != count; ++i)
	{
		threads[i] = CreateThread(NULL, stack_size, function, (LPVOID)i, CREATE_SUSPENDED, NULL);
		CMTS_UNLIKELY_IF(threads[i] == NULL)
			return CMTS_ERROR_THREAD_CREATION;
		desired.Group = (WORD)(i >> 6);
		desired.Mask = 1ULL << ((uint8_t)i & 63);
		CMTS_UNLIKELY_IF(!SetThreadGroupAffinity(threads[i], &desired, &prior))
			return CMTS_ERROR_THREAD_AFFINITY;
		CMTS_UNLIKELY_IF(ResumeThread(threads[i]) == MAXDWORD)
			return CMTS_ERROR_RESUME_THREAD;
	}
	return CMTS_OK;
}

CMTS_INLINE_ALWAYS static cmts_result cmts_os_init_threads_custom(thread_type* threads, uint_fast32_t count, size_t stack_size, LPTHREAD_START_ROUTINE function, const uint32_t* affinities)
{
	GROUP_AFFINITY desired, prior;
	uint32_t i;
	(void)memset(desired.Reserved, 0, sizeof(desired.Reserved));
	for (i = 0; i != count; ++i)
	{
		threads[i] = CreateThread(NULL, stack_size, function, (LPVOID)(size_t)i, CREATE_SUSPENDED, NULL);
		CMTS_UNLIKELY_IF(threads[i] == NULL)
			return CMTS_ERROR_THREAD_CREATION;
		desired.Group = affinities[i] >> 6;
		desired.Mask = 1ULL << (affinities[i] & 63);
		CMTS_UNLIKELY_IF(!SetThreadGroupAffinity(threads[i], &desired, &prior))
			return CMTS_ERROR_THREAD_AFFINITY;
		CMTS_UNLIKELY_IF(ResumeThread(threads[i]) == MAXDWORD)
			return CMTS_ERROR_RESUME_THREAD;
	}
	return CMTS_OK;
}

CMTS_INLINE_ALWAYS static cmts_bool cmts_os_await_threads(thread_type* threads, uint_fast32_t count)
{
	return (cmts_bool)(WaitForMultipleObjects(count, threads, TRUE, INFINITE) == WAIT_OBJECT_0);
}

CMTS_INLINE_ALWAYS static void cmts_os_exit_thread(uint_fast32_t code)
{
	ExitThread(code);
}

CMTS_INLINE_ALWAYS static cmts_bool cmts_os_pause_threads(thread_type* threads, uint_fast32_t count)
{
	size_t i;
	for (i = 0; i != count; ++i)
		CMTS_UNLIKELY_IF(SuspendThread(threads[i]) == MAXDWORD)
			return CMTS_FALSE;
	return CMTS_TRUE;
}

CMTS_INLINE_ALWAYS static cmts_bool cmts_os_resume_threads(thread_type* threads, uint_fast32_t count)
{
	size_t i;
	for (i = 0; i != count; ++i)
		CMTS_UNLIKELY_IF(ResumeThread(threads[i]) == MAXDWORD)
			return CMTS_FALSE;
	return CMTS_TRUE;
}

CMTS_INLINE_ALWAYS static cmts_bool cmts_os_terminate_threads(thread_type* threads, uint_fast32_t count)
{
	size_t i;
	for (i = 0; i != count; ++i)
		CMTS_UNLIKELY_IF(!TerminateThread(threads[i], MAXDWORD))
			return CMTS_FALSE;
	return CMTS_TRUE;
}

CMTS_INLINE_ALWAYS static void cmts_os_futex_signal(volatile void* ptr)
{
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
	wake_by_address_single((PVOID)ptr);
#endif
}

CMTS_INLINE_ALWAYS static void cmts_os_futex_await(volatile void* ptr, void* prior, uint_fast8_t size)
{
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
	(void)wait_on_address(ptr, prior, size, INFINITE);
#endif
}

CMTS_INLINE_ALWAYS static uint64_t cmts_os_time()
{
	LARGE_INTEGER k;
	(void)QueryPerformanceCounter(&k);
	return k.QuadPart;
}

CMTS_INLINE_ALWAYS static uint64_t cmts_os_time_ns(uint64_t timestamp)
{
	return timestamp / qpc_frequency;
}

CMTS_INLINE_ALWAYS static cmts_bool cmts_os_csprng(void* out, size_t out_size)
{
	typedef NTSTATUS(WINAPI* BCryptGenRandom_T)(BCRYPT_ALG_HANDLE, PUCHAR, ULONG, ULONG);
	HMODULE lib;
	lib = GetModuleHandle(TEXT("Bcrypt.lib"));
	CMTS_UNLIKELY_IF(lib == NULL)
		lib = GetModuleHandle(TEXT("Bcrypt.dll"));
	CMTS_UNLIKELY_IF(lib == NULL)
		return CMTS_FALSE;
	return BCRYPT_SUCCESS(((BCryptGenRandom_T)GetProcAddress(lib, "BCryptGenRandom"))(NULL, (PUCHAR)out, (ULONG)out_size, (ULONG)BCRYPT_USE_SYSTEM_PREFERRED_RNG));
}

typedef PVOID cmts_context;

CMTS_INLINE_ALWAYS static cmts_bool cmts_context_init(cmts_context* ctx, cmts_fn_task function, void* param, size_t stack_size)
{
	CMTS_INVARIANT(function != NULL);
	CMTS_INVARIANT(param != NULL);
	*ctx = CreateFiberEx(stack_size, stack_size, FIBER_FLAG_FLOAT_SWITCH, function, param); return *ctx != NULL;
	return *ctx != NULL;
}

CMTS_INLINE_ALWAYS static cmts_bool cmts_context_is_valid(cmts_context* ctx)
{
	return *ctx != NULL;
}

CMTS_INLINE_ALWAYS static void cmts_context_switch(cmts_context* from, cmts_context* to)
{
	CMTS_INVARIANT(*from != NULL);
	CMTS_INVARIANT(*to != NULL);
	SwitchToFiber(*to);
}

CMTS_INLINE_ALWAYS static void cmts_context_wipe(cmts_context* ctx)
{
	*ctx = NULL;
}

CMTS_INLINE_ALWAYS static void cmts_context_delete(cmts_context* ctx)
{
	CMTS_INVARIANT(*ctx != NULL);
	DeleteFiber(*ctx);
	cmts_context_wipe(ctx);
}
#endif

enum
{
	CMTS_TASK_STATE_INACTIVE,
	CMTS_TASK_STATE_RUNNING,
	CMTS_TASK_STATE_GOING_TO_SLEEP,
	CMTS_TASK_STATE_SLEEPING,
};

typedef uint32_t(*cmts_fn_remainder)(uint32_t);

typedef struct cmts_ext_header
{
	const cmts_ext_header* next;
	cmts_ext_type type;
} cmts_ext_header;

typedef struct cmts_romu2jr
{
	uint64_t x, y;
} cmts_romu2jr;

typedef struct cmts_index_generation_pair
{
	uint32_t index, generation;
} cmts_index_generation_pair;

typedef union cmts_index_generation_pair_union
{
	cmts_index_generation_pair pair;
	uint64_t packed;
} cmts_index_generation_pair_union;

typedef struct cmts_task_state
{
	cmts_context ctx;
	cmts_fn_task fn;
	void* param;
	void* sync_object;
	uint32_t next;
	uint32_t generation;
	uint32_t thread_affinity;
	uint8_t priority;
	uint8_t sync_type;
	CMTS_ATOMIC(uint8_t) state;
} cmts_task_state;

typedef struct cmts_task_queue
{
	uint32_t tail;
	CMTS_ATOMIC(uint32_t)* values;
	CMTS_SHARED_ATTR CMTS_ATOMIC(uint32_t) count;
	CMTS_SHARED_ATTR CMTS_ATOMIC(uint32_t) head;
} cmts_task_queue;

typedef struct cmts_wait_queue_data
{
	uint32_t head, tail;
} cmts_wait_queue_data;

typedef union cmts_wait_queue_union
{
	uint64_t packed;
	cmts_wait_queue_data queue;
} cmts_wait_queue_union;

typedef CMTS_ATOMIC(uint32_t) cmts_fence_data;
typedef CMTS_ATOMIC(uint64_t) cmts_event_data;
typedef CMTS_ATOMIC(uint32_t) cmts_mutex_data;

typedef struct cmts_counter_data
{
	CMTS_ATOMIC(uint64_t) queue;
	CMTS_ATOMIC(uint64_t) value;
} cmts_counter_data;

typedef struct cmts_cache_aligned_atomic_counter
{
	CMTS_SHARED_ATTR CMTS_ATOMIC(uint32_t) value;
} cmts_cache_aligned_atomic_counter;

enum
{
	CMTS_SCHEDULER_STATE_OFFLINE,
	CMTS_SCHEDULER_STATE_ONLINE,
	CMTS_SCHEDULER_STATE_PAUSED,
};

CMTS_SHARED_ATTR static CMTS_ATOMIC(uint8_t) lib_lock;
CMTS_SHARED_ATTR static CMTS_ATOMIC(uint8_t) init_state;
CMTS_SHARED_ATTR static CMTS_ATOMIC(uint8_t) scheduler_state;
CMTS_SHARED_ATTR static CMTS_ATOMIC(uint8_t) should_continue;
CMTS_SHARED_ATTR static CMTS_ATOMIC(uint64_t) task_pool_flist;
CMTS_SHARED_ATTR static CMTS_ATOMIC(uint32_t) task_pool_bump;
static cmts_task_queue* queues[CMTS_MAX_PRIORITY];
static cmts_task_state* task_pool;
static thread_type* threads;
static cmts_cache_aligned_atomic_counter* thread_generation_counters;
static uint32_t thread_count;
static uint32_t queue_capacity;
static uint32_t queue_capacity_mask;
static uint32_t task_pool_capacity;
static uint32_t thread_remainder_param;
static uint32_t task_stack_size;
static size_t thread_stack_size;
static cmts_fn_remainder thread_remainder;
#ifdef CMTS_DEBUG
static CMTS_THREAD_LOCAL(uint8_t) yield_trap_flag;
#endif
static CMTS_THREAD_LOCAL(cmts_context) root_task;
static CMTS_THREAD_LOCAL(size_t) yield_trap_depth;
static CMTS_THREAD_LOCAL(uint32_t) thread_index;
static CMTS_THREAD_LOCAL(uint32_t) task_index;
static CMTS_THREAD_LOCAL(cmts_romu2jr) prng;
static CMTS_THREAD_LOCAL(uint64_t) prng_last_seed;

CMTS_INLINE_NEVER static void cmts_finalize_check_noinline()
{
	if (cmts_is_task())
		if (cmts_context_is_valid(&task_pool[task_index].ctx))
			cmts_context_wipe(&task_pool[task_index].ctx);
	cmts_os_exit_thread(0);
}

CMTS_INLINE_ALWAYS static void cmts_finalize_check()
{
	CMTS_UNLIKELY_IF(!CMTS_ATOMIC_LOAD_ACQ_U8(&should_continue))
		cmts_finalize_check_noinline();
}

CMTS_INLINE_ALWAYS static uint32_t cmts_remainder_range_reduce(uint32_t index, uint32_t range, uint8_t shift)
{
	return (uint32_t)(((uint64_t)index * (uint64_t)range) >> 32);
}

CMTS_INLINE_ALWAYS static uint32_t cmts_thread_remainder_pow2(uint32_t index)
{
	return index & thread_remainder_param;
}

CMTS_INLINE_ALWAYS static uint32_t cmts_thread_remainder_range_reduce(uint32_t index)
{
	return cmts_remainder_range_reduce(index, thread_count, (uint8_t)thread_remainder_param);
}

CMTS_INLINE_ALWAYS static void cmts_romu2jr_reseed(cmts_romu2jr* state)
{
	uint64_t buffer[2];
	(void)memset(buffer, 0, 16);
	(void)cmts_os_csprng(buffer, 16);
	state->x ^= buffer[0];
	state->y ^= buffer[1];
	prng_last_seed = cmts_os_time();
}

CMTS_INLINE_NEVER static void cmts_romu2jr_reseed_noinline(cmts_romu2jr* state)
{
	cmts_romu2jr_reseed(state);
}

CMTS_INLINE_ALWAYS static void cmts_romu2jr_init(cmts_romu2jr* state, size_t seed)
{
	// https://nullprogram.com/blog/2018/07/31/
#if UINTPTR_MAX == UINT32_MAX
	seed ^= seed >> 16;
	seed *= UINT32_C(0x7feb352d);
	seed ^= seed >> 15;
	seed *= UINT32_C(0x846ca68b);
	seed ^= seed >> 16;
#else
	seed ^= seed >> 32;
	seed *= UINT64_C(0xd6e8feb86659fd93);
	seed ^= seed >> 32;
	seed *= UINT64_C(0xd6e8feb86659fd93);
	seed ^= seed >> 32;
#endif
	state->x = state->y = seed;
	state->x ^= UINT64_C(0x9e3779b97f4a7c15);
	state->y ^= UINT64_C(0xd1b54a32d192ed03);
	cmts_romu2jr_reseed(state);
}

CMTS_INLINE_ALWAYS static uint64_t cmts_romu2jr_get(cmts_romu2jr* state)
{
	uint64_t k, now;
	k = state->x;
	state->x = 15241094284759029579u * state->y;
	state->y = state->y - k;
	state->y = CMTS_ROL64(state->y, 27);
	now = cmts_os_time();
	CMTS_UNLIKELY_IF(cmts_os_time_ns(now - prng_last_seed) >= UINT32_MAX)
		cmts_romu2jr_reseed_noinline(state);
	return k;
}

CMTS_INLINE_ALWAYS static uint_fast32_t cmts_pop_task()
{
	cmts_task_queue* queue;
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
	uint32_t last;
#endif
	uint_fast32_t index;
	uint_fast8_t i;
	for (;; cmts_finalize_check())
	{
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
		last = CMTS_ATOMIC_LOAD_ACQ_U32(&thread_generation_counters[thread_index].value);
#endif
		for (i = 0; i != CMTS_MAX_PRIORITY; ++i)
		{
			queue = queues[i] + thread_index;
			// https://github.com/dbittman/waitfree-mpsc-queue
			CMTS_UNLIKELY_IF(CMTS_ATOMIC_LOAD_ACQ_U32(&queue->count) == 0)
				continue;
			CMTS_UNLIKELY_IF(CMTS_ATOMIC_LOAD_ACQ_U32(&queue->values[queue->tail]) == UINT32_MAX)
				continue;
			index = CMTS_ATOMIC_XCHG_ACQ_U32(&queue->values[queue->tail], UINT32_MAX);
			CMTS_INVARIANT(index != UINT32_MAX);
			++queue->tail;
			queue->tail &= queue_capacity_mask;
			(void)CMTS_ATOMIC_DECREMENT_REL_U32(&queue->count);
			return index;
		}
#if defined(CMTS_NO_BUSY_WAIT) || defined(CMTS_HYBRID_MUTEX)
		cmts_os_futex_await(&thread_generation_counters[thread_index].value, &last, sizeof(uint32_t));
#endif
	}
}

CMTS_INLINE_ALWAYS static cmts_bool cmts_try_push_task_to(uint_fast32_t index, uint_fast8_t priority, size_t thread_index)
{
	cmts_task_queue* queue;
	uint_fast32_t prior;
	queue = queues[priority] + thread_index;
	// https://github.com/dbittman/waitfree-mpsc-queue
	prior = CMTS_ATOMIC_LOAD_ACQ_U32(&queue->count);
	CMTS_UNLIKELY_IF(prior >= queue_capacity)
		return CMTS_FALSE;
	prior = CMTS_ATOMIC_INCREMENT_ACQ_U32(&queue->count);
	CMTS_UNLIKELY_IF(prior >= queue_capacity)
	{
		(void)CMTS_ATOMIC_DECREMENT_REL_U32(&queue->count);
		return CMTS_FALSE;
	}
	prior = CMTS_ATOMIC_INCREMENT_ACQ_U32(&queue->head);
	prior &= queue_capacity_mask;
	(void)CMTS_ATOMIC_STORE_REL_U32(&queue->values[prior], index);
	(void)CMTS_ATOMIC_INCREMENT_REL_U32(&thread_generation_counters[thread_index].value);
	cmts_os_futex_signal(&thread_generation_counters[thread_index].value);
	return CMTS_TRUE;
}

CMTS_INLINE_ALWAYS static void cmts_push_task(uint_fast32_t index)
{
	cmts_task_state* task;
	uint32_t i, target_thread;
	task = task_pool + index;
	for (;; cmts_finalize_check())
	{
		for (i = 0; i != CMTS_SPIN_THRESHOLD; ++i)
		{
			target_thread = task->thread_affinity == thread_count ? thread_remainder((uint32_t)cmts_romu2jr_get(&prng)) : thread_index;
			CMTS_LIKELY_IF(cmts_try_push_task_to(index, task->priority, target_thread))
				return;
			CMTS_SPIN_WAIT;
		}
	}
}

CMTS_INLINE_NEVER static void cmts_await_task_pool_futex()
{
	cmts_index_generation_pair_union info;
	info.packed = CMTS_ATOMIC_LOAD_ACQ_U64(&task_pool_flist);
	CMTS_UNLIKELY_IF(info.pair.index == UINT32_MAX)
		cmts_os_futex_await(&task_pool_flist, &info.packed, 8);
}

CMTS_INLINE_ALWAYS static uint_fast32_t cmts_try_acquire_task()
{
	uint_fast32_t k;
	cmts_index_generation_pair_union prior, desired;
	CMTS_SPIN_LOOP
	{
		prior.packed = CMTS_ATOMIC_LOAD_ACQ_U64(&task_pool_flist);
		CMTS_UNLIKELY_IF(prior.pair.index == UINT32_MAX)
			break;
		desired.pair.index = task_pool[prior.pair.index].next;
		desired.pair.generation = prior.pair.generation + 1;
		CMTS_LIKELY_IF(CMTS_ATOMIC_CMPXCHG_ACQ_U64(&task_pool_flist, &prior.packed, desired.packed))
			return prior.pair.index;
	}
	CMTS_UNLIKELY_IF(CMTS_ATOMIC_LOAD_ACQ_U32(&task_pool_bump) >= task_pool_capacity)
		return UINT32_MAX;
	k = CMTS_ATOMIC_INCREMENT_ACQ_U32(&task_pool_bump);
	CMTS_LIKELY_IF(k < task_pool_capacity)
		return k;
	(void)CMTS_ATOMIC_DECREMENT_REL_U32(&task_pool_capacity);
	return UINT32_MAX;
}

CMTS_INLINE_ALWAYS static uint_fast32_t cmts_acquire_task()
{
	typedef void (*cmts_fn_yield)();
	size_t i;
	uint_fast32_t r;
	cmts_fn_yield yield_fn;
	yield_fn = cmts_is_task() ? cmts_yield : cmts_await_task_pool_futex;
	for (;; yield_fn())
	{
		for (i = 0; i != CMTS_SPIN_THRESHOLD; ++i)
		{
			r = cmts_try_acquire_task();
			CMTS_LIKELY_IF(r != UINT32_MAX)
				return r;
		}
	}
}

CMTS_INLINE_ALWAYS static void cmts_release_task(uint_fast32_t index)
{
	cmts_index_generation_pair_union prior, desired;
	desired.pair.index = index;
	CMTS_SPIN_LOOP
	{
		prior.packed = CMTS_ATOMIC_LOAD_ACQ_U64(&task_pool_flist);
		task_pool[index].next = prior.pair.index;
		desired.pair.generation = prior.pair.generation + 1;
		CMTS_LIKELY_IF(CMTS_ATOMIC_CMPXCHG_REL_U64(&task_pool_flist, &prior.packed, desired.packed))
			break;
	}
}

#define CMTS_YIELD_IMPL cmts_context_switch(&task_pool[task_index].ctx, &root_task)

static void cmts_impl_sleep_task()
{
	cmts_task_state* task;
	task = task_pool + task_index;
	CMTS_ATOMIC_STORE_REL_U8(&task->state, CMTS_TASK_STATE_GOING_TO_SLEEP);
	CMTS_YIELD_IMPL;
}

static void cmts_impl_wake_task(uint_fast32_t index)
{
	cmts_task_state* task;
	task = task_pool + index;
	while (CMTS_ATOMIC_LOAD_ACQ_U8(&task->state) != CMTS_TASK_STATE_SLEEPING)
		CMTS_SPIN_WAIT;
	CMTS_ATOMIC_STORE_REL_U8(&task->state, CMTS_TASK_STATE_INACTIVE);
	cmts_push_task(index);
}

static void cmts_wait_queue_init(CMTS_ATOMIC(uint64_t)* queue)
{
	(void)memset((void*)queue, 0xff, 8);
}

static cmts_result cmts_wait_queue_state(cmts_wait_queue_data info)
{
	if (info.head == UINT32_MAX)
		return CMTS_NOT_READY;
	if (info.tail == UINT32_MAX)
		return CMTS_SYNC_OBJECT_EXPIRED;
	return CMTS_OK;
}

static cmts_bool cmts_wait_queue_is_closed(cmts_wait_queue_data info)
{
	return info.head != UINT32_MAX && info.tail == UINT32_MAX;
}

static cmts_result cmts_wait_queue_push_current(CMTS_ATOMIC(uint64_t)* queue)
{
	cmts_wait_queue_union prior, desired;
	CMTS_SPIN_LOOP
	{
		prior.packed = CMTS_ATOMIC_LOAD_ACQ_U64(queue);
		CMTS_UNLIKELY_IF(prior.queue.head != UINT32_MAX && prior.queue.tail == UINT32_MAX)
			return CMTS_SYNC_OBJECT_EXPIRED;
		desired.queue.head = prior.queue.head == UINT32_MAX ? task_index : prior.queue.head;
		desired.queue.tail = task_index;
		CMTS_LIKELY_IF(CMTS_ATOMIC_CMPXCHG_REL_U64(queue, &prior.packed, desired.packed))
			break;
	}
	CMTS_LIKELY_IF(prior.queue.tail != UINT32_MAX)
	{
		CMTS_INVARIANT(task_pool[prior.queue.tail].next == UINT32_MAX);
		task_pool[prior.queue.tail].next = task_index;
	}
	cmts_impl_sleep_task();
	return CMTS_OK;
}

static cmts_bool cmts_wait_queue_pop_one(CMTS_ATOMIC(uint64_t)* queue)
{
	cmts_wait_queue_union prior, desired;
	CMTS_SPIN_LOOP
	{
		prior.packed = CMTS_ATOMIC_LOAD_ACQ_U64(queue);
		CMTS_UNLIKELY_IF(prior.queue.head == UINT32_MAX)
			return CMTS_FALSE;
		desired.queue.head = task_pool[prior.queue.head].next;
		desired.queue.tail = prior.queue.tail;
		CMTS_LIKELY_IF(CMTS_ATOMIC_CMPXCHG_ACQ_U64(queue, &prior.packed, desired.packed))
			break;
	}
	task_pool[prior.queue.head].next = UINT32_MAX;
	cmts_impl_wake_task(prior.queue.head);
	return CMTS_TRUE;
}

static cmts_result cmts_wait_queue_pop_all(CMTS_ATOMIC(uint64_t)* queue)
{
	cmts_wait_queue_union prior, desired;
	uint_fast32_t n, next;
	desired.queue.head = 0;
	desired.queue.tail = UINT32_MAX;
	prior.packed = CMTS_ATOMIC_XCHG_ACQ_U64(queue, desired.packed);
	CMTS_UNLIKELY_IF(prior.queue.head != UINT32_MAX && prior.queue.tail == UINT32_MAX)
		return CMTS_SYNC_OBJECT_EXPIRED;
	CMTS_UNLIKELY_IF(prior.queue.head == UINT32_MAX)
		return CMTS_OK;
	n = prior.queue.head;
	for (;; n = next)
	{
		while (CMTS_ATOMIC_LOAD_ACQ_U8(&task_pool[n].state) != CMTS_TASK_STATE_SLEEPING)
			CMTS_SPIN_WAIT;
		next = task_pool[n].next;
		task_pool[n].next = UINT32_MAX;
		CMTS_ATOMIC_STORE_REL_U8(&task_pool[n].state, CMTS_TASK_STATE_INACTIVE);
		cmts_push_task(n);
		CMTS_UNLIKELY_IF(n == prior.queue.tail)
			return CMTS_OK;
	}
}

static cmts_result cmts_wait_queue_reset(CMTS_ATOMIC(uint64_t)* queue)
{
	cmts_wait_queue_union prior;
	prior.packed = CMTS_ATOMIC_LOAD_ACQ_U64(queue);
	CMTS_UNLIKELY_IF(prior.queue.head != UINT32_MAX && prior.queue.tail == UINT32_MAX)
		return CMTS_NOT_READY;
	(void)memset((void*)queue, 0xff, 8);
	return CMTS_OK;
}

static thread_return_type CMTS_THREAD_CALLING_CONVENTION cmts_thread_entry_point(void* param)
{
	typedef cmts_result(CMTS_CALL *cmts_fn_sync_callback)(void*);
	cmts_task_state* task;
	void* sync_object;
	cmts_fn_sync_callback callback;
	uint_fast8_t k;
	thread_index = (uint32_t)(size_t)param;
	cmts_romu2jr_init(&prng, (size_t)cmts_os_time() ^ thread_index);
#ifdef CMTS_WINDOWS
	root_task = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
#endif
	for (;; cmts_finalize_check())
	{
		task_index = cmts_pop_task();
		task = task_pool + task_index;
		CMTS_ATOMIC_STORE_REL_U8(&task->state, CMTS_TASK_STATE_RUNNING);
#ifdef CMTS_WINDOWS
		CMTS_ASSERT(GetCurrentFiber() == root_task);
#endif
		cmts_context_switch(&root_task, &task->ctx);
#ifdef CMTS_WINDOWS
		CMTS_ASSERT(GetCurrentFiber() == root_task);
#endif
		k = CMTS_ATOMIC_LOAD_ACQ_U8(&task->state);
		CMTS_UNLIKELY_IF(k == CMTS_TASK_STATE_GOING_TO_SLEEP)
			CMTS_ATOMIC_STORE_REL_U8(&task->state, CMTS_TASK_STATE_SLEEPING);
		if (task->fn != NULL)
		{
			CMTS_LIKELY_IF(k != CMTS_TASK_STATE_GOING_TO_SLEEP)
				cmts_push_task(task_index);
		}
		else
		{
			k = task->sync_type;
			sync_object = task->sync_object;
			cmts_release_task(task_index);
			CMTS_LIKELY_IF(k == CMTS_SYNC_TYPE_NONE)
				continue;
			callback = k != CMTS_SYNC_TYPE_COUNTER ? (cmts_fn_sync_callback)cmts_event_signal : (cmts_fn_sync_callback)cmts_counter_decrement;
			(void)callback(sync_object);
		}
	}
}

static void cmts_task_entry_point(void* ptr)
{
	cmts_task_state* task;
	task = (cmts_task_state*)ptr;
	while (CMTS_ATOMIC_LOAD_ACQ_U8(&should_continue))
	{
		task->fn(task->param);
		task->fn = NULL;
		cmts_yield();
	}
	cmts_os_exit_thread(0);
}

CMTS_INLINE_ALWAYS static size_t cmts_round_pow2(size_t value)
{
#if UINTPTR_MAX == UINT32_MAX
	return UINT32_C(1) << (32 - CMTS_CLZ32(value));
#else
	return UINT64_C(1) << (64 - CMTS_CLZ64(value));
#endif
}

static void cmts_lib_lock()
{
	uint8_t i;
	uint32_t expected;
	
	for (;;)
	{
		for (i = 0; i != CMTS_SPIN_THRESHOLD; ++i)
		{
			expected = CMTS_ATOMIC_LOAD_ACQ_U32(&lib_lock);
			CMTS_LIKELY_IF(!expected && CMTS_ATOMIC_CMPXCHG_ACQ_U32(&lib_lock, &expected, 1))
				return;
		}
		cmts_os_futex_await(&lib_lock, &expected, 1);
	}
}

static void cmts_lib_unlock()
{
	CMTS_ATOMIC_STORE_REL_U32(&lib_lock, 0);
	cmts_os_futex_signal(&lib_lock);
}

CMTS_INLINE_ALWAYS static size_t cmts_required_memory_size()
{
	size_t r = sizeof(thread_type) * thread_count;
	r += sizeof(cmts_task_state) * task_pool_capacity;
	size_t queues_count = (size_t)thread_count * CMTS_MAX_PRIORITY;
	r += sizeof(cmts_task_queue) * queues_count;
	r += sizeof(uint32_t) * cmts_round_pow2(task_pool_capacity / thread_count) * queues_count;
#ifdef CMTS_NO_BUSY_WAIT
	r += sizeof(uint32_t) * thread_count;
#endif
	return r;
}

static cmts_result cmts_handle_extension(const cmts_init_options* options, const cmts_ext_header* header)
{
	switch (header->type)
	{
	case CMTS_EXT_TYPE_DEBUGGER:
		debugger_callback = ((const cmts_ext_debug_init_options*)header)->message_callback;
		debugger_context = ((const cmts_ext_debug_init_options*)header)->context;
		return CMTS_OK;
	default:
		return CMTS_ERROR_INVALID_EXTENSION_TYPE;
	}
}

CMTS_INLINE_ALWAYS static void cmts_common_init(uint8_t* buffer)
{
	size_t i, j, thread_size, task_pool_size, queue_size, thread_generation_counters_size;
	cmts_task_queue* q;
	thread_size = CMTS_ROUND_CACHE_LINE_SIZE(sizeof(thread_type) * thread_count);
	task_pool_size = sizeof(cmts_task_state) * task_pool_capacity;
	queue_size = sizeof(uint32_t) * queue_capacity;
	thread_generation_counters_size = (size_t)CMTS_CACHE_LINE_SIZE * thread_count;
	threads = (thread_type*)buffer;
	buffer += thread_size;
	task_pool = (cmts_task_state*)buffer;
	buffer += task_pool_size;
	thread_generation_counters = (cmts_cache_aligned_atomic_counter*)buffer;
	buffer += thread_generation_counters_size;
	(void)memset(thread_generation_counters, 0, thread_generation_counters_size);
	for (i = 0; i != CMTS_MAX_PRIORITY; ++i)
	{
		queues[i] = (cmts_task_queue*)buffer;
		buffer += sizeof(cmts_task_queue) * thread_count;
	}
	for (i = 0; i != CMTS_MAX_PRIORITY; ++i)
	{
		for (j = 0; j != thread_count; ++j)
		{
			q = queues[i] + j;
			q->values = (CMTS_ATOMIC(uint32_t)*)buffer;
			(void)memset(buffer, 0xff, queue_size);
			buffer += queue_size;
			q->tail = 0;
			q->head = 0;
			q->count = 0;
		}
	}
	cmts_index_generation_pair flist;
	flist.index = UINT32_MAX;
	flist.generation = 0;
	(void)memcpy((void*)&task_pool_flist, &flist, 8);
	for (i = 0; i != task_pool_capacity; ++i)
	{
		(void)memset(task_pool + i, 0, sizeof(cmts_task_state));
		task_pool[i].thread_affinity = thread_count;
		task_pool[i].next = UINT32_MAX;
	}
	CMTS_LIKELY_IF(CMTS_POPCNT32(thread_count) == 1)
	{
		thread_remainder = cmts_thread_remainder_pow2;
		thread_remainder_param = thread_count - 1;
	}
	else
	{
		thread_remainder = cmts_thread_remainder_range_reduce;
		thread_remainder_param = 32 - CMTS_CLZ32(thread_count);
	}
}

static cmts_result cmts_library_init_default()
{
	size_t buffer_size;
	uint8_t* buffer;
	thread_count = (uint32_t)cmts_processor_count();
	task_pool_capacity = CMTS_DEFAULT_TASKS_PER_THREAD * thread_count;
	queue_capacity = (uint32_t)cmts_round_pow2(task_pool_capacity / thread_count);
	queue_capacity_mask = queue_capacity - 1;
	task_stack_size = (uint32_t)cmts_default_task_stack_size();
	buffer_size = cmts_required_memory_size();
	buffer = (uint8_t*)cmts_os_malloc(buffer_size);
	CMTS_UNLIKELY_IF(buffer == NULL)
		return CMTS_ERROR_MEMORY_ALLOCATION;
	cmts_common_init(buffer);
	return cmts_os_init_threads(threads, thread_count, thread_stack_size, (LPTHREAD_START_ROUTINE)cmts_thread_entry_point);
}

static cmts_result cmts_library_init_custom(const cmts_init_options* options)
{
	cmts_result r;
	size_t buffer_size;
	uint8_t* buffer;
	const cmts_ext_header* ext;
	thread_count = options->thread_count;
	task_pool_capacity = options->max_tasks;
	queue_capacity = (uint32_t)cmts_round_pow2(task_pool_capacity / options->thread_count);
	queue_capacity_mask = queue_capacity - 1;
	task_stack_size = (uint32_t)options->task_stack_size;
	buffer_size = (uint32_t)cmts_required_memory_size();
	CMTS_LIKELY_IF(options->allocate_function == NULL)
		buffer = (uint8_t*)cmts_os_malloc(buffer_size);
	else
		buffer = (uint8_t*)options->allocate_function(buffer_size);
	CMTS_UNLIKELY_IF(buffer == NULL)
		return CMTS_ERROR_MEMORY_ALLOCATION;
	cmts_common_init(buffer);
	CMTS_UNLIKELY_IF(options->thread_affinities != NULL)
		r = cmts_os_init_threads_custom(threads, thread_count, thread_stack_size, (LPTHREAD_START_ROUTINE)cmts_thread_entry_point, options->thread_affinities);
	else
		r = cmts_os_init_threads(threads, thread_count, thread_stack_size, (LPTHREAD_START_ROUTINE)cmts_thread_entry_point);
	CMTS_UNLIKELY_IF(r != CMTS_OK)
		return r;
	for (ext = (const cmts_ext_header*)options->next_ext; ext != NULL; ext = ext->next)
	{
		r = cmts_handle_extension(options, ext);
		CMTS_UNLIKELY_IF(r != CMTS_OK)
			return r;
	}
	return CMTS_OK;
}

static cmts_result cmts_common_cleanup(cmts_fn_deallocate deallocate)
{
	uint_fast32_t n;
	size_t i;
	n = CMTS_ATOMIC_LOAD_ACQ_U32(&task_pool_bump);
	CMTS_UNLIKELY_IF(n > task_pool_capacity)
		n = task_pool_capacity;
	for (i = 0; i != task_pool_bump; ++i)
		CMTS_LIKELY_IF(cmts_context_is_valid(&task_pool[i].ctx))
			cmts_context_delete(&task_pool[i].ctx);
	i = cmts_required_memory_size();
	CMTS_LIKELY_IF(deallocate == NULL)
		deallocate = cmts_os_free;
	CMTS_UNLIKELY_IF(!deallocate(threads, i))
		return CMTS_ERROR_MEMORY_DEALLOCATION;
	CMTS_ATOMIC_STORE_REL_U8(&should_continue, 0);
	CMTS_ATOMIC_STORE_REL_U8(&init_state, 0);
	CMTS_ATOMIC_STORE_REL_U8(&scheduler_state, CMTS_SCHEDULER_STATE_OFFLINE);
	return CMTS_OK;
}

extern "C"
{
CMTS_ATTR cmts_result CMTS_CALL cmts_init(const cmts_init_options* options)
{
	cmts_result r;
	uint8_t expected;
	expected = 0;
	CMTS_UNLIKELY_IF(cmts_is_initialized())
		return CMTS_ALREADY_INITIALIZED;
	CMTS_UNLIKELY_IF(!CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U8(&init_state, &expected, 1))
		return CMTS_INITIALIZATION_IN_PROGRESS;
	CMTS_ATOMIC_STORE_REL_U8(&should_continue, 1);
	CMTS_ATOMIC_STORE_REL_U8(&init_state, 2);
	CMTS_ATOMIC_STORE_REL_U8(&scheduler_state, CMTS_SCHEDULER_STATE_ONLINE);
	CMTS_UNLIKELY_IF(!cmts_os_init())
		return CMTS_ERROR_OS_INIT;
	CMTS_UNLIKELY_IF(options == NULL)
		r = cmts_library_init_default();
	else
		r = cmts_library_init_custom(options);
	CMTS_UNLIKELY_IF(r == CMTS_OK)
		return CMTS_OK;
	CMTS_ATOMIC_STORE_REL_U8(&should_continue, 0);
	CMTS_ATOMIC_STORE_REL_U8(&init_state, 0);
	CMTS_ATOMIC_STORE_REL_U8(&scheduler_state, CMTS_SCHEDULER_STATE_OFFLINE);
	return r;
}

CMTS_ATTR cmts_result CMTS_CALL cmts_pause()
{
	uint8_t expected;
	expected = CMTS_SCHEDULER_STATE_ONLINE;
	CMTS_UNLIKELY_IF(!CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U8(&scheduler_state, &expected, CMTS_SCHEDULER_STATE_PAUSED))
		return cmts_is_initialized() ? CMTS_OK : CMTS_ERROR_LIBRARY_UNINITIALIZED;
	CMTS_UNLIKELY_IF(cmts_os_pause_threads(threads, thread_count))
		return CMTS_ERROR_SUSPEND_THREAD;
	return CMTS_OK;
}

CMTS_ATTR cmts_result CMTS_CALL cmts_resume()
{
	uint8_t expected;
	expected = CMTS_SCHEDULER_STATE_PAUSED;
	CMTS_UNLIKELY_IF(!CMTS_ATOMIC_CMPXCHG_STRONG_ACQ_U8(&scheduler_state, &expected, CMTS_SCHEDULER_STATE_ONLINE))
		return cmts_is_initialized() ? CMTS_OK : CMTS_ERROR_LIBRARY_UNINITIALIZED;
	CMTS_UNLIKELY_IF(cmts_os_resume_threads(threads, thread_count))
		return CMTS_ERROR_RESUME_THREAD;
	return CMTS_OK;
}

CMTS_ATTR void CMTS_CALL cmts_finalize_signal()
{
	size_t i;
	CMTS_ATOMIC_STORE_REL_U8(&should_continue, 0);
#ifdef CMTS_NO_BUSY_WAIT
	for (i = 0; i != thread_count; ++i)
		cmts_os_futex_signal(&thread_generation_counters[i].value);
#endif
	CMTS_LIKELY_IF(cmts_is_task())
		cmts_os_exit_thread(0);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_finalize_await(cmts_fn_deallocate deallocate)
{
	CMTS_UNLIKELY_IF(!cmts_is_initialized())
		return CMTS_ERROR_LIBRARY_UNINITIALIZED;
	CMTS_UNLIKELY_IF(!cmts_os_await_threads(threads, thread_count))
		return CMTS_ERROR_AWAIT_THREAD;
	return cmts_common_cleanup(deallocate);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_terminate(cmts_fn_deallocate deallocate)
{
	CMTS_UNLIKELY_IF(!cmts_is_initialized())
		return CMTS_ERROR_LIBRARY_UNINITIALIZED;
	CMTS_UNLIKELY_IF(!cmts_os_terminate_threads(threads, thread_count))
		return CMTS_ERROR_TERMINATE_THREAD;
	return cmts_common_cleanup(deallocate);
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_is_initialized()
{
	return CMTS_ATOMIC_LOAD_ACQ_U8(&init_state) == 2;
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_is_online()
{
	return cmts_is_initialized() && CMTS_ATOMIC_LOAD_ACQ_U8(&should_continue);
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_is_paused()
{
	return CMTS_ATOMIC_LOAD_ACQ_U8(&scheduler_state) == CMTS_SCHEDULER_STATE_PAUSED;
}

CMTS_ATTR uint32_t CMTS_CALL cmts_purge(uint32_t max_purged_tasks)
{
	uint_fast32_t n, k, i;
	n = CMTS_ATOMIC_LOAD_ACQ_U32(&task_pool_bump);
	CMTS_UNLIKELY_IF(n > task_pool_capacity)
		n = task_pool_capacity;
	k = 0;
	for (i = 0; i != n && k != max_purged_tasks; ++i)
	{
		CMTS_LIKELY_IF(cmts_context_is_valid(&task_pool[i].ctx))
			cmts_context_delete(&task_pool[i].ctx);
	}
	return k;
}

CMTS_ATTR uint32_t CMTS_CALL cmts_purge_all()
{
	uint_fast32_t n, k, i;
	n = CMTS_ATOMIC_LOAD_ACQ_U32(&task_pool_bump);
	CMTS_UNLIKELY_IF(n > task_pool_capacity)
		n = task_pool_capacity;
	k = 0;
	for (i = 0; i != n; ++i)
	{
		CMTS_LIKELY_IF(cmts_context_is_valid(&task_pool[i].ctx))
			cmts_context_delete(&task_pool[i].ctx);
	}
	return k;
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_is_worker_thread()
{
	return root_task != NULL;
}

CMTS_ATTR uint32_t CMTS_CALL cmts_worker_thread_index()
{
	CMTS_UNLIKELY_IF(!cmts_is_worker_thread())
		return thread_count;
	return thread_index;
}

CMTS_ATTR uint32_t CMTS_CALL cmts_worker_thread_count()
{
	return thread_count;
}

CMTS_ATTR cmts_result CMTS_CALL cmts_dispatch(cmts_fn_task entry_point, cmts_dispatch_options* options)
{
	cmts_dispatch_options o;
	cmts_task_state* task;
	uint_fast32_t index, generation;
	CMTS_UNLIKELY_IF(options == NULL)
	{
		(void)memset(&o, 0, sizeof(cmts_dispatch_options));
		options = &o;
	}
	index = ((options->flags & CMTS_DISPATCH_FLAGS_FORCE) ? cmts_acquire_task : cmts_try_acquire_task)();
	CMTS_UNLIKELY_IF(index == UINT32_MAX)
		return CMTS_ERROR_TASK_POOL_CAPACITY;
	task = task_pool + index;
	CMTS_UNLIKELY_IF(!cmts_context_is_valid(&task->ctx))
	{
		CMTS_UNLIKELY_IF(!cmts_context_init(&task->ctx, cmts_task_entry_point, task, task_stack_size))
		{
			cmts_release_task(index);
			return CMTS_ERROR_TASK_ALLOCATION;
		}
	}
	task->fn = entry_point;
	task->param = options->parameter;
	task->sync_object = options->sync_object;
	task->next = UINT32_MAX;
	generation = ++task->generation;
	task->thread_affinity = options->locked_thread != NULL ? *options->locked_thread : thread_count;
	task->priority = options->priority;
	task->sync_type = options->sync_type;
	CMTS_ASSERT(CMTS_ATOMIC_LOAD_ACQ_U8(&task->state) == CMTS_TASK_STATE_INACTIVE);
	cmts_push_task(index);
	CMTS_UNLIKELY_IF(options->out_task_id != NULL)
		*options->out_task_id = CMTS_MAKE_HANDLE(index, generation);
	return CMTS_OK;
}

CMTS_ATTR void CMTS_CALL cmts_yield()
{
	CMTS_ATOMIC_STORE_REL_U8(&task_pool[task_index].state, CMTS_TASK_STATE_INACTIVE);
	CMTS_YIELD_IMPL;
}

CMTS_NORETURN CMTS_ATTR void CMTS_CALL cmts_exit()
{
	cmts_task_state* task;
	task = task_pool + task_index;
	task->fn = NULL;
	CMTS_ATOMIC_STORE_REL_U8(&task->state, CMTS_TASK_STATE_INACTIVE);
	CMTS_YIELD_IMPL;
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_is_task()
{
	return root_task != NULL;
}

CMTS_ATTR cmts_task_id CMTS_CALL cmts_this_task_id()
{
	CMTS_ASSERT(cmts_is_worker_thread());
	return CMTS_MAKE_HANDLE(task_index, task_pool[task_index].generation);
}

CMTS_NODISCARD CMTS_ATTR cmts_task_id CMTS_CALL cmts_task_allocate()
{
	uint_fast32_t index;
	cmts_task_state* task;
	index = cmts_try_acquire_task();
	CMTS_UNLIKELY_IF(index == UINT32_MAX)
		return CMTS_INVALID_TASK_ID;
	task = task_pool + index;
	CMTS_INVARIANT(task->next == UINT32_MAX);
	CMTS_INVARIANT(task->fn == NULL);
	++task->generation;
	return CMTS_MAKE_HANDLE(index, task->generation);
}

#define CMTS_TASK_COMMON_VARIABLES						\
	uint_fast32_t index, generation;					\
	cmts_task_state* task;								\
	CMTS_BREAK_HANDLE(task_id, index, generation);		\
	task = task_pool + index;							\
	CMTS_INVARIANT(task->next == UINT32_MAX);			\
	CMTS_INVARIANT(task->generation == generation);

CMTS_ATTR uint8_t CMTS_CALL cmts_task_get_priority(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	return task->priority;
}

CMTS_ATTR void CMTS_CALL cmts_task_set_priority(cmts_task_id task_id, uint8_t new_priority)
{
	CMTS_TASK_COMMON_VARIABLES;
	task->priority = new_priority;
}

CMTS_ATTR void CMTS_CALL cmts_task_set_parameter(cmts_task_id task_id, void* new_parameter)
{
	CMTS_TASK_COMMON_VARIABLES;
	task->param = new_parameter;
}

CMTS_ATTR void* CMTS_CALL cmts_task_get_parameter(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	return task->param;
}

CMTS_ATTR void CMTS_CALL cmts_task_set_function(cmts_task_id task_id, cmts_fn_task new_function)
{
	CMTS_TASK_COMMON_VARIABLES;
	task->fn = new_function;
}

CMTS_ATTR cmts_fn_task CMTS_CALL cmts_task_get_function(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	return task->fn;
}

CMTS_ATTR void CMTS_CALL cmts_task_attach_event(cmts_task_id task_id, cmts_event* event)
{
	CMTS_TASK_COMMON_VARIABLES;
	task->sync_type = CMTS_SYNC_TYPE_EVENT;
	task->sync_object = event;
}

CMTS_ATTR void CMTS_CALL cmts_task_attach_counter(cmts_task_id task_id, cmts_counter* counter)
{
	CMTS_TASK_COMMON_VARIABLES;
	task->sync_type = CMTS_SYNC_TYPE_COUNTER;
	task->sync_object = counter;
}

CMTS_ATTR void CMTS_CALL cmts_task_sleep(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	CMTS_ATOMIC_STORE_REL_U8(&task_pool[index].state, CMTS_TASK_STATE_GOING_TO_SLEEP);
}

CMTS_ATTR void CMTS_CALL cmts_task_resume(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	cmts_impl_wake_task(index);
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_is_valid_task_id(cmts_task_id task_id)
{
	uint_fast32_t index, generation;
	cmts_task_state* task;
	CMTS_BREAK_HANDLE(task_id, index, generation);
	CMTS_UNLIKELY_IF(index >= task_pool_capacity)
		return CMTS_FALSE;
	CMTS_UNLIKELY_IF(index >= CMTS_ATOMIC_LOAD_ACQ_U32(&task_pool_bump))
		return CMTS_FALSE;
	task = task_pool + index;
	return task->next == UINT32_MAX && task->generation == generation;
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_task_is_sleeping(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	return CMTS_ATOMIC_LOAD_ACQ_U8(&task_pool[index].state) == CMTS_TASK_STATE_SLEEPING;
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_task_is_running(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	return CMTS_ATOMIC_LOAD_ACQ_U8(&task_pool[index].state) == CMTS_TASK_STATE_RUNNING;
}

CMTS_ATTR void CMTS_CALL cmts_task_dispatch(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	cmts_push_task(index);
}

CMTS_ATTR void CMTS_CALL cmts_task_deallocate(cmts_task_id task_id)
{
	CMTS_TASK_COMMON_VARIABLES;
	cmts_release_task(index);
}

CMTS_ATTR void CMTS_CALL cmts_fence_init(cmts_fence* fence)
{
	(void)memset(fence, 0xff, sizeof(cmts_fence));
}

CMTS_ATTR void CMTS_CALL cmts_fence_signal(cmts_fence* fence)
{
	cmts_fence_data* e;
	e = (cmts_fence_data*)fence;
	uint32_t index;
	CMTS_SPIN_LOOP
	{
		index = CMTS_ATOMIC_LOAD_ACQ_U32(e);
		CMTS_LIKELY_IF(index != UINT32_MAX)
			break;
	}
	cmts_impl_wake_task(index);
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_fence_try_await(cmts_fence* fence)
{
	cmts_fence_data* e;
	uint32_t prior, desired;
	e = (cmts_fence_data*)fence;
	prior = UINT32_MAX;
	desired = task_index;
	CMTS_UNLIKELY_IF(!CMTS_ATOMIC_CMPXCHG_ACQ_U32(e, &prior, desired))
		return CMTS_FALSE;
	cmts_impl_sleep_task();
	return CMTS_TRUE;
}

CMTS_ATTR void CMTS_CALL cmts_fence_await(cmts_fence* fence)
{
	cmts_fence_data* e;
	e = (cmts_fence_data*)fence;
#ifdef CMTS_DEBUG
	CMTS_ASSERT(CMTS_ATOMIC_XCHG_REL_U32(e, task_index) == UINT32_MAX);
#else
	CMTS_ATOMIC_STORE_REL_U32(e, task_index);
#endif
	cmts_impl_sleep_task();
}

CMTS_ATTR void CMTS_CALL cmts_event_init(cmts_event* event)
{
	cmts_wait_queue_init((CMTS_ATOMIC(uint64_t)*)event);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_event_state(const cmts_event* event)
{
	cmts_wait_queue_union info;
	cmts_event_data* e;
	e = (cmts_event_data*)event;
	info.packed = CMTS_ATOMIC_LOAD_ACQ_U64(e);
	return cmts_wait_queue_state(info.queue);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_event_signal(cmts_event* event)
{
	cmts_event_data* e;
	e = (cmts_event_data*)event;
	return cmts_wait_queue_pop_all(e);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_event_await(cmts_event* event)
{
	cmts_event_data* e;
	e = (cmts_event_data*)event;
	return cmts_wait_queue_push_current(e);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_event_reset(cmts_event* event)
{
	cmts_event_data* e;
	e = (cmts_event_data*)event;
	return cmts_wait_queue_reset(e);
}

CMTS_ATTR void CMTS_CALL cmts_counter_init(cmts_counter* counter, uint64_t start_value)
{
	cmts_counter_data* c;
	c = (cmts_counter_data*)counter;
	cmts_wait_queue_init(&c->queue);
	*(uint64_t*)&c->value = start_value;
}

CMTS_ATTR uint64_t CMTS_CALL cmts_counter_value(const cmts_counter* counter)
{
	cmts_counter_data* c;
	c = (cmts_counter_data*)counter;
	return CMTS_ATOMIC_LOAD_ACQ_U64(&c->value);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_counter_state(const cmts_counter* counter)
{
	cmts_counter_data* c;
	cmts_wait_queue_union info;
	c = (cmts_counter_data*)counter;
	info.packed = CMTS_ATOMIC_LOAD_ACQ_U64(&c->queue);
	return cmts_wait_queue_state(info.queue);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_counter_increment(cmts_counter* counter)
{
	cmts_counter_data* c;
	cmts_wait_queue_union info;
	uint64_t prior, desired;
	c = (cmts_counter_data*)counter;
	CMTS_SPIN_LOOP
	{
		info.packed = CMTS_ATOMIC_LOAD_ACQ_U64(&c->queue);
		CMTS_UNLIKELY_IF(cmts_wait_queue_is_closed(info.queue))
			return CMTS_SYNC_OBJECT_EXPIRED;
		prior = CMTS_ATOMIC_LOAD_ACQ_U64(&c->value);
		desired = prior + 1;
		CMTS_LIKELY_IF(CMTS_ATOMIC_CMPXCHG_REL_U64(&c->value, &prior, desired))
			break;
	}
	return CMTS_OK;
}

CMTS_ATTR cmts_result CMTS_CALL cmts_counter_decrement(cmts_counter* counter)
{
	cmts_counter_data* c;
	cmts_wait_queue_union info;
	uint64_t k;
	c = (cmts_counter_data*)counter;
	info.packed = CMTS_ATOMIC_LOAD_ACQ_U64(&c->queue);
	CMTS_UNLIKELY_IF(cmts_wait_queue_is_closed(info.queue))
		return CMTS_SYNC_OBJECT_EXPIRED;
	k = CMTS_ATOMIC_DECREMENT_ACQ_U64(&c->value);
	CMTS_INVARIANT(k != 0);
	CMTS_UNLIKELY_IF(k != 1)
		return CMTS_NOT_READY;
	return cmts_wait_queue_pop_all(&c->queue);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_counter_await(cmts_counter* counter)
{
	cmts_counter_data* c;
	c = (cmts_counter_data*)counter;
	return cmts_wait_queue_push_current(&c->queue);
}

CMTS_ATTR cmts_result CMTS_CALL cmts_counter_reset(cmts_counter* counter, uint64_t new_start_value)
{
	cmts_counter_data* c;
	c = (cmts_counter_data*)counter;
	CMTS_ASSERT(cmts_wait_queue_is_closed(*(cmts_wait_queue_data*)c->queue));
	*(uint64_t*)&c->value = new_start_value;
	return cmts_wait_queue_reset(&c->queue);
}

CMTS_ATTR void CMTS_CALL cmts_mutex_init(cmts_mutex* mutex)
{
	(void)memset(mutex, 0xff, 4);
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_mutex_is_locked(const cmts_mutex* mutex)
{
	cmts_mutex_data* c;
	c = (cmts_mutex_data*)mutex;
	return CMTS_ATOMIC_LOAD_ACQ_U32(c) != UINT32_MAX;
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_mutex_try_lock(cmts_mutex* mutex)
{
	cmts_mutex_data* c;
	uint32_t prior, desired;
	CMTS_ASSERT(cmts_is_task());
	c = (cmts_mutex_data*)mutex;
	prior = UINT32_MAX;
	desired = task_index;
	return CMTS_ATOMIC_CMPXCHG_ACQ_U32(c, &prior, desired);
}

CMTS_ATTR void CMTS_CALL cmts_mutex_lock(cmts_mutex* mutex)
{
	cmts_mutex_data* c;
	uint32_t prior;
	CMTS_ASSERT(cmts_is_task());
	c = (cmts_mutex_data*)mutex;
	prior = CMTS_ATOMIC_XCHG_ACQ_U32(c, task_index);
	CMTS_LIKELY_IF(prior == UINT32_MAX)
		return;
	task_pool[prior].next = task_index;
	cmts_impl_sleep_task();
}

CMTS_ATTR void CMTS_CALL cmts_mutex_unlock(cmts_mutex* mutex)
{
	cmts_mutex_data* c;
	uint32_t index;
	CMTS_ASSERT(cmts_is_task());
	c = (cmts_mutex_data*)mutex;
	CMTS_LIKELY_IF(task_pool[task_index].next == UINT32_MAX)
	{
		index = task_index;
		CMTS_LIKELY_IF(CMTS_ATOMIC_CMPXCHG_STRONG_REL_U32(c, &index, UINT32_MAX))
			return;
		CMTS_SPIN_LOOP
		{
			index = task_pool[task_index].next;
			if (index == UINT32_MAX)
				break;
		}
	}
	else
	{
		index = task_pool[task_index].next;
	}
	CMTS_INVARIANT(index != UINT32_MAX);
	task_pool[task_index].next = UINT32_MAX;
	cmts_impl_wake_task(index);
}

CMTS_ATTR void CMTS_CALL cmts_rcu_read_begin()
{
#ifdef CMTS_DEBUG
	CMTS_ASSERT(cmts_is_task());
	CMTS_LIKELY_IF(yield_trap_depth == 0)
		cmts_ext_debug_enable_yield_trap(CMTS_TRUE);
	++yield_trap_depth;
#endif
}

CMTS_ATTR void CMTS_CALL cmts_rcu_read_end()
{
#ifdef CMTS_DEBUG
	CMTS_ASSERT(cmts_is_task());
	--yield_trap_depth;
	CMTS_LIKELY_IF(yield_trap_depth == 0)
		cmts_ext_debug_enable_yield_trap(CMTS_FALSE);
#endif
}

#define CMTS_RCU_SYNC_GROUP_SIZE (CMTS_CACHE_LINE_SIZE / sizeof(uint32_t))

CMTS_ATTR void CMTS_CALL cmts_rcu_sync()
{
	uint32_t group[CMTS_RCU_SYNC_GROUP_SIZE];
	uint32_t i, j, k, next;
	for (i = 0; i != thread_count; i = next)
	{
		next = i + CMTS_RCU_SYNC_GROUP_SIZE;
		CMTS_UNLIKELY_IF(next > thread_count)
			next = thread_count;
		for (j = i; j != next; ++j)
			group[j - i] = CMTS_ATOMIC_LOAD_ACQ_U32(&thread_generation_counters[j].value);
		cmts_yield();
		for (j = i; j != next; ++j)
		{
			for (k = 0; group[j - i] == CMTS_ATOMIC_LOAD_ACQ_U32(&thread_generation_counters[j].value);)
			{
				CMTS_UNLIKELY_IF(++k == CMTS_SPIN_THRESHOLD)
				{
					cmts_yield();
					k = 0;
				}
			}
		}
	}
}

CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot_requirements(cmts_memory_requirements* out_requirements)
{
	out_requirements->size = thread_count * sizeof(uint32_t);
	out_requirements->alignment_log2 = 2;
}

CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot(void* snapshot_buffer)
{
	uint32_t* out = (uint32_t*)snapshot_buffer;
	uint32_t i;
	for (i = 0; i != thread_count; ++i)
		out[i] = CMTS_ATOMIC_LOAD_ACQ_U32(&thread_generation_counters[i].value);
}

CMTS_ATTR uint32_t CMTS_CALL cmts_rcu_try_snapshot_sync(const void* snapshot_buffer, uint32_t prior_result)
{
	uint32_t* e = (uint32_t*)snapshot_buffer;
	uint32_t i;
	for (i = prior_result; i != thread_count; ++i)
	{
		CMTS_UNLIKELY_IF(i == thread_index)
			continue;
		CMTS_LIKELY_IF(CMTS_ATOMIC_LOAD_ACQ_U32(&thread_generation_counters[i].value) == e[i])
			break;
	}
	return i;
}

CMTS_ATTR void CMTS_CALL cmts_rcu_snapshot_sync(const void* snapshot_buffer)
{
	CMTS_ASSERT(cmts_is_task());
	uint32_t* e = (uint32_t*)snapshot_buffer;
	uint32_t i;
	for (i = 0; i != thread_count; ++i)
	{
		CMTS_UNLIKELY_IF(i == thread_index)
			continue;
		while (CMTS_ATOMIC_LOAD_ACQ_U32(&thread_generation_counters[i].value) == e[i])
			cmts_yield();
	}
}

CMTS_ATTR void CMTS_CALL cmts_hazard_ptr_requirements(cmts_memory_requirements* out_requirements)
{
	out_requirements->size = (size_t)thread_count * sizeof(void*);
#if UINTPTR_MAX == UINT32_MAX
	out_requirements->alignment_log2 = 2;
#else
	out_requirements->alignment_log2 = 3;
#endif
}

CMTS_ATTR void CMTS_CALL cmts_hazard_ptr_init(cmts_hazard_context* hctx, void* buffer)
{
	(void)memset((void*)buffer, 0, (size_t)thread_count * sizeof(void*));
	*hctx = (size_t)buffer;
}

CMTS_ATTR void CMTS_CALL cmts_hazard_ptr_protect(cmts_hazard_context* hctx, void* ptr)
{
	CMTS_ASSERT(cmts_is_task());
	cmts_rcu_read_begin();
#ifdef CMTS_DEBUG
	CMTS_ASSERT(CMTS_ATOMIC_XCHG_REL_UPTR((CMTS_ATOMIC(void*)*)hctx + thread_index, (size_t)ptr) == NULL);
#else
	CMTS_ATOMIC_STORE_REL_UPTR((CMTS_ATOMIC(void*)*)hctx + thread_index, (size_t)ptr);
#endif
}

CMTS_ATTR void CMTS_CALL cmts_hazard_ptr_release(cmts_hazard_context* hctx)
{
	CMTS_ASSERT(cmts_is_task());
#ifdef CMTS_DEBUG
	CMTS_ASSERT(CMTS_ATOMIC_XCHG_REL_UPTR((CMTS_ATOMIC(void*)*)hctx + thread_index, 0) != NULL);
#else
	CMTS_ATOMIC_STORE_REL_UPTR((CMTS_ATOMIC(void*)*)hctx + thread_index, 0);
#endif
	cmts_rcu_read_end();
}

CMTS_ATTR void* CMTS_CALL cmts_hazard_ptr_get(cmts_hazard_context* hctx)
{
	CMTS_ASSERT(cmts_is_task());
	return (void*)CMTS_ATOMIC_LOAD_ACQ_UPTR((CMTS_ATOMIC(void*)*)hctx + thread_index);
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_hazard_ptr_is_unreachable(const cmts_hazard_context* hctx, const void* ptr)
{
	uint8_t* i;
	uint8_t* end;
	end = (uint8_t*)hctx + thread_count * sizeof(void*);
	for (i = (uint8_t*)hctx; i != end; i += sizeof(void*))
	{
		CMTS_UNLIKELY_IF((void*)CMTS_ATOMIC_LOAD_ACQ_UPTR((CMTS_ATOMIC(void*)*)i) == ptr)
			return CMTS_FALSE;
	}
	return CMTS_TRUE;
}

CMTS_ATTR size_t CMTS_CALL cmts_processor_count()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
}

CMTS_ATTR size_t CMTS_CALL cmts_this_processor_index()
{
	PROCESSOR_NUMBER k;
	GetCurrentProcessorNumberEx(&k);
	return ((size_t)k.Group << 6) | k.Number;
}

CMTS_ATTR size_t CMTS_CALL cmts_default_task_stack_size()
{
	return 65536;
}

#ifdef CMTS_FORMAT_RESULT
static const CMTS_CHAR* format_result_names[] =
{
	"CMTS_ERROR_MEMORY_ALLOCATION",
	"CMTS_ERROR_MEMORY_DEALLOCATION",
	"CMTS_ERROR_THREAD_CREATION",
	"CMTS_ERROR_THREAD_AFFINITY",
	"CMTS_ERROR_RESUME_THREAD",
	"CMTS_ERROR_SUSPEND_THREAD",
	"CMTS_ERROR_TERMINATE_THREAD",
	"CMTS_ERROR_AWAIT_THREAD",
	"CMTS_ERROR_TASK_POOL_CAPACITY",
	"CMTS_ERROR_AFFINITY",
	"CMTS_ERROR_TASK_ALLOCATION",
	"CMTS_ERROR_FUTEX",
	"CMTS_ERROR_LIBRARY_UNINITIALIZED",
	"CMTS_ERROR_OS_INIT",
	"CMTS_OK",
	"CMTS_SYNC_OBJECT_EXPIRED",
	"CMTS_NOT_READY",
	"CMTS_ALREADY_INITIALIZED",
	"CMTS_INITIALIZATION_IN_PROGRESS"
};

static const size_t format_result_sizes[] =
{
	CMTS_STRING_SIZE("CMTS_ERROR_MEMORY_ALLOCATION"),
	CMTS_STRING_SIZE("CMTS_ERROR_MEMORY_DEALLOCATION"),
	CMTS_STRING_SIZE("CMTS_ERROR_THREAD_CREATION"),
	CMTS_STRING_SIZE("CMTS_ERROR_THREAD_AFFINITY"),
	CMTS_STRING_SIZE("CMTS_ERROR_RESUME_THREAD"),
	CMTS_STRING_SIZE("CMTS_ERROR_SUSPEND_THREAD"),
	CMTS_STRING_SIZE("CMTS_ERROR_TERMINATE_THREAD"),
	CMTS_STRING_SIZE("CMTS_ERROR_AWAIT_THREAD"),
	CMTS_STRING_SIZE("CMTS_ERROR_TASK_POOL_CAPACITY"),
	CMTS_STRING_SIZE("CMTS_ERROR_AFFINITY"),
	CMTS_STRING_SIZE("CMTS_ERROR_TASK_ALLOCATION"),
	CMTS_STRING_SIZE("CMTS_ERROR_FUTEX"),
	CMTS_STRING_SIZE("CMTS_ERROR_LIBRARY_UNINITIALIZED"),
	CMTS_STRING_SIZE("CMTS_ERROR_OS_INIT"),
	CMTS_STRING_SIZE("CMTS_OK"),
	CMTS_STRING_SIZE("CMTS_SYNC_OBJECT_EXPIRED"),
	CMTS_STRING_SIZE("CMTS_NOT_READY"),
	CMTS_STRING_SIZE("CMTS_ALREADY_INITIALIZED"),
	CMTS_STRING_SIZE("CMTS_INITIALIZATION_IN_PROGRESS")
};

CMTS_ATTR const CMTS_CHAR* CMTS_CALL cmts_format_result(cmts_result result, size_t* out_size)
{
	size_t i;
	i = (size_t)result;
	i += CMTS_RESULT_BEGIN_ENUM;
	CMTS_UNLIKELY_IF(i >= CMTS_ARRAY_SIZE(format_result_names))
		return NULL;
	CMTS_LIKELY_IF(out_size != NULL)
		*out_size = format_result_sizes[i];
	return format_result_names[i];
}
#endif

CMTS_ATTR cmts_bool CMTS_CALL cmts_ext_debug_enable_yield_trap(cmts_bool enable)
{
	cmts_bool r;
#ifdef CMTS_DEBUG
	r = yield_trap_flag;
	yield_trap_flag = enable;
#else
	r = CMTS_FALSE;
#endif
	return r;
}

CMTS_ATTR cmts_bool CMTS_CALL cmts_ext_debug_enabled()
{
#ifdef CMTS_DEBUG
	return debugger_callback != NULL;
#else
	return 0;
#endif
}

CMTS_ATTR void CMTS_CALL cmts_ext_debug_write(const cmts_ext_debug_message* message)
{
#ifdef CMTS_DEBUG
	CMTS_UNLIKELY_IF(debugger_callback != NULL)
		debugger_callback(debugger_context, message);
#endif
}
}
#endif