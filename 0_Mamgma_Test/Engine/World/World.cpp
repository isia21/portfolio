#include "stdafx.h"


//	#include "../Engine/3DObject.h"
//	#include "../Engine/Camera.h"
//	#include "../Engine/Scene.h"
#include "../Graphics.h"

#include "World.h"
#include "Scene.h"


//-----------------------------------------------------------------------------
// World
//-----------------------------------------------------------------------------
CWorld::CWorld()
	: m_pCamera(nullptr), m_pScene(nullptr)
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
	// --- Initialize camera ---
	m_pCamera = new CCamera();
	if (m_pCamera == nullptr)
		return false;

	// --- Initialize scene and objects ---
	m_pScene = new CScene("Main Scene");
	if (m_pScene == nullptr)
		return false;

	// Create default scene objects
	m_pScene->CreateDefault(); 
	
	Utils::ODS("[INFO] World initialized. Objects: %d", static_cast<int>(GetObjects().size()));

	return true;
}

void CWorld::Shutdown()
{
	// --- Destroy owned camera and scene ---
	SAFEDELETE(m_pCamera);
	SAFEDELETE(m_pScene);
}


//-----------------------------------------------------------------------------
// Update
//-----------------------------------------------------------------------------
void CWorld::Update(float fDeltaTime)
{
	if (m_pCamera != nullptr)
		m_pCamera->Update(fDeltaTime);

	if (m_pScene != nullptr)
		m_pScene->Update(fDeltaTime);

	// TODO:
	// Object transforms.
	// Object animation.
	// World state.
	// Simulation.
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
	static const std::vector<C3DObject*> s_empty;
	return (m_pScene != nullptr) ? m_pScene->GetObjects() : s_empty;
}