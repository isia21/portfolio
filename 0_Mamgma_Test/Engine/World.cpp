#include "stdafx.h"

#include "World.h"

#include "../Engine/3DObject.h"
#include "../Engine/Camera.h"


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

C3DObject* CWorld::CreateObject(EObjectType eType)
{
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