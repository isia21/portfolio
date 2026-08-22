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

private:
	CCamera* m_pCamera;
	CScene* m_pScene;
};