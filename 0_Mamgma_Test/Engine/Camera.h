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

	void SetDrunkMode(bool bEnabled);
	void SetDrunkStrength(float fStrength);

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
	void UpdateOrbit();
	void UpdateDrunkOffset();

private:
	float m_fTargetX;
	float m_fTargetY;
	float m_fTargetZ;

	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;

	float m_fZoom;

	float m_fOrbitAngle;
	float m_fOrbitSpeed;

	float m_fDrunkTime;
	float m_fDrunkStrength;

	float m_fDrunkPitch;
	float m_fDrunkYaw;
	float m_fDrunkRoll;

	int m_lViewportWidth;
	int m_lViewportHeight;

	bool m_bDrunkMode;
};