#include "vulkan_state.h"
#include "../algorithm_thread.h"
#include "../defer.h"

static uint32_t frame_index;

void build_commands(uint32_t index)
{
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	constexpr VkClearValue clear_value =
	{
		{ 0, 1.0f / 12.0f, 1.0f / 13.0f, 1.0 }
	};
	VkRenderPassBeginInfo rinfo = {};
	rinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rinfo.framebuffer = framebuffers[index];
	rinfo.clearValueCount = 1;
	rinfo.pClearValues = &clear_value;
	rinfo.renderArea = { {}, swapchain_extent };
	rinfo.renderPass = renderpass;
	
	const auto cmd = command_buffers[index];
	vkBeginCommandBuffer(cmd, &info);
	vkCmdBeginRenderPass(cmd, &rinfo, VK_SUBPASS_CONTENTS_INLINE);
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &main_array_buffer, &offset);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, bar_graph_pipeline);
	vkCmdDraw(cmd, 6, array_size, 0, 0);
	vkCmdEndRenderPass(cmd);
	vkEndCommandBuffer(cmd);
}

void draw_main_array()
{
	array_lock.lock();
	DEFER{ array_lock.unlock(); };

	if (main_array_buffer == VK_NULL_HANDLE)
		return;

	const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

	uint32_t image_index = 0;

	auto result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, begin_render_semaphore[frame_index], VK_NULL_HANDLE, &image_index);

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		BREAKPOINT;
		return;
	}

	build_commands(image_index);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[image_index];
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &begin_render_semaphore[frame_index];
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &end_render_semaphore[frame_index];

	vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &end_render_semaphore[image_index];
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain;
	present_info.pImageIndices = &image_index;

	vkQueuePresentKHR(present_queue, &present_info);
}