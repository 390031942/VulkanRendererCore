#include "AbstractPipeline.h"

BasicDrawPipeline::BasicDrawPipeline()
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
	m_createInfo->setUniformFormat({}, { "float" });
	
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

	// line Width
	m_createInfo->setLineWidth(1);

	VkAssert(this->create());
}

void AbstractPipeline::initDescriptorSet()
{
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = m_createInfo->getDescriptorMaxCount();
	descriptorPoolCreateInfo.poolSizeCount = m_createInfo->getDescriptorSetLayoutBindingCount();
	descriptorPoolCreateInfo.pPoolSizes = m_createInfo->getDescriptorPoolSizes();

	VkAssert(vkCreateDescriptorPool(Asset.vkDevice, &descriptorPoolCreateInfo, NULL, &m_vkDescriptorPool) == VK_SUCCESS);

	// declare a vector to store VkDescriptorSetLayout ��������
	vector<VkDescriptorSetLayout> layouts;
	for (int i = 0; i < m_createInfo->getDescriptorMaxCount(); i++)
	{
		layouts.push_back(m_vkDescriptorLayouts[0]);// ��������
	}


	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo[1];
	descriptorSetAllocateInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo[0].descriptorPool = m_vkDescriptorPool;
	descriptorSetAllocateInfo[0].descriptorSetCount = m_createInfo->getDescriptorMaxCount();
	descriptorSetAllocateInfo[0].pSetLayouts = layouts.data();

	m_vkDescriptorSet.resize(m_createInfo->getDescriptorMaxCount());
	VkAssert(vkAllocateDescriptorSets(Asset.vkDevice, descriptorSetAllocateInfo, m_vkDescriptorSet.data()) == VK_SUCCESS);

	m_vkWriteDescriptorSet = new VkWriteDescriptorSet[m_createInfo->getDescriptorSetLayoutBindingCount()];

	//init writeDescriptorSet
	for (int i = 0; i < m_createInfo->getDescriptorSetLayoutBindingCount(); i++)
	{
		m_vkWriteDescriptorSet[i] = {};
		m_vkWriteDescriptorSet[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		m_vkWriteDescriptorSet[i].descriptorCount = 1;                                                              // ��
		m_vkWriteDescriptorSet[i].descriptorType = m_createInfo->getDescriptorSetLayoutBinding()[i].descriptorType;// ��
		m_vkWriteDescriptorSet[i].dstArrayElement = 0;                                                              // ��
		m_vkWriteDescriptorSet[i].dstBinding = i;                                                              // ��

		if (m_vkWriteDescriptorSet[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			m_vkWriteDescriptorSet[i].pBufferInfo = &m_vkUniformBufferInfo;
		}
	}
}

bool AbstractPipeline::createPipeline()
{
	// Here is the final stage of Pipeline's creation
	
	// an array to store VkDynamicState
	VkDynamicState dynamicStateEnables[1];

	memset(dynamicStateEnables, false, sizeof(dynamicStateEnables));
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables;   // ��
	dynamicState.dynamicStateCount = 0;                  // ��

	VkPipelineVertexInputStateCreateInfo vi;
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.vertexBindingDescriptionCount = 1;                                  // �����������������
	vi.pVertexBindingDescriptions = m_createInfo->getVertexBinding();      // ��������������б�
	vi.vertexAttributeDescriptionCount = 2;                                  // ����������������
	vi.pVertexAttributeDescriptions = m_createInfo->getVertexAttributes(); // �����������������б�

	VkPipelineInputAssemblyStateCreateInfo ia;
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.primitiveRestartEnable = VK_FALSE;                               // �ر�ͼԪ����,            ��
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	                // ����������ͼԪ�б�ģʽ�� ��

														// ��դ����Ϣ
	VkPipelineRasterizationStateCreateInfo rs;
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = m_createInfo->getPolygonMode();     // ���Ʒ�ʽΪ���, ��
	rs.cullMode = VK_CULL_MODE_NONE;                  // ��ʹ�ñ�����ã���
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	 // ���Ʒ���Ϊ��ʱ��,��
	rs.depthClampEnable = VK_FALSE;						 	 // ��Ƚ�ȡ
	rs.rasterizerDiscardEnable = VK_FALSE;							 // ���ù�դ����������ΪTRUE���դ���������κ�ƬԪ��,��
	rs.depthBiasEnable = VK_FALSE;							 // ���������ƫ��
	rs.depthBiasConstantFactor = 0;									 // ���ƫ�Ƴ�������
	rs.depthBiasClamp = 0;								 	 // ���ƫ��ֵ�����ޣ���Ϊ����Ϊ���ޣ�Ϊ����Ϊ���ޣ�
	rs.depthBiasSlopeFactor = 0;								 	 // ���ƫ��б������
	rs.lineWidth = m_createInfo->getLineWidth();       // �߿�ȣ������߻���ģʽ�����ã�,��

													   // ������ɫ��ϸ���״̬����
	VkPipelineColorBlendAttachmentState att_state[1];
	att_state[0].colorWriteMask = 0xf;                                // ����д������
	att_state[0].blendEnable = VK_TRUE;                            // �رջ��
	att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;                    // ����Alphaͨ����Ϸ�ʽ
	att_state[0].colorBlendOp = VK_BLEND_OP_ADD;                    // ����RGBͨ����Ϸ�ʽ
	att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;          // ����Դ��ɫ�������
	att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;// ����Ŀ����ɫ�������
	att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;          // ����ԴAlpha�������
	att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;// ����Ŀ��Alpha�������

																		   // ���ߵ���ɫ���״̬������Ϣ
	VkPipelineColorBlendStateCreateInfo cb;
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;	             // ��ɫ��ϸ�������
	cb.pAttachments = att_state;        // ��ɫ��ϸ����б�
	cb.logicOpEnable = VK_TRUE;          // �����߼�������ԭ��������
	cb.logicOp = VK_LOGIC_OP_NO_OP;// �߼���������Ϊ��
	cb.blendConstants[0] = 1.0f;             // ��ϳ���R����
	cb.blendConstants[1] = 1.0f;             // ��ϳ���G����
	cb.blendConstants[2] = 1.0f;             // ��ϳ���B����
	cb.blendConstants[3] = 1.0f;             // ��ϳ���A����

											 // �ӿ���Ϣ ��
	VkViewport viewports;
	viewports.minDepth = 0.0f;             // �ӿ���С���
	viewports.maxDepth = 1.0f;             // �ӿ�������
	viewports.x = 0;				   // �ӿ�X����
	viewports.y = 0;			       // �ӿ�Y����
	viewports.width = Asset.screenWidth;// �ӿڿ��
	viewports.height = Asset.screenWidth;// �ӿڸ߶�

										 // ���ô�����Ϣ
	VkRect2D scissor;
	scissor.extent.width = Asset.screenWidth;// ���ô��ڵĿ��
	scissor.extent.height = Asset.screenWidth;// ���ô��ڵĸ߶�
	scissor.offset.x = 0;                // ���ô��ڵ�X����
	scissor.offset.y = 0;                // ���ô��ڵ�Y����

										 // �����ӿ�״̬������Ϣ
	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;         // �ӿڵ�����
	vp.scissorCount = 1;         // ���ô��ڵ�����
	vp.pScissors = &scissor;  // ���ô�����Ϣ�б�
	vp.pViewports = &viewports;// �ӿ���Ϣ�б�

							   // ������ȼ�ģ��״̬������Ϣ ��
	VkPipelineDepthStencilStateCreateInfo ds;
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	// ��Ȳ��� ���
	ds.depthTestEnable = m_createInfo->isDepthTestEnabled();   // ������Ȳ��� ��
	ds.depthWriteEnable = VK_TRUE;                              // �������ֵд��
	ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;          // ��ȼ��Ƚϲ���
	ds.depthBoundsTestEnable = VK_FALSE;                             // �ر���ȱ߽����
	ds.minDepthBounds = 0;                                    // ��С��ȱ߽�
	ds.maxDepthBounds = 0;                                    // �����ȱ߽�

															  // ģ����� ���
	ds.stencilTestEnable = m_createInfo->isStencilTestEnabled(); // ����ģ����� ��
	ds.back.passOp = VK_STENCIL_OP_KEEP;                   // ģ����ԡ���Ȳ��Զ�ͨ��ʱ�Ĳ���
	ds.back.compareOp = VK_COMPARE_OP_ALWAYS;                 // ģ����ԵıȽϲ���
	ds.back.compareMask = 0;                                    // ģ����ԱȽ�����
	ds.back.reference = 0;                                    // ģ����Բο�ֵ

															  // �������
	ds.back.failOp = VK_STENCIL_OP_KEEP;                   // δͨ��ģ�����ʱ�Ĳ���
	ds.back.depthFailOp = VK_STENCIL_OP_KEEP;                   // δͨ����Ȳ���ʱ�Ĳ���
	ds.back.writeMask = 0;//д������
	ds.front = ds.back;

	//���߶��ز���״̬������Ϣ (MSAA)
	VkPipelineMultisampleStateCreateInfo ms;
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pSampleMask = NULL;                                   // ��������
	ms.rasterizationSamples = m_createInfo->getMultiSampleLevelFlag();// ��դ���׶β�������
	ms.sampleShadingEnable = VK_FALSE;							   // �رղ�����ɫ
	ms.alphaToCoverageEnable = VK_FALSE;							   // ������alphaToCoverage
	ms.alphaToOneEnable = VK_FALSE;							   // ������alphaToOne
	ms.minSampleShading = 0.0;									   // ��С������ɫ

																   // ͼ�ι��ߴ�����Ϣ
	VkGraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = m_vkPipelineLayout;  // ָ�����߲���
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;		// �����߾��
	pipelineInfo.basePipelineIndex = 0;					// ����������
	pipelineInfo.flags = 0;					// ��־
	pipelineInfo.pVertexInputState = &vi;					// ���ߵĶ�����������״̬��Ϣ
	pipelineInfo.pInputAssemblyState = &ia;					// ���ߵ�ͼԪ��װ״̬��Ϣ
	pipelineInfo.pRasterizationState = &rs;					// ���ߵĹ�դ��״̬��Ϣ
	pipelineInfo.pColorBlendState = &cb;					// ���ߵ���ɫ���״̬��Ϣ
	pipelineInfo.pTessellationState = NULL;				// ���ߵ�����ϸ��״̬��Ϣ
	pipelineInfo.pMultisampleState = &ms;					// ���ߵĶ��ز���״̬��Ϣ
	pipelineInfo.pDynamicState = &dynamicState;		      // ���ߵĶ�̬״̬��Ϣ
	pipelineInfo.pViewportState = &vp;					// ���ߵ��ӿ�״̬��Ϣ
	pipelineInfo.pDepthStencilState = &ds;					// ���ߵ����ģ�����״̬��Ϣ
	pipelineInfo.stageCount = 2;					// ���ߵ���ɫ�׶�����
	pipelineInfo.pStages = m_vkShaderStage;		// ���ߵ���ɫ�׶��б�
	pipelineInfo.renderPass = Asset.renderPass;	// ָ������Ⱦͨ��
	pipelineInfo.subpass = 0;					// ���ù���ִ�ж�Ӧ����Ⱦ��ͨ��

												// ���߻��崴����Ϣ
	VkPipelineCacheCreateInfo pipelineCacheInfo;
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheInfo.initialDataSize = 0;// ��ʼ���ݳߴ�
	pipelineCacheInfo.pInitialData = NULL;// ��ʼ�������ݣ��˴�ΪNULL

	VkAssert(vkCreatePipelineCache(Asset.vkDevice, &pipelineCacheInfo, NULL, &m_vkPipelineCache) == VK_SUCCESS);
	VkAssert(vkCreateGraphicsPipelines(Asset.vkDevice, m_vkPipelineCache, 1, &pipelineInfo, NULL, &m_vkPipeline) == VK_SUCCESS);

	return true;
}

void AbstractPipeline::createUniformBuffer()
{
	// get the properties of GPU
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(Asset.gpus[VulkanConfig::GPU::DefaultGpuIndex], &physicalDeviceProperties);

	// get alignment from GPU's limit information
	const VkPhysicalDeviceLimits* pdevLimits = &(physicalDeviceProperties.limits);
	const VkDeviceSize            uniAlign = pdevLimits->minUniformBufferOffsetAlignment;

	// get something information from createInfo
	int bindingCount = m_createInfo->getDescriptorSetLayoutBindingCount();
	int vsUniformBufferByteCount = m_createInfo->getVertexStageUniformBytesCount();
	int fsUniformBufferByteCount = m_createInfo->getFragmentStageUniformBytesCount();

	// get aligned allocateSize for uniform buffer used in vertex and fragment stage
	const VkDeviceSize vsUniformBufferAllocSize = aligned(vsUniformBufferByteCount, uniAlign);
	const VkDeviceSize fsUniformBufferAllocSize = aligned(fsUniformBufferByteCount, uniAlign);
	const VkDeviceSize allocateSize = vsUniformBufferAllocSize + fsUniformBufferAllocSize;

	if (allocateSize == 0) return;

	// Create an instance of VkBufferCreateInfo
	// �� Specially, we care about 'sharingMode',here are official explanation of it:

	// VK_SHARING_MODE_EXCLUSIVE specifies that access to any range or image subresource of the object will be exclusive to a single queue family at a time.
	// VK_SHARING_MODE_CONCURRENT specifies that concurrent access to any range or image subresource of the object from multiple queue families is supported.

	// VK_SHARING_MODE_CONCURRENT may result in lower performance access to the buffer or image than VK_SHARING_MODE_EXCLUSIVE.

	// A queue family can take ownership of an image subresource or buffer range of a resource created with VK_SHARING_MODE_EXCLUSIVE,
	// without an ownership transfer, in the same way as for a resource that was just created; 
	// However, taking ownership in this way has the effect that the contents of the image subresource or buffer range are undefined.

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCreateInfo.size = allocateSize;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = NULL;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.flags = 0;

	VkAssert(vkCreateBuffer(Asset.vkDevice, &bufferCreateInfo, NULL, &m_vkUniformBuffer) == VK_SUCCESS);

	// get the memory requirement of Uniform Buffer
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(Asset.vkDevice, m_vkUniformBuffer, &memoryRequirements);

	// Create an instance of VkMemoryAllocateInfo
	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.memoryTypeIndex = 0;
	alloc_info.allocationSize = memoryRequirements.size; // ��

														 // Ensure the GPU's memory property
	VkFlags requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	bool flag = memoryTypeFromProperties(Asset.gpuMemoryProperty, memoryRequirements.memoryTypeBits, requirements_mask, &alloc_info.memoryTypeIndex);
	// if (flag) { printf("ȷ���ڴ����ͳɹ� ��������Ϊ%d", alloc_info.memoryTypeIndex); }
	// else { printf("ȷ���ڴ�����ʧ��!"); }

	VkAssert(vkAllocateMemory(Asset.vkDevice, &alloc_info, NULL, &m_vkUniformBufferMemory) == VK_SUCCESS);
	VkAssert(vkBindBufferMemory(Asset.vkDevice, m_vkUniformBuffer, m_vkUniformBufferMemory, 0) == VK_SUCCESS);

	// Fill the Uniform Buffer Info
	m_vkUniformBufferInfo.buffer = m_vkUniformBuffer;
	m_vkUniformBufferInfo.offset = 0;
	m_vkUniformBufferInfo.range = allocateSize;
}

void AbstractPipeline::createPipelineLayout()
{
	// firstly, we should create descriptorSet's layout
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = m_createInfo->getDescriptorSetLayoutBindingCount(); // ��
	layoutCreateInfo.pBindings = m_createInfo->getDescriptorSetLayoutBinding();      // ��

	m_vkDescriptorLayouts.resize(m_createInfo->getDescriptorMaxCount());

	VkAssert(vkCreateDescriptorSetLayout(Asset.vkDevice, &layoutCreateInfo, NULL, m_vkDescriptorLayouts.data()) == VK_SUCCESS);

	// and then, we create pipeline's layout
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = m_createInfo->getPushConstantRangeCount();
	pPipelineLayoutCreateInfo.pPushConstantRanges = m_createInfo->getPushConstantRanges();
	pPipelineLayoutCreateInfo.setLayoutCount = m_createInfo->getDescriptorMaxCount();
	pPipelineLayoutCreateInfo.pSetLayouts = m_vkDescriptorLayouts.data();

	VkAssert(vkCreatePipelineLayout(Asset.vkDevice, &pPipelineLayoutCreateInfo, NULL, &m_vkPipelineLayout) == VK_SUCCESS);
}

void AbstractPipeline::createShader()
{
	// Vertex Shader
	m_vkShaderStage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_vkShaderStage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;   // ��
	m_vkShaderStage[0].pName = "main";
	m_vkShaderStage[0].module = m_createInfo->getVertexShaderModule();

	// Fragment Shader
	m_vkShaderStage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_vkShaderStage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // ��
	m_vkShaderStage[1].pName = "main";
	m_vkShaderStage[1].module = m_createInfo->getFragmentShaderModule();
}

void AbstractPipeline::destroyShader()
{
	vkDestroyShaderModule(Asset.vkDevice, m_vkShaderStage[0].module, NULL);
	vkDestroyShaderModule(Asset.vkDevice, m_vkShaderStage[1].module, NULL);
}

void AbstractPipeline::destroyPipeline()
{
	vkDestroyPipeline(Asset.vkDevice, m_vkPipeline, NULL);
	vkDestroyPipelineCache(Asset.vkDevice, m_vkPipelineCache, NULL);
}

void AbstractPipeline::destoryUniformBuffer()
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

void AbstractPipeline::destoryPipelineLayout()
{
	for (int i = 0; i < m_createInfo->getDescriptorSetLayoutBindingCount(); i++)
	{
		vkDestroyDescriptorSetLayout(Asset.vkDevice, m_vkDescriptorLayouts[i], NULL);
	}
	vkDestroyPipelineLayout(Asset.vkDevice, m_vkPipelineLayout, NULL);
}

bool AbstractPipeline::create()
{
	if (!m_createInfo || m_createInfo->checkInfoValid()) { return false; };

	createUniformBuffer();
	createPipelineLayout();
	initDescriptorSet();
	createShader();
	createPipeline();

	return true;
}

AbstractPipeline::~AbstractPipeline()
{
	destroyPipeline();
	destroyShader();
	destoryPipelineLayout();
	destoryUniformBuffer();

	delete m_createInfo;
	m_createInfo = nullptr;
}

void PipelineInfo::addVertexInput(VkFormat vertexFormat)
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
		//�����Ч
		validInfo = false;
		errorMessage = "VK_FORMAT Error";
		return;
	}

	vertexInputBinding.binding = 0;//��Ӧ�󶨵�
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	//��������Ƶ��Ϊÿ����
	vertexInputBinding.stride = vertexOffset;//ÿ�����ݵĿ���ֽ���
}

void PipelineInfo::setVertexShader(QString path)
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

void PipelineInfo::setFragmentShader(QString path)
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

void PipelineInfo::setDescriptorMaxCount(int count)
{
	descriptorSetMaxCount = count;
	calledRecordBit = calledRecordBit | 0x08;
}

void PipelineInfo::addDescriptorLayoutBinding(VkDescriptorType type, VkShaderStageFlags stage)
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

void PipelineInfo::addPushConstantRange(int constantCount, VkShaderStageFlags stage)
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

void PipelineInfo::setMSAALevel(int level)
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

void PipelineInfo::setDepthTestEnable(bool enabled)
{
	this->enableDepthTest = true;

	calledRecordBit = calledRecordBit | 0x80;
}

void PipelineInfo::setStencilTestEnable(bool enabled)
{
	this->enableStencilTest = true;

	calledRecordBit = calledRecordBit | 0x100;
}

void PipelineInfo::setPolygonMode(VkPolygonMode mode, int lineWidth_if_use_lineMode)
{
	this->polygonMode = mode;
	if (mode == VK_POLYGON_MODE_LINE)
	{
		lineWidth = max(1, lineWidth_if_use_lineMode);
	}

	calledRecordBit = calledRecordBit | 0x200;
}

void PipelineInfo::setUniformBytesCount(int stage_vertex_count, int stage_fragment_count)
{
	this->vsUniformBytesCount = stage_vertex_count;
	this->fsUniformBytesCount = stage_fragment_count;
	calledRecordBit = calledRecordBit | 0x400;
}

void PipelineInfo::setUniformFormat(vector<QString> vertex_fmt, vector<QString> fragment_fmt)
{
	for (int i = 0; i < vertex_fmt.size(); i++)
	{
		if (vertex_fmt[i] == "int")         { this->vsUniformBytesCount += sizeof(int); }
		else if (vertex_fmt[i] == "float")  { this->vsUniformBytesCount += sizeof(float); }
		else if (vertex_fmt[i] == "double") { this->vsUniformBytesCount += sizeof(double); }
		else if (vertex_fmt[i] == "uint")   { this->vsUniformBytesCount += sizeof(unsigned int); }
	}
	for (int i = 0; i < fragment_fmt.size(); i++)
	{
		if (vertex_fmt[i] == "int")         { this->fsUniformBytesCount += sizeof(int); }
		else if (vertex_fmt[i] == "float")  { this->fsUniformBytesCount += sizeof(float); }
		else if (vertex_fmt[i] == "double") { this->fsUniformBytesCount += sizeof(double); }
		else if (vertex_fmt[i] == "uint")   { this->fsUniformBytesCount += sizeof(unsigned int); }
	}
	calledRecordBit = calledRecordBit | 0x400;
}

void PipelineInfo::setLineWidth(unsigned int width)
{
	lineWidth = width;
	calledRecordBit = calledRecordBit | 0x800;
}
