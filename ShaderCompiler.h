#pragma once

#include "vulkanrenderercore_global.h"
using namespace std;

class ShaderCompiler
{
private:
	EShLanguage    findLanguageFromShaderStage(const VkShaderStageFlagBits shaderStage);
	void           initResources(TBuiltInResource& Resources);
	bool           GLSLtoSPV(const VkShaderStageFlagBits shaderStage, const char* pshader, std::vector<unsigned int>& spirv);
	QString        loadShaderScriptString(QString fileName, bool& success);

public:   
	VkShaderModule compile(QString path, VkShaderStageFlagBits shaderStage);
	ShaderCompiler();
	~ShaderCompiler();
};

static ShaderCompiler Shader;

