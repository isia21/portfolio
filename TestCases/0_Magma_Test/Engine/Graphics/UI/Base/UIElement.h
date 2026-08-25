#pragma once
class CRenderer;

// Fixed value for auto padding / rows sys
const int UI_AUTO = -999999;

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
	virtual void Update() {};
	virtual void Render(CRenderer* pRenderer) = 0;

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

	// --- Parent-Child Relationship ---
	void SetParent(CUIElement* pParent) { m_pParent = pParent; }
	CUIElement* GetParent() const { return m_pParent; }

	// --- Absolute Positioning (relative to parent hierarchy) ---
	int GetAbsoluteX() const { return m_lX + (m_pParent != nullptr ? m_pParent->GetAbsoluteX() : 0); }
	int GetAbsoluteY() const { return m_lY + (m_pParent != nullptr ? m_pParent->GetAbsoluteY() : 0); }


protected:
	int m_lX;
	int m_lY;
	int m_lWidth;
	int m_lHeight;

	CUIElement* m_pParent;

	bool m_bVisible;
	bool m_bEnabled;
	bool m_bHovered;
	bool m_bPressed;
};