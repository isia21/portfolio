#pragma once

//-----------------------------------------------------------------------------
// UI Manager / Canvas (Stores elements, handles dispatching)
//-----------------------------------------------------------------------------
class CUIManager
{
public:
	CUIManager();
	~CUIManager();

public:
	void AddElement(CUIElement* pElement);
	void RemoveElement(CUIElement* pElement);
	void Clear();

	void Update();
	void Render(CRenderer* pRenderer);

	//	// --- Dispatchers for Win32 Messages (WM_MOUSEMOVE, WM_LBUTTONDOWN, WM_LBUTTONUP) ---
	//	bool ProcessMouseMove(int lMouseX, int lMouseY);
	//	bool ProcessMouseDown(int lMouseX, int lMouseY, int lButton = 0);
	//	bool ProcessMouseUp(int lMouseX, int lMouseY, int lButton = 0);
	//	bool ProcessMouseWheel(int lMouseX, int lMouseY, int zDelta);

private:
	std::vector<CUIElement*> m_vElements;
};