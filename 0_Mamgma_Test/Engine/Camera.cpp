#include "stdafx.h"
#include "Camera.h"


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
namespace
{
	const float CAMERA_PI = 3.14159265358979323846f;

	const float CAMERA_DEFAULT_ZOOM = 10.0f;
	const float CAMERA_DEFAULT_ORBIT_SPEED = 0.35f;

	const float CAMERA_DEFAULT_DRUNK_STRENGTH = 2.0f;

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
	, m_fPosZ(CAMERA_DEFAULT_ZOOM)
	, m_fZoom(CAMERA_DEFAULT_ZOOM)
	, m_fOrbitAngle(0.0f)
	, m_fOrbitSpeed(CAMERA_DEFAULT_ORBIT_SPEED)
	, m_fDrunkTime(0.0f)
	, m_fDrunkStrength(CAMERA_DEFAULT_DRUNK_STRENGTH)
	, m_fDrunkPitch(0.0f)
	, m_fDrunkYaw(0.0f)
	, m_fDrunkRoll(0.0f)
	, m_lViewportWidth(1280)
	, m_lViewportHeight(720)
	, m_bDrunkMode(false)
{
	UpdateOrbit();
}

CCamera::~CCamera()
{}

//-----------------------------------------------------------------------------
// Update
//-----------------------------------------------------------------------------
void CCamera::Update(float fDeltaTime)
{
	if (fDeltaTime < 0.0f)
		fDeltaTime = 0.0f;

	m_fOrbitAngle += m_fOrbitSpeed * fDeltaTime;

	if (m_fOrbitAngle > CAMERA_PI * 2.0f)
		m_fOrbitAngle -= CAMERA_PI * 2.0f;

	m_fDrunkTime += fDeltaTime;

	UpdateDrunkOffset();
	UpdateOrbit();
}

//-----------------------------------------------------------------------------
// Orbit
//-----------------------------------------------------------------------------
void CCamera::UpdateOrbit()
{
	const float fHorizontalRadius = m_fZoom * std::cos(m_fDrunkPitch);
	const float fVerticalOffset = m_fZoom * std::sin(m_fDrunkPitch);

	m_fPosX = m_fTargetX + std::sin(m_fOrbitAngle + m_fDrunkYaw) * fHorizontalRadius;
	m_fPosZ = m_fTargetZ + std::cos(m_fOrbitAngle + m_fDrunkYaw) * fHorizontalRadius;
	m_fPosY = m_fTargetY + fVerticalOffset;
}

//-----------------------------------------------------------------------------
// Drunk camera
//-----------------------------------------------------------------------------
void CCamera::UpdateDrunkOffset()
{
	if (!m_bDrunkMode)
	{
		m_fDrunkPitch = 0.0f;
		m_fDrunkYaw = 0.0f;
		m_fDrunkRoll = 0.0f;

		return;
	}

	const float fTime = m_fDrunkTime;
	m_fDrunkPitch = std::sin(fTime * 1.7f) * 0.12f * m_fDrunkStrength;
	m_fDrunkYaw = std::sin(fTime * 2.3f) * 0.08f * m_fDrunkStrength;
	m_fDrunkRoll = std::sin(fTime * 3.1f) * 3.0f * m_fDrunkStrength;
}

//-----------------------------------------------------------------------------
// Target / zoom
//-----------------------------------------------------------------------------
void CCamera::SetTarget(float fX, float fY, float fZ)
{
	m_fTargetX = fX;
	m_fTargetY = fY;
	m_fTargetZ = fZ;

	UpdateOrbit();
}

void CCamera::SetZoom(float fZoom)
{
	if (fZoom < CAMERA_MIN_ZOOM)
		fZoom = CAMERA_MIN_ZOOM;

	if (fZoom > CAMERA_MAX_ZOOM)
		fZoom = CAMERA_MAX_ZOOM;

	m_fZoom = fZoom;

	UpdateOrbit();
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

//-----------------------------------------------------------------------------
// Drunk mode
//-----------------------------------------------------------------------------
void CCamera::SetDrunkMode(bool bEnabled)
{
	m_bDrunkMode = bEnabled;

	if (!m_bDrunkMode)
	{
		m_fDrunkPitch = 0.0f;
		m_fDrunkYaw = 0.0f;
		m_fDrunkRoll = 0.0f;

		UpdateOrbit();
	}
}

void CCamera::SetDrunkStrength(float fStrength)
{
	if (fStrength < 0.0f)
		fStrength = 0.0f;

	m_fDrunkStrength = fStrength;
}

//-----------------------------------------------------------------------------
// OpenGL matrices
//-----------------------------------------------------------------------------
void CCamera::ApplyView() const
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -m_fZoom);
	glRotatef(m_fDrunkPitch * 57.2957795f, 1.0f, 0.0f, 0.0f);
	glRotatef(m_fDrunkYaw * 57.2957795f, 0.0f, 1.0f, 0.0f);
	glRotatef(m_fDrunkRoll, 0.0f, 0.0f, 1.0f);
	glRotatef(-m_fOrbitAngle * 57.2957795f, 0.0f, 1.0f, 0.0f);
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

	const float fTop = std::tan((fFov * CAMERA_PI / 180.0f) * 0.5f) * fNear;
	const float fRight = fTop * fAspect;

	glFrustum(-fRight, fRight, -fTop, fTop, fNear, fFar);

	glMatrixMode(GL_MODELVIEW);
}