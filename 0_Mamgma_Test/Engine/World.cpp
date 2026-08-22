#include "stdafx.h"


#include "../Engine/3DObject.h"
#include "../Engine/Camera.h"

#include "World.h"


//-----------------------------------------------------------------------------
// World
//-----------------------------------------------------------------------------
CWorld::CWorld()
	: m_pCamera(nullptr)
{}

CWorld::~CWorld()
{
	Shutdown();
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CWorld::Init()
{
	m_pCamera = new CCamera();

	if (m_pCamera == nullptr)
		return false;


	//-------------------------------------------------------------------------
	// Test scene
	//-------------------------------------------------------------------------

	// --- Ground plane ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Plane, 10.0f);

		if (pObject != nullptr)
		{
			pObject->SetPosition(0.0f, -2.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0x808080FF);
			m_vObjects.push_back(pObject);
		}
	}

	// --- Cube #1 ---
	{
		C3DObject* pObject =
			C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.0f);

		if (pObject != nullptr)
		{
			pObject->SetPosition(-3.0f, 0.0f, 0.0f);
			pObject->SetRotation(0.0f, 0.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0xFF0000FF);
			m_vObjects.push_back(pObject);
		}
	}

	// --- Cube #2 ---
	{
		C3DObject* pObject =
			C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.5f);

		if (pObject != nullptr)
		{
			pObject->SetPosition(3.0f, 0.0f, 0.0f);
			pObject->SetRotation(0.0f, 45.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0x00FF00FF);

			m_vObjects.push_back(pObject);
		}
	}

	// --- Sphere #1 ---
	{
		C3DObject* pObject =
			C3DObject::CreatePrimitive(C3DObject::ePR_Sphere, 1.0f);

		if (pObject != nullptr)
		{
			pObject->SetPosition(-2.0f, 0.0f, 4.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0xFFFF00FF);

			m_vObjects.push_back(pObject);
		}
	}

	// --- Sphere #2 ---
	{
		C3DObject* pObject =
			C3DObject::CreatePrimitive(C3DObject::ePR_Sphere, 1.5f);

		if (pObject != nullptr)
		{
			pObject->SetPosition(2.0f, 1.0f, 4.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0x0000FFFF);
			pObject->SetRenderType(C3DObject::ERenderType::eRT_Wireframe);

			m_vObjects.push_back(pObject);
		}
	}

	Utils::ODS(
		"[INFO] World initialized. Objects: %d",
		static_cast<int>(m_vObjects.size()));

	return true;


	return true;
}

void CWorld::Shutdown()
{
	for (std::vector<C3DObject*>::iterator it = m_vObjects.begin(); it != m_vObjects.end(); ++it)
	{
		delete* it;
	}

	m_vObjects.clear();

	SAFEDELETE(m_pCamera);
	//	if (m_pCamera != nullptr)
	//	{
	//		delete m_pCamera;
	//		m_pCamera = nullptr;
	//	}
}


//-----------------------------------------------------------------------------
// Update
//-----------------------------------------------------------------------------
void CWorld::Update(float fDeltaTime)
{
	if (m_pCamera != nullptr)
		m_pCamera->Update(fDeltaTime);

	// TODO:
	// Object transforms.
	// Object animation.
	// World state.
	// Simulation.
}

//-----------------------------------------------------------------------------
// Objects
//-----------------------------------------------------------------------------
void CWorld::AddObject(C3DObject* pObject)
{
	if (pObject == nullptr)
		return;

	m_vObjects.push_back(pObject);
}

void CWorld::RemoveObject(C3DObject* pObject)
{
	if (pObject == nullptr)
		return;

	for (std::vector<C3DObject*>::iterator it = m_vObjects.begin(); it != m_vObjects.end(); ++it)
	{
		if (*it == pObject)
		{
			delete* it;
			m_vObjects.erase(it);
			return;
		}
	}
}

C3DObject* CWorld::CreateObject(int lType)
{
	C3DObject::EObjectType eType = (C3DObject::EObjectType)lType;
	C3DObject* pObject = new C3DObject();

	if (pObject == nullptr)
		return nullptr;

	pObject->SetObjectType(eType);

	AddObject(pObject);

	return pObject;
}


//-----------------------------------------------------------------------------
// Camera
//-----------------------------------------------------------------------------
CCamera* CWorld::GetCamera() const
{
	return m_pCamera;
}

//-----------------------------------------------------------------------------
// Objects
//-----------------------------------------------------------------------------
const std::vector<C3DObject*>& CWorld::GetObjects() const
{
	return m_vObjects;
}