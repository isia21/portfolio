#pragma once
class CRenderer;
//-----------------------------------------------------------------------------
// Button Element (TextBox + Hover/Press States + Callback)
//-----------------------------------------------------------------------------
class CUIButton : public CUITextBox
{
public:
	CUIButton(
		int lX = 0, int lY = 0,
		int lWidth = 120, int lHeight = 35,
		const char* pszText = "Button",
		const UICallbackFn& fnCallback = nullptr);

	virtual ~CUIButton() override = default;

public:
	virtual void Update() override;
	virtual void Render(CRenderer* pRenderer) override;

	//	virtual bool OnMouseMove(int lMouseX, int lMouseY) override;
	//	virtual bool OnMouseDown(int lMouseX, int lMouseY, int lButton) override;
	//	virtual bool OnMouseUp(int lMouseX, int lMouseY, int lButton) override;

public:
	void SetOnClick(UIEventCallback pfnCallback) { m_fnOnClick = pfnCallback; }
	void SetOnClick(const UICallbackFn& fnCallback) { m_fnOnClick = fnCallback; }

	void SetNormalColor(unsigned int dwColor) { m_dwNormalColor = dwColor; }
	void SetHoverColor(unsigned int dwColor) { m_dwHoverColor = dwColor; }
	void SetPressedColor(unsigned int dwColor) { m_dwPressedColor = dwColor; }

protected:
	unsigned int m_dwNormalColor;
	unsigned int m_dwHoverColor;
	unsigned int m_dwPressedColor;

	UICallbackFn m_fnOnClick;
}; 
