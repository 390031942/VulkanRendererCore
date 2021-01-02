#pragma once
#include "AbstractRenderable.h"

class BasicShape :public AbstractRenderable
{
private:
	const static int ParamCount = 7;
protected:
	BasicShape(int vertexCount = 4);

	float* m_VertexData;
	int							 m_DataByteCount;
	int							 m_VertexCount;
	int							 m_VertexIndex;
	vector<uint16_t>             m_Indices;
	VkDeviceSize                 m_IndexSize;
	bool                         m_IsValid;
	VkDeviceSize                 m_VertexSize;

public:
	void   setVertex2f(float x, float y);
	void   setVertex3f(float x, float y, float z);
	void   setColor3f(float r, float g, float b);
	void   setColor4f(float r, float g, float b, float a);

	void   createVertexData();
	void   render();
};

