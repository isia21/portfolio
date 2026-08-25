#include "stdafx.h"
#include "../Renderer.h"
#include "../../Input.h"
#include "Base/UIElement.h"

#include "UIManager.h"

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