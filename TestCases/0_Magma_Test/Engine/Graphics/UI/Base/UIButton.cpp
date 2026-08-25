#pragma once
#include "stdafx.h"
#include "../../Renderer.h"
#include "UIElement.h"
#include "UITextBox.h"
#include "UIButton.h"
#include "../../../Input.h"

//-----------------------------------------------------------------------------
// CUIButton
//-----------------------------------------------------------------------------
CUIButton::CUIButton(
	int lX, int lY,
	int lWidth, int lHeight,
	const char* pszText,
	const UICallbackFn& fnCallback)
	: CUITextBox(lX, lY, lWidth, lHeight, pszText, 0x404040FF, 0xFFFFFFFF, 14, TEXT_ALIGN_CENTER)
	, m_dwNormalColor(0x404040FF)
	, m_dwHoverColor(0x606060FF)
	, m_dwPressedColor(0x202020FF)
	, m_fnOnClick(fnCallback)
{}

void CUIButton::Update()
{
	CMouse* pMouse = CMouse::GetInstance();
	m_bHovered = IsPointInside(pMouse->GetX(), pMouse->GetY());

	if (m_bPressed)
	{
		// --- If phys btn unpressed ---
		if (!pMouse->IsButtonDownRaw(CMouse::Button_Left))
		{
			m_bPressed = false;

			// register click 
			if (m_bHovered && m_fnOnClick)
				m_fnOnClick();

			pMouse->ConsumeButton(CMouse::Button_Left);
		}
		// --- Until we press mouse btn over UI Button - reset m.Button.state (world must ignore this pressed state)
		else
			pMouse->ConsumeButton(CMouse::Button_Left);
	}
	else
	{
		// --- Catch moment of start press btn ---
		if (m_bHovered && pMouse->IsButtonPressed(CMouse::Button_Left))
		{
			m_bPressed = true;
			pMouse->ConsumeButton(CMouse::Button_Left);
		}
	}
}
void CUIButton::Render(CRenderer* pRenderer)
{
	if (!m_bVisible || pRenderer == nullptr)
		return;

	if (!m_bEnabled)
	{
		m_dwBgColor = 0x25252580;
	}
	else if (m_bPressed)
	{
		m_dwBgColor = m_dwPressedColor;
	}
	else if (m_bHovered)
	{
		m_dwBgColor = m_dwHoverColor;
	}
	else
	{
		m_dwBgColor = m_dwNormalColor;
	}

	CUITextBox::Render(pRenderer);
}