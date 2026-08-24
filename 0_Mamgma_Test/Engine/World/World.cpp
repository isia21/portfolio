#include "stdafx.h"

#include "../Input.h"
#include "../Graphics.h"

#include "World.h"
#include "Scene.h"

#include "../Graphics/UI/Pages/ObjectInspectorPage.h"
#include "../Graphics/UI/Pages/ScenePage.h"
#include "../../App/Application.h"


//-----------------------------------------------------------------------------
// World
//-----------------------------------------------------------------------------
CWorld::CWorld()
	: m_pCamera(nullptr), m_pScene(nullptr)
	, m_pHoveredObject(nullptr)
	, m_bIsCuttingGesture(false)
	, m_lCutStartX(0), m_lCutStartY(0)
	, m_lCutCurX(0), m_lCutCurY(0)
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
	// --- Runtime slicing by (CTRL + LBM Drag) ---
	CKeyboard* pKb = CKeyboard::GetInstance();
	CMouse* pMouse = CMouse::GetInstance();

	// --- 3D RAY CAST / HOVER & SELECTION 
	if (m_pCamera && m_pScene && pMouse)
	{
		const int mouseX = pMouse->GetX();
		const int mouseY = pMouse->GetY();

		Vector3 rayDir = UnprojectScreenToRay(mouseX, mouseY);
		Vector3 camPos(m_pCamera->GetPosX(), m_pCamera->GetPosY(), m_pCamera->GetPosZ());

		float hitDist = 0.0f;
		C3DObject* pCurrentHovered = m_pScene->Raycast(camPos, rayDir, hitDist);

		// --- Hovered object change detect (wo flood) ---
		if (pCurrentHovered != m_pHoveredObject)
		{
			Utils::ODS("[HOVER] Cursor target changed: '%s' -> '%s'",
				m_pHoveredObject ? m_pHoveredObject->GetName() : "None",
				pCurrentHovered ? pCurrentHovered->GetName() : "None");

			m_pHoveredObject = pCurrentHovered;
		}

		// --- LMB on hovered object == new CurSelObj --
		// IGNORE IF kb.key.CTRL pressed
		if (!pKb->IsKeyDown(VK_CONTROL) && pMouse->IsButtonPressed(CMouse::Button_Left))
		{
			CApplication* pApp = CApplication::GetInstance();
			if (pApp)
			{
				pApp->SetSelectedObject(m_pHoveredObject);

				if (pApp->GetObjectInspectorPage())
					pApp->GetObjectInspectorPage()->UpdateInspector();

				if (pApp->GetScenePage())
					pApp->GetScenePage()->RebuildSceneTree();

				if (m_pHoveredObject)
					Utils::ODS("[PICK] Selected 3D Object: '%s'", m_pHoveredObject->GetName());
				else
					Utils::ODS("[PICK] Selection cleared (clicked empty space)");
			}
		}
	}

	if (pKb && pMouse)
	{
		const bool bCtrl = pKb->IsKeyDown(VK_CONTROL);

		if (bCtrl)
		{
			// Src Point (kife start)
			if (pMouse->IsButtonPressed(CMouse::Button_Left))
			{
				m_bIsCuttingGesture = true;
				m_lCutStartX = pMouse->GetX();
				m_lCutStartY = pMouse->GetY();
				m_lCutCurX = m_lCutStartX;
				m_lCutCurY = m_lCutStartY;
				pMouse->ConsumeButton(CMouse::Button_Left);
			}
			else if (m_bIsCuttingGesture)
			{
				m_lCutCurX = pMouse->GetX();
				m_lCutCurY = pMouse->GetY();
				pMouse->ConsumeButton(CMouse::Button_Left);

				// Dst Point (knife end)
				if (!pMouse->IsButtonDownRaw(CMouse::Button_Left))
				{
					m_bIsCuttingGesture = false;

					// debug, line must be > 15px
					const int dx = m_lCutCurX - m_lCutStartX;
					const int dy = m_lCutCurY - m_lCutStartY;
					if ((dx * dx + dy * dy) > 225)
					{
						CreateSlicerFromScreenLine(m_lCutStartX, m_lCutStartY, m_lCutCurX, m_lCutCurY);
					}
				}
			}
		}
		else
		{
			m_bIsCuttingGesture = false;
		}
	}

	if (m_pCamera != nullptr)
		m_pCamera->Update(fDeltaTime);

	if (m_pScene != nullptr)
		m_pScene->Update(fDeltaTime);
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

//-----------------------------------------------------------------------------
// Runtime slicing
//-----------------------------------------------------------------------------

Vector3 CWorld::UnprojectScreenToRay(int screenX, int screenY) const
{
	if (!m_pCamera) 
		return Vector3(0, 0, -1);

	const float w = static_cast<float>(m_pCamera->GetViewportWidth());
	const float h = static_cast<float>(m_pCamera->GetViewportHeight());
	const float aspect = h > 0 ? (w / h) : 1.0f;

	// FOV 60 deg -> half 30 deg
	const float tanHalfFov = tanf(30.0f * MATH_DEG2RAD);

	// Translate px into NDC coord (-1 .. +1)
	const float ndcX = (2.0f * screenX / w) - 1.0f;
	const float ndcY = 1.0f - (2.0f * screenY / h);

	// View Space Ray
	const float vx = ndcX * aspect * tanHalfFov;
	const float vy = ndcY * tanHalfFov;
	const float vz = -1.0f;

	// Translate ViewSpace to WorldSpace  (inv rot Pitch n Yaw)
	const float pitch = m_pCamera->GetPitch();
	const float yaw = m_pCamera->GetYaw();

	// 1. Rot X by -Pitch
	const float cosP = cosf(-pitch), sinP = sinf(-pitch);
	const float rx = vx;
	const float ry = vy * cosP - vz * sinP;
	const float rz = vy * sinP + vz * cosP;

	// 2. Rot Y by -Yaw
	const float cosY = cosf(-yaw), sinY = sinf(-yaw);
	Vector3 worldRay;
	worldRay.x = rx * cosY + rz * sinY;
	worldRay.y = ry;
	worldRay.z = -rx * sinY + rz * cosY;

	return worldRay.Normalized();
}

void CWorld::CreateSlicerFromScreenLine(int x1, int y1, int x2, int y2)
{
	if (!m_pCamera || !m_pScene) 
		return;

	// 1. Make 2 rays throw ends ov sceen line
	Vector3 r1 = UnprojectScreenToRay(x1, y1);
	Vector3 r2 = UnprojectScreenToRay(x2, y2);

	// 2. Plane Normal - is Cross Product of Rays
	Vector3 normal = r1.Cross(r2);
	if (normal.LengthSquared() < 1e-6f)
		return; // Too short or Rays is parallel

	normal.Normalize();

	// 3. Calc Slicer DOT (knife pos)
	Vector3 midRay = (r1 + r2).Normalized();
	const float zoomDist = m_pCamera->GetZoom();
	Vector3 camPos(m_pCamera->GetPosX(), m_pCamera->GetPosY(), m_pCamera->GetPosZ());
	Vector3 spawnPos = camPos + (midRay * zoomDist);

	// 4. Find kinfe rot angles (RotX, RotZ) by calced Normal
	//	float rotZ = atan2f(-normal.x, normal.y) * 57.2957795f;
	//	float rotX = atan2f(-normal.z, sqrtf(normal.x * normal.x + normal.y * normal.y)) * 57.2957795f;
	float rotX = atan2f(normal.z, normal.y) * MATH_RAD2DEG;
	float rotZ = atan2f(-normal.x, sqrtf(normal.y * normal.y + normal.z * normal.z)) * MATH_RAD2DEG;

	// 5. Create NEW Slicer/Knife on Scene
	size_t slicerCount = 0;
	for (C3DObject* pObj : m_pScene->GetObjects())
	{
		if (pObj && pObj->GetObjectType() == C3DObject::eOT_Slicer)
			++slicerCount;
	}

	CSlicer* pNewSlicer = new CSlicer(zoomDist * 0.8f, 0xFF005580);
	pNewSlicer->SetObjectType(C3DObject::eOT_Slicer);

	char szName[64] = {};
	sprintf_s(szName, sizeof(szName), "Slicer #%zu (Gesture)", slicerCount + 1);
	pNewSlicer->SetName(szName);

	pNewSlicer->SetPosition(spawnPos.x, spawnPos.y, spawnPos.z);
	pNewSlicer->SetRotation(rotX, 0.0f, rotZ);

	m_pScene->AddObject(pNewSlicer);
	Utils::ODS("[GESTURE_CUT] Created interactive 3D Slicer along screen gesture!");
}
