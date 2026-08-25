#pragma once
#include "stdafx.h"
#include "../../Renderer.h"
#include "UIElement.h"
#include "UISeparator.h"

//-----------------------------------------------------------------------------
// CUIButton
//-----------------------------------------------------------------------------
CUISeparator::CUISeparator(int lX, int lY, int lWidth, int lHeight, unsigned int dwColor)
	: CUIElement(lX, lY, lWidth, lHeight), m_dwColor(dwColor) 
{}

void CUISeparator::Render(CRenderer* pRenderer) {
	if (!m_bVisible || pRenderer == nullptr) 
		return;
	pRenderer->DrawRect(GetAbsoluteX(), GetAbsoluteY(), m_lWidth, m_lHeight, m_dwColor);
}