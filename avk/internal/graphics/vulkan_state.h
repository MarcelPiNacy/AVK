#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <algorithm>
#include <vector>
#include <new>
#include "../common.h"
#include "../locks.h"
#undef max
#undef min

struct item;



inline VkInstance instance;
#ifdef DEBUG
inline VkDebugUtilsMessengerEXT debugger;
#endif
inline VkPhysicalDevice physical_device;
inline VkPhysicalDeviceMemoryProperties physical_device_properties;
inline VkSurfaceKHR surface;
inline VkDevice device;

inline uint32_t graphics_queue_index;
inline uint32_t present_queue_index;
inline uint32_t transfer_queue_index;
inline VkQueue graphics_queue;
inline VkQueue present_queue;
inline VkQueue transfer_queue;

inline VkSwapchainKHR swapchain;
inline VkSurfaceFormatKHR swapchain_format;
inline VkExtent2D swapchain_extent;
inline VkPresentModeKHR swapchain_present_mode;

inline VkRenderPass renderpass;

inline VkCommandPool command_pool;
inline VkCommandBuffer array_command_buffer;

inline std::vector<VkImage> swapchain_images;
inline std::vector<VkImageView> swapchain_image_views;
inline std::vector<VkFramebuffer> framebuffers;
inline std::vector<VkCommandBuffer> command_buffers;
inline std::vector<VkSemaphore> begin_render_semaphore;
inline std::vector<VkSemaphore> end_render_semaphore;

inline VkPipeline bar_graph_pipeline;
inline VkPipelineLayout bar_graph_pipeline_layout;

inline uint32_t main_array_capacity;
inline uint32_t main_array_size;
inline VkBuffer main_array_buffer;
inline VkDeviceMemory main_array_memory;
inline item* main_array_mapping;



struct shader_args
{
	uint32_t array_size;
};