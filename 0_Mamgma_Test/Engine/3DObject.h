#pragma once

//-----------------------------------------------------------------------------
// Object type
//-----------------------------------------------------------------------------
enum EObjectType
{
	OBJECT_TYPE_SOURCE_MODEL = 0,
	OBJECT_TYPE_SLICER,
	OBJECT_TYPE_MESH_PART
};


//-----------------------------------------------------------------------------
// 3D vector
//-----------------------------------------------------------------------------
struct Vector3
{
	float x;
	float y;
	float z;
};


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
	~C3DObject();

public:
	void Render();

	bool Load(const char* pszFileName);
	bool Save(const char* pszFileName);

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

	const Vector3& GetPosition() const { return m_vPosition; }
	const Vector3& GetRotation() const { return m_vRotation; }
	const Vector3& GetScale() const { return m_vScale; }

	void SetVisible(bool bVisible) { m_bVisible = bVisible; }
	bool IsVisible() const { return m_bVisible; }

	void SetObjectType(EObjectType eType) { m_eObjectType = eType; }
	EObjectType GetObjectType() const { return m_eObjectType; }

public:
	void SetParent(C3DObject* pParent) { m_pParent = pParent; }
	C3DObject* GetParent() const { return m_pParent; }

	const std::vector<C3DObject*>& GetChildren() const { return m_vChildren; }

protected:

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