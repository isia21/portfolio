#pragma once
#include "stdafx.h"
#include "../../Renderer.h"
#include "UIElement.h"
#include "UISmartTree.h"
#include "../../../Input.h"

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
