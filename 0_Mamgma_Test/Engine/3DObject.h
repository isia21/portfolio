#pragma once

//-----------------------------------------------------------------------------
// 3D vertex
//-----------------------------------------------------------------------------
struct Vertex3D
{
	float x;
	float y;
	float z;

	unsigned int dwColor;
};

//-----------------------------------------------------------------------------
// 3D object
//-----------------------------------------------------------------------------
class C3DObject
{
public:
	C3DObject();
	virtual ~C3DObject();

public:
	virtual void Render();

protected:
	std::vector<Vertex3D> m_vVertices;
	std::vector<unsigned int> m_vIndices;
};