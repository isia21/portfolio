#include "stdafx.h"

#include "3DObject.h"
#include "Scene.h"

//-----------------------------------------------------------------------------
// Construction / Destruction
//-----------------------------------------------------------------------------
CScene::CScene(const char* pszName)
	: m_sName(pszName != nullptr ? pszName : "Untitled Scene")
	, m_sFilePath("")
	, m_bModified(false)
	, m_bStructureChanged(true)
{}

CScene::~CScene()
{
	Clear();
}

//-----------------------------------------------------------------------------
// Lifecycle
//-----------------------------------------------------------------------------
void CScene::Clear()
{
	for (C3DObject* pObject : m_vObjects)
	{
		delete pObject;
	}
	m_vObjects.clear();

	m_bModified = false;

	Utils::ODS("[SCENE] Scene '%s' cleared.", m_sName.c_str());
}

void CScene::CreateDefault()
{
	Clear();

	m_sName = "Default Test Scene";

	// --- 1. Ground plane ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Plane, 10.0f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(0.0f, -2.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0x808080FF);
			pObject->SetName("Ground Plane");
			AddObject(pObject);
		}
	}

	// --- 2. Red Cube ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.0f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(-3.0f, 0.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0xFF0000FF);
			pObject->SetName("Cube #1 (Red)");
			AddObject(pObject);
		}
	}

	// --- 3. Green Cube (Rotated) ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.5f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(3.0f, 0.0f, 0.0f);
			pObject->SetRotation(0.0f, 45.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0x00FF00FF);
			pObject->SetName("Cube #2 (Green)");
			AddObject(pObject);
		}
	}

	// --- 4. Yellow Sphere ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Sphere, 1.0f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(-2.0f, 0.0f, 4.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0xFFFF00FF);
			pObject->SetName("Sphere #1 (Yellow)");
			AddObject(pObject);
		}
	}

	// --- 5. Blue Sphere (Wireframe) ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Sphere, 1.5f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(2.0f, 1.0f, 4.0f);
			pObject->SetRenderType( C3DObject::eRT_Wireframe);
			pObject->SetModelColor(0x0000FFFF);
			pObject->SetVisible(true);
			pObject->SetName("Sphere #2 (Blue Wire)");
			AddObject(pObject);
		}
	}

	// --- 6. Sine Plane (Сложный невыпуклый меш для нарезки) ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_SinePlane, 10.0f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(0.0f, 2.0f, -8.0f);
			pObject->SetModelColor(0xFF00FFFF);
			pObject->SetVisible(true);
			pObject->SetName("Sine Mesh (Source)");
			AddObject(pObject);
		}
	}

	m_bModified = false;

	Utils::ODS("[SCENE] Default scene '%s' created with %d objects.", m_sName.c_str(), static_cast<int>(m_vObjects.size()));
}

void CScene::Update(float fDeltaTime)
{
	// Анимации и обновления объектов сцены
	// (например, вращение ножа или покачивание волн)
}

//-----------------------------------------------------------------------------
// Object Management
//-----------------------------------------------------------------------------
void CScene::AddObject(C3DObject* pObject)
{
	if (pObject == nullptr)
		return;

	m_vObjects.push_back(pObject);
	m_bModified = true;
	m_bStructureChanged = true;
}

void CScene::RemoveObject(C3DObject* pObject)
{
	if (pObject == nullptr)
		return;

	for (auto it = m_vObjects.begin(); it != m_vObjects.end(); ++it)
	{
		if (*it == pObject)
		{
			delete* it;
			m_vObjects.erase(it);
			m_bModified = true;
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// I/O and Serialization
//-----------------------------------------------------------------------------
bool CScene::LoadFromFile(const char* pszFilePath)
{
	if (pszFilePath == nullptr || strlen(pszFilePath) == 0)
		return false;

	Utils::ODS("[SCENE] Loading scene from: %s", pszFilePath);

	// TODO: Реализация чтения JSON / Binary формата сцены
	// 1. Очистить текущую сцену: Clear()
	// 2. Считать имя, объекты, их координаты и цвета

	m_sFilePath = pszFilePath;
	m_bModified = false;
	return true;
}

bool CScene::SaveToFile(const char* pszFilePath)
{
	const char* pszTargetFile = (pszFilePath != nullptr && strlen(pszFilePath) > 0) ? pszFilePath : m_sFilePath.c_str();

	if (strlen(pszTargetFile) == 0)
	{
		Utils::ODS("[SCENE_ERROR] Cannot save scene: File path is empty.");
		return false;
	}

	Utils::ODS("[SCENE] Saving scene to: %s", pszTargetFile);

	// TODO: Реализация сериализации (сохранение списка m_vObjects)

	m_sFilePath = pszTargetFile;
	m_bModified = false;
	return true;
}

bool CScene::ExportToOBJ(const char* pszFilePath)
{
	if (pszFilePath == nullptr || strlen(pszFilePath) == 0)
		return false;

	Utils::ODS("[SCENE] Exporting all scene meshes to Wavefront OBJ: %s", pszFilePath);

	// TODO: Пройтись по всем m_vObjects и записать вершины (v) и грани (f) в .obj файл

	return true;
}

//-----------------------------------------------------------------------------
// Properties
//-----------------------------------------------------------------------------
void CScene::SetName(const char* pszName)
{
	m_sName = (pszName != nullptr) ? pszName : "Untitled Scene";
	m_bModified = true;
}

void CScene::SetFilePath(const char* pszFilePath)
{
	m_sFilePath = (pszFilePath != nullptr) ? pszFilePath : "";
}