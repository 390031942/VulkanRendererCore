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

		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;//ȷ����������ʾģʽ

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
			//���ÿ��Ϊ������
			Asset.vkSwapchainExtent.width = Asset.screenWidth;
			//���ø߶�Ϊ����߶�
			Asset.vkSwapchainExtent.height = Asset.screenHeight;
			//�������ֵ���Ƶ����ֵ����Сֵ֮��
			if (Asset.vkSwapchainExtent.width < vkSurfaceCapabilities.minImageExtent.width)
			{
				Asset.vkSwapchainExtent.width = vkSurfaceCapabilities.minImageExtent.width;
			}
			else if (Asset.vkSwapchainExtent.width > vkSurfaceCapabilities.maxImageExtent.width)
			{
				Asset.vkSwapchainExtent.width = vkSurfaceCapabilities.maxImageExtent.width;
			}
			//�߶�����ֵ���Ƶ����ֵ����Сֵ֮��
			if (Asset.vkSwapchainExtent.height < vkSurfaceCapabilities.minImageExtent.height)
			{
				Asset.vkSwapchainExtent.height = vkSurfaceCapabilities.minImageExtent.height;
			}
			else if (Asset.vkSwapchainExtent.height > vkSurfaceCapabilities.maxImageExtent.height)
			{
				Asset.vkSwapchainExtent.height = vkSurfaceCapabilities.maxImageExtent.height;
			}
			printf("ʹ���Լ����õ� ��� %d �߶� %d\n", Asset.vkSwapchainExtent.width, Asset.vkSwapchainExtent.height);
		}
		else
		{
			//��������ȷ���ߴ�
			Asset.vkSwapchainExtent = vkSurfaceCapabilities.currentExtent;
			printf("ʹ�û�ȡ��surface�����е� ��� %d �߶� %d\n", Asset.vkSwapchainExtent.width, Asset.vkSwapchainExtent.height);
		}

		Asset.screenWidth = Asset.vkSwapchainExtent.width;//��¼ʵ�ʲ��õĿ��
		Asset.screenHeight = Asset.vkSwapchainExtent.height;//��¼ʵ�ʲ��õĸ߶�

		//�����������е�����ͼ������
		uint32_t desiredMinNumberOfSwapChainImages = vkSurfaceCapabilities.minImageCount + 1;
		//���������Ƶ���Χ��
		if ((vkSurfaceCapabilities.maxImageCount > 0) && (desiredMinNumberOfSwapChainImages > vkSurfaceCapabilities.maxImageCount))
		{
			desiredMinNumberOfSwapChainImages = vkSurfaceCapabilities.maxImageCount;
		}

		//KHR����任��־
		VkSurfaceTransformFlagBitsKHR preTransform;
		if (vkSurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)//��֧������ı任
		{
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else//����֧�ֱ任
		{
			preTransform = vkSurfaceCapabilities.currentTransform;
		}

		VkSwapchainCreateInfoKHR  swapChainCreateInfo = {};//����������������Ϣ�ṹ��ʵ��
		swapChainCreateInfo.sType                     = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;//�ṹ������
		swapChainCreateInfo.surface                   = Asset.vkSurface;//ָ��KHR����
		swapChainCreateInfo.minImageCount             = desiredMinNumberOfSwapChainImages;//����ͼ������
		swapChainCreateInfo.imageFormat               = Asset.vkFormats[0];//ͼ���ʽ
		swapChainCreateInfo.imageExtent.width         = Asset.vkSwapchainExtent.width;//������ͼ����
		swapChainCreateInfo.imageExtent.height        = Asset.vkSwapchainExtent.height;//������ͼ��߶�
		swapChainCreateInfo.preTransform              = preTransform;//ָ���任��־
		swapChainCreateInfo.compositeAlpha            = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//���Alphaֵ
		swapChainCreateInfo.imageArrayLayers          = 1;//ͼ���������
		swapChainCreateInfo.presentMode               = swapchainPresentMode;//����������ʾģʽ
		swapChainCreateInfo.oldSwapchain              = VK_NULL_HANDLE;//ǰ��������
		swapChainCreateInfo.clipped                   = true;//��������
		swapChainCreateInfo.imageColorSpace           = VK_COLORSPACE_SRGB_NONLINEAR_KHR;//ɫ�ʿռ�
		swapChainCreateInfo.imageUsage                = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//ͼ����;
		swapChainCreateInfo.imageSharingMode          = VK_SHARING_MODE_EXCLUSIVE;	//ͼ����ģʽ
		swapChainCreateInfo.queueFamilyIndexCount     = 0;//���м�������
		swapChainCreateInfo.pQueueFamilyIndices       = NULL;//���м��������б�

		if (Asset.gpuQueueGraphicsFamilyIndex != Asset.gpuQueuePresentFamilyIndex)//��֧��ͼ�κ���ʾ�����Ķ��м��岻��ͬ
		{
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;//����������Ķ��м�����������Ϊ2
			uint32_t queueFamilyIndices[2] = { Asset.gpuQueueGraphicsFamilyIndex,Asset.gpuQueuePresentFamilyIndex };
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;//����������Ķ��м��������б�
		}

		VkAssert(vkCreateSwapchainKHR(Asset.vkDevice, &swapChainCreateInfo, NULL, &Asset.vkSwapChain) == VK_SUCCESS); //����������
		
		 //��ȡ�������е�ͼ������
		VkAssert(vkGetSwapchainImagesKHR(Asset.vkDevice, Asset.vkSwapChain, &Asset.swapchainImageCount, NULL) == VK_SUCCESS);
		printf("[SwapChain�е�Image����Ϊ%d]\n", Asset.swapchainImageCount);//����Ƿ��ȡ�ɹ�

		Asset.swapchainImages.resize(Asset.swapchainImageCount);//����ͼ���б�ߴ�
		//��ȡ�������е�ͼ���б�
		VkAssert(vkGetSwapchainImagesKHR(Asset.vkDevice, Asset.vkSwapChain, &Asset.swapchainImageCount, Asset.swapchainImages.data()) == VK_SUCCESS);

		Asset.swapchainImageViews.resize(Asset.swapchainImageCount);//����ͼ����ͼ�б�ߴ�
		for (uint32_t i = 0; i < Asset.swapchainImageCount; i++)//Ϊ�������еĸ���ͼ�񴴽�ͼ����ͼ
		{
			VkImageViewCreateInfo color_image_view = {};//����ͼ����ͼ������Ϣ�ṹ��ʵ��
			color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;//���ýṹ������
			color_image_view.pNext = NULL;//�Զ������ݵ�ָ��
			color_image_view.flags = 0;//������ʹ�õı�־
			color_image_view.image = Asset.swapchainImages[i];//��Ӧ������ͼ��
			color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;//ͼ����ͼ������
			color_image_view.format = Asset.vkFormats[0];//ͼ����ͼ��ʽ
			color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;//����Rͨ������
			color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;//����Gͨ������
			color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;//����Bͨ������
			color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;//����Aͨ������
			color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//ͼ����ͼʹ�÷���
			color_image_view.subresourceRange.baseMipLevel = 0;//����Mipmap����
			color_image_view.subresourceRange.levelCount = 1;//Mipmap ���������
			color_image_view.subresourceRange.baseArrayLayer = 0;//���������
			color_image_view.subresourceRange.layerCount = 1;//����������

			VkAssert(vkCreateImageView(Asset.vkDevice, &color_image_view, NULL, &Asset.swapchainImageViews[i]) == VK_SUCCESS);
		}
#else 
    #pragma error ("Complie Vulkan Renderer failed:UnKnown Platform")
#endif
	}
	void doRenderPassCreation()
	{
		//�����ź���������Ϣ�ṹ��ʵ��
		VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;		
		imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;//�ṹ������
		imageAcquiredSemaphoreCreateInfo.pNext = NULL;//�Զ������ݵ�ָ��
		imageAcquiredSemaphoreCreateInfo.flags = 0;//������ʹ�õı�־

		VkAssert(vkCreateSemaphore(Asset.vkDevice, &imageAcquiredSemaphoreCreateInfo, NULL, &Asset.semaphore) == VK_SUCCESS);//����ź����Ƿ񴴽��ɹ�

		VkAttachmentDescription attachments[2];//����������Ϣ����
		attachments[0].format = Asset.vkFormats[0];//������ɫ�����ĸ�ʽ
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;//���ò���ģʽ
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//����ʱ�Ը����Ĳ���
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;//�洢ʱ�Ը����Ĳ���
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//ģ�����ʱ�Ը����Ĳ���
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//ģ��洢ʱ�Ը����Ĳ���
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//��ʼ�Ĳ���
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//����ʱ�����ղ���
		attachments[0].flags = 0;//����λ����

		attachments[1].format = Asset.depthImageFormat;//������ȸ����ĸ�ʽ
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;//���ò���ģʽ
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//����ʱ�Ը����Ĳ���
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//�洢ʱ�Ը����Ĳ���
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//ģ�����ʱ�Ը����Ĳ���
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//ģ��洢ʱ�Ը����Ĳ���
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 	//��ʼ�Ĳ���
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//����ʱ�Ĳ���
		attachments[1].flags = 0;//����λ����

		VkAttachmentReference color_reference = {};//��ɫ��������
		color_reference.attachment = 0;//��Ӧ����������Ϣ�����±�
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//���ø�������

		VkAttachmentReference depth_reference = {};//��ȸ�������
		depth_reference.attachment = 1;//��Ӧ����������Ϣ�����±�
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//���ø�������

		VkSubpassDescription subpass = {};//������Ⱦ��ͨ�������ṹ��ʵ��
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//���ù��߰󶨵�
		subpass.flags = 0;//��������
		subpass.inputAttachmentCount = 0;//���븽������
		subpass.pInputAttachments = NULL;//���븽���б�
		subpass.colorAttachmentCount = 1;//��ɫ��������
		subpass.pColorAttachments = &color_reference;//��ɫ�����б�
		subpass.pResolveAttachments = NULL;//Resolve ����
		subpass.pDepthStencilAttachment = &depth_reference;//���ģ�帽��
		subpass.preserveAttachmentCount = 0;//preserve ��������
		subpass.pPreserveAttachments = NULL;//preserve �����б�

		VkRenderPassCreateInfo rp_info = {};//������Ⱦͨ��������Ϣ�ṹ��ʵ��
		rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;//�ṹ������
		rp_info.pNext = NULL;//�Զ������ݵ�ָ��
		rp_info.attachmentCount = 2;//����������
		rp_info.pAttachments = attachments;//�����б�
		rp_info.subpassCount = 1;//��Ⱦ��ͨ������
		rp_info.pSubpasses = &subpass;//��Ⱦ��ͨ���б�
		rp_info.dependencyCount = 0;//��ͨ����������
		rp_info.pDependencies = NULL;//��ͨ�������б�

		VkAssert(vkCreateRenderPass(Asset.vkDevice, &rp_info, NULL, &Asset.renderPass) == VK_SUCCESS);

		Asset.clearValue[0].color.float32[0] = 0.3f;//֡���������R����ֵ
		Asset.clearValue[0].color.float32[1] = 0.3f;//֡���������G����ֵ
		Asset.clearValue[0].color.float32[2] = 0.3f;//֡���������B����ֵ
		Asset.clearValue[0].color.float32[3] = 0.3f;//֡���������A����ֵ
		Asset.clearValue[1].depthStencil.depth = 1.0f;//֡������������ֵ
		Asset.clearValue[1].depthStencil.stencil = 0;//֡���������ģ��ֵ

		/*
		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;//��Ⱦͨ��������Ϣ�ṹ������
		vkRenderPassBeginInfo.pNext = NULL;//�Զ������ݵ�ָ��
		vkRenderPassBeginInfo.renderPass = vkRenderPass;//ָ��Ҫ��������Ⱦͨ��
		vkRenderPassBeginInfo.renderArea.offset.x = 0;//��Ⱦ������ʼX����
		vkRenderPassBeginInfo.renderArea.offset.y = 0;//��Ⱦ������ʼY����
		vkRenderPassBeginInfo.renderArea.extent.width = screenWidth;//��Ⱦ������
		vkRenderPassBeginInfo.renderArea.extent.height = screenHeight;//��Ⱦ����߶�
		vkRenderPassBeginInfo.clearValueCount = 2;//֡�������ֵ����
		vkRenderPassBeginInfo.pClearValues = vkClearValues;//֡�������ֵ����
		*/

	}

	//������Ȼ������
	void doDepthBufferCreation()
	{
		//�������ͼ�񴴽���Ϣ�ṹ��ʵ��
		VkImageCreateInfo depthImageCreateInfo = {};

		VkFormatProperties vkDepthFormatProps;
		vkGetPhysicalDeviceFormatProperties(Asset.gpus[0], Asset.depthImageFormat, &vkDepthFormatProps);//��ȡ�����豸֧�ֵ�ָ����ʽ������
		//ȷ��ƽ�̷�ʽ
		if (vkDepthFormatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)//�Ƿ�֧��������Ƭ��֯��ʽ
		{
			depthImageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;//����������Ƭ��֯��ʽ
			printf("tilingΪVK_IMAGE_TILING_LINEAR!\n");
		}
		else if (vkDepthFormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)//�Ƿ�֧��������Ƭ��֯��ʽ
		{
			depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;//����������Ƭ��֯��ʽ
			printf("tilingΪVK_IMAGE_TILING_OPTIMAL!\n");
		}
		else
		{
			printf("��֧��VK_FORMAT_D16_UNORM!\n");//��ӡ��֧��ָ����ʽ����ʾ��Ϣ
		}
		depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;	//ָ���ṹ������
		depthImageCreateInfo.pNext = NULL;//�Զ������ݵ�ָ��
		depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;//ͼ������
		depthImageCreateInfo.format = Asset.depthImageFormat;//ͼ���ʽ
		depthImageCreateInfo.extent.width = Asset.screenWidth;//ͼ����
		depthImageCreateInfo.extent.height = Asset.screenHeight;//ͼ��߶�
		depthImageCreateInfo.extent.depth = 1;//ͼ�����
		depthImageCreateInfo.mipLevels = 1;//ͼ��mipmap����
		depthImageCreateInfo.arrayLayers = 1;//ͼ�����������
		depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;//����ģʽ
		depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//��ʼ����
		depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;//ͼ����;
		depthImageCreateInfo.queueFamilyIndexCount = 0;//���м�������
		depthImageCreateInfo.pQueueFamilyIndices = NULL;//���м��������б�
		depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//����ģʽ
		depthImageCreateInfo.flags = 0;//��־

		VkMemoryAllocateInfo memoryAllocateInfo = {};//�����ڴ������Ϣ�ṹ��ʵ��
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//�ṹ������
		memoryAllocateInfo.pNext = NULL;//�Զ������ݵ�ָ��
		memoryAllocateInfo.allocationSize = 0;//������ڴ��ֽ���
		memoryAllocateInfo.memoryTypeIndex = 0;//�ڴ����������

		VkImageViewCreateInfo depthImageViewCreateInfo = {};//�������ͼ����ͼ������Ϣ�ṹ��ʵ��
		depthImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;//���ýṹ������
		depthImageViewCreateInfo.pNext = NULL;//�Զ������ݵ�ָ��
		depthImageViewCreateInfo.image = VK_NULL_HANDLE;//��Ӧ��ͼ��
		depthImageViewCreateInfo.format = Asset.depthImageFormat;//ͼ����ͼ�ĸ�ʽ
		depthImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;//����Rͨ������
		depthImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;//����Gͨ������
		depthImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;//����Bͨ������
		depthImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;//����Aͨ������
		depthImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;//ͼ����ͼʹ�÷���
		depthImageViewCreateInfo.subresourceRange.baseMipLevel = 0;//����Mipmap����
		depthImageViewCreateInfo.subresourceRange.levelCount = 1;//Mipmap ���������
		depthImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;//���������
		depthImageViewCreateInfo.subresourceRange.layerCount = 1;//����������
		depthImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;//ͼ����ͼ������
		depthImageViewCreateInfo.flags = 0;//��־

		VkAssert(vkCreateImage(Asset.vkDevice, &depthImageCreateInfo, NULL, &Asset.depthImage) == VK_SUCCESS);

		//��ȡͼ���ڴ�����
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(Asset.vkDevice, Asset.depthImage, &memoryRequirements);
		//��ȡ�����ڴ��ֽ���
		memoryAllocateInfo.allocationSize = memoryRequirements.size;

		VkFlags requirements_mask = 0;//��Ҫ���ڴ���������
		//��ȡ�����ڴ���������
		VkAssert(memoryTypeFromProperties(Asset.gpuMemoryProperty, memoryRequirements.memoryTypeBits, requirements_mask, &memoryAllocateInfo.memoryTypeIndex));
		printf("ȷ���ڴ����ͳɹ� ��������Ϊ%d\n", memoryAllocateInfo.memoryTypeIndex);

		VkAssert(vkAllocateMemory(Asset.vkDevice, &memoryAllocateInfo, NULL, &Asset.depthMemory) == VK_SUCCESS);

		VkAssert(vkBindImageMemory(Asset.vkDevice, Asset.depthImage, Asset.depthMemory, 0) == VK_SUCCESS);//��ͼ����ڴ�

		//ָ��ͼ����ͼ��Ӧͼ��
		depthImageViewCreateInfo.image = Asset.depthImage;
		//�������ͼ����ͼ
		VkAssert(vkCreateImageView(Asset.vkDevice, &depthImageViewCreateInfo, NULL, &Asset.depthImageView) == VK_SUCCESS);
	}

	//����֡����
	void doFrameBufferCreationf()
	{
		VkImageView attachments[2];//����ͼ����ͼ����
		attachments[1] = Asset.depthImageView;//�������ͼ����ͼ

		VkFramebufferCreateInfo fb_info = {};//����֡���崴����Ϣ�ṹ��ʵ��
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;//�ṹ������
		fb_info.pNext = NULL;//�Զ������ݵ�ָ��
		fb_info.renderPass = Asset.renderPass;//ָ����Ⱦͨ��
		fb_info.attachmentCount = 2;//��������
		fb_info.pAttachments = attachments;//����ͼ����ͼ����
		fb_info.width = Asset.screenWidth;//���
		fb_info.height = Asset.screenHeight;//�߶�
		fb_info.layers = 1;//����

		uint32_t i;//ѭ�����Ʊ���
		//Ϊ֡�������ж�̬�����ڴ�
		VkFramebuffer* vkFrameBuffers = new VkFramebuffer[Asset.swapchainImageCount];

		VkAssert(vkFrameBuffers != NULL, "AllocateFrameBuffers");//����ڴ�����Ƿ�ɹ�
		//�����������еĸ���ͼ��
		for (i = 0; i < Asset.swapchainImageCount; i++)
		{
			attachments[0] = Asset.swapchainImageViews[i];//������ɫ������Ӧͼ����ͼ
			VkAssert(vkCreateFramebuffer(Asset.vkDevice, &fb_info, NULL, &vkFrameBuffers[i]) == VK_SUCCESS);//����Ƿ񴴽��ɹ�
			printf("[����֡����%d �ɹ���]\n", i);
		}
	}

	void doFenceCreation()
	{
		VkFenceCreateInfo fenceInfo;//դ��������Ϣ�ṹ��ʵ��
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;//�ṹ������
		fenceInfo.pNext = NULL;//�Զ������ݵ�ָ��
		fenceInfo.flags = 0;//������ʹ�õı�־λ
		vkCreateFence(Asset.vkDevice, &fenceInfo, NULL, &Asset.renderTaskFinishedFence);//����ʱդ��
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
