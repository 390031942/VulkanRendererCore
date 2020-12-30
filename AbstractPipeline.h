#pragma once

#include <vulkan\vulkan_core.h>

#include <string>
#include <vector>
#include <QString>
#include <QFile>

#include "ShaderCompiler.h"
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
	void                   addVertexInput(VkFormat vertexFormat = VK_FORMAT_R8G8B8A8_UNORM)
	{
		if (vertexAttributeDescriptionCount == MaxArraySize)
		{
			validInfo = false;
			errorMessage = "vertexAttribute DescriptionCount Can not be more than MaxArraySize 32";
			return;
		}

		vertexAttributes[vertexAttributeDescriptionCount].binding = 0;
		vertexAttributes[vertexAttributeDescriptionCount].format = vertexFormat;
		vertexAttributes[vertexAttributeDescriptionCount].location = vertexAttributeDescriptionCount;
		vertexAttributes[vertexAttributeDescriptionCount].offset = vertexOffset;

		vertexAttributeDescriptionCount++;

		calledRecordBit = calledRecordBit | 0x01;

		switch (vertexAttributes[vertexAttributeDescriptionCount - 1].format)
		{
		case VK_FORMAT_R8_UNORM: vertexOffset += 1 * sizeof(float); break;
		case VK_FORMAT_R8G8_UNORM:vertexOffset += 2 * sizeof(float); break;
		case VK_FORMAT_R8G8B8_UNORM:vertexOffset += 3 * sizeof(float); break;
		case VK_FORMAT_R8G8B8A8_UNORM:vertexOffset += 4 * sizeof(float); break;
		default:
			//类别无效
			validInfo = false;
			errorMessage = "VK_FORMAT Error";
			return;
		}

		vertexInputBinding.binding = 0;//对应绑定点
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	//数据输入频率为每顶点
		vertexInputBinding.stride = vertexOffset;//每组数据的跨度字节数
	}

	void                   setVertexShader(QString path)
	{
		QFile file(path);

		if (!file.exists())
		{
			validInfo = false;
			errorMessage = "setVertexShader(QString path) Failed : can not find file:" + path;
			return;
		}

		vkVertexShaderModule = Shader.compile(path, VK_SHADER_STAGE_VERTEX_BIT);

		calledRecordBit = calledRecordBit | 0x02;
	}

	void                   setFragmentShader(QString path)
	{
		QFile file(path);

		if (!file.exists())
		{
			validInfo = false;
			errorMessage = "setFragmentShader(QString path) Failed : can not find file:" + path;
			return;
		}

		vkFragmentShaderModule = Shader.compile(path, VK_SHADER_STAGE_FRAGMENT_BIT);

		calledRecordBit = calledRecordBit | 0x02;
	}

	void                   setDescriptorMaxCount(int count)
	{
		descriptorSetMaxCount = count;
		calledRecordBit = calledRecordBit | 0x08;
	}

	void                   addDescriptorLayoutBinding(VkDescriptorType type, VkShaderStageFlags stage)
	{
		if (descriptorSetLayoutBindingCount == MaxArraySize)
		{
			validInfo = false;
			errorMessage = "vkDescriptorSetLayoutBindingCount Can not be more than MaxArraySize 32";
			return;
		}
		layoutBinding[descriptorSetLayoutBindingCount].binding = descriptorSetLayoutBindingCount;
		layoutBinding[descriptorSetLayoutBindingCount].descriptorCount = descriptorSetMaxCount;
		layoutBinding[descriptorSetLayoutBindingCount].descriptorType = type;
		layoutBinding[descriptorSetLayoutBindingCount].pImmutableSamplers = NULL;
		layoutBinding[descriptorSetLayoutBindingCount].stageFlags = stage;


		descriptPoolSize[descriptorSetLayoutBindingCount].descriptorCount = descriptorSetMaxCount;
		descriptPoolSize[descriptorSetLayoutBindingCount].type = type;

		descriptorSetLayoutBindingCount++;

		calledRecordBit = calledRecordBit | 0x10;
	}

	void                   addPushConstantRange(int constantCount, VkShaderStageFlags stage)
	{
		if (pushConstantRangeCount == MaxArraySize)
		{
			validInfo = false;
			errorMessage = "pushConstantRangeCount Can not be more than MaxArraySize 32";
			return;
		}

		pushConstantRanges[pushConstantRangeCount].stageFlags = stage;
		pushConstantRanges[pushConstantRangeCount].size = constantCount * sizeof(float);
		pushConstantRanges[pushConstantRangeCount].offset = pushConstantOffset;

		pushConstantOffset += constantCount * sizeof(float);

		calledRecordBit = calledRecordBit | 0x20;
	}

	void                   setMSAALevel(int level)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], &physicalDeviceProperties);

		VkSampleCountFlags counts = min(physicalDeviceProperties.limits.framebufferColorSampleCounts,
			physicalDeviceProperties.limits.framebufferDepthSampleCounts);

		VkSampleCountFlagBits result;
		if (counts & VK_SAMPLE_COUNT_64_BIT & level == 64) {
			result = VK_SAMPLE_COUNT_64_BIT;
		}

		if (counts & VK_SAMPLE_COUNT_32_BIT & level == 32) {
			result = VK_SAMPLE_COUNT_32_BIT;
		}

		if (counts & VK_SAMPLE_COUNT_16_BIT & level == 16) {
			result = VK_SAMPLE_COUNT_16_BIT;
		}

		if (counts & VK_SAMPLE_COUNT_8_BIT & level == 8) {
			result = VK_SAMPLE_COUNT_8_BIT;
		}

		if (counts & VK_SAMPLE_COUNT_4_BIT & level == 4) {
			result = VK_SAMPLE_COUNT_4_BIT;
		}

		if (counts & VK_SAMPLE_COUNT_2_BIT & level == 2) {
			result = VK_SAMPLE_COUNT_2_BIT;
		}
		else {
			result = VK_SAMPLE_COUNT_1_BIT;
		}

		multiSampleLevelFlag = result;

		calledRecordBit = calledRecordBit | 0x40;
	}

	void                   setDepthTestEnable(bool enabled)
	{
		this->enableDepthTest = true;

		calledRecordBit = calledRecordBit | 0x80;
	}

	void                   setStencilTestEnable(bool enabled)
	{
		this->enableStencilTest = true;

		calledRecordBit = calledRecordBit | 0x100;
	}

	void                   setPolygonMode(VkPolygonMode mode, int lineWidth_if_use_lineMode = 1)
	{
		this->polygonMode = mode;
		if (mode == VK_POLYGON_MODE_LINE)
		{
			lineWidth = max(1, lineWidth_if_use_lineMode);
		}

		calledRecordBit = calledRecordBit | 0x200;
	}

	void                   setUniformBytesCount(int stage_vertex_count, int stage_fragment_count)
	{
		this->vsUniformBytesCount = stage_vertex_count;
		this->fsUniformBytesCount = stage_fragment_count;
		calledRecordBit = calledRecordBit | 0x400;
	}

	void                   setUniformFormat(vector<QString> vertex_fmt, vector<QString> fragment_fmt)
	{
		for (int i = 0; i < vertex_fmt.size(); i++)
		{
			if (vertex_fmt[i] == "int") { this->vsUniformBytesCount += sizeof(int); }
			else if (vertex_fmt[i] == "float") { this->vsUniformBytesCount += sizeof(float); }
			else if (vertex_fmt[i] == "double") { this->vsUniformBytesCount += sizeof(double); }
			else if (vertex_fmt[i] == "uint") { this->vsUniformBytesCount += sizeof(unsigned int); }
		}
		for (int i = 0; i < fragment_fmt.size(); i++)
		{
			if (vertex_fmt[i] == "int") { this->fsUniformBytesCount += sizeof(int); }
			else if (vertex_fmt[i] == "float") { this->fsUniformBytesCount += sizeof(float); }
			else if (vertex_fmt[i] == "double") { this->fsUniformBytesCount += sizeof(double); }
			else if (vertex_fmt[i] == "uint") { this->fsUniformBytesCount += sizeof(unsigned int); }
		}
		calledRecordBit = calledRecordBit | 0x400;
	}

	inline bool checkInfoValid() { return validInfo && (calledRecordBit == 0x7ff);}

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

	inline VkDescriptorSetLayoutBinding* getDescriptorSetLayoutBinding()
	{
		return layoutBinding;
	}

	inline VkDescriptorPoolSize* getDescriptorPoolSizes()
	{
		return descriptPoolSize;
	}

	inline VkPushConstantRange* getPushConstantRanges()
	{
		return pushConstantRanges;
	}

	inline VkSampleCountFlagBits getMultiSampleLevelFlag()
	{
		return multiSampleLevelFlag;
	}

	inline VkPolygonMode getPolygonMode()
	{
		return polygonMode;
	}
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

	// Pipeline handle
	VkPipeline                         m_vkPipeline;

public:
	virtual void initDescriptorSet()
	{

	}

	virtual bool createPipeline()
	{

	}

	virtual void createUniformBuffer()
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], &physicalDeviceProperties);

		const VkPhysicalDeviceLimits* pdevLimits = &(physicalDeviceProperties.limits);
		const VkDeviceSize uniAlign = pdevLimits->minUniformBufferOffsetAlignment;

		int bindingCount = m_createInfo->getDescriptorSetLayoutBindingCount();

		int vsUniformBufferByteCount = m_createInfo->getVertexStageUniformBytesCount();
		int fsUniformBufferByteCount = m_createInfo->getFragmentStageUniformBytesCount();

		const VkDeviceSize vsUniformBufferAllocSize = aligned(vsUniformBufferByteCount, uniAlign);
		const VkDeviceSize fsUniformBufferAllocSize = aligned(fsUniformBufferByteCount, uniAlign);

		const VkDeviceSize allocateSize = vsUniformBufferAllocSize + fsUniformBufferAllocSize;//一致变量缓冲的总字节数

		if (allocateSize == 0)
			return;

		VkBufferCreateInfo bufferCreateInfo    = {};//构建一致变量缓冲创建信息结构体实例
		bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;	//结构体的类型
		bufferCreateInfo.usage                 = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;//缓冲的用途
		bufferCreateInfo.size                  = allocateSize;//缓冲总字节数
		bufferCreateInfo.queueFamilyIndexCount = 0;	//队列家族数量
		bufferCreateInfo.pQueueFamilyIndices   = NULL;//队列家族索引列表
		bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;//共享模式
		bufferCreateInfo.flags                 = 0;//标志

		VkAssert(vkCreateBuffer(Asset.vkDevice, &bufferCreateInfo, NULL, &m_vkUniformBuffer) == VK_SUCCESS);

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(Asset.vkDevice, m_vkUniformBuffer, &memoryRequirements);//获取此缓冲的内存需求

		VkMemoryAllocateInfo alloc_info = {};//构建内存分配信息结构体实例
		alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//结构体类型
		alloc_info.memoryTypeIndex      = 0;//内存类型索引
		alloc_info.allocationSize       = memoryRequirements.size;//缓冲内存分配字节数

		VkFlags requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;//需要的内存类型掩码
		bool flag = memoryTypeFromProperties(Asset.gpuMemoryProperty, memoryRequirements.memoryTypeBits, requirements_mask, &alloc_info.memoryTypeIndex);	//获取所需内存类型索引
		if (flag) { printf("确定内存类型成功 类型索引为%d", alloc_info.memoryTypeIndex); }
		else { printf("确定内存类型失败!"); }

		VkAssert(vkAllocateMemory(Asset.vkDevice, &alloc_info, NULL, &m_vkUniformBufferMemory) == VK_SUCCESS);
		VkAssert(vkBindBufferMemory(Asset.vkDevice, m_vkUniformBuffer, m_vkUniformBufferMemory, 0) == VK_SUCCESS);

		m_vkUniformBufferInfo.buffer = m_vkUniformBuffer;//指定一致变量缓冲
		m_vkUniformBufferInfo.offset = 0;//起始偏移量
		m_vkUniformBufferInfo.range = allocateSize;//一致变量缓冲总字节数
	}

	virtual void createPipelineLayout()
	{

	}

	virtual void createShader()
	{

	}

	virtual void destroyShader()
	{

	}

	virtual void destroyPipeline()
	{

	}

	virtual void destoryUniformBuffer()
	{
		if (m_vkUniformBuffer) {
			vkDestroyBuffer(Asset.vkDevice, m_vkUniformBuffer, NULL);
			m_vkUniformBuffer = VK_NULL_HANDLE;
		}

		if (m_vkUniformBufferMemory) {
			vkFreeMemory(Asset.vkDevice, m_vkUniformBufferMemory, NULL);
			m_vkUniformBufferMemory = VK_NULL_HANDLE;
		}
	}

	virtual void destoryPipelineLayout()
	{

	}
};

class BasicDrawPipeline :public AbstractPipeline
{
	BasicDrawPipeline()
	{
		// Pipeline Info

		m_createInfo = new PipelineInfo;
		// Vertex Format
		m_createInfo->addVertexInput(VK_FORMAT_R8G8B8A8_UNORM);

		// Shader Setting
		m_createInfo->setVertexShader("shaders/build_in_basic.vert");
		m_createInfo->setFragmentShader("shaders/build_in_basic.frag");

		// Uniform Buffer Format
		// m_createInfo->setUniformBytesCount(0, 4);
		m_createInfo->setUniformFormat({}, {"float"});

		// Multi-Sample Level
		m_createInfo->setMSAALevel(16);

		// push Constant Range
		m_createInfo->addPushConstantRange(16, VK_SHADER_STAGE_VERTEX_BIT);

		// Max Count of DescriptorSet 
		m_createInfo->setDescriptorMaxCount(1024);

		// Test Setting
		m_createInfo->setDepthTestEnable(true);
		m_createInfo->setStencilTestEnable(false);

		// Polygon Mode
		m_createInfo->setPolygonMode(VK_POLYGON_MODE_FILL);
	}

	~BasicDrawPipeline()
	{
		delete m_createInfo;
		m_createInfo = nullptr;
	}
};

static BasicDrawPipeline BasicPipeline;
