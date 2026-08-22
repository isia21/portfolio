#pragma once
class CCamera;
class C3DObject;

class CWorld
{
public:
	CWorld();
	~CWorld();

public:
	bool Init();
	void Shutdown();

	void Update(float fDeltaTime);

	void AddObject(C3DObject* pObject);
	void RemoveObject(C3DObject* pObject);

	C3DObject* CreateObject(int lType);

	CCamera* GetCamera() const;

	const std::vector<C3DObject*>& GetObjects() const;

private:
	CCamera* m_pCamera;
	std::vector<C3DObject*> m_vObjects;
};