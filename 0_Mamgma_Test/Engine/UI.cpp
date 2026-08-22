#include "stdafx.h"
#include "Renderer.h"
#include "UI.h"

//-----------------------------------------------------------------------------
// CUIManager
//-----------------------------------------------------------------------------
CUIManager::CUIManager()
{}

CUIManager::~CUIManager()
{
	Clear();
}

void CUIManager::AddElement(CUIElement* pElement)
{
	if (pElement == nullptr)
		return;

	m_vElements.push_back(pElement);
}

void CUIManager::RemoveElement(CUIElement* pElement)
{
	if (pElement == nullptr)
		return;

	for (auto it = m_vElements.begin(); it != m_vElements.end(); ++it)
	{
		if (*it == pElement)
		{
			delete* it;
			m_vElements.erase(it);
			return;
		}
	}
}

void CUIManager::Clear()
{
	for (CUIElement* pElement : m_vElements)
	{
		delete pElement;
	}
	m_vElements.clear();
}

void CUIManager::Render(CRenderer* pRenderer)
{
	if (pRenderer == nullptr)
		return;

	for (CUIElement* pElement : m_vElements)
	{
		if (pElement != nullptr && pElement->IsVisible())
		{
			pElement->Render(pRenderer);
		}
	}
}

bool CUIManager::ProcessMouseMove(int lMouseX, int lMouseY)
{
	bool bHandled = false;

	for (CUIElement* pElement : m_vElements)
	{
		if (pElement != nullptr && pElement->IsVisible() && pElement->IsEnabled())
		{
			if (pElement->OnMouseMove(lMouseX, lMouseY))
			{
				bHandled = true;
			}
		}
	}

	return bHandled;
}

bool CUIManager::ProcessMouseDown(int lMouseX, int lMouseY, int lButton)
{
	for (auto it = m_vElements.rbegin(); it != m_vElements.rend(); ++it)
	{
		CUIElement* pElement = *it;
		if (pElement != nullptr && pElement->IsVisible() && pElement->IsEnabled())
		{
			if (pElement->OnMouseDown(lMouseX, lMouseY, lButton))
				return true; 
		}
	}

	return false;
}

bool CUIManager::ProcessMouseUp(int lMouseX, int lMouseY, int lButton)
{
	bool bHandled = false;

	for (auto it = m_vElements.rbegin(); it != m_vElements.rend(); ++it)
	{
		CUIElement* pElement = *it;
		if (pElement != nullptr && pElement->IsVisible() && pElement->IsEnabled())
		{
			if (pElement->OnMouseUp(lMouseX, lMouseY, lButton))
			{
				bHandled = true;
			}
		}
	}

	return bHandled;
}

//-----------------------------------------------------------------------------
// CUIElement
//-----------------------------------------------------------------------------
CUIElement::CUIElement(int lX, int lY, int lWidth, int lHeight)
	: m_lX(lX)
	, m_lY(lY)
	, m_lWidth(lWidth)
	, m_lHeight(lHeight)
	, m_bVisible(true)
	, m_bEnabled(true)
	, m_bHovered(false)
	, m_bPressed(false)
{}

bool CUIElement::IsPointInside(int lX, int lY) const
{
	return (lX >= m_lX && lX <= (m_lX + m_lWidth) &&
		lY >= m_lY && lY <= (m_lY + m_lHeight));
}

bool CUIElement::OnMouseMove(int lMouseX, int lMouseY)
{
	if (!m_bVisible || !m_bEnabled)
	{
		m_bHovered = false;
		m_bPressed = false;
		return false;
	}

	m_bHovered = IsPointInside(lMouseX, lMouseY);

	if (!m_bHovered)
		m_bPressed = false;

	return m_bHovered;
}

bool CUIElement::OnMouseDown(int lMouseX, int lMouseY, int lButton)
{
	if (!m_bVisible || !m_bEnabled)
		return false;

	if (lButton == 0 && IsPointInside(lMouseX, lMouseY)) // 0 = Left Mouse Button
	{
		m_bPressed = true;
		return true;
	}

	return false;
}

bool CUIElement::OnMouseUp(int lMouseX, int lMouseY, int lButton)
{
	if (!m_bVisible || !m_bEnabled)
		return false;

	const bool bWasPressed = m_bPressed;
	m_bPressed = false;

	if (lButton == 0 && bWasPressed && IsPointInside(lMouseX, lMouseY))
	{
		return true;
	}

	return false;
}

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

	if ((m_dwBgColor & 0xFF) > 0)
	{
		pRenderer->DrawRect(m_lX, m_lY, m_lWidth, m_lHeight, m_dwBgColor);
	}

	if (!m_szText.empty())
	{
		int lTextX = m_lX;

		switch (m_eAlignment)
		{
		case TEXT_ALIGN_CENTER:
			lTextX = m_lX + (m_lWidth / 2);
			break;

		case TEXT_ALIGN_RIGHT:
			lTextX = m_lX + m_lWidth - 6; 
			break;

		case TEXT_ALIGN_LEFT:
		default:
			lTextX = m_lX + 6; 
			break;
		}

		const int lTextY = m_lY + (m_lHeight / 2) + (m_lFontSize / 3);

		pRenderer->DrawText(
			lTextX,
			lTextY,
			m_szText.c_str(),
			m_dwTextColor,
			m_lFontSize,
			m_eAlignment);
	}
}

//-----------------------------------------------------------------------------
// CUIButton
//-----------------------------------------------------------------------------
CUIButton::CUIButton(
	int lX, int lY,
	int lWidth, int lHeight,
	const char* pszText,
	UIEventCallback pfnCallback)
	: CUITextBox(lX, lY, lWidth, lHeight, pszText, 0x404040FF, 0xFFFFFFFF, 14, TEXT_ALIGN_CENTER)
	, m_dwNormalColor(0x404040FF)
	, m_dwHoverColor(0x606060FF)
	, m_dwPressedColor(0x202020FF)
	, m_fnOnClick(pfnCallback)
{}

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

bool CUIButton::OnMouseMove(int lMouseX, int lMouseY)
{
	return CUIElement::OnMouseMove(lMouseX, lMouseY);
}

bool CUIButton::OnMouseDown(int lMouseX, int lMouseY, int lButton)
{
	return CUIElement::OnMouseDown(lMouseX, lMouseY, lButton);
}

bool CUIButton::OnMouseUp(int lMouseX, int lMouseY, int lButton)
{
	if (CUIElement::OnMouseUp(lMouseX, lMouseY, lButton))
	{
		if (m_fnOnClick != nullptr)
		{
			m_fnOnClick();
		}
		return true;
	}

	return false;
}