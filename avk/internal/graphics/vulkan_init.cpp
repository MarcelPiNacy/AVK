#include "../common.h"
#include "vulkan_state.h"
#include "../main_array.h"
#include "../defer.h"
#include "../enforce.h"

using std::vector;

extern HINSTANCE hinstance;
extern HWND hwnd;

#ifdef DEBUG
static VkBool32 VKAPI_CALL debugger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* payload, void* pUserData)
{
	static char buffer[65536];
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		strcpy_s(buffer, "[ verbose ]: ");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		strcpy_s(buffer, "[ info ]: ");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		strcpy_s(buffer, "[ warning ]: ");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		strcpy_s(buffer, "[ error ]: ");
		break;
	default:
		break;
	}

	strcat_s(buffer, payload->pMessage);
	strcat_s(buffer, "\n");
	OutputDebugStringA(buffer);
	puts(buffer);

	return false;
}
#endif



static vector<char> read_file(const char* path) noexcept //TODO: IMPROVE THIS. USE MAPPED FILE?
{
	vector<char> r;
	FILE* f = nullptr;
	fopen_s(&f, path, "rb");
	if (f == nullptr)
		return r;
	fseek(f, 0, SEEK_END);
	const auto k = (uint)ftell(f);
	fseek(f, 0, SEEK_SET);
	r.resize(k);
	fread(r.data(), 1, k, f);
	fclose(f);
	return r;
}



int init_vulkan()
{
	VkResult result;
	uint32_t k;

	constexpr auto base_counter = __COUNTER__;

	{
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Array-VK";
		app_info.applicationVersion = 0;
		app_info.pEngineName = "Array-VK";
		app_info.engineVersion = 0;
		app_info.apiVersion = VK_API_VERSION_1_1;

		constexpr const char* instance_extensions[] =
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	#ifdef DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	#endif
		};

		VkInstanceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pApplicationInfo = &app_info;
		info.enabledExtensionCount = (uint32_t)c_array_size(instance_extensions);
		info.ppEnabledExtensionNames = instance_extensions;

		result = vkCreateInstance(&info, nullptr, &instance);
		if (result != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}
	}

#ifdef DEBUG
	{
		auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (fn == nullptr)
			return -(__COUNTER__ - base_counter);

		VkDebugUtilsMessengerCreateInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = debugger_callback;
		info.pUserData = nullptr;

		result = fn(instance, &info, nullptr, &debugger);
		if (result != VK_SUCCESS)
			return -(__COUNTER__ - base_counter);
	}
#endif

	{
		VkWin32SurfaceCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		info.hinstance = GetModuleHandle(nullptr);
		info.hwnd = hwnd;
		result = vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface);
		if (result != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}
	}

	{
		vkEnumeratePhysicalDevices(instance, &k, nullptr);
		auto devices = vector<VkPhysicalDevice>(k);
		vkEnumeratePhysicalDevices(instance, &k, devices.data());
		physical_device = devices[0];
		vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_properties);
	}

	{
		constexpr const char* device_extensions[] =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &k, nullptr);
		auto qfp = vector<VkQueueFamilyProperties>(k);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &k, qfp.data());

		graphics_queue_index = UINT32_MAX;
		present_queue_index = UINT32_MAX;
		transfer_queue_index = UINT32_MAX;

		uint_fast32_t found_count = 0;
		for (uint_fast32_t i = 0; i < k && found_count < 3; ++i)
		{
			const auto& e = qfp[i];
			if ((e.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (graphics_queue_index == UINT32_MAX || present_queue_index == UINT32_MAX))
			{
				graphics_queue_index = i;
				++found_count;
				const auto flag = vkGetPhysicalDeviceWin32PresentationSupportKHR(physical_device, i);
				if (flag)
				{
					present_queue_index = i;
					++found_count;
				}
			}

			if ((e.queueFlags & VK_QUEUE_TRANSFER_BIT) && transfer_queue_index == UINT32_MAX)
			{
				transfer_queue_index = i;
				++found_count;
			}
		}

		const float priorities[] = { 1.0f, 1.0f };
		const bool same = graphics_queue_index == present_queue_index && transfer_queue_index == graphics_queue_index;

		VkDeviceQueueCreateInfo queue_info[3] = {};
		if (same)
		{
			queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_info[0].queueFamilyIndex = graphics_queue_index;
			queue_info[0].pQueuePriorities = priorities;
			queue_info[0].queueCount = 3;
		}
		else
		{
			queue_info[0].sType = queue_info[1].sType = queue_info[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_info[0].queueFamilyIndex = graphics_queue_index;
			queue_info[1].queueFamilyIndex = present_queue_index;
			queue_info[2].queueFamilyIndex = transfer_queue_index;
			queue_info[0].pQueuePriorities = queue_info[1].pQueuePriorities = queue_info[2].pQueuePriorities = priorities;
			queue_info[0].queueCount = queue_info[1].queueCount = queue_info[2].queueCount = 1;
		}

		VkPhysicalDeviceFeatures features = {};

		VkDeviceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.queueCreateInfoCount = same ? 1 : 3;
		info.pQueueCreateInfos = queue_info;
		info.enabledExtensionCount = (uint32_t)c_array_size(device_extensions);
		info.ppEnabledExtensionNames = device_extensions;
		info.pEnabledFeatures = &features;

		if (vkCreateDevice(physical_device, &info, nullptr, &device) != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}

		vkGetDeviceQueue(device, graphics_queue_index, 0, &graphics_queue);
		vkGetDeviceQueue(device, present_queue_index, 0, &present_queue);
		vkGetDeviceQueue(device, transfer_queue_index, 0, &transfer_queue);
	}

	{
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &k, nullptr);
		auto formats = vector<VkSurfaceFormatKHR>(k);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &k, formats.data());

		if ((k == 1) & (formats[0].format == VK_FORMAT_UNDEFINED))
		{
			swapchain_format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
		else
		{
			for (uint_fast32_t i = 0; i < k; ++i)
			{
				swapchain_format = formats[i];
				if ((swapchain_format.format == VK_FORMAT_B8G8R8A8_UNORM) & (swapchain_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
					break;
			}
		}

		const uint32_t image_count =
			(capabilities.maxImageCount > 0) &
			(capabilities.minImageCount + 1 > capabilities.maxImageCount) ?
			capabilities.maxImageCount :
			capabilities.minImageCount + 1;

		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			swapchain_extent = capabilities.currentExtent;
		}
		else
		{
			if (capabilities.currentExtent.height == UINT32_MAX)
			{
				BREAKPOINT;
				return -(__COUNTER__ - base_counter);
			}

			RECT rect;
			GetWindowRect(hwnd, &rect);
			swapchain_extent =
			{
				std::max<uint32_t>(capabilities.minImageExtent.width, std::min<uint32_t>(capabilities.maxImageExtent.width, rect.right - rect.left)),
				std::max<uint32_t>(capabilities.minImageExtent.height, std::min<uint32_t>(capabilities.maxImageExtent.height, rect.bottom - rect.top))
			};
		}

		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &k, nullptr);
		auto present_modes = vector<VkPresentModeKHR>(k);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &k, present_modes.data());
		for (uint_fast32_t i = 0; i < k; ++i)
		{
			const bool flag = present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR;
			if (flag | (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				swapchain_present_mode = present_modes[i];
				if (flag)
					break;
			}
		}

		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = surface;
		info.minImageCount = image_count;
		info.imageFormat = swapchain_format.format;
		info.imageColorSpace = swapchain_format.colorSpace;
		info.imageExtent = swapchain_extent;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices = nullptr;
		info.preTransform = capabilities.currentTransform;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = swapchain_present_mode;
		info.clipped = false;
		info.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &info, nullptr, &swapchain) != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}

		vkGetSwapchainImagesKHR(device, swapchain, &k, nullptr);
		swapchain_images.resize(k);
		vkGetSwapchainImagesKHR(device, swapchain, &k, swapchain_images.data());
		swapchain_image_views.resize(k);
		framebuffers.resize(k);
		command_buffers.resize(k);
		begin_render_semaphore.resize(k);
		end_render_semaphore.resize(k);
	}

	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = swapchain_format.format;
		info.components = {};
		info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		for (uint_fast32_t i = 0; i < swapchain_images.size(); ++i)
		{
			info.image = swapchain_images[i];
			result = vkCreateImageView(device, &info, nullptr, &swapchain_image_views[i]);
			if (result != VK_SUCCESS)
			{
				BREAKPOINT;
				return -(__COUNTER__ - base_counter);
			}
		}
	}

	{
		VkAttachmentDescription attachment_description = {};
		VkSubpassDescription subpass_description = {};
		VkSubpassDependency subpass_dependency = {};

		attachment_description.format = swapchain_format.format;
		attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment{};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_attachment;

		subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass = 0;
		subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.srcAccessMask = 0;
		subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment_description;
		info.subpassCount = 1;
		info.pSubpasses = &subpass_description;
		info.dependencyCount = 1;
		info.pDependencies = &subpass_dependency;

		result = vkCreateRenderPass(device, &info, nullptr, &renderpass);
		if (result != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}
	}

	{
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = renderpass;
		info.attachmentCount = 1;
		info.width = swapchain_extent.width;
		info.height = swapchain_extent.height;
		info.layers = 1;
		for (uint_fast32_t i = 0; i < swapchain_images.size(); ++i)
		{
			info.pAttachments = &swapchain_image_views[i];
			result = vkCreateFramebuffer(device, &info, nullptr, &framebuffers[i]);
			if (result != VK_SUCCESS)
			{
				BREAKPOINT;
				return -(__COUNTER__ - base_counter);
			}
		}
	}

	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex = graphics_queue_index;
		result = vkCreateCommandPool(device, &info, nullptr, &command_pool);
		if (result != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}
	}

	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandPool = command_pool;
		info.commandBufferCount = (uint32_t)swapchain_images.size();
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		result = vkAllocateCommandBuffers(device, &info, command_buffers.data());
		if (result != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}

		info.commandBufferCount = 1;
		result = vkAllocateCommandBuffers(device, &info, &array_command_buffer);
		if (result != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}
	}

	{
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for (uint_fast32_t i = 0; i < swapchain_images.size(); ++i)
		{
			result = vkCreateSemaphore(device, &info, nullptr, &begin_render_semaphore[i]);
			if (result != VK_SUCCESS)
			{
				BREAKPOINT;
				return -(__COUNTER__ - base_counter);
			}
		}
		for (uint_fast32_t i = 0; i < swapchain_images.size(); ++i)
		{
			result = vkCreateSemaphore(device, &info, nullptr, &end_render_semaphore[i]);
			if (result != VK_SUCCESS)
			{
				BREAKPOINT;
				return -(__COUNTER__ - base_counter);
			}
		}
	}

	{
		VkPushConstantRange push_consts = {};
		push_consts.offset = 0;
		push_consts.size = sizeof(shader_args);
		push_consts.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pushConstantRangeCount = 1;
		info.pPushConstantRanges = &push_consts;
		result = vkCreatePipelineLayout(device, &info, nullptr, &bar_graph_pipeline_layout);
		if (result != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}
	}

	{
		VkShaderModule vertex_shader = VK_NULL_HANDLE;
		VkShaderModule fragment_shader = VK_NULL_HANDLE;
		DEFER
		{
			if (vertex_shader != VK_NULL_HANDLE)
				vkDestroyShaderModule(device, vertex_shader, nullptr);
			if (fragment_shader != VK_NULL_HANDLE)
				vkDestroyShaderModule(device, fragment_shader, nullptr);
		};

		{
			auto code = read_file("bar_graph_vs.spv");
			enforce(code.size() != 0);
			VkShaderModuleCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.codeSize = code.size();
			info.pCode = (const uint32_t*)code.data();
			result = vkCreateShaderModule(device, &info, nullptr, &vertex_shader);
			if (result != VK_SUCCESS)
			{
				BREAKPOINT;
				return -(__COUNTER__ - base_counter);
			}
			code = read_file("bar_graph_fs.spv");
			enforce(code.size() != 0);
			info.codeSize = code.size();
			info.pCode = (const uint32_t*)code.data();
			result = vkCreateShaderModule(device, &info, nullptr, &fragment_shader);
			if (result != VK_SUCCESS)
			{
				BREAKPOINT;
				return -(__COUNTER__ - base_counter);
			}
		}

		VkPipelineShaderStageCreateInfo stages[2] = {};
		stages[0].sType = stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[0].pName = stages[1].pName = "main";
		stages[0].module = vertex_shader;
		stages[1].module = fragment_shader;

		const VkVertexInputAttributeDescription attrs[] =
		{
			{ 0, 0, VK_FORMAT_R32_UINT, offsetof(item, value) },
			{ 1, 0, VK_FORMAT_R32_UINT, offsetof(item, original_position) },
			{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(item, color) }
		};

		const VkVertexInputBindingDescription bindings[] =
		{
			0, sizeof(item), VK_VERTEX_INPUT_RATE_INSTANCE
		};

		VkPipelineVertexInputStateCreateInfo inputs = {};
		inputs.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		inputs.vertexAttributeDescriptionCount = c_array_size(attrs);
		inputs.pVertexAttributeDescriptions = attrs;
		inputs.vertexBindingDescriptionCount = c_array_size(bindings);
		inputs.pVertexBindingDescriptions = bindings;

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		const VkViewport vp =
		{
			0, 0,
			(float)swapchain_extent.width, (float)swapchain_extent.height,
			0.0f, 1.0f
		};

		const VkRect2D scissor = { {}, swapchain_extent };

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.pViewports = &vp;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterization_info = {};
		rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_info.lineWidth = 1.0f;
		rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

		VkPipelineMultisampleStateCreateInfo multisample_info = {};
		multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState color_attachment = {};
		color_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo color_blend_info = {};
		color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_info.logicOp = VK_LOGIC_OP_COPY;
		color_blend_info.attachmentCount = 1;
		color_blend_info.pAttachments = &color_attachment;

		VkGraphicsPipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.stageCount = 2;
		info.pStages = stages;
		info.pVertexInputState = &inputs;
		info.pInputAssemblyState = &input_assembly;
		info.pViewportState = &viewport_info;
		info.pRasterizationState = &rasterization_info;
		info.pMultisampleState = &multisample_info;
		info.pColorBlendState = &color_blend_info;
		info.layout = bar_graph_pipeline_layout;
		info.renderPass = renderpass;

		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &bar_graph_pipeline);
		if (result != VK_SUCCESS)
		{
			BREAKPOINT;
			return -(__COUNTER__ - base_counter);
		}
	}

	return 0;
}