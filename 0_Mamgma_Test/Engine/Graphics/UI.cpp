#include "stdafx.h"
#include "Renderer.h"
#include "UI.h"
#include "../Input.h"
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

void CUIManager::Update()
{
	for (auto it = m_vElements.rbegin(); it != m_vElements.rend(); ++it)
	{
		if ((*it) != nullptr && (*it)->IsVisible() && (*it)->IsEnabled())
		{
			(*it)->Update();
		}
	}
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

//-----------------------------------------------------------------------------
// CUISmartTree
//-----------------------------------------------------------------------------
CUISmartTree::CUISmartTree(
	int lX, int lY,
	int lWidth, int lHeight,
	int lItemHeight,
	int lFontSize)
	: CUIElement(lX, lY, lWidth, lHeight)
	, m_pSelectedNode(nullptr)
	, m_pHoveredNode(nullptr)
	, m_lItemHeight(lItemHeight)
	, m_lIndentSize(16)
	, m_lFontSize(lFontSize)
	, m_dwBgColor(0x161616EE)
	, m_dwSelectedBgColor(0x1E4D78FF)
	, m_dwHoverBgColor(0x282828AA)
	, m_dwTextColor(0xEEEEEEFF)
	, m_lScrollY(0)
{}

CUISmartTree::~CUISmartTree()
{
	Clear();
}

void CUISmartTree::Clear()
{
	m_pSelectedNode = nullptr;
	m_pHoveredNode = nullptr;

	for (SUITreeNode* pRoot : m_vRoots)
	{
		delete pRoot;
	}
	m_vRoots.clear();
}

SUITreeNode* CUISmartTree::AddRoot(const char* pszText, void* pUserData)
{
	SUITreeNode* pNewRoot = new SUITreeNode(pszText, pUserData, nullptr);
	m_vRoots.push_back(pNewRoot);
	return pNewRoot;
}

SUITreeNode* CUISmartTree::AddChild(SUITreeNode* pParent, const char* pszText, void* pUserData)
{
	if (pParent == nullptr)
		return AddRoot(pszText, pUserData);

	return pParent->AddChild(pszText, pUserData);
}

void CUISmartTree::SetSelectedNode(SUITreeNode* pNode)
{
	if (m_pSelectedNode != nullptr)
		m_pSelectedNode->bSelected = false;

	m_pSelectedNode = pNode;

	if (m_pSelectedNode != nullptr)
	{
		m_pSelectedNode->bSelected = true;
		if (m_fnOnSelect != nullptr)
			m_fnOnSelect(m_pSelectedNode);
	}
}

void CUISmartTree::FlattenVisibleNodes(SUITreeNode* pNode, int lLevel, int& lCurrentY, std::vector<SFlatItem>& vOutList) const
{
	if (pNode == nullptr)
		return;

	// --- Add current node to the flat list ---
	SFlatItem item;
	item.pNode = pNode;
	item.lLevel = lLevel;
	item.lItemY = lCurrentY;
	vOutList.push_back(item);

	lCurrentY += m_lItemHeight;

	// --- If the node is expanded, recursively flatten its children ---
	if (pNode->bExpanded)
	{
		for (SUITreeNode* pChild : pNode->vChildren)
		{
			FlattenVisibleNodes(pChild, lLevel + 1, lCurrentY, vOutList);
		}
	}
}

void CUISmartTree::Update()
{
	CMouse* pMouse = CMouse::GetInstance();
	int mx = pMouse->GetX();
	int my = pMouse->GetY();

	if (!IsPointInside(mx, my))
	{
		m_pHoveredNode = nullptr;
		return;
	}

	const int absX = GetAbsoluteX();
	const int absY = GetAbsoluteY();

	std::vector<SFlatItem> vFlatNodes;
	int lCurrentY = 0;
	for (SUITreeNode* pRoot : m_vRoots)
		FlattenVisibleNodes(pRoot, 0, lCurrentY, vFlatNodes);

	// --- 1. Find Hovered elem ---
	m_pHoveredNode = nullptr;
	for (const SFlatItem& item : vFlatNodes)
	{
		int itemTopY = absY + item.lItemY - m_lScrollY;
		if (my >= itemTopY && my < (itemTopY + m_lItemHeight))
		{
			m_pHoveredNode = item.pNode;
			break;
		}
	}

	// --- 2. Process click --- 
	if (pMouse->IsButtonPressed(CMouse::Button_Left))
	{
		if (m_pHoveredNode != nullptr)
		{
			// --- Find elem for get Level and calac offsets ---
			for (const SFlatItem& item : vFlatNodes)
			{
				if (item.pNode == m_pHoveredNode)
				{
					int curX = absX + 4 + (item.lLevel * m_lIndentSize);

					// --- Click on [+/-]
					if (!item.pNode->vChildren.empty() && mx >= curX && mx < (curX + 12))
					{
						item.pNode->bExpanded = !item.pNode->bExpanded;
					}
					// ---  Click on [V/.]
					else if (mx >= curX + 12 && mx < curX + 34)
					{
						item.pNode->bObjectVisible = !item.pNode->bObjectVisible; 
						if (m_fnOnToggleVisibility)
							m_fnOnToggleVisibility(item.pNode, item.pNode->bObjectVisible);
					}
					// --- Click on Text of item
					else
					{
						SetSelectedNode(item.pNode); 
					}
					break;
				}
			}
		}
		pMouse->ConsumeButton(CMouse::Button_Left);
	}

	// --- 3. Scroll processing
	int zDelta = pMouse->GetWheelDelta();
	if (zDelta != 0)
	{
		m_lScrollY -= (zDelta / 120) * (m_lItemHeight * 2);
		if (m_lScrollY < 0) m_lScrollY = 0;
		pMouse->ConsumeWheel();
	}
}

void CUISmartTree::Render(CRenderer* pRenderer)
{
	if (!m_bVisible || pRenderer == nullptr)
		return;

	const int absX = GetAbsoluteX();
	const int absY = GetAbsoluteY();

	if ((m_dwBgColor & 0xFF) > 0)
		pRenderer->DrawRect(absX, absY, m_lWidth, m_lHeight, m_dwBgColor);

	
	std::vector<SFlatItem> vFlatNodes;
	int lCurrentY = 0;
	for (SUITreeNode* pRoot : m_vRoots)
		FlattenVisibleNodes(pRoot, 0, lCurrentY, vFlatNodes);

	
	for (const SFlatItem& item : vFlatNodes)
	{
		const int itemTopY = absY + item.lItemY - m_lScrollY;

		if (itemTopY + m_lItemHeight <= absY || itemTopY >= absY + m_lHeight)
			continue;

		SUITreeNode* pNode = item.pNode;

		if (pNode->bSelected)
			pRenderer->DrawRect(absX, itemTopY, m_lWidth, m_lItemHeight, m_dwSelectedBgColor);
		else if (pNode == m_pHoveredNode)
			pRenderer->DrawRect(absX, itemTopY, m_lWidth, m_lItemHeight, m_dwHoverBgColor);


		int curX = absX + 4 + (item.lLevel * m_lIndentSize);
		const int textBaselineY = itemTopY + (m_lItemHeight / 2) + (m_lFontSize / 3);

		// --- Minimize/Expand symbol: "-" for expanded, "+" for collapsed ---
		if (!pNode->vChildren.empty())
		{
			const char* pszExpandSymbol = pNode->bExpanded ? "-" : "+";
			pRenderer->DrawText(curX, textBaselineY, pszExpandSymbol, 0xAAAAAAFF, m_lFontSize, TEXT_ALIGN_LEFT);
		}
		curX += 12;

		// --- Visible/Hidden symbol: "[V]" for visible, "[.]" for hidden ---
		const char* pszVisSymbol = pNode->bObjectVisible ? "[V]" : "[.]";
		const unsigned int dwVisColor = pNode->bObjectVisible ? 0x00FF88FF : 0x666666FF;
		pRenderer->DrawText(curX, textBaselineY, pszVisSymbol, dwVisColor, m_lFontSize, TEXT_ALIGN_LEFT);
		curX += 22;

		// --- Node text ---
		const unsigned int dwFinalTextColor = pNode->bObjectVisible ? m_dwTextColor : 0x777777FF;
		pRenderer->DrawText(curX, textBaselineY, pNode->sText.c_str(), dwFinalTextColor, m_lFontSize, TEXT_ALIGN_LEFT);
	}
}

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

void CUIWindow::AddChild(CUIElement* pChild)
{
	if (pChild == nullptr)
		return;

	pChild->SetParent(this);
	m_vChildren.push_back(pChild);
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