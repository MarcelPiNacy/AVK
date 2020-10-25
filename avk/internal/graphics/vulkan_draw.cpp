#include "vulkan_state.h"
#include "../algorithm_thread.h"

static uint32_t frame_index;

static constexpr float clear_color[] = {};

void build_commands(uint32_t index)
{
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	VkRenderPassBeginInfo rinfo = {};
	rinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rinfo.framebuffer = framebuffers[index];
	
	const auto cmd = command_buffers[index];
	vkBeginCommandBuffer(cmd, &info);
	vkCmdBeginRenderPass(cmd, )
	vkEndCommandBuffer(cmd);
}

void draw_main_array()
{
	
}