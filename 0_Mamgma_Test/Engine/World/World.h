#pragma once
class CCamera;
class C3DObject;
class CScene;

class CWorld
{
public:
	CWorld();
	~CWorld();

public:
	bool Init();
	void Shutdown();

	void Update(float fDeltaTime);

	CCamera* GetCamera() const;
	const std::vector<C3DObject*>& GetObjects() const;
	CScene* GetScene() const { return m_pScene; }

	bool IsCuttingGestureActive() const { return m_bIsCuttingGesture; }
	void GetCuttingLine(int& x1, int& y1, int& x2, int& y2) const {
		x1 = m_lCutStartX; y1 = m_lCutStartY; x2 = m_lCutCurX; y2 = m_lCutCurY;
	}

	C3DObject* GetHoveredObject() const { return m_pHoveredObject; }

private:
	void CreateSlicerFromScreenLine(int x1, int y1, int x2, int y2);
	Vector3 UnprojectScreenToRay(int screenX, int screenY) const;

	CCamera* m_pCamera;
	CScene* m_pScene;
	C3DObject* m_pHoveredObject;

	// Gesture Cut State
	bool m_bIsCuttingGesture;
	int m_lCutStartX, m_lCutStartY;
	int m_lCutCurX, m_lCutCurY;
};