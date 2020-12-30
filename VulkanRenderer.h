#pragma once

#include "vulkanrenderercore_global.h"
#include "AbstractRenderer.h"
#include "Component.h"
#include "GameObject.h"
#include "VulkanConfig.h"
#include "ShaderCompiler.h"

#include <vector>
#include <QMessageBox>
#include <vulkan/vulkan_win32.h>

using namespace std;

class VULKANRENDERERCORE_EXPORT VulkanRenderer:public AbstractRenderer
{
private:
    void doInstanceCreation()
    {
		//the extensions of Vulkan instance
		vector<const char*> vkInstanceExtensionNames;
		vkInstanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);      
		vkInstanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		vkInstanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		/* Create the instance of Vulkan Application*/
		VkApplicationInfo app_info  = {};
		app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;  // the type of struct
		app_info.pNext              = NULL;                                // pointer of user data
		app_info.pApplicationName   = VulkanConfig::AppInfo::AppName;      // the name of application
		app_info.applicationVersion = VulkanConfig::AppInfo::AppVersion;   // the version of application
		app_info.pEngineName        = VulkanConfig::AppInfo::EngineName;   // the name of engine
		app_info.engineVersion      = VulkanConfig::AppInfo::EngineVersion;// the version of engine
		app_info.apiVersion         = VulkanConfig::AppInfo::ApiVersion;   // vulkan API version

		/* the names of layer that we except to launch*/
		std::vector<const char*> exceptedLayerNames;
		exceptedLayerNames.push_back("VK_LAYER_LUNARG_core_validation");
		exceptedLayerNames.push_back("VK_LAYER_LUNARG_parameter_validation");
		exceptedLayerNames.push_back("VK_LAYER_LUNARG_standard_validation");

		uint32_t layerCount = -1; // the count of layer 

		//try to get the count of layer
		vkEnumerateInstanceLayerProperties(&layerCount, NULL);

		//declare a vector for saving the VkLayerProperties,and then resize it to layerCount
		vector<VkLayerProperties> vkLayerProperties;
		vkLayerProperties.resize(layerCount);

		// get all vkLayerProperties
		vkEnumerateInstanceLayerProperties(&layerCount, vkLayerProperties.data());

		// a vector which saves the layer that support to be verified
		std::vector<std::string*> layerNames;

		// a vector which saves the extensions that support to be verified
		std::vector<std::string*> extensionNames;

		// and then traverse all VklayerProperties
		for (int i = 0; i < vkLayerProperties.size(); i++)
		{
			// get current layerProperty
			VkLayerProperties layerProperty = vkLayerProperties[i];

			// a flag represents whether the layer is need or not
			bool flag = false;

			// figure out whether current layerProperty is in exceptedLayer;
			for (auto s : exceptedLayerNames) 
			{
				if (strcmp(s, layerProperty.layerName) == 0) 
				{
					flag = true; 
					break;
				}
			}
			//add the current layer name to the layerNames
			if (flag) 
			{
				layerNames.push_back(new std::string(layerProperty.layerName));
			}

			// the count of extension's property corresponding to current layer
			uint32_t propertyCount;
			vkEnumerateInstanceExtensionProperties(layerProperty.layerName, &propertyCount, NULL);

			// a vector to save extension's property
			std::vector<VkExtensionProperties> propertiesList;

			// resize it to propertyCount
			propertiesList.resize(propertyCount);

			vkEnumerateInstanceExtensionProperties(layerProperty.layerName, &propertyCount, propertiesList.data());
			for (auto ep : propertiesList) {
				// if the validation layer is need
				if (flag) {	
					// figure out whether current extension's name is in extensionName;
					bool contains = false;
					for (auto s : exceptedLayerNames)
					{
						if (strcmp(ep.extensionName, s) == 0)
						{
							contains = true;
							break;
						}
					}
					// if not contain, add it to extensionName 
					if (!contains)
					{
						extensionNames.push_back(new string(ep.extensionName));
					}
				}
			}
		}
		// add needed extensions to extensionNames
		for (auto s :extensionNames) 
		{
			vkInstanceExtensionNames.push_back((*s).c_str());
		}
		// clear exceptedLayerNames
		exceptedLayerNames.clear();

		//add the layer that support to be verified to exceptedLayerNames
		for (auto s :layerNames) 
		{
			exceptedLayerNames.push_back((*s).c_str());
		}

		VkInstanceCreateInfo inst_info    = {};
		inst_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO	;// the type of struct
		inst_info.pNext                   = NULL;									 // pointer of user data
		inst_info.pApplicationInfo        = &app_info;								 // bind app info struct
		inst_info.enabledExtensionCount   = vkInstanceExtensionNames.size();		 // count of extensions
		inst_info.ppEnabledExtensionNames = vkInstanceExtensionNames.data();		 // pointer of extensions
		inst_info.enabledLayerCount       = exceptedLayerNames.size();				 // count of layer
		inst_info.ppEnabledLayerNames     = exceptedLayerNames.data();				 // pointer of layer

		VkAssert(vkCreateInstance(&inst_info, NULL, &Asset.vkInstance) == VK_SUCCESS);
    }
	void doEnumPhysicalDevices()
	{
		// get the count of physical device 
		VkAssert(vkEnumeratePhysicalDevices(Asset.vkInstance, &Asset.gpuCount, NULL) == VK_SUCCESS);

		// resize gpus vector
		Asset.gpus.resize(Asset.gpuCount);

		// get physical device
		VkAssert(vkEnumeratePhysicalDevices(Asset.vkInstance, &Asset.gpuCount, Asset.gpus.data()) == VK_SUCCESS);
		// get memory property of first-used GPU
		vkGetPhysicalDeviceMemoryProperties(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], &Asset.gpuMemoryProperty);

		// get GPU's property
		vkGetPhysicalDeviceProperties(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], &Asset.gpuProperty);
	}
	void doLogicalDeviceCreation()
	{
		// get the count of QueueFamily in GPU 0
		vkGetPhysicalDeviceQueueFamilyProperties(Asset.gpus[0], &Asset.gpuQueueFamilyCount, NULL);

		// resize gpuQueueFamilyProperty
		Asset.gpuQueueFamilyProperty.resize(Asset.gpuQueueFamilyCount);

		// get the QueueFamilyProperty of GPU 0
		vkGetPhysicalDeviceQueueFamilyProperties(Asset.gpus[0], &Asset.gpuQueueFamilyCount, Asset.gpuQueueFamilyProperty.data());

		// fill VkDeviceQueueCreateInfo struct
		VkDeviceQueueCreateInfo queueInfo = {};

		// flag 'found' is to record whether we find a QueueFamily that supports Graphic work
		// if we find it ,record the index of QueueFamily 
		bool found = false;
		for (unsigned int i = 0; i < Asset.gpuQueueFamilyCount; i++) 
		{
			// VK_QUEUE_GRAPHICS_BIT represents a QueueFamily supports Graphic work
			if (Asset.gpuQueueFamilyProperty[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				// if we find it
				queueInfo.queueFamilyIndex = i;
				Asset.gpuQueueGraphicsFamilyIndex = i;
				found = true;
				break;
			}
		}

		// declare an array to record the priorities of Queues
		float queue_priorities[1] = { 0.0 };

		queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pNext            = NULL;
		queueInfo.queueCount       = 1;
		queueInfo.pQueuePriorities = queue_priorities;
		queueInfo.queueFamilyIndex = Asset.gpuQueueGraphicsFamilyIndex;

		// a vector to store device extension's name;
		vector<const char*> vkDeviceExtensionNames;

		//VK_KHR_SWAPCHAIN_EXTENSION_NAME : whether GPU support swapchain
		vkDeviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		// fill the VkDeviceCreateInfo struct
		VkDeviceCreateInfo deviceInfo      = {};
		deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext                   = NULL;
		deviceInfo.queueCreateInfoCount    = 1;
		deviceInfo.pQueueCreateInfos       = &queueInfo;
		deviceInfo.enabledExtensionCount   = vkDeviceExtensionNames.size();// the count of Device Extensions
		deviceInfo.ppEnabledExtensionNames = vkDeviceExtensionNames.data();// the pointer of Device Extensions
		deviceInfo.enabledLayerCount       = 0;                             // the count of layer
		deviceInfo.ppEnabledLayerNames     = NULL;                          // the pointer of layer
		deviceInfo.pEnabledFeatures        = NULL;                          // the pointer of enabled features

		// Create Logical Device
		VkAssert(vkCreateDevice(Asset.gpus[0], &deviceInfo, NULL, &Asset.vkDevice) == VK_SUCCESS);
	}
	void doCommandBufferCreation()
	{
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {}; 
		cmdPoolCreateInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; 	      //  the type of struct
		cmdPoolCreateInfo.queueFamilyIndex        = Asset.gpuQueueGraphicsFamilyIndex ;               //  bind index of queueGraphicsFamily 
		cmdPoolCreateInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;	  //  the behavior flag of CommandBuffer

		// Create Command Pool
		VkAssert(vkCreateCommandPool(Asset.vkDevice, &cmdPoolCreateInfo, NULL, &Asset.vkCmdPool) == VK_SUCCESS);

		// Create an instance of command buffer allocation information structure
		VkCommandBufferAllocateInfo cmdBAI = {};
		cmdBAI.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;   //  the type of struct
		cmdBAI.commandPool                 = Asset.vkCmdPool;                               //  bind the Command Pool
		cmdBAI.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;                  //  the level of allocated Command Buffer 
		cmdBAI.commandBufferCount          = 1;                                                //  count of Command Buffer

		// Create Command Buffer
		VkAssert(vkAllocateCommandBuffers(Asset.vkDevice, &cmdBAI, &Asset.vkCmdBuffer) == VK_SUCCESS);  
	}
	void doSwapChainCreation()
	{
#ifdef _WIN32
		// fill instance of KHR surface CreateInfo structure 
		VkWin32SurfaceCreateInfoKHR createInfo  = {};
		createInfo.sType						= VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd							= Asset.hWnd;  // window to render
		createInfo.hinstance					= Asset.hInst; // instance of Windows Application

		// Create KHR surface
		VkAssert(vkCreateWin32SurfaceKHR(Asset.vkInstance, &createInfo, nullptr, &Asset.vkSurface) == VK_SUCCESS);

		// traverse all the queue families to find one that support present
		VkBool32* pSupportsPresent = new VkBool32[Asset.gpuQueueFamilyCount];
		for (uint32_t i = 0; i < Asset.gpuQueueFamilyCount; i++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], i, Asset.vkSurface, &pSupportsPresent[i]);
		}

		// the index of Queue Family that support Graphics Work 
		Asset.gpuQueueGraphicsFamilyIndex = UINT32_MAX;  

		// the index of Queue Family that support Present  Work 
		Asset.gpuQueuePresentFamilyIndex  = UINT32_MAX; 

		// traverse all the Queue Families
		for (uint32_t i = 0; i < Asset.gpuQueueFamilyCount; ++i)
		{
			// if supports Graphics work
			if ((Asset.gpuQueueFamilyProperty[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				// record the index
				if (Asset.gpuQueueGraphicsFamilyIndex == UINT32_MAX)
				{
					Asset.gpuQueueGraphicsFamilyIndex = i;
				}
				// if supports both Graphics and Present work
				if (pSupportsPresent[i] == VK_TRUE)
				{
					Asset.gpuQueueGraphicsFamilyIndex = Asset.gpuQueuePresentFamilyIndex = i;
					break;
				}
			}
		}
		// if didn't find a Queue Family that supports both Graphics and Present work
		// find and record it's index
		if (Asset.gpuQueuePresentFamilyIndex == UINT32_MAX)
		{
			for (size_t i = 0; i < Asset.gpuQueueFamilyCount; ++i)
			{
				if (pSupportsPresent[i] == VK_TRUE)
				{
					Asset.gpuQueuePresentFamilyIndex = i;
					break;
				}
			}
		}
		delete pSupportsPresent;
		pSupportsPresent = NULL;

		// make sure we have found found Queue Family supports Graphics work and Queue Family supports Present work
		VkAssert(Asset.gpuQueueGraphicsFamilyIndex == UINT32_MAX || Asset.gpuQueuePresentFamilyIndex == UINT32_MAX);

		// count of Surface format
		uint32_t formatCount = -1;
		
		VkAssert(vkGetPhysicalDeviceSurfaceFormatsKHR(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], Asset.vkSurface, &formatCount, NULL) == VK_SUCCESS);

		VkSurfaceFormatKHR* vkSurfaceformats = new VkSurfaceFormatKHR [formatCount];

		// get the supported formats of Surface
		VkAssert(vkGetPhysicalDeviceSurfaceFormatsKHR(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], Asset.vkSurface, &formatCount, vkSurfaceformats) == VK_SUCCESS);


		for (unsigned int i = 0; i < formatCount; i++) 
		{
			// record supported formats
			Asset.vkFormats[i] = vkSurfaceformats[i].format;
		}

		// if format is undefined , use generic format to instead
		if (formatCount == 1 && vkSurfaceformats[0].format == VK_FORMAT_UNDEFINED)
		{
			Asset.vkFormats[0] = VK_FORMAT_B8G8R8A8_UNORM;
		}

		delete vkSurfaceformats;
		vkSurfaceformats = NULL;

		VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
		// get capabilities of Surface
		VkAssert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex],Asset.vkSurface, &vkSurfaceCapabilities) == VK_SUCCESS);

		uint32_t vkPresentModeCount = -1;
		VkAssert(vkGetPhysicalDeviceSurfacePresentModesKHR(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], Asset.vkSurface, &vkPresentModeCount, NULL) == VK_SUCCESS);

		std::vector<VkPresentModeKHR> vkPresentModes;
		vkPresentModes.resize(vkPresentModeCount);

		VkAssert(vkGetPhysicalDeviceSurfacePresentModesKHR(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], Asset.vkSurface, &vkPresentModeCount, vkPresentModes.data()) == VK_SUCCESS);

		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;//确定交换链显示模式

		// Detail of VK PRESENT MODE

		// VK_PRESENT_MODE_IMMEDIATE_KHR:
		// Means that images submitted will be immediately presented to the screen,which may cause tearing;

		// VK_PRESENT_MODE_FIFO_KHR:
		// Means that the swap chain is a queue
		// When it is displayed, it takes an image from the head of the queue
		// and the program inserts the rendered image to the end of the queue.
		// If the queue is full, the program has to wait, which is almost like vertical synchronization
		// and the display refresh time is vertical blank;


		// VK_PRESENT_MODE_FIFO_RELAXED_KHR:
		// In the last vertical blank, if the application is late and the queue is empty, the mode will be different from the previous one.In this way, instead of waiting for the next vertical blank, the image will be transmitted directly to the screen, which may cause tearing;

		// VK_PRESENT_MODE_MAILBOX_KHR:
		// This is another variation of the second mode.When the queue is full, it will not block the application, and the images already in the queue will be replaced with new ones.This mode can achieve triple bufferingand avoid tearing, which is much less delay than vertical synchronization using double buffering.
		
		for (size_t i = 0; i < swapchainPresentMode; i++)
		{
			if (vkPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (vkPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		// Get the width and height of surface

		// if size if not defined(0xFFFFFFFF)
		if (vkSurfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
		{
			RECT rect;
			GetWindowRect(Asset.hWnd, &rect);
			Asset.screenWidth = rect.right - rect.left;
			Asset.screenHeight = rect.bottom - rect.left;
			//设置宽度为窗体宽度
			Asset.vkSwapchainExtent.width = Asset.screenWidth;
			//设置高度为窗体高度
			Asset.vkSwapchainExtent.height = Asset.screenHeight;
			//宽度设置值限制到最大值与最小值之间
			if (Asset.vkSwapchainExtent.width < vkSurfaceCapabilities.minImageExtent.width)
			{
				Asset.vkSwapchainExtent.width = vkSurfaceCapabilities.minImageExtent.width;
			}
			else if (Asset.vkSwapchainExtent.width > vkSurfaceCapabilities.maxImageExtent.width)
			{
				Asset.vkSwapchainExtent.width = vkSurfaceCapabilities.maxImageExtent.width;
			}
			//高度设置值限制到最大值与最小值之间
			if (Asset.vkSwapchainExtent.height < vkSurfaceCapabilities.minImageExtent.height)
			{
				Asset.vkSwapchainExtent.height = vkSurfaceCapabilities.minImageExtent.height;
			}
			else if (Asset.vkSwapchainExtent.height > vkSurfaceCapabilities.maxImageExtent.height)
			{
				Asset.vkSwapchainExtent.height = vkSurfaceCapabilities.maxImageExtent.height;
			}
			printf("使用自己设置的 宽度 %d 高度 %d\n", Asset.vkSwapchainExtent.width, Asset.vkSwapchainExtent.height);
		}
		else
		{
			//若表面有确定尺寸
			Asset.vkSwapchainExtent = vkSurfaceCapabilities.currentExtent;
			printf("使用获取的surface能力中的 宽度 %d 高度 %d\n", Asset.vkSwapchainExtent.width, Asset.vkSwapchainExtent.height);
		}

		Asset.screenWidth = Asset.vkSwapchainExtent.width;//记录实际采用的宽度
		Asset.screenHeight = Asset.vkSwapchainExtent.height;//记录实际采用的高度

		//期望交换链中的最少图像数量
		uint32_t desiredMinNumberOfSwapChainImages = vkSurfaceCapabilities.minImageCount + 1;
		//将数量限制到范围内
		if ((vkSurfaceCapabilities.maxImageCount > 0) && (desiredMinNumberOfSwapChainImages > vkSurfaceCapabilities.maxImageCount))
		{
			desiredMinNumberOfSwapChainImages = vkSurfaceCapabilities.maxImageCount;
		}

		//KHR表面变换标志
		VkSurfaceTransformFlagBitsKHR preTransform;
		if (vkSurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)//若支持所需的变换
		{
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else//若不支持变换
		{
			preTransform = vkSurfaceCapabilities.currentTransform;
		}

		VkSwapchainCreateInfoKHR  swapChainCreateInfo = {};//构建交换链创建信息结构体实例
		swapChainCreateInfo.sType                     = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;//结构体类型
		swapChainCreateInfo.surface                   = Asset.vkSurface;//指定KHR表面
		swapChainCreateInfo.minImageCount             = desiredMinNumberOfSwapChainImages;//最少图像数量
		swapChainCreateInfo.imageFormat               = Asset.vkFormats[0];//图像格式
		swapChainCreateInfo.imageExtent.width         = Asset.vkSwapchainExtent.width;//交换链图像宽度
		swapChainCreateInfo.imageExtent.height        = Asset.vkSwapchainExtent.height;//交换链图像高度
		swapChainCreateInfo.preTransform              = preTransform;//指定变换标志
		swapChainCreateInfo.compositeAlpha            = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//混合Alpha值
		swapChainCreateInfo.imageArrayLayers          = 1;//图像数组层数
		swapChainCreateInfo.presentMode               = swapchainPresentMode;//交换链的显示模式
		swapChainCreateInfo.oldSwapchain              = VK_NULL_HANDLE;//前导交换链
		swapChainCreateInfo.clipped                   = true;//开启剪裁
		swapChainCreateInfo.imageColorSpace           = VK_COLORSPACE_SRGB_NONLINEAR_KHR;//色彩空间
		swapChainCreateInfo.imageUsage                = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//图像用途
		swapChainCreateInfo.imageSharingMode          = VK_SHARING_MODE_EXCLUSIVE;	//图像共享模式
		swapChainCreateInfo.queueFamilyIndexCount     = 0;//队列家族数量
		swapChainCreateInfo.pQueueFamilyIndices       = NULL;//队列家族索引列表

		if (Asset.gpuQueueGraphicsFamilyIndex != Asset.gpuQueuePresentFamilyIndex)//若支持图形和显示工作的队列家族不相同
		{
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;//交换链所需的队列家族索引数量为2
			uint32_t queueFamilyIndices[2] = { Asset.gpuQueueGraphicsFamilyIndex,Asset.gpuQueuePresentFamilyIndex };
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;//交换链所需的队列家族索引列表
		}

		VkAssert(vkCreateSwapchainKHR(Asset.vkDevice, &swapChainCreateInfo, NULL, &Asset.vkSwapChain) == VK_SUCCESS); //创建交换链
		
		 //获取交换链中的图像数量
		VkAssert(vkGetSwapchainImagesKHR(Asset.vkDevice, Asset.vkSwapChain, &Asset.swapchainImageCount, NULL) == VK_SUCCESS);
		printf("[SwapChain中的Image数量为%d]\n", Asset.swapchainImageCount);//检查是否获取成功

		Asset.swapchainImages.resize(Asset.swapchainImageCount);//调整图像列表尺寸
		//获取交换链中的图像列表
		VkAssert(vkGetSwapchainImagesKHR(Asset.vkDevice, Asset.vkSwapChain, &Asset.swapchainImageCount, Asset.swapchainImages.data()) == VK_SUCCESS);

		Asset.swapchainImageViews.resize(Asset.swapchainImageCount);//调整图像视图列表尺寸
		for (uint32_t i = 0; i < Asset.swapchainImageCount; i++)//为交换链中的各幅图像创建图像视图
		{
			VkImageViewCreateInfo color_image_view = {};//构建图像视图创建信息结构体实例
			color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;//设置结构体类型
			color_image_view.pNext = NULL;//自定义数据的指针
			color_image_view.flags = 0;//供将来使用的标志
			color_image_view.image = Asset.swapchainImages[i];//对应交换链图像
			color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;//图像视图的类型
			color_image_view.format = Asset.vkFormats[0];//图像视图格式
			color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;//设置R通道调和
			color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;//设置G通道调和
			color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;//设置B通道调和
			color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;//设置A通道调和
			color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//图像视图使用方面
			color_image_view.subresourceRange.baseMipLevel = 0;//基础Mipmap级别
			color_image_view.subresourceRange.levelCount = 1;//Mipmap 级别的数量
			color_image_view.subresourceRange.baseArrayLayer = 0;//基础数组层
			color_image_view.subresourceRange.layerCount = 1;//数组层的数量

			VkAssert(vkCreateImageView(Asset.vkDevice, &color_image_view, NULL, &Asset.swapchainImageViews[i]) == VK_SUCCESS);
		}
#else 
    #pragma error ("Complie Vulkan Renderer failed:UnKnown Platform")
#endif
	}
	void doRenderPassCreation()
	{
		//构建信号量创建信息结构体实例
		VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;		
		imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;//结构体类型
		imageAcquiredSemaphoreCreateInfo.pNext = NULL;//自定义数据的指针
		imageAcquiredSemaphoreCreateInfo.flags = 0;//供将来使用的标志

		VkAssert(vkCreateSemaphore(Asset.vkDevice, &imageAcquiredSemaphoreCreateInfo, NULL, &Asset.semaphore) == VK_SUCCESS);//检测信号量是否创建成功

		VkAttachmentDescription attachments[2];//附件描述信息数组
		attachments[0].format = Asset.vkFormats[0];//设置颜色附件的格式
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;//设置采样模式
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//加载时对附件的操作
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;//存储时对附件的操作
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//模板加载时对附件的操作
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//模板存储时对附件的操作
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//初始的布局
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//结束时的最终布局
		attachments[0].flags = 0;//设置位掩码

		attachments[1].format = Asset.depthImageFormat;//设置深度附件的格式
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;//设置采样模式
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//加载时对附件的操作
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//存储时对附件的操作
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//模板加载时对附件的操作
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//模板存储时对附件的操作
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 	//初始的布局
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//结束时的布局
		attachments[1].flags = 0;//设置位掩码

		VkAttachmentReference color_reference = {};//颜色附件引用
		color_reference.attachment = 0;//对应附件描述信息数组下标
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//设置附件布局

		VkAttachmentReference depth_reference = {};//深度附件引用
		depth_reference.attachment = 1;//对应附件描述信息数组下标
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//设置附件布局

		VkSubpassDescription subpass = {};//构建渲染子通道描述结构体实例
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//设置管线绑定点
		subpass.flags = 0;//设置掩码
		subpass.inputAttachmentCount = 0;//输入附件数量
		subpass.pInputAttachments = NULL;//输入附件列表
		subpass.colorAttachmentCount = 1;//颜色附件数量
		subpass.pColorAttachments = &color_reference;//颜色附件列表
		subpass.pResolveAttachments = NULL;//Resolve 附件
		subpass.pDepthStencilAttachment = &depth_reference;//深度模板附件
		subpass.preserveAttachmentCount = 0;//preserve 附件数量
		subpass.pPreserveAttachments = NULL;//preserve 附件列表

		VkRenderPassCreateInfo rp_info = {};//构建渲染通道创建信息结构体实例
		rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;//结构体类型
		rp_info.pNext = NULL;//自定义数据的指针
		rp_info.attachmentCount = 2;//附件的数量
		rp_info.pAttachments = attachments;//附件列表
		rp_info.subpassCount = 1;//渲染子通道数量
		rp_info.pSubpasses = &subpass;//渲染子通道列表
		rp_info.dependencyCount = 0;//子通道依赖数量
		rp_info.pDependencies = NULL;//子通道依赖列表

		VkAssert(vkCreateRenderPass(Asset.vkDevice, &rp_info, NULL, &Asset.renderPass) == VK_SUCCESS);

		Asset.clearValue[0].color.float32[0] = 0.3f;//帧缓冲清除用R分量值
		Asset.clearValue[0].color.float32[1] = 0.3f;//帧缓冲清除用G分量值
		Asset.clearValue[0].color.float32[2] = 0.3f;//帧缓冲清除用B分量值
		Asset.clearValue[0].color.float32[3] = 0.3f;//帧缓冲清除用A分量值
		Asset.clearValue[1].depthStencil.depth = 1.0f;//帧缓冲清除用深度值
		Asset.clearValue[1].depthStencil.stencil = 0;//帧缓冲清除用模板值

		/*
		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;//渲染通道启动信息结构体类型
		vkRenderPassBeginInfo.pNext = NULL;//自定义数据的指针
		vkRenderPassBeginInfo.renderPass = vkRenderPass;//指定要启动的渲染通道
		vkRenderPassBeginInfo.renderArea.offset.x = 0;//渲染区域起始X坐标
		vkRenderPassBeginInfo.renderArea.offset.y = 0;//渲染区域起始Y坐标
		vkRenderPassBeginInfo.renderArea.extent.width = screenWidth;//渲染区域宽度
		vkRenderPassBeginInfo.renderArea.extent.height = screenHeight;//渲染区域高度
		vkRenderPassBeginInfo.clearValueCount = 2;//帧缓冲清除值数量
		vkRenderPassBeginInfo.pClearValues = vkClearValues;//帧缓冲清除值数组
		*/

	}

	//创建深度缓冲相关
	void doDepthBufferCreation()
	{
		//构建深度图像创建信息结构体实例
		VkImageCreateInfo depthImageCreateInfo = {};

		VkFormatProperties vkDepthFormatProps;
		vkGetPhysicalDeviceFormatProperties(Asset.gpus[0], Asset.depthImageFormat, &vkDepthFormatProps);//获取物理设备支持的指定格式的属性
		//确定平铺方式
		if (vkDepthFormatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)//是否支持线性瓦片组织方式
		{
			depthImageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;//采用线性瓦片组织方式
			printf("tiling为VK_IMAGE_TILING_LINEAR!\n");
		}
		else if (vkDepthFormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)//是否支持最优瓦片组织方式
		{
			depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;//采用最优瓦片组织方式
			printf("tiling为VK_IMAGE_TILING_OPTIMAL!\n");
		}
		else
		{
			printf("不支持VK_FORMAT_D16_UNORM!\n");//打印不支持指定格式的提示信息
		}
		depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;	//指定结构体类型
		depthImageCreateInfo.pNext = NULL;//自定义数据的指针
		depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;//图像类型
		depthImageCreateInfo.format = Asset.depthImageFormat;//图像格式
		depthImageCreateInfo.extent.width = Asset.screenWidth;//图像宽度
		depthImageCreateInfo.extent.height = Asset.screenHeight;//图像高度
		depthImageCreateInfo.extent.depth = 1;//图像深度
		depthImageCreateInfo.mipLevels = 1;//图像mipmap级数
		depthImageCreateInfo.arrayLayers = 1;//图像数组层数量
		depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;//采样模式
		depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//初始布局
		depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;//图像用途
		depthImageCreateInfo.queueFamilyIndexCount = 0;//队列家族数量
		depthImageCreateInfo.pQueueFamilyIndices = NULL;//队列家族索引列表
		depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//共享模式
		depthImageCreateInfo.flags = 0;//标志

		VkMemoryAllocateInfo memoryAllocateInfo = {};//构建内存分配信息结构体实例
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//结构体类型
		memoryAllocateInfo.pNext = NULL;//自定义数据的指针
		memoryAllocateInfo.allocationSize = 0;//分配的内存字节数
		memoryAllocateInfo.memoryTypeIndex = 0;//内存的类型索引

		VkImageViewCreateInfo depthImageViewCreateInfo = {};//构建深度图像视图创建信息结构体实例
		depthImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;//设置结构体类型
		depthImageViewCreateInfo.pNext = NULL;//自定义数据的指针
		depthImageViewCreateInfo.image = VK_NULL_HANDLE;//对应的图像
		depthImageViewCreateInfo.format = Asset.depthImageFormat;//图像视图的格式
		depthImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;//设置R通道调和
		depthImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;//设置G通道调和
		depthImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;//设置B通道调和
		depthImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;//设置A通道调和
		depthImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;//图像视图使用方面
		depthImageViewCreateInfo.subresourceRange.baseMipLevel = 0;//基础Mipmap级别
		depthImageViewCreateInfo.subresourceRange.levelCount = 1;//Mipmap 级别的数量
		depthImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;//基础数组层
		depthImageViewCreateInfo.subresourceRange.layerCount = 1;//数组层的数量
		depthImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;//图像视图的类型
		depthImageViewCreateInfo.flags = 0;//标志

		VkAssert(vkCreateImage(Asset.vkDevice, &depthImageCreateInfo, NULL, &Asset.depthImage) == VK_SUCCESS);

		//获取图像内存需求
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(Asset.vkDevice, Asset.depthImage, &memoryRequirements);
		//获取所需内存字节数
		memoryAllocateInfo.allocationSize = memoryRequirements.size;

		VkFlags requirements_mask = 0;//需要的内存类型掩码
		//获取所需内存类型索引
		VkAssert(memoryTypeFromProperties(Asset.gpuMemoryProperty, memoryRequirements.memoryTypeBits, requirements_mask, &memoryAllocateInfo.memoryTypeIndex));
		printf("确定内存类型成功 类型索引为%d\n", memoryAllocateInfo.memoryTypeIndex);

		VkAssert(vkAllocateMemory(Asset.vkDevice, &memoryAllocateInfo, NULL, &Asset.depthMemory) == VK_SUCCESS);

		VkAssert(vkBindImageMemory(Asset.vkDevice, Asset.depthImage, Asset.depthMemory, 0) == VK_SUCCESS);//绑定图像和内存

		//指定图像视图对应图像
		depthImageViewCreateInfo.image = Asset.depthImage;
		//创建深度图像视图
		VkAssert(vkCreateImageView(Asset.vkDevice, &depthImageViewCreateInfo, NULL, &Asset.depthImageView) == VK_SUCCESS);
	}

	//创建帧缓冲
	void doFrameBufferCreationf()
	{
		VkImageView attachments[2];//附件图像视图数组
		attachments[1] = Asset.depthImageView;//给定深度图像视图

		VkFramebufferCreateInfo fb_info = {};//构建帧缓冲创建信息结构体实例
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;//结构体类型
		fb_info.pNext = NULL;//自定义数据的指针
		fb_info.renderPass = Asset.renderPass;//指定渲染通道
		fb_info.attachmentCount = 2;//附件数量
		fb_info.pAttachments = attachments;//附件图像视图数组
		fb_info.width = Asset.screenWidth;//宽度
		fb_info.height = Asset.screenHeight;//高度
		fb_info.layers = 1;//层数

		uint32_t i;//循环控制变量
		//为帧缓冲序列动态分配内存
		VkFramebuffer* vkFrameBuffers = new VkFramebuffer[Asset.swapchainImageCount];

		VkAssert(vkFrameBuffers != NULL, "AllocateFrameBuffers");//检查内存分配是否成功
		//遍历交换链中的各个图像
		for (i = 0; i < Asset.swapchainImageCount; i++)
		{
			attachments[0] = Asset.swapchainImageViews[i];//给定颜色附件对应图像视图
			VkAssert(vkCreateFramebuffer(Asset.vkDevice, &fb_info, NULL, &vkFrameBuffers[i]) == VK_SUCCESS);//检查是否创建成功
			printf("[创建帧缓冲%d 成功！]\n", i);
		}
	}

	void doFenceCreation()
	{
		VkFenceCreateInfo fenceInfo;//栅栏创建信息结构体实例
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;//结构体类型
		fenceInfo.pNext = NULL;//自定义数据的指针
		fenceInfo.flags = 0;//供将来使用的标志位
		vkCreateFence(Asset.vkDevice, &fenceInfo, NULL, &Asset.renderTaskFinishedFence);//创建时栅栏
	}
public:
    bool startUp()
    {	
		doInstanceCreation();
		doEnumPhysicalDevices();
		doLogicalDeviceCreation();
		doCommandBufferCreation();
		doSwapChainCreation();
		doDepthBufferCreation();

		return true;
    }
	bool shutdown()
	{
		vkDestroyInstance(Asset.vkInstance, NULL);
		return true;
	}
    VulkanRenderer();
};
