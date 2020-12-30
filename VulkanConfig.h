#pragma once
#include <vulkan/vulkan.h>
#ifndef EasyVulkanConfig_H
#define EasyVulkanConfig_H

typedef unsigned int uint32_t;
#define DECLARE_CONFIG_BEGIN class VulkanConfig { public:
#define DECLARE_CONFIG_END };

#define DECLARE_CONFIG_CATEGORY_BEGIN(name) struct name 
#define DECLARE_CONFIG_CATEGORY_END ;

#define DECLARE_CONFIG_ATTRIBUTE(type,name) static type name;

#define DECLARE_ATTRIBUTE(category_name,attribute_name,type,value) type VulkanConfig::category_name::attribute_name = value;


DECLARE_CONFIG_BEGIN

DECLARE_CONFIG_CATEGORY_BEGIN(AppInfo)
{
	DECLARE_CONFIG_ATTRIBUTE(char*,    AppName       )
	DECLARE_CONFIG_ATTRIBUTE(int,      AppVersion    )
	DECLARE_CONFIG_ATTRIBUTE(char*,    EngineName    )
	DECLARE_CONFIG_ATTRIBUTE(uint32_t, EngineVersion )
	DECLARE_CONFIG_ATTRIBUTE(uint32_t, ApiVersion    )
}
DECLARE_CONFIG_CATEGORY_END

DECLARE_CONFIG_CATEGORY_BEGIN(GPU)
{
	DECLARE_CONFIG_ATTRIBUTE(uint32_t, DefaultGpuIndex)

}
DECLARE_CONFIG_CATEGORY_END

DECLARE_CONFIG_CATEGORY_BEGIN(Display)
{
	DECLARE_CONFIG_ATTRIBUTE(int,  MAX_FPS     )
	DECLARE_CONFIG_ATTRIBUTE(bool, USE_MAX_FPS )
}
DECLARE_CONFIG_CATEGORY_END

DECLARE_CONFIG_CATEGORY_BEGIN(DrawQueueWatcher)
{
	DECLARE_CONFIG_ATTRIBUTE(int, ScanDelayMillionSeconds);
}
DECLARE_CONFIG_CATEGORY_END

DECLARE_CONFIG_END


#undef DECLARE_CONFIG_BEGIN
#undef DECLARE_CONFIG_END 
#undef DECLARE_CONFIG_CATEGORY_BEGIN
#undef DECLARE_CONFIG_CATEGORY_END 
#undef DECLARE_CONFIG_ATTRIBUTE


#endif // !EasyVulkanConfig_H
