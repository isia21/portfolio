#include "stdafx.h"
#include "../../../Graphics.h"
#include "../../../../App/Application.h"
#include "../../../World.h"

#include "ScenePage.h"
#include "ObjectInspectorPage.h"


const int lInspWidth = 250;
const int lInspHeight = 230;
//const int lInspX = CApplication::GetInstance()->GetWidth() - lInspWidth - 10;

CObjectInspectorPage::CObjectInspectorPage() :
	CUIWindow(
		CApplication::GetInstance()->GetWidth() - lInspWidth - 10, 10,
		lInspWidth, lInspHeight,
		"Object Inspector",
		0x1E1E1EF2, 0x282828FF, 0x454545FF, 1)
{}

CObjectInspectorPage::~CObjectInspectorPage() {}

//==========================

void CObjectInspectorPage::Init()
{
	// --- Text Out ---
	m_pTxtInspectorName = new CUITextBox(6, 30, 238, 22, "No Selection", 0x141414AA, 0x00FF88FF, 12, TEXT_ALIGN_LEFT);
	AddChild(m_pTxtInspectorName);

	m_pTxtInspectorStats = new CUITextBox(6, 56, 238, 22, "Verts: 0 | Tris: 0", 0x141414AA, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
	AddChild(m_pTxtInspectorStats);

	m_pTxtInspectorTransform = new CUITextBox(6, 82, 238, 40, "Pos: (0.0, 0.0, 0.0)", 0x141414AA, 0xCCCCCCFF, 11, TEXT_ALIGN_LEFT);
	AddChild(m_pTxtInspectorTransform);

	// --- Controls ---
	m_pBtnToggleWireframe = new CUIButton(6, 130, 238, 26, "Toggle Wireframe", [this]() { ObjectToggleWireframe(); });
	AddChild(m_pBtnToggleWireframe);
	
	m_pBtnDeleteObject = new CUIButton(6, 162, 238, 26, "Delete Entity", [this]() { ObjectDelete(); });
	m_pBtnDeleteObject->SetHoverColor(0x8B0000FF);
	AddChild(m_pBtnDeleteObject);
}

//====================================
// Object buttons callbacks
//====================================
void CObjectInspectorPage::ObjectToggleWireframe()
{
	CApplication* pApp = CApplication::GetInstance();
	C3DObject* pObj = pApp->GetSelectedObject();

	if (pObj != nullptr)
	{

		pObj->SetRenderType(
			(pObj->GetRenderType() == C3DObject::eRT_Wireframe)
			? C3DObject::eRT_Poligon
			: C3DObject::eRT_Wireframe
		);

		UpdateInspector();
	}
}

void CObjectInspectorPage::ObjectDelete()
{
	CApplication* pApp = CApplication::GetInstance();
	C3DObject* pObj = pApp->GetSelectedObject();
	CWorld* pWorld = pApp->GetWorld();
	CScenePage* pScenePage = pApp->GetScenePage();

	if (pObj != nullptr && pWorld != nullptr && pWorld->GetScene() != nullptr)
	{
		pWorld->GetScene()->RemoveObject(pObj);
		pApp->SetSelectedObject(nullptr);
		pScenePage->RebuildSceneTree();
		UpdateInspector();
	}
}

//===============

void CObjectInspectorPage::UpdateInspector()
{
	CApplication* pApp = CApplication::GetInstance();
	C3DObject* pObj = pApp->GetSelectedObject();

	// Если объект не выбран:
	if (pObj == nullptr)
	{
		m_pTxtInspectorName->SetText("No Selection");
		m_pTxtInspectorStats->SetText("Verts: 0 | Tris: 0");
		m_pTxtInspectorTransform->SetText("Pos: (0.0, 0.0, 0.0)\nRot: (0.0, 0.0, 0.0)");

		m_pBtnToggleWireframe->SetEnabled(false);
		m_pBtnDeleteObject->SetEnabled(false);
		return;
	}

	// Если объект выбран — разблокируем кнопки и выводим данные:
	m_pBtnToggleWireframe->SetEnabled(true);
	m_pBtnDeleteObject->SetEnabled(true);

	// 1. Имя объекта
	m_pTxtInspectorName->SetText(pObj->GetName());

	// 2. Статистика геометрии
	char szStats[64] = {};
	sprintf_s(szStats, sizeof(szStats), "Verts: %zu | Tris: %zu",
		pObj->GetVertexCount(),
		pObj->GetTriangleCount());
	m_pTxtInspectorStats->SetText(szStats);

	// 3. Координаты
	char szTransform[128] = {};
	sprintf_s(szTransform, sizeof(szTransform), "Pos: (%.1f, %.1f, %.1f) | %s",
		pObj->GetPosition().x,
		pObj->GetPosition().y,
		pObj->GetPosition().z,
		(pObj->GetRenderType() == C3DObject::eRT_Wireframe ? "Wireframe" : "Solid"));
	m_pTxtInspectorTransform->SetText(szTransform);
}
