#pragma once

class C3DObject;

//-----------------------------------------------------------------------------
// Scene Class (Owns 3D entities, scene state, and serialization)
//-----------------------------------------------------------------------------
class CScene
{
public:
	CScene(const char* pszName = "Untitled Scene");
	~CScene();

public:
	// --- Scene lifecycle ---
	void CreateDefault();
	void Clear();
	void Update(float fDeltaTime);

public:
	// --- Object management ---
	void AddObject(C3DObject* pObject);
	void RemoveObject(C3DObject* pObject);

	const std::vector<C3DObject*>& GetObjects() const { return m_vObjects; }
	size_t GetObjectCount() const { return m_vObjects.size(); }

	// --- Structure change flag (addition, removal, slicing) ---
	bool HasStructureChanged() const { return m_bStructureChanged; }
	void ResetStructureChanged() { m_bStructureChanged = false; }
	void MarkStructureChanged() { m_bStructureChanged = true; }

public:
	// --- Scene I/O and Serialization ---
	bool LoadFromFile(const char* pszFilePath);
	bool SaveToFile(const char* pszFilePath);
	bool ExportToOBJ(const char* pszFilePath);

public:
	// --- Properties and state --- 
	const char* GetName() const { return m_sName.c_str(); }
	void SetName(const char* pszName);

	const char* GetFilePath() const { return m_sFilePath.c_str(); }
	void SetFilePath(const char* pszFilePath);

	bool IsModified() const { return m_bModified; }
	void SetModified(bool bModified) { m_bModified = bModified; }

private:
	std::string m_sName;
	std::string m_sFilePath;
	bool m_bModified;

	bool m_bStructureChanged; // <-- Добавить флаг

	std::vector<C3DObject*> m_vObjects;
};