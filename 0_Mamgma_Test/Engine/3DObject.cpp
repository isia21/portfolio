#include "stdafx.h"
#include "3DObject.h"

//-----------------------------------------------------------------------------
// 3D object
//-----------------------------------------------------------------------------
C3DObject::C3DObject()
	: m_eObjectType(OBJECT_TYPE_SOURCE_MODEL)
	, m_bVisible(true)
	, m_vPosition({ 0.0f, 0.0f, 0.0f })
	, m_vRotation({ 0.0f, 0.0f, 0.0f })
	, m_vScale({ 1.0f, 1.0f, 1.0f })
	, m_pParent(nullptr)
{}

C3DObject::~C3DObject()
{
	m_vChildren.clear();
	m_vVertices.clear();
	m_vIndices.clear();

	m_pParent = nullptr;
}

//-----------------------------------------------------------------------------
// Rendering
//-----------------------------------------------------------------------------
void C3DObject::Render()
{
	if (!m_bVisible)
		return;

	// TODO:
	// Apply object transform.
	// Render indexed geometry from m_vVertices / m_vIndices.
}

//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------
bool C3DObject::Load(const char* pszFileName)
{
	if (pszFileName == nullptr)
		return false;

	// TODO:
	// Load object geometry and properties from XML.

	return false;
}

bool C3DObject::Save(const char* pszFileName)
{
	if (pszFileName == nullptr)
		return false;

	// TODO:
	// Save object geometry and properties to XML.

	return false;
}

//-----------------------------------------------------------------------------
// Transform
//-----------------------------------------------------------------------------
void C3DObject::SetPosition(float x, float y, float z)
{
	m_vPosition.x = x;
	m_vPosition.y = y;
	m_vPosition.z = z;
}

void C3DObject::SetRotation(float x, float y, float z)
{
	m_vRotation.x = x;
	m_vRotation.y = y;
	m_vRotation.z = z;
}

void C3DObject::SetScale(float x, float y, float z)
{
	m_vScale.x = x;
	m_vScale.y = y;
	m_vScale.z = z;
}