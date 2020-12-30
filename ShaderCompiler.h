#pragma once
/*SPRIV*/
#include "SPIRV/GlslangToSpv.h"
/*EASY_VULKAN*/
#include <vulkan/vulkan_core.h>
/*STD*/
#include <fstream>

#include <QString>
#include <QFile>
#include "vulkanrenderercore_global.h"
using namespace std;

class ShaderCompiler
{
private:
	VulkanAsset*   vkAsset = nullptr;
	EShLanguage    findLanguageFromShaderStage(const VkShaderStageFlagBits shaderStage);
	void           initResources(TBuiltInResource& Resources);
	bool           GLSLtoSPV(const VkShaderStageFlagBits shaderStage, const char* pshader, std::vector<unsigned int>& spirv);
	QString        loadShaderScriptString(QString fileName, bool& success);

public:   
	VkShaderModule compile(QString path, VkShaderStageFlagBits shaderStage);
	ShaderCompiler(VulkanAsset* pAsset);
	~ShaderCompiler();
};

static ShaderCompiler Shader;

