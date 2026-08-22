#include "stdafx.h"

#include "3DObject.h"

//-----------------------------------------------------------------------------
// C3DObject
//-----------------------------------------------------------------------------
C3DObject::C3DObject()
	: m_eObjectType(eOT_SourceModel)
	, m_bVisible(true)
	, m_vPosition(0.0f, 0.0f, 0.0f)
	, m_vRotation(0.0f, 0.0f, 0.0f)
	, m_vScale(1.0f, 1.0f, 1.0f)
	, m_pParent(nullptr)
	, m_dwModelColor(0xFFFFFFFF)
	, m_eRenderType(eRT_Poligon)
{}

C3DObject::~C3DObject()
{
	m_vChildren.clear();
	m_vVertices.clear();
	m_vIndices.clear();

	m_pParent = nullptr;
}


//-----------------------------------------------------------------------------
// Primitive factory
//-----------------------------------------------------------------------------
C3DObject* C3DObject::CreatePrimitive(EPrimitiveType eType, float fFactor)
{
	if (fFactor <= 0.0f)
		return nullptr;


	switch (eType)
	{
	case ePR_Cube:
		return CreateCube(fFactor);

	case ePR_Plane:
		return CreatePlane(fFactor);

	case ePR_Sphere:
		return CreateSphere(fFactor);

	case ePR_SinePlane:
		return CreateSinePlane(fFactor);

	default:
		break;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------
// Create cube
//
// fHalfSize defines half of the cube side.
// Local coordinates:
//
//        (-s,+s,+s)       (+s,+s,+s)
//              +-------------+
//             /|            /|
//            / |           / |
//           +-------------+  |
//           |  |          |  |
//           |  +----------|--+
//           | /           | /
//           |/            |/
//           +-------------+
//
// The cube is centered at local origin.
//-----------------------------------------------------------------------------
C3DObject* C3DObject::CreateCube(float fHalfSize)
{
	C3DObject* pObject = new C3DObject();

	pObject->m_eObjectType = eOT_SourceModel;

	const float s = fHalfSize;

	const unsigned int dwColor = 0xFFFFFFFF;

	pObject->m_vVertices =
	{
		{ -s, -s, -s, dwColor },
		{  s, -s, -s, dwColor },
		{  s,  s, -s, dwColor },
		{ -s,  s, -s, dwColor },

		{ -s, -s,  s, dwColor },
		{  s, -s,  s, dwColor },
		{  s,  s,  s, dwColor },
		{ -s,  s,  s, dwColor }
	};

	pObject->m_vIndices =
	{
		// Front
		4, 5, 6,
		4, 6, 7,

		// Back
		0, 2, 1,
		0, 3, 2,

		// Left
		0, 4, 7,
		0, 7, 3,

		// Right
		1, 2, 6,
		1, 6, 5,

		// Top
		3, 7, 6,
		3, 6, 2,

		// Bottom
		0, 1, 5,
		0, 5, 4
	};

	return pObject;
}


//-----------------------------------------------------------------------------
// Create plane
//
// fHalfSize defines half of the plane side.
// The plane lies on the XZ plane and is centered at local origin.
//-----------------------------------------------------------------------------
C3DObject* C3DObject::CreatePlane(float fHalfSize)
{
	C3DObject* pObject = new C3DObject();

	pObject->m_eObjectType = eOT_SourceModel;

	const float s = fHalfSize;

	const unsigned int dwColor = 0xFFFFFFFF;

	pObject->m_vVertices =
	{
		{ -s, 0.0f, -s, dwColor },
		{  s, 0.0f, -s, dwColor },
		{  s, 0.0f,  s, dwColor },
		{ -s, 0.0f,  s, dwColor }
	};

	// Counter-clockwise when viewed from +Y.
	pObject->m_vIndices =
	{
		0, 1, 2,
		0, 2, 3
	};

	return pObject;
}


//-----------------------------------------------------------------------------
// Create sphere
//
// fRadius defines sphere radius.
//
// The sphere is generated as a latitude/longitude grid.
// This deliberately produces ordinary vertices and triangle indices,
// because the resulting geometry will later be processed by slicing.
//-----------------------------------------------------------------------------
C3DObject* C3DObject::CreateSphere(float fRadius)
{
	C3DObject* pObject = new C3DObject();

	pObject->m_eObjectType = eOT_SourceModel;

	const int lSegments = 32;
	const int lRings = 16;

	const unsigned int dwColor = 0xFFFFFFFF;

	const float fPi = 3.14159265358979323846f;

	for (int y = 0; y <= lRings; ++y)
	{
		const float fV = static_cast<float>(y) / static_cast<float>(lRings);
		const float fPhi = fV * fPi;

		const float fSinPhi = sinf(fPhi);
		const float fCosPhi = cosf(fPhi);

		for (int x = 0; x <= lSegments; ++x)
		{
			const float fU = static_cast<float>(x) / static_cast<float>(lSegments);
			const float fTheta = fU * 2.0f * fPi;

			const float fSinTheta = sinf(fTheta);
			const float fCosTheta = cosf(fTheta);

			Vertex3D vertex = {};

			vertex.x = fRadius * fSinPhi * fCosTheta;
			vertex.y = fRadius * fCosPhi;
			vertex.z = fRadius * fSinPhi * fSinTheta;
			vertex.dwColor = dwColor;

			pObject->m_vVertices.push_back(vertex);
		}
	}

	const int lStride = lSegments + 1;

	for (int y = 0; y < lRings; ++y)
	{
		for (int x = 0; x < lSegments; ++x)
		{
			const unsigned int dwCurrent =
				static_cast<unsigned int>(y * lStride + x);

			const unsigned int dwNext =
				dwCurrent + 1;

			const unsigned int dwBelow =
				static_cast<unsigned int>((y + 1) * lStride + x);

			const unsigned int dwBelowNext =
				dwBelow + 1;

			pObject->m_vIndices.push_back(dwCurrent);
			pObject->m_vIndices.push_back(dwBelow);
			pObject->m_vIndices.push_back(dwNext);

			pObject->m_vIndices.push_back(dwNext);
			pObject->m_vIndices.push_back(dwBelow);
			pObject->m_vIndices.push_back(dwBelowNext);
		}
	}

	return pObject;
}

//-----------------------------------------------------------------------------
// Create sine plane
//
// fSize defines the width and length of the plane.
//
// Generates a wavy plane using sine and cosine functions.
// Highly non-convex, perfect for testing complex mesh slicing.
//-----------------------------------------------------------------------------
C3DObject* C3DObject::CreateSinePlane(float fSize)
{
	C3DObject* pObject = new C3DObject();

	if (pObject == nullptr)
		return nullptr;

	pObject->m_eObjectType = eOT_SourceModel;

	const int lSegments = 64; // Grid resolution (64x64 faces)
	const unsigned int dwColor = 0xFFFFFFFF;

	const float fHalfSize = fSize / 2.0f;
	const float fStep = fSize / static_cast<float>(lSegments);

	// Wave parameters (tweak these to change the shape)
	const float fFrequency = 1.5f;
	const float fAmplitude = fSize * 0.15f;

	// --- Generate vertices ---
	for (int z = 0; z <= lSegments; ++z)
	{
		const float fPosZ = -fHalfSize + (static_cast<float>(z) * fStep);
		const float fCosZ = cosf(fPosZ * fFrequency);

		for (int x = 0; x <= lSegments; ++x)
		{
			const float fPosX = -fHalfSize + (static_cast<float>(x) * fStep);
			const float fSinX = sinf(fPosX * fFrequency);

			// Math: Y = sin(X) * cos(Z) * Amplitude
			const float fPosY = fSinX * fCosZ * fAmplitude;

			Vertex3D vertex = {};

			vertex.x = fPosX;
			vertex.y = fPosY;
			vertex.z = fPosZ;
			vertex.dwColor = dwColor;

			pObject->m_vVertices.push_back(vertex);
		}
	}

	// --- Generate indices ---
	const int lStride = lSegments + 1;

	for (int z = 0; z < lSegments; ++z)
	{
		for (int x = 0; x < lSegments; ++x)
		{
			const unsigned int dwCurrent = static_cast<unsigned int>(z * lStride + x);
			const unsigned int dwNext = dwCurrent + 1;
			const unsigned int dwBelow = static_cast<unsigned int>((z + 1) * lStride + x);
			const unsigned int dwBelowNext = dwBelow + 1;

			// Triangle 1
			pObject->m_vIndices.push_back(dwCurrent);
			pObject->m_vIndices.push_back(dwBelow);
			pObject->m_vIndices.push_back(dwNext);

			// Triangle 2
			pObject->m_vIndices.push_back(dwNext);
			pObject->m_vIndices.push_back(dwBelow);
			pObject->m_vIndices.push_back(dwBelowNext);
		}
	}

	return pObject;
}
//-----------------------------------------------------------------------------
// Transform
//-----------------------------------------------------------------------------
void C3DObject::SetPosition(float x, float y, float z)
{
	m_vPosition = Vector3(x, y, z);
}

void C3DObject::SetRotation(float x, float y, float z)
{
	m_vRotation = Vector3(x, y, z);
}

void C3DObject::SetScale(float x, float y, float z)
{
	m_vScale = Vector3(x, y, z);
}


//-----------------------------------------------------------------------------
// Hierarchy
//-----------------------------------------------------------------------------
void C3DObject::SetParent(C3DObject* pParent)
{
	if (m_pParent == pParent)
		return;

	m_pParent = pParent;

	if (m_pParent != nullptr)
		m_pParent->m_vChildren.push_back(this);
}


//-----------------------------------------------------------------------------
// Rendering
//-----------------------------------------------------------------------------
void C3DObject::Render()
{
	if (!m_bVisible || m_vVertices.empty() || m_vIndices.empty())
		return;

	glPushMatrix();

	// --- Object transform ---
	glTranslatef(m_vPosition.x, m_vPosition.y, m_vPosition.z);

	glRotatef(m_vRotation.x, 1.0f, 0.0f, 0.0f);
	glRotatef(m_vRotation.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_vRotation.z, 0.0f, 0.0f, 1.0f);

	glScalef(m_vScale.x, m_vScale.y, m_vScale.z);

	// --- Render Config ---
	if (m_eRenderType == eRT_Wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBegin(GL_TRIANGLES);

	// --- Prepare model color ---
	const float modelR = static_cast<float>((m_dwModelColor >> 24) & 0xFF) / 255.0f;
	const float modelG = static_cast<float>((m_dwModelColor >> 16) & 0xFF) / 255.0f;
	const float modelB = static_cast<float>((m_dwModelColor >> 8) & 0xFF) / 255.0f;
	const float modelA = static_cast<float>(m_dwModelColor & 0xFF) / 255.0f;

	for (size_t i = 0; i < m_vIndices.size(); ++i)
	{
		const unsigned int index = m_vIndices[i];

		if (index >= m_vVertices.size())
			continue;

		const Vertex3D& vertex = m_vVertices[index];

		// --- Prepare vertex color ---
		const float vertR = static_cast<float>((vertex.dwColor >> 24) & 0xFF) / 255.0f;
		const float vertG = static_cast<float>((vertex.dwColor >> 16) & 0xFF) / 255.0f;
		const float vertB = static_cast<float>((vertex.dwColor >> 8) & 0xFF) / 255.0f;
		const float vertA = static_cast<float>(vertex.dwColor & 0xFF) / 255.0f;

		float finalR = modelR * vertR;
		float finalG = modelG * vertG;
		float finalB = modelB * vertB;
		float finalA = modelA * vertA;

		glColor4f(finalR, finalG, finalB, finalA);
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}
	glEnd();

	// --- Render reset states
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (m_eRenderType == eRT_Wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	glPopMatrix();

#ifdef _DEBUG
	static unsigned long s_lFrame = 0;
	Utils::ODS("[OBJECT] Frame %lu", ++s_lFrame);
#endif
}


//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------
bool C3DObject::Load()
{
	// XML loading will be implemented later.
	return false;
}

bool C3DObject::Save()
{
	// XML saving will be implemented later.
	return false;
}