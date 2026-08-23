#pragma once
#include "stdafx.h"
#include "../../Renderer.h"
#include "UIElement.h"
#include "UITextBox.h"
#include "../../../Input.h"

//-----------------------------------------------------------------------------
// CUITextBox
//-----------------------------------------------------------------------------
CUITextBox::CUITextBox(
	int lX, int lY,
	int lWidth, int lHeight,
	const char* pszText,
	unsigned int dwBgColor,
	unsigned int dwTextColor,
	int lFontSize,
	ETextAlignment eAlignment)
	: CUIElement(lX, lY, lWidth, lHeight)
	, m_szText(pszText != nullptr ? pszText : "")
	, m_dwBgColor(dwBgColor)
	, m_dwTextColor(dwTextColor)
	, m_lFontSize(lFontSize)
	, m_eAlignment(eAlignment)
{}

void CUITextBox::SetText(const char* pszText)
{
	m_szText = (pszText != nullptr) ? pszText : "";
}

void CUITextBox::Render(CRenderer* pRenderer)
{
	if (!m_bVisible || pRenderer == nullptr)
		return;

	const int absX = GetAbsoluteX();
	const int absY = GetAbsoluteY();

	if ((m_dwBgColor & 0xFF) > 0)
	{
		pRenderer->DrawRect(absX, absY, m_lWidth, m_lHeight, m_dwBgColor);
	}

	if (!m_szText.empty())
	{
		int lTextX = absX;

		switch (m_eAlignment)
		{
		case TEXT_ALIGN_CENTER:
			lTextX = absX + (m_lWidth / 2);
			break;

		case TEXT_ALIGN_RIGHT:
			lTextX = absX + m_lWidth - 6;
			break;

		case TEXT_ALIGN_LEFT:
		default:
			lTextX = absX + 6;
			break;
		}

		const int lTextY = absY + (m_lHeight / 2) + (m_lFontSize / 3);

		pRenderer->DrawText(
			lTextX,
			lTextY,
			m_szText.c_str(),
			m_dwTextColor,
			m_lFontSize,
			m_eAlignment);
	}
}