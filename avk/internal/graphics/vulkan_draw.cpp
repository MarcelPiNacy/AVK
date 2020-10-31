#include "vulkan_state.h"
#include "../algorithm_thread.h"
#include "../defer.h"
#include "../enforce.h"

static uint32_t frame_index;

void build_commands(uint32_t index)
{
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	constexpr VkClearValue clear_value = {};
	VkRenderPassBeginInfo rinfo = {};
	rinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rinfo.framebuffer = framebuffers[index];
	rinfo.clearValueCount = 1;
	rinfo.pClearValues = &clear_value;
	rinfo.renderArea = { {}, swapchain_extent };
	rinfo.renderPass = renderpass;
	
	enforce(main_array_size != 0);
	enforce(swapchain_extent.width != 0);
	enforce(swapchain_extent.height != 0);

	const shader_args args =
	{
		main_array_size
	};

	const auto cmd = command_buffers[index];
	vkBeginCommandBuffer(cmd, &info);
	vkCmdBeginRenderPass(cmd, &rinfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, bar_graph_pipeline);
	const VkDeviceSize offsets[1] = {};
	vkCmdBindVertexBuffers(cmd, 0, 1, &main_array_buffer, offsets);
	vkCmdPushConstants(cmd, bar_graph_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(shader_args), &args);
	vkCmdDraw(cmd, 6, main_array_size, 0, 0);
	vkCmdEndRenderPass(cmd);
	vkEndCommandBuffer(cmd);
}

void draw_main_array()
{
	main_array_lock.lock();
	DEFER{ main_array_lock.unlock(); };

	if (main_array_size == 0)
		return;

	const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

	uint32_t image_index = 0;

	VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, begin_render_semaphore[frame_index], VK_NULL_HANDLE, &image_index);

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		BREAKPOINT;
		return;
	}

	build_commands(frame_index);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[image_index];
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &begin_render_semaphore[frame_index];
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &end_render_semaphore[frame_index];

	result = vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
	{
		BREAKPOINT;
		return;
	}

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &end_render_semaphore[frame_index];
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain;
	present_info.pImageIndices = &image_index;

	result = vkQueuePresentKHR(present_queue, &present_info);
	if (result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			return;
		BREAKPOINT;
		return;
	}

	++frame_index;
	if (frame_index == swapchain_images.size())
		frame_index = 0;
	vkQueueWaitIdle(present_queue);
}