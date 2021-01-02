#pragma once
#include <string>
#include <vector>
#include <QString>
#include <QFile>

#include "ShaderCompiler.h"
#include "vulkanrenderercore_global.h"
using namespace std;

class PipelineInfo
{
private:
	const  static  int                MaxArraySize = 32;
	int                               vertexAttributeDescriptionCount;

	VkVertexInputBindingDescription   vertexInputBinding;
	VkVertexInputAttributeDescription vertexAttributes[MaxArraySize];

	VkShaderModule                    vkVertexShaderModule = VK_NULL_HANDLE;
	VkShaderModule                    vkFragmentShaderModule = VK_NULL_HANDLE;

	int                               descriptorSetMaxCount;
	int                               descriptorSetLayoutBindingCount;
	VkDescriptorSetLayoutBinding      layoutBinding[MaxArraySize];
	VkDescriptorPoolSize              descriptPoolSize[MaxArraySize];

	int                               pushConstantRangeCount;
	VkPushConstantRange               pushConstantRanges[MaxArraySize];

	VkSampleCountFlagBits             multiSampleLevelFlag;

	bool                              enableDepthTest;
	bool                              enableStencilTest;

	VkPolygonMode                     polygonMode;
	int                               lineWidth;

	int                               vsUniformBytesCount;
	int                               fsUniformBytesCount;

	bool                              validInfo = true;
	int                               calledRecordBit = 0;
	QString                           errorMessage;

	VkDeviceSize                      vertexOffset = 0;
	VkDeviceSize                      pushConstantOffset = 0;

public:
	void                   addVertexInput(VkFormat vertexFormat = VK_FORMAT_R8G8B8A8_UNORM);

	void                   setVertexShader(QString path);

	void                   setFragmentShader(QString path);

	void                   setDescriptorMaxCount(int count);

	void                   addDescriptorLayoutBinding(VkDescriptorType type, VkShaderStageFlags stage);

	void                   addPushConstantRange(int constantCount, VkShaderStageFlags stage);

	void                   setMSAALevel(int level);

	void                   setDepthTestEnable(bool enabled);

	void                   setStencilTestEnable(bool enabled);

	void                   setPolygonMode(VkPolygonMode mode, int lineWidth_if_use_lineMode = 1);

	void                   setUniformBytesCount(int stage_vertex_count, int stage_fragment_count);

	void                   setUniformFormat(vector<QString> vertex_fmt, vector<QString> fragment_fmt);

	void                   setLineWidth(unsigned int width);

	inline bool checkInfoValid() { return validInfo && (calledRecordBit == 0xfff);}

	inline bool isDepthTestEnabled() { return enableDepthTest;}

	inline bool isStencilTestEnabled() { return enableStencilTest;}

	inline int getDescriptorMaxCount() { return descriptorSetMaxCount; }

	inline int getDescriptorSetLayoutBindingCount() { return descriptorSetLayoutBindingCount; }

	inline int getVertexAttributeCount() { return vertexAttributeDescriptionCount; }

	inline int getPushConstantRangeCount() { return pushConstantRangeCount;}

	inline int getLineWidth() { return lineWidth;}

	inline int getVertexStageUniformBytesCount() { return vsUniformBytesCount;}

	inline int getFragmentStageUniformBytesCount() { return fsUniformBytesCount;}

	inline VkVertexInputAttributeDescription* getVertexAttributes() { return vertexAttributes;}

	inline VkVertexInputBindingDescription* getVertexBinding() { return &vertexInputBinding;}

	inline VkShaderModule getVertexShaderModule() { return vkVertexShaderModule;}

	inline VkShaderModule getFragmentShaderModule() { return vkFragmentShaderModule;}

	inline VkDescriptorSetLayoutBinding* getDescriptorSetLayoutBinding() { return layoutBinding;}

	inline VkDescriptorPoolSize* getDescriptorPoolSizes() { return descriptPoolSize;}

	inline VkPushConstantRange* getPushConstantRanges() { return pushConstantRanges;}

	inline VkSampleCountFlagBits getMultiSampleLevelFlag() { return multiSampleLevelFlag;}

	inline VkPolygonMode getPolygonMode() { return polygonMode;}
};

class AbstractPipeline
{
protected:
	// Uniform Buffer
	VkBuffer                           m_vkUniformBuffer;

	// Uniform Buffer Memory
	VkDeviceMemory                     m_vkUniformBufferMemory;

	// Descriptor Buffer Info
	VkDescriptorBufferInfo             m_vkUniformBufferInfo;

	// Descriptor Sets Layouts
	std::vector<VkDescriptorSetLayout> m_vkDescriptorLayouts;

	// Descriptor Pool
	VkDescriptorPool                   m_vkDescriptorPool;

	// Write DescriptorSets
	VkWriteDescriptorSet*              m_vkWriteDescriptorSet; 

	// Pointer of Pipeline Info
	PipelineInfo*                      m_createInfo = nullptr;

	// list of descriptor set
	vector<VkDescriptorSet>            m_vkDescriptorSet;

	// Pipeline layout
	VkPipelineLayout                   m_vkPipelineLayout;

	// Pipeline cache 
	VkPipelineCache                    m_vkPipelineCache;

	// Pipeline handle
	VkPipeline                         m_vkPipeline;

	// Shader Module info
	VkPipelineShaderStageCreateInfo    m_vkShaderStage[2];


public:
	virtual void initDescriptorSet();

	virtual bool createPipeline();

	virtual void createUniformBuffer();

	virtual void createPipelineLayout();

	virtual void createShader();

	virtual void destroyShader();

	virtual void destroyPipeline();

	virtual void destoryUniformBuffer();

	virtual void destoryPipelineLayout();

	bool create();

	~AbstractPipeline();
};

class BasicDrawPipeline :public AbstractPipeline
{
public:
	BasicDrawPipeline();
};

static BasicDrawPipeline BasicPipeline;
