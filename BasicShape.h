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
	void   SetVertex2f(float x, float y);
	void   SetVertex3f(float x, float y, float z);
	void   SetColor3f(float r, float g, float b);
	void   SetColor4f(float r, float g, float b, float a);

	void   CreateVertexData();
	void   Draw();

};

