#pragma once

//-----------------------------------------------------------------------------
// UI Separator Element (hor line-separator)
//-----------------------------------------------------------------------------
class CUISeparator : public CUIElement
{
public:
	CUISeparator(int lX, int lY, int lWidth, int lHeight = 2, unsigned int dwColor = 0x454545FF)
		: CUIElement(lX, lY, lWidth, lHeight), m_dwColor(dwColor) {}
	virtual ~CUISeparator() override = default;

public:
	virtual void Render(CRenderer* pRenderer) override {
		if (!m_bVisible || pRenderer == nullptr) return;
		pRenderer->DrawRect(GetAbsoluteX(), GetAbsoluteY(), m_lWidth, m_lHeight, m_dwColor);
	}

private:
	unsigned int m_dwColor;
};