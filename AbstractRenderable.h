#pragma once
#include "vulkanrenderercore_global.h"

class AbstractRenderable
{
protected:
	VkBuffer        m_vkVertexBuffer;
	VkDeviceMemory  m_vkVertexBufferMemory;

	VkPipeline*     m_pPipeline;
	VkDescriptorSet m_vkDescriptorSet;

public:
	void setVkPipeLine(VkPipeline* pPipeline)
	{
		m_pPipeline = pPipeline;
	}

	void setVkDescriptorSet(VkDescriptorSet vkDescriptorSet)
	{
		m_vkDescriptorSet = vkDescriptorSet;
	}
public:
	virtual void  createVertexData() = 0;
	virtual void  draw() = 0;
};