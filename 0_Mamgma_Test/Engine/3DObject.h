#pragma once

#include "../Math/Vector3.h"

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
	enum EObjectType
	{
		eOT_SourceModel = 0,
		eOT_Slicer,
		eOT_MeshParts
	};
	enum ERenderType
	{
		eRT_Poligon,
		eRT_Wireframe
	};

	enum EPrimitiveType
	{
		ePR_Cube = 0,
		ePR_Plane,
		ePR_Sphere,

		ePR_SinePlane
	};


public:
	C3DObject();
	~C3DObject();


public:
	// --- Object params ---
	void SetModelColor(DWORD dwColor) { m_dwModelColor = dwColor; }
	void SetRenderType(ERenderType eType) { m_eRenderType = eType; }
	ERenderType GetRenderType() const { return m_eRenderType; }

	// --- Primitive factory ---
	static C3DObject* CreatePrimitive(EPrimitiveType eType, float fFactor);

	// --- Object type ---
	void SetObjectType(EObjectType eType) { m_eObjectType = eType; }
	EObjectType GetObjectType() const { return m_eObjectType; }

	// --- Object visibility ---
	void SetVisible(bool bVisible) { m_bVisible = bVisible; }
	bool IsVisible() const { return m_bVisible; }

	// --- Object transform ---
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

	// --- Object transform getters ---
	const Vector3& GetPosition() const { return m_vPosition; }
	const Vector3& GetRotation() const { return m_vRotation; }
	const Vector3& GetScale() const { return m_vScale; }

	// --- Object hierarchy ---
	void SetParent(C3DObject* pParent);
	C3DObject* GetParent() const { return m_pParent; }

	const std::vector<C3DObject*>& GetChildren() const { return m_vChildren; }

	// --- Geometry ---
	const std::vector<Vertex3D>& GetVertices() const { return m_vVertices; }
	const std::vector<unsigned int>& GetIndices() const { return m_vIndices; }

	std::vector<Vertex3D>& GetVertices() { return m_vVertices; }
	std::vector<unsigned int>& GetIndices() { return m_vIndices; }

	// --- Rendering ---
	void Render();

	// --- Serialization ---
	bool Load();
	bool Save();

	// --- Object name ---
	const char* GetName() const { return m_strName.c_str(); }
	void SetName(const char* pszName) { m_strName = (pszName != nullptr) ? pszName : ""; }

	// --- Geometry stats ---
	size_t GetVertexCount() const { return m_vVertices.size(); }
	size_t GetTriangleCount() const { return m_vIndices.size() / 3; }

private:
	// --- Primitive generators ---
	static C3DObject* CreateCube(float fHalfSize);
	static C3DObject* CreatePlane(float fHalfSize);
	static C3DObject* CreateSphere(float fRadius);
	static C3DObject* CreateSinePlane(float fSize);

private:
	// --- Object stats ---
	DWORD m_dwModelColor;
	ERenderType m_eRenderType;
	std::string m_strName;

	// --- Object type ---
	EObjectType m_eObjectType;

	// --- Object visibility ---
	bool m_bVisible;

	// --- Object transform ---
	Vector3 m_vPosition;
	Vector3 m_vRotation;
	Vector3 m_vScale;

	// --- Object hierarchy ---
	C3DObject* m_pParent;
	std::vector<C3DObject*> m_vChildren;

	// --- Object geometry ---
	std::vector<Vertex3D> m_vVertices;
	std::vector<unsigned int> m_vIndices;
};