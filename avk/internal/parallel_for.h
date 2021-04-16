#pragma once
#include <cmts.h>
#include "platform.h"
#include "common.h"

template <typename I, typename F>
void parallel_for(I begin, I end, F&& body)
{
	if (begin == end)
		return;
	AVK_ASSERT(begin < end);
	AVK_ASSERT(cmts_is_task());
	size_t count;
	if constexpr (std::is_integral_v<I>)
	{
		if constexpr (std::is_signed_v<I>)
			count = end - begin;
		else
			count = (size_t)end - (size_t)begin;
	}
	else
	{
		count = std::distance(begin, end);
	}

	AVK_ASSERT(count < UINT32_MAX);

	cmts_result_t code;
	struct wrapper_type
	{
		F function;
		I current;
		cmts_fence_t fence;
	};
	wrapper_type wrapper = { std::forward<F>(body), begin };
	cmts_counter_t counter = CMTS_COUNTER_INIT((uint32_t)count);
	cmts_dispatch_options_t options = {};
	options.parameter = &wrapper;
	options.sync_type = CMTS_SYNC_TYPE_COUNTER;
	options.sync_object = &counter;
	for (; begin != end; ++begin)
	{
		wrapper.current = begin;
		wrapper.fence = CMTS_FENCE_INIT;

		code = cmts_dispatch([](void* ptr)
		{
			wrapper_type& wrapper = *(wrapper_type*)ptr;
			I it = std::move(wrapper.current);
			cmts_fence_signal(&wrapper.fence);
			wrapper.function(it);
		}, &options);

		if (code == CMTS_OK)
		{
			cmts_fence_await(&wrapper.fence);
		}
		else
		{
			cmts_counter_decrement(&counter);
			body(begin);
		}
	}
	code = cmts_counter_await(&counter);
}