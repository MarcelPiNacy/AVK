#include "algorithm_thread.h"
#include "common.h"
#include <Comet.hpp>

namespace algorithm_thread
{
	static bool done = true;

	void launch(sort_function_pointer sort)
	{
		done = false;
		sort_stats::clear();
		if (Comet::GetSchedulerState() == Comet::SchedulerState::Uninitialized)
		{
			auto options = Comet::InitOptions::Default();
			options.max_tasks = main_array::size();
			options.max_tasks *= floor_log2(options.max_tasks);
			Comet::Init();
		}
		Comet::Dispatch([](void* param)
		{
			((sort_function_pointer)param)({});
			done = true;
		}, sort);
	}

	bool is_paused()
	{
		return Comet::GetSchedulerState() == Comet::SchedulerState::Paused;
	}

	bool is_idle()
	{
		return done;
	}

	void pause()
	{
		Comet::Pause();
	}

	void resume()
	{
		Comet::Resume();
	}

	void await_exit()
	{
		Comet::Shutdown();
		Comet::Finalize();
	}

	void terminate()
	{
		Comet::Terminate();
		done = true;
	}
}