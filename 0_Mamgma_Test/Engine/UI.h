#pragma once
//-----------------------------------------------------------------------------
// UI Callback definitions
//-----------------------------------------------------------------------------
typedef void(*UIEventCallback)();
typedef std::function<void()> UICallbackFn;

//-----------------------------------------------------------------------------
// Base UI Element
//-----------------------------------------------------------------------------
class CUIElement
{
public:
	CUIElement(int lX = 0, int lY = 0, int lWidth = 0, int lHeight = 0);
	virtual ~CUIElement() = default;

public:
	virtual void Render(CRenderer* pRenderer) = 0;

	// Mouse event handlers (return true if event was consumed by element)
	virtual bool OnMouseMove(int lMouseX, int lMouseY);
	virtual bool OnMouseDown(int lMouseX, int lMouseY, int lButton);
	virtual bool OnMouseUp(int lMouseX, int lMouseY, int lButton);

public:
	void SetPosition(int lX, int lY) { m_lX = lX; m_lY = lY; }
	void SetSize(int lWidth, int lHeight) { m_lWidth = lWidth; m_lHeight = lHeight; }

	int GetX() const { return m_lX; }
	int GetY() const { return m_lY; }
	int GetWidth() const { return m_lWidth; }
	int GetHeight() const { return m_lHeight; }

	void SetVisible(bool bVisible) { m_bVisible = bVisible; }
	bool IsVisible() const { return m_bVisible; }

	void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }
	bool IsEnabled() const { return m_bEnabled; }

	bool IsHovered() const { return m_bHovered; }
	bool IsPressed() const { return m_bPressed; }

	bool IsPointInside(int lX, int lY) const;

protected:
	int m_lX;
	int m_lY;
	int m_lWidth;
	int m_lHeight;

	bool m_bVisible;
	bool m_bEnabled;
	bool m_bHovered;
	bool m_bPressed;
};


//-----------------------------------------------------------------------------
// TextBox Element (Background Rect + Text Label)
//-----------------------------------------------------------------------------
class CUITextBox : public CUIElement
{
public:
	CUITextBox(
		int lX = 0, int lY = 0,
		int lWidth = 100, int lHeight = 30,
		const char* pszText = "",
		unsigned int dwBgColor = 0x303030FF,
		unsigned int dwTextColor = 0xFFFFFFFF,
		int lFontSize = 14,
		ETextAlignment eAlignment = TEXT_ALIGN_CENTER);

	virtual ~CUITextBox() override = default;

public:
	virtual void Render(CRenderer* pRenderer) override;

public:
	void SetText(const char* pszText);
	const char* GetText() const { return m_szText.c_str(); }

	void SetBgColor(unsigned int dwColor) { m_dwBgColor = dwColor; }
	unsigned int GetBgColor() const { return m_dwBgColor; }

	void SetTextColor(unsigned int dwColor) { m_dwTextColor = dwColor; }
	unsigned int GetTextColor() const { return m_dwTextColor; }

	void SetFontSize(int lSize) { m_lFontSize = lSize; }
	int GetFontSize() const { return m_lFontSize; }

	void SetTextAlignment(ETextAlignment eAlignment) { m_eAlignment = eAlignment; }
	ETextAlignment GetTextAlignment() const { return m_eAlignment; }

protected:
	std::string m_szText;

	unsigned int m_dwBgColor;
	unsigned int m_dwTextColor;

	int m_lFontSize;
	ETextAlignment m_eAlignment;
};


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
		UIEventCallback pfnCallback = nullptr);

	virtual ~CUIButton() override = default;

public:
	virtual void Render(CRenderer* pRenderer) override;

	virtual bool OnMouseMove(int lMouseX, int lMouseY) override;
	virtual bool OnMouseDown(int lMouseX, int lMouseY, int lButton) override;
	virtual bool OnMouseUp(int lMouseX, int lMouseY, int lButton) override;

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

	void Render(CRenderer* pRenderer);

	// --- Dispatchers for Win32 Messages (WM_MOUSEMOVE, WM_LBUTTONDOWN, WM_LBUTTONUP) ---
	bool ProcessMouseMove(int lMouseX, int lMouseY);
	bool ProcessMouseDown(int lMouseX, int lMouseY, int lButton = 0);
	bool ProcessMouseUp(int lMouseX, int lMouseY, int lButton = 0);

private:
	std::vector<CUIElement*> m_vElements;
};