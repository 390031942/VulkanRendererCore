#include "VulkanConfig.h"


DECLARE_ATTRIBUTE(AppInfo, AppName,           char*,    "TimorVulaknRender"       )
DECLARE_ATTRIBUTE(AppInfo, EngineName,		  char*,    "TimorVulaknRenderEngine" )
DECLARE_ATTRIBUTE(AppInfo, AppVersion,		  int,      1                         )
DECLARE_ATTRIBUTE(AppInfo, EngineVersion,	  uint32_t, 1                         )
DECLARE_ATTRIBUTE(AppInfo, ApiVersion,        uint32_t, VK_API_VERSION_1_0        )

DECLARE_ATTRIBUTE(GPU,     DefaultGpuIndex,   uint32_t, 0                         )

DECLARE_ATTRIBUTE(Display, MAX_FPS,           int,      60                        )
DECLARE_ATTRIBUTE(Display, USE_MAX_FPS,       bool,     true                      )

DECLARE_ATTRIBUTE(DrawQueueWatcher, ScanDelayMillionSeconds, int, 5)