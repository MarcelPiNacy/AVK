#ifndef CMTS_INCLUDED
#define CMTS_INCLUDED
#include <stdint.h>
#include "cmts_defs.h"

CMTS_EXTERN_C_BEGIN

// CORE DATA TYPES:

#ifdef __cplusplus
typedef bool cmts_bool_t;
#else
typedef _Bool cmts_boolean_t;
#endif

typedef uint64_t cmts_task_id_t;
typedef uint32_t cmts_tss_id_t;

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
	CMTS_EXT_TYPE_TASK_NAME,
	CMTS_EXT_TYPE_DEBUGGER,

	CMTS_EXT_TYPE_MIN_ENUM = CMTS_EXT_TYPE_TASK_NAME,
	CMTS_EXT_TYPE_MAX_ENUM = CMTS_EXT_TYPE_DEBUGGER,
} cmts_ext_type_t;

typedef struct cmts_init_options_t
{
	cmts_allocate_function_pointer_t allocate_function;
	size_t task_stack_size;
	cmts_init_flags_t flags;
	uint32_t thread_count;
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

typedef struct cmts_barrier_t
{
	uint8_t data[CMTS_BARRIER_DATA_SIZE];
} cmts_barrier_t;

typedef struct cmts_event_t
{
	uint8_t data[CMTS_EVENT_DATA_SIZE];
} cmts_event_t;

typedef struct cmts_counter_t
{
	uint8_t data[CMTS_COUNTER_DATA_SIZE];
} cmts_counter_t;

typedef struct cmts_mutex_t
{
	uint8_t data[CMTS_MUTEX_DATA_SIZE];
} cmts_mutex_t;

typedef struct cmts_rwlock_t
{
	uint8_t data[CMTS_RWLOCK_DATA_SIZE];
} cmts_rwlock_t;

typedef struct cmts_minimize_options_t
{
	const void* ext;
	uint64_t timeout_nanoseconds;
} cmts_minimize_options_t;

// EXTENSION-ONLY TYPES:
// EXT - DEBUGGER TYPES:

typedef enum cmts_ext_debugger_message_severity_t
{
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_INFO,
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_WARNING,
	CMTS_EXT_DEBUGGER_MESSAGE_SEVERITY_ERROR,
} cmts_ext_debugger_message_severity_t;

typedef struct cmts_ext_debugger_message_t
{
	const void* ext;
	const CMTS_CHAR* message;
	size_t message_length;
	cmts_ext_debugger_message_severity_t severity;
} cmts_ext_debugger_message_t;

typedef void(CMTS_CALL* cmts_ext_debugger_message_callback_t)(void* context, const cmts_ext_debugger_message_t* message);

typedef struct cmts_ext_debugger_init_options_t
{
	const void* next;
	cmts_ext_type_t ext_type; //Must be set to CMTS_EXT_TYPE_DEBUGGER.
	void* context;
	cmts_ext_debugger_message_callback_t message_callback;
} cmts_ext_debugger_init_options_t;

// EXT - TASK NAME TYPES:

typedef struct cmts_ext_task_name_t
{
	const void* next;
	cmts_ext_type_t ext_type;
	uint32_t length;
	const char* name;
} cmts_ext_task_name_t;



CMTS_ATTR cmts_result_t CMTS_CALL cmts_lib_init(const cmts_init_options_t* options);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_lib_pause();
CMTS_ATTR cmts_result_t CMTS_CALL cmts_lib_resume();
CMTS_ATTR void CMTS_CALL cmts_lib_exit_signal();
CMTS_ATTR cmts_result_t CMTS_CALL cmts_lib_exit_await(cmts_deallocate_function_pointer_t deallocate);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_lib_terminate(cmts_deallocate_function_pointer_t deallocate);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_lib_is_initialized();
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_lib_is_online();
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_lib_is_paused();
CMTS_ATTR cmts_result_t CMTS_CALL cmts_lib_minimize(const cmts_minimize_options_t* options);

CMTS_ATTR cmts_result_t CMTS_CALL cmts_dispatch(cmts_task_function_pointer_t entry_point, cmts_dispatch_options_t* options);
CMTS_ATTR void CMTS_CALL cmts_yield();
CMTS_NORETURN CMTS_ATTR void CMTS_CALL cmts_exit();
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_is_task();
CMTS_ATTR cmts_task_id_t CMTS_CALL cmts_this_task_id();

CMTS_ATTR cmts_tss_id_t CMTS_CALL cmts_tss_new(cmts_destructor_function_pointer_t destructor);
CMTS_ATTR void* CMTS_CALL cmts_tss_get(cmts_tss_id_t id);
CMTS_ATTR void CMTS_CALL cmts_tss_set(cmts_tss_id_t id, void* ptr);
CMTS_ATTR void CMTS_CALL cmts_tss_delete(cmts_tss_id_t id);

CMTS_NODISCARD CMTS_ATTR cmts_task_id_t CMTS_CALL cmts_task_new();
CMTS_ATTR uint8_t CMTS_CALL cmts_task_get_priority(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_set_priority(cmts_task_id_t task_id, uint8_t priority);
CMTS_ATTR void CMTS_CALL cmts_task_set_parameter(cmts_task_id_t task_id, void* parameter);
CMTS_ATTR void CMTS_CALL cmts_task_set_function(cmts_task_id_t task_id, cmts_task_function_pointer_t function);
CMTS_ATTR cmts_task_function_pointer_t CMTS_CALL cmts_task_get_function(cmts_task_id_t task_id);
CMTS_ATTR void* CMTS_CALL cmts_task_get_parameter(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_attach_event(cmts_task_id_t task_id, cmts_event_t* event);
CMTS_ATTR void CMTS_CALL cmts_task_attach_counter(cmts_task_id_t task_id, cmts_counter_t* counter);
CMTS_ATTR void CMTS_CALL cmts_task_sleep(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_wake(cmts_task_id_t task_id);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_valid(cmts_task_id_t task_id);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_sleeping(cmts_task_id_t task_id);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_task_is_running(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_dispatch(cmts_task_id_t task_id);
CMTS_ATTR void CMTS_CALL cmts_task_delete(cmts_task_id_t task_id);

CMTS_ATTR void CMTS_CALL cmts_barrier_init(cmts_barrier_t* barrier);
CMTS_ATTR void CMTS_CALL cmts_barrier_signal(cmts_barrier_t* barrier);
CMTS_ATTR void CMTS_CALL cmts_barrier_await(cmts_barrier_t* barrier);
CMTS_ATTR void CMTS_CALL cmts_barrier_reset(cmts_barrier_t* barrier);

CMTS_ATTR void CMTS_CALL cmts_event_init(cmts_event_t* event);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_signal(cmts_event_t* event);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_await(cmts_event_t* event);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_event_reset(cmts_event_t* event);

CMTS_ATTR void CMTS_CALL cmts_counter_init(cmts_counter_t* counter, size_t start_value);
CMTS_ATTR size_t CMTS_CALL cmts_counter_query(const cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_increment(cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_decrement(cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_await(cmts_counter_t* counter);
CMTS_ATTR cmts_result_t CMTS_CALL cmts_counter_reset(cmts_counter_t* counter, size_t new_start_value);

CMTS_ATTR void CMTS_CALL cmts_mutex_init(cmts_mutex_t* mutex);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_mutex_is_locked(const cmts_mutex_t* mutex);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_mutex_try_lock(cmts_mutex_t* mutex);
CMTS_ATTR void CMTS_CALL cmts_mutex_lock(cmts_mutex_t* mutex);
CMTS_ATTR void CMTS_CALL cmts_mutex_unlock(cmts_mutex_t* mutex);

CMTS_ATTR void CMTS_CALL cmts_rwlock_init(cmts_rwlock_t* rwlock);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_rwlock_is_locked(const cmts_rwlock_t* rwlock);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_rwlock_is_locked_exclusive(const cmts_rwlock_t* rwlock);
CMTS_ATTR size_t CMTS_CALL cmts_rwlock_shared_count(const cmts_rwlock_t* rwlock);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_rwlock_try_lock(cmts_rwlock_t* rwlock);
CMTS_ATTR void CMTS_CALL cmts_rwlock_lock(cmts_rwlock_t* rwlock);
CMTS_ATTR void CMTS_CALL cmts_rwlock_unlock(cmts_rwlock_t* rwlock);
CMTS_ATTR cmts_bool_t CMTS_CALL cmts_rwlock_try_lock_exclusive(cmts_rwlock_t* rwlock);
CMTS_ATTR void CMTS_CALL cmts_rwlock_lock_exclusive(cmts_rwlock_t* rwlock);
CMTS_ATTR void CMTS_CALL cmts_rwlock_unlock_exclusive(cmts_rwlock_t* rwlock);
CMTS_ATTR void CMTS_CALL cmts_rwlock_switch_to_exclusive(cmts_rwlock_t* rwlock);
CMTS_ATTR void CMTS_CALL cmts_rwlock_switch_to_shared(cmts_rwlock_t* rwlock);

CMTS_ATTR size_t CMTS_CALL cmts_this_worker_thread_index();
CMTS_ATTR size_t CMTS_CALL cmts_worker_thread_count();
CMTS_ATTR size_t CMTS_CALL cmts_processor_count();
CMTS_ATTR size_t CMTS_CALL cmts_this_processor_index();
CMTS_ATTR size_t CMTS_CALL cmts_default_task_stack_size();

CMTS_ATTR void CMTS_CALL cmts_enable_yield_trap();
CMTS_ATTR void CMTS_CALL cmts_disable_yield_trap();

CMTS_ATTR cmts_bool_t CMTS_ATTR cmts_ext_debugger_enabled();

CMTS_ATTR void CMTS_ATTR cmts_ext_task_name_set(cmts_task_id_t id, const CMTS_CHAR* name, size_t length);
CMTS_ATTR void CMTS_ATTR cmts_ext_task_name_swap(cmts_task_id_t id, const CMTS_CHAR* name, size_t length, const CMTS_CHAR** out_old_name, size_t* out_old_length);
CMTS_ATTR void CMTS_ATTR cmts_ext_task_name_get(cmts_task_id_t id, const CMTS_CHAR** out_name, size_t* out_length);
CMTS_ATTR void CMTS_ATTR cmts_ext_task_name_remove(cmts_task_id_t id);
CMTS_ATTR cmts_bool_t cmts_ext_task_name_enabled();

CMTS_EXTERN_C_END

#ifdef CMTS_IMPLEMENTATION
#include "../source/cmts_implementation.inl"
#endif

#endif