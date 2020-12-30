#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(VULKANRENDERERCORE_LIB)
#  define VULKANRENDERERCORE_EXPORT Q_DECL_EXPORT
# else
#  define VULKANRENDERERCORE_EXPORT Q_DECL_IMPORT
# endif
#else
# define VULKANRENDERERCORE_EXPORT

#endif

#pragma comment(lib,"vulkan-1.lib")
#include <qmessagebox.h>
#include <QFileInfo>
#include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include "VulkanConfig.h"
using namespace std;
#define VkAssertTitle QString("Vulkan Fatal Error!")
#define VkQuitCode    -2181

#define VK_ASSERT_ENABLE
#ifdef  VK_ASSERT_ENABLE
#define VkAssert(expression) if(!expression) {\
                             QMessageBox::critical(NULL,VkAssertTitle,QString("Vulkan Runtime Error at File:%1 Line %2 ").\
                             arg(QFileInfo(__FILE__).fileName()).arg(__LINE__));\
                             exit(VkQuitCode);}

struct VulkanAsset
{
#if _WIN32
	/******** Windows Handle ********/
	HINSTANCE hInst;
	HWND hWnd;
#endif
	/******** Instance Creation stage ********/
	VkInstance vkInstance = VK_NULL_HANDLE;

	/******** Enum Physical Device stage ********/
	uint32_t   gpuCount = 0;
	vector<VkPhysicalDevice> gpus;
	VkPhysicalDeviceMemoryProperties gpuMemoryProperty;
	VkPhysicalDeviceProperties gpuProperty;

	/******** Logical Device Creation stage ********/
	vector<VkQueueFamilyProperties> gpuQueueFamilyProperty;
	uint32_t gpuQueueFamilyCount = 0;
	uint32_t gpuQueueGraphicsFamilyIndex = -1;
	VkDevice vkDevice = VK_NULL_HANDLE;

	/******** Command Buffer Creation stage ********/
	VkCommandPool vkCmdPool = VK_NULL_HANDLE;
	VkCommandBuffer vkCmdBuffer = VK_NULL_HANDLE;

	/******** Swap Chain Creation stage ********/
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	uint32_t gpuQueuePresentFamilyIndex = -1;
	VkExtent2D vkSwapchainExtent;
	VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;
	int screenHeight;
	int screenWidth;
	std::vector<VkFormat> vkFormats;
	uint32_t swapchainImageCount = -1;
	vector<VkImage> swapchainImages;
	vector<VkImageView> swapchainImageViews;

	/******** Depth Buffer Creation stage ********/
	VkImage depthImage = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	VkDeviceMemory depthMemory = VK_NULL_HANDLE;
	VkFormat depthImageFormat = VK_FORMAT_D16_UNORM;

	/******** Render Pass Creation stage ********/
	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkClearValue clearValue[2];

	/******** Fence Creation stage ********/
	VkFence renderTaskFinishedFence;
};

static VulkanAsset Asset;

static bool memoryTypeFromProperties(VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex)
{
	//循环确定内存类型索引
	for (uint32_t i = 0; i < 32; i++)
	{
		//若对应类型比特位为1

		if ((typeBits & 1) == 1)
		{
			//此类型与掩码匹配
			if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	//没有找到所需的类型索引
	return false;
}
static VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
	return (v + byteAlign - 1) & ~(byteAlign - 1);
}
#else
#define VkAssert(expression) 
#endif

