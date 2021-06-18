#include "algorithm_thread.h"
#include "common.h"
#include <thread>
#include <cmts.h>
#pragma comment(lib, "Synchronization.lib")

static cmts_event done = CMTS_EVENT_INIT;

namespace algorithm_thread
{
	void launch(sort_function_pointer sort)
	{
		cmts_result result;
		bool initialized = cmts_is_initialized();
		if (cmts_event_state(&done) == CMTS_OK || !initialized)
		{
			if (initialized)
			{
				cmts_purge_all();
				return;
			}
			cmts_init_options init_options = {};
			init_options.allocate_function = nullptr;
			init_options.task_stack_size = 4096;
			init_options.thread_count = (uint32_t)cmts_processor_count();
			init_options.flags = 0;
			init_options.max_tasks = (uint32_t)main_array::size();
			result = cmts_init(&init_options);
		}
		done = CMTS_EVENT_INIT;
		cmts_dispatch_options options = {};
		options.parameter = sort;
		options.sync_object = &done;
		options.sync_type = CMTS_SYNC_TYPE_EVENT;
		options.flags = CMTS_DISPATCH_FLAGS_FORCE;
		result = cmts_dispatch([](void* param)
		{
			sort_stats::clear();
			((sort_function_pointer)param)({});
		}, &options);
		AVK_ASSERT(result == CMTS_OK);
	}

	bool is_paused()
	{
		return cmts_is_paused();
	}

	bool is_idle()
	{
		return cmts_event_state(&done) != CMTS_OK;
	}

	void pause()
	{
		cmts_pause();
	}

	void resume()
	{
		cmts_resume();
	}

	void terminate()
	{
		cmts_terminate(nullptr);
	}
}