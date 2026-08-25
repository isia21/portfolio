#pragma once
#include "stdafx.h"
#include "../../Renderer.h"
#include "UIElement.h"
#include "UITextBox.h"
#include "UIButton.h"
#include "UIWindow.h"
#include "../../../Input.h"

//-----------------------------------------------------------------------------
// CUIWindow
//-----------------------------------------------------------------------------
CUIWindow::CUIWindow(
	int lX, int lY,
	int lWidth, int lHeight,
	const char* pszTitle,
	unsigned int dwBgColor,
	unsigned int dwHeaderColor,
	unsigned int dwBorderColor,
	int lBorderSize)
	: CUIElement(lX, lY, lWidth, lHeight)
	, m_szTitle(pszTitle != nullptr ? pszTitle : "Window")
	, m_lHeaderHeight(24)
	, m_dwBgColor(dwBgColor)
	, m_dwHeaderColor(dwHeaderColor)
	, m_dwBorderColor(dwBorderColor)
	, m_lBorderSize(lBorderSize)
	, m_bCollapsed(false)
	, m_bDragging(false)
	, m_lDragOffsetX(0)
	, m_lDragOffsetY(0)
{
	const int lBtnSize = 18;
	const int lBtnX = m_lWidth - lBtnSize - 3;
	const int lBtnY = 3;

	m_pBtnCollapse = new CUIButton(
		lBtnX, lBtnY,
		lBtnSize, lBtnSize,
		"-",
		[this]() {
			SetCollapsed(!m_bCollapsed);
		});

	m_pBtnCollapse->SetParent(this);
	m_pBtnCollapse->SetFontSize(12);
	m_pBtnCollapse->SetNormalColor(0x00000000);
	m_pBtnCollapse->SetHoverColor(0x555555AA);
	m_pBtnCollapse->SetPressedColor(0x2060A0FF);
}

CUIWindow::~CUIWindow()
{
	if (m_pBtnCollapse != nullptr)
	{
		delete m_pBtnCollapse;
		m_pBtnCollapse = nullptr;
	}

	ClearChildren();
}

void CUIWindow::SetTitle(const char* pszTitle)
{
	m_szTitle = (pszTitle != nullptr) ? pszTitle : "";
}

void CUIWindow::SetCollapsed(bool bCollapsed)
{
	m_bCollapsed = bCollapsed;
	if (m_pBtnCollapse != nullptr)
	{
		m_pBtnCollapse->SetText(m_bCollapsed ? "+" : "-");
	}
}

bool CUIWindow::IsPointInsideHeader(int lX, int lY) const
{
	const int absX = GetAbsoluteX();
	const int absY = GetAbsoluteY();

	return (lX >= absX && lX <= (absX + m_lWidth) &&
		lY >= absY && lY <= (absY + m_lHeaderHeight));
}

int CUIWindow::GetContentBottomY() const
{
	int maxY = m_lHeaderHeight;
	for (CUIElement* pChild : m_vChildren)
	{
		if (pChild != nullptr)
		{
			int bottom = pChild->GetY() + pChild->GetHeight();
			if (bottom > maxY)
				maxY = bottom;
		}
	}
	return maxY;
}

void CUIWindow::AddChild(CUIElement* pChild)
{
	if (pChild == nullptr)
		return;

	// auto Y pos
	if (pChild->GetY() == UI_AUTO || pChild->GetY() <= -999900)
	{
		int nextY = GetContentBottomY() + m_lPaddingY;
		pChild->SetPosition(pChild->GetX(), nextY);
	}

	// auto X pos
	if (pChild->GetX() == UI_AUTO || pChild->GetX() <= -999900)
	{
		pChild->SetPosition(m_lPaddingX, pChild->GetY());
	}

	// auto Width (window width)
	if (pChild->GetWidth() == UI_AUTO || pChild->GetWidth() <= -999900)
	{
		const int autoWidth = m_lWidth - (m_lPaddingX * 2);
		pChild->SetSize(autoWidth, pChild->GetHeight());
	}

	pChild->SetParent(this);
	m_vChildren.push_back(pChild);
}

void CUIWindow::AddChild(const std::vector<CUIElement*>& vRow)
{
	if (vRow.empty())
		return;

	std::vector<CUIElement*> validElements;
	for (CUIElement* pElem : vRow)
	{
		if (pElem != nullptr)
			validElements.push_back(pElem);
	}

	if (validElements.empty())
		return;

	// 1. Calc common row Y
	const int rowY = GetContentBottomY() + m_lPaddingY;

	// 2. Calc widths (Flexbox logic)
	const int totalAvailableWidth = m_lWidth - (m_lPaddingX * 2);
	const int totalGapsWidth = static_cast<int>(validElements.size() - 1) * m_lPaddingX;

	int fixedWidthSum = 0;
	int autoWidthCount = 0;

	for (CUIElement* pElem : validElements)
	{
		if (pElem->GetWidth() == UI_AUTO || pElem->GetWidth() <= -999900)
			autoWidthCount++;
		else
			fixedWidthSum += pElem->GetWidth();
	}

	
	const int remainingWidth = totalAvailableWidth - fixedWidthSum - totalGapsWidth;
	const int autoItemWidth = (autoWidthCount > 0) ? /*std::max*/Utils::MaxInt(10, remainingWidth / autoWidthCount) : 0;

	// 3. Set pos for elems in row l->r
	int curX = m_lPaddingX;
	for (CUIElement* pElem : validElements)
	{
		if (pElem->GetWidth() == UI_AUTO || pElem->GetWidth() <= -999900)
		{
			pElem->SetSize(autoItemWidth, pElem->GetHeight());
		}

		int elemX = (pElem->GetX() == UI_AUTO || pElem->GetX() <= -999900) ? curX : pElem->GetX();
		int elemY = (pElem->GetY() == UI_AUTO || pElem->GetY() <= -999900) ? rowY : pElem->GetY();

		pElem->SetPosition(elemX, elemY);
		pElem->SetParent(this);
		m_vChildren.push_back(pElem);

		curX = elemX + pElem->GetWidth() + m_lPaddingX;
	}
}

void CUIWindow::AddChild(std::initializer_list<CUIElement*> list)
{
	AddChild(std::vector<CUIElement*>(list));
}

void CUIWindow::RemoveChild(CUIElement* pChild)
{
	if (pChild == nullptr)
		return;

	for (auto it = m_vChildren.begin(); it != m_vChildren.end(); ++it)
	{
		if (*it == pChild)
		{
			pChild->SetParent(nullptr);
			delete* it;
			m_vChildren.erase(it);
			return;
		}
	}
}

void CUIWindow::ClearChildren()
{
	for (CUIElement* pChild : m_vChildren)
		delete pChild;

	m_vChildren.clear();
}

void CUIWindow::Update()
{
	CMouse* pMouse = CMouse::GetInstance();
	int mx = pMouse->GetX();
	int my = pMouse->GetY();

	// --- 1. Drag n Drop logic
	if (m_bDragging)
	{
		// --- Check RAW State of btn, cause we dont care, will State Consumed (WE own State) ---
		if (pMouse->IsButtonDownRaw(CMouse::Button_Left))
		{
			m_lX = mx - m_lDragOffsetX;
			m_lY = my - m_lDragOffsetY;
			pMouse->ConsumeButton(CMouse::Button_Left);
		}
		else
			m_bDragging = false;
	}

	// --- 2. Process child elems ---
	if (!m_bCollapsed)
	{
		for (auto it = m_vChildren.rbegin(); it != m_vChildren.rend(); ++it)
		{
			if ((*it) != nullptr && (*it)->IsVisible() && (*it)->IsEnabled())
				(*it)->Update();
		}
	}

	if (m_pBtnCollapse)
		m_pBtnCollapse->Update();

	// --- 3. Window rect ---
	bool bInsideHeader = IsPointInsideHeader(mx, my);
	bool bInsideBody = m_bCollapsed ? bInsideHeader : IsPointInside(mx, my);
	m_bHovered = bInsideBody;

	// Drag n Drop start 
	if (!m_bDragging && bInsideHeader && pMouse->IsButtonPressed(CMouse::Button_Left))
	{
		m_bDragging = true;
		m_lDragOffsetX = mx - m_lX;
		m_lDragOffsetY = my - m_lY;
		pMouse->ConsumeButton(CMouse::Button_Left);
	}

	// --- 4. Reset any mouse states if Cursor Point inside Window Rect
	if (bInsideBody)
	{
		if (pMouse->IsButtonPressed(CMouse::Button_Left))
			pMouse->ConsumeButton(CMouse::Button_Left);
		if (pMouse->IsButtonPressed(CMouse::Button_Right))
			pMouse->ConsumeButton(CMouse::Button_Right);
		if (pMouse->GetWheelDelta() != 0)
			pMouse->ConsumeWheel();
	}
}

void CUIWindow::Render(CRenderer* pRenderer)
{
	if (!m_bVisible || pRenderer == nullptr)
		return;

	const int absX = GetAbsoluteX();
	const int absY = GetAbsoluteY();
	const int effectiveHeight = m_bCollapsed ? m_lHeaderHeight : m_lHeight;

	// --- Draw the window background (only if not collapsed and has a visible background color) ---
	if (!m_bCollapsed && (m_dwBgColor & 0xFF) > 0)
		pRenderer->DrawRect(absX, absY, m_lWidth, effectiveHeight, m_dwBgColor);

	// --- Draw the header background (only if has a visible header color) ---
	if ((m_dwHeaderColor & 0xFF) > 0)
		pRenderer->DrawRect(absX, absY, m_lWidth, m_lHeaderHeight, m_dwHeaderColor);

	// --- Draw the window title text (if not empty) ---
	if (!m_szTitle.empty())
	{
		const int lTitleY = absY + (m_lHeaderHeight / 2) + 4;
		pRenderer->DrawText(absX + 8, lTitleY, m_szTitle.c_str(), 0xFFFFFFFF, 12, TEXT_ALIGN_LEFT);
	}

	// --- Draw the collapse/expand button ---
	if (m_pBtnCollapse != nullptr)
		m_pBtnCollapse->Render(pRenderer);

	// --- Draw child elements (only if the window is NOT collapsed) ---
	if (!m_bCollapsed)
	{
		for (CUIElement* pChild : m_vChildren)
		{
			if (pChild != nullptr && pChild->IsVisible())
			{
				pChild->Render(pRenderer);
			}
		}

		// --- Draw a separator line between the header and the content (only if border size is greater than 0 and border color is visible) ---
		if (m_lBorderSize > 0 && (m_dwBorderColor & 0xFF) > 0)
			pRenderer->DrawRect(absX, absY + m_lHeaderHeight, m_lWidth, m_lBorderSize, m_dwBorderColor);
	}

	// --- Draw the border around the window (only if border size is greater than 0 and border color is visible) ---
	if (m_lBorderSize > 0 && (m_dwBorderColor & 0xFF) > 0)
	{
		pRenderer->DrawRect(absX, absY, m_lWidth, m_lBorderSize, m_dwBorderColor);
		pRenderer->DrawRect(absX, absY + effectiveHeight - m_lBorderSize, m_lWidth, m_lBorderSize, m_dwBorderColor);
		pRenderer->DrawRect(absX, absY, m_lBorderSize, effectiveHeight, m_dwBorderColor);
		pRenderer->DrawRect(absX + m_lWidth - m_lBorderSize, absY, m_lBorderSize, effectiveHeight, m_dwBorderColor);
	}
}