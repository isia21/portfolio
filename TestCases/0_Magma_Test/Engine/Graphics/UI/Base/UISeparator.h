#pragma once
class CRenderer;

//-----------------------------------------------------------------------------
// UI Separator Element (hor line-separator)
//-----------------------------------------------------------------------------
class CUISeparator : public CUIElement
{
public:
	CUISeparator(int lX, int lY, int lWidth, int lHeight = 2, unsigned int dwColor = 0x454545FF);
	virtual ~CUISeparator() override = default;

public:
	virtual void Render(CRenderer* pRenderer) override;

private:
	unsigned int m_dwColor;
};