#pragma once
class CRenderer;
#include "defs.h"
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
