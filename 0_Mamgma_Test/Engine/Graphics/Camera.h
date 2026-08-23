#pragma once

class CCamera
{
public:
	CCamera();
	~CCamera();

public:
	void Update(float fDeltaTime);

	void SetTarget(float fX, float fY, float fZ);
	void SetZoom(float fZoom);
	void Zoom(float fDelta);

	void SetViewport(int lWidth, int lHeight);

	void ApplyView() const;
	void ApplyProjection() const;

public:
	float GetPosX() const { return m_fPosX; }
	float GetPosY() const { return m_fPosY; }
	float GetPosZ() const { return m_fPosZ; }

	float GetTargetX() const { return m_fTargetX; }
	float GetTargetY() const { return m_fTargetY; }
	float GetTargetZ() const { return m_fTargetZ; }

	float GetZoom() const { return m_fZoom; }

private:
	void UpdatePosition();

private:
	// --- Camera look at ---
	float m_fTargetX;
	float m_fTargetY;
	float m_fTargetZ;

	// --- Camera pos ---
	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;

	float m_fZoom;

	// --- Rotation degs ---
	float m_fPitch; // Up/Down		(rel X)
	float m_fYaw;   // Left/Right	(rel Y)

	int m_lViewportWidth;
	int m_lViewportHeight;
};