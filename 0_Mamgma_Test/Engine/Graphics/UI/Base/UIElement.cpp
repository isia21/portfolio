#pragma once
#include "stdafx.h"
#include "../../Renderer.h"
#include "UIElement.h"

//-----------------------------------------------------------------------------
// CUIElement
//-----------------------------------------------------------------------------
CUIElement::CUIElement(int lX, int lY, int lWidth, int lHeight)
	: m_lX(lX)
	, m_lY(lY)
	, m_lWidth(lWidth)
	, m_lHeight(lHeight)
	, m_pParent(nullptr)
	, m_bVisible(true)
	, m_bEnabled(true)
	, m_bHovered(false)
	, m_bPressed(false)
{}

bool CUIElement::IsPointInside(int lX, int lY) const
{
	const int absX = GetAbsoluteX();
	const int absY = GetAbsoluteY();

	return (lX >= absX && lX <= (absX + m_lWidth) &&
		lY >= absY && lY <= (absY + m_lHeight));
}
