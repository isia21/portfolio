#include "stdafx.h"
#include "Camera.h"
#include "../Input.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
namespace
{
	const float CAMERA_MIN_ZOOM = 1.0f;
	const float CAMERA_MAX_ZOOM = 1000.0f;
}


//-----------------------------------------------------------------------------
// CCamera
//-----------------------------------------------------------------------------
CCamera::CCamera()
	: m_fTargetX(0.0f)
	, m_fTargetY(0.0f)
	, m_fTargetZ(0.0f)
	, m_fPosX(0.0f)
	, m_fPosY(0.0f)
	, m_fPosZ(0.0f)
	, m_fZoom(15.0f)
	, m_fPitch(0.5f) // From UP
	, m_fYaw(0.785f) // By 45deg
	, m_lViewportWidth(1280)
	, m_lViewportHeight(720)
{
	UpdatePosition();
}

CCamera::~CCamera()
{}

//-----------------------------------------------------------------------------
// Update
//-----------------------------------------------------------------------------
void CCamera::Update(float fDeltaTime)
{
	CMouse* pMouse = CMouse::GetInstance();
	if (!pMouse) return;

	// --- 1. Zoom (scroll by mouse wheel) ---
	int wheel = pMouse->GetWheelDelta();
	if (wheel != 0)
	{
		// --- zoom progression/speed
		float zoomSpeed = m_fZoom * 0.1f;
		Zoom(wheel > 0 ? -zoomSpeed : zoomSpeed);
	}

	// --- 2. Orbit (Rotate: Right Button) ---
	bool bIsOrbiting = pMouse->IsButtonDown(CMouse::Button_Right);
	bool bIsShiftDown = CKeyboard::GetInstance()->IsKeyDown(VK_SHIFT);

	if (bIsOrbiting && !bIsShiftDown)
	{
		float dx = static_cast<float>(pMouse->GetDeltaX());
		float dy = static_cast<float>(pMouse->GetDeltaY());

		m_fYaw += dx * 0.005f;
		m_fPitch += dy * 0.005f;

		// limit +-89deg for pitch (up/down
		const float pitchLimit = 1.55f;
		if (m_fPitch > pitchLimit) 
			m_fPitch = pitchLimit;
		if (m_fPitch < -pitchLimit) 
			m_fPitch = -pitchLimit;
	}

	//	meh. idk. maybe will fine wo ths 	if (pMouse->IsButtonDown(CMouse::Button_Middle) || (bIsOrbiting && bIsShiftDown))
	//	meh. idk. maybe will fine wo ths 	{
	//	meh. idk. maybe will fine wo ths 		float dx = static_cast<float>(pMouse->GetDeltaX());
	//	meh. idk. maybe will fine wo ths 		float dy = static_cast<float>(pMouse->GetDeltaY());
	//	meh. idk. maybe will fine wo ths 	
	//	meh. idk. maybe will fine wo ths 		float panSpeed = m_fZoom * 0.0015f;
	//	meh. idk. maybe will fine wo ths 	
	//	meh. idk. maybe will fine wo ths 		float yawCos = cosf(m_fYaw);
	//	meh. idk. maybe will fine wo ths 		float yawSin = sinf(m_fYaw);
	//	meh. idk. maybe will fine wo ths 		float pitchCos = cosf(m_fPitch);
	//	meh. idk. maybe will fine wo ths 		float pitchSin = sinf(m_fPitch);
	//	meh. idk. maybe will fine wo ths 	
	//	meh. idk. maybe will fine wo ths 		float rightX = yawCos;
	//	meh. idk. maybe will fine wo ths 		float rightY = 0.0f;
	//	meh. idk. maybe will fine wo ths 		float rightZ = yawSin;
	//	meh. idk. maybe will fine wo ths 	
	//	meh. idk. maybe will fine wo ths 		float upX = -yawSin * pitchSin;
	//	meh. idk. maybe will fine wo ths 		float upY = pitchCos;
	//	meh. idk. maybe will fine wo ths 		float upZ = yawCos * pitchSin;
	//	meh. idk. maybe will fine wo ths 	
	//	meh. idk. maybe will fine wo ths 		m_fTargetX -= (rightX * dx + upX * dy) * panSpeed;
	//	meh. idk. maybe will fine wo ths 		m_fTargetY -= (rightY * dx + upY * dy) * panSpeed;
	//	meh. idk. maybe will fine wo ths 		m_fTargetZ -= (rightZ * dx + upZ * dy) * panSpeed;
	//	meh. idk. maybe will fine wo ths 	}

	// --- 3. Camera movement (WASD + Shift) ---
	CKeyboard* pKb = CKeyboard::GetInstance();
	if (pKb != nullptr)
	{
		// Speed camera units per sec
		float fMoveSpeed = 15.0f * fDeltaTime;

		// Increase speed w Pressed SHIFT
		if (pKb->IsKeyDown(VK_SHIFT))
			fMoveSpeed *= 3.0f;

		// Calc view direction vector (forward)
		float fwdX = sinf(m_fYaw) * cosf(m_fPitch);
		float fwdY = -sinf(m_fPitch);
		float fwdZ = -cosf(m_fYaw) * cosf(m_fPitch);

		// calc "Right" perp by camera vector
		float rightX = cosf(m_fYaw);
		float rightY = 0.0f;
		float rightZ = sinf(m_fYaw);

		// cur frame offset
		float moveX = 0.0f, moveY = 0.0f, moveZ = 0.0f;

		// check move control kb.key by uppercase literal
		if (pKb->IsKeyDown('W'))
		{
			moveX += fwdX; moveY += fwdY; moveZ += fwdZ;
		}
		if (pKb->IsKeyDown('S'))
		{
			moveX -= fwdX; moveY -= fwdY; moveZ -= fwdZ;
		}
		if (pKb->IsKeyDown('D'))
		{
			moveX += rightX; moveY += rightY; moveZ += rightZ;
		}
		if (pKb->IsKeyDown('A'))
		{
			moveX -= rightX; moveY -= rightY; moveZ -= rightZ;
		}

		// Q n E - movement UP/DOWN by Y
		if (pKb->IsKeyDown('E')) moveY += 1.0f;
		if (pKb->IsKeyDown('Q')) moveY -= 1.0f;

		// Norm vector (move speed <= x1.4 limiter diag)
		float fMoveLenSq = moveX * moveX + moveY * moveY + moveZ * moveZ;
		if (fMoveLenSq > 0.0001f)
		{
			float fInvLen = 1.0f / sqrtf(fMoveLenSq);
			moveX *= fInvLen;
			moveY *= fInvLen;
			moveZ *= fInvLen;

			// move Target pos
			m_fTargetX += moveX * fMoveSpeed;
			m_fTargetY += moveY * fMoveSpeed;
			m_fTargetZ += moveZ * fMoveSpeed;
		}
	}

	UpdatePosition();
}

//-----------------------------------------------------------------------------
// Camera position | FROM
//-----------------------------------------------------------------------------
void CCamera::UpdatePosition()
{
	float horizontalDist = m_fZoom * cosf(m_fPitch);
	float verticalDist = m_fZoom * sinf(m_fPitch);

	m_fPosX = m_fTargetX - horizontalDist * sinf(m_fYaw);
	m_fPosY = m_fTargetY + verticalDist;
	m_fPosZ = m_fTargetZ + horizontalDist * cosf(m_fYaw);
}

//-----------------------------------------------------------------------------
// Target / zoom | TO / RADIUS
//-----------------------------------------------------------------------------
void CCamera::SetTarget(float fX, float fY, float fZ)
{
	m_fTargetX = fX;
	m_fTargetY = fY;
	m_fTargetZ = fZ;

	UpdatePosition();
}

void CCamera::SetZoom(float fZoom)
{
	if (fZoom < CAMERA_MIN_ZOOM) 
		fZoom = CAMERA_MIN_ZOOM;
	if (fZoom > CAMERA_MAX_ZOOM) 
		fZoom = CAMERA_MAX_ZOOM;

	m_fZoom = fZoom;
	UpdatePosition();
}

void CCamera::Zoom(float fDelta)
{
	SetZoom(m_fZoom + fDelta);
}

//-----------------------------------------------------------------------------
// Viewport
//-----------------------------------------------------------------------------
void CCamera::SetViewport(int lWidth, int lHeight)
{
	if (lWidth <= 0 || lHeight <= 0)
		return;

	m_lViewportWidth = lWidth;
	m_lViewportHeight = lHeight;
}

//		//-----------------------------------------------------------------------------
//		// Drunk mode
//		//-----------------------------------------------------------------------------
//		void CCamera::SetDrunkMode(bool bEnabled)
//		{
//			m_bDrunkMode = bEnabled;
//		
//			if (!m_bDrunkMode)
//			{
//				m_fDrunkPitch = 0.0f;
//				m_fDrunkYaw = 0.0f;
//				m_fDrunkRoll = 0.0f;
//		
//				UpdateOrbit();
//			}
//		}
//		
//		void CCamera::SetDrunkStrength(float fStrength)
//		{
//			if (fStrength < 0.0f)
//				fStrength = 0.0f;
//		
//			m_fDrunkStrength = fStrength;
//		}

//-----------------------------------------------------------------------------
// OpenGL matrices
//-----------------------------------------------------------------------------
void CCamera::ApplyView() const
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// --- Move back by zoom ---
	glTranslatef(0.0f, 0.0f, -m_fZoom);

	// --- Rotate camera (as degs converted to rads) ---
	glRotatef(m_fPitch * MATH_RAD2DEG, 1.0f, 0.0f, 0.0f);
	glRotatef(m_fYaw * MATH_RAD2DEG, 0.0f, 1.0f, 0.0f);

	// --- Translate world to Focus target ---
	glTranslatef(-m_fTargetX, -m_fTargetY, -m_fTargetZ);
}


void CCamera::ApplyProjection() const
{
	const float fAspect = m_lViewportHeight > 0 ? static_cast<float>(m_lViewportWidth) / static_cast<float>(m_lViewportHeight) : 1.0f;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	const float fFov = 60.0f;
	const float fNear = 0.1f;
	const float fFar = 1000.0f;

	const float fTop = std::tan((fFov * MATH_PI / 180.0f) * 0.5f) * fNear;
	const float fRight = fTop * fAspect;

	glFrustum(-fRight, fRight, -fTop, fTop, fNear, fFar);

	glMatrixMode(GL_MODELVIEW);
}