#include "stdafx.h"
#include "../../../Graphics.h"
#include "../../../../App/Application.h"
#include "../../../World.h"

#include "ScenePage.h"
#include "ObjectInspectorPage.h"


const int lInspWidth = 250;
const int lInspHeight = 310;
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


	// ====================================================================
	// --- СЛАЙДЕРЫ ВРАЩЕНИЯ ---
	// Общая лямбда для обновления текста в инспекторе без пересоздания UI
	auto updateTransformText = [this]() {
		CApplication* pApp = CApplication::GetInstance();
		C3DObject* pObj = pApp->GetSelectedObject();
		if (pObj && m_pTxtInspectorTransform) {
			char szTransform[128] = {};
			sprintf_s(szTransform, sizeof(szTransform), "Pos: (%.1f, %.1f, %.1f) | %s\nRot: (%.1f, %.1f, %.1f)",
				pObj->GetPosition().x, pObj->GetPosition().y, pObj->GetPosition().z,
				(pObj->GetRenderType() == C3DObject::eRT_Wireframe ? "Wireframe" : "Solid"),
				pObj->GetRotation().x, pObj->GetRotation().y, pObj->GetRotation().z);
			m_pTxtInspectorTransform->SetText(szTransform);
		}
		};

	// ROT X
	m_pTxtRotX = new CUITextBox(6, 126, 30, 22, "R.X", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
	m_pSliderRotX = new CUISlider(40, 126, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
	m_pSliderRotX->SetOnValueChanged([this, updateTransformText](float fVal) {
		CApplication* pApp = CApplication::GetInstance();
		C3DObject* pObj = pApp->GetSelectedObject();
		if (pObj) {
			Vector3 rot = pObj->GetRotation();
			pObj->SetRotation(fVal, rot.y, rot.z);
			updateTransformText();
		}
		});
	AddChild(m_pTxtRotX);
	AddChild(m_pSliderRotX);

	// ROT Y
	m_pTxtRotY = new CUITextBox(6, 152, 30, 22, "R.Y", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
	m_pSliderRotY = new CUISlider(40, 152, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
	m_pSliderRotY->SetOnValueChanged([this, updateTransformText](float fVal) {
		CApplication* pApp = CApplication::GetInstance();
		C3DObject* pObj = pApp->GetSelectedObject();
		if (pObj) {
			Vector3 rot = pObj->GetRotation();
			pObj->SetRotation(rot.x, fVal, rot.z);
			updateTransformText();
		}
		});
	AddChild(m_pTxtRotY);
	AddChild(m_pSliderRotY);

	// ROT Z
	m_pTxtRotZ = new CUITextBox(6, 178, 30, 22, "R.Z", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
	m_pSliderRotZ = new CUISlider(40, 178, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
	m_pSliderRotZ->SetOnValueChanged([this, updateTransformText](float fVal) {
		CApplication* pApp = CApplication::GetInstance();
		C3DObject* pObj = pApp->GetSelectedObject();
		if (pObj) {
			Vector3 rot = pObj->GetRotation();
			pObj->SetRotation(rot.x, rot.y, fVal);
			updateTransformText();
		}
		});
	AddChild(m_pTxtRotZ);
	AddChild(m_pSliderRotZ);
	// ====================================================================


	// --- Controls ---
	m_pBtnToggleWireframe = new CUIButton(6, 210, 238, 26, "Toggle Wireframe", [this]() { ObjectToggleWireframe(); });
	AddChild(m_pBtnToggleWireframe);
	
	m_pBtnDeleteObject = new CUIButton(6, 242, 238, 26, "Delete Entity", [this]() { ObjectDelete(); });
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

		// Прячем слайдеры
		m_pTxtRotX->SetVisible(false); m_pSliderRotX->SetVisible(false);
		m_pTxtRotY->SetVisible(false); m_pSliderRotY->SetVisible(false);
		m_pTxtRotZ->SetVisible(false); m_pSliderRotZ->SetVisible(false);
		return;
	}

	// Разблокируем кнопки и показываем данные:
	m_pBtnToggleWireframe->SetEnabled(true);
	m_pBtnDeleteObject->SetEnabled(true);

	// Показываем слайдеры (мы позволяем вращать любой объект, не только слайсер)
	m_pTxtRotX->SetVisible(true); m_pSliderRotX->SetVisible(true);
	m_pTxtRotY->SetVisible(true); m_pSliderRotY->SetVisible(true);
	m_pTxtRotZ->SetVisible(true); m_pSliderRotZ->SetVisible(true);

	// 1. Имя объекта
	m_pTxtInspectorName->SetText(pObj->GetName());

	// 2. Статистика
	char szStats[64] = {};
	sprintf_s(szStats, sizeof(szStats), "Verts: %zu | Tris: %zu",
		pObj->GetVertexCount(), pObj->GetTriangleCount());
	m_pTxtInspectorStats->SetText(szStats);

	// 3. Координаты и Трансформации
	char szTransform[128] = {};
	sprintf_s(szTransform, sizeof(szTransform), "Pos: (%.1f, %.1f, %.1f) | %s\nRot: (%.1f, %.1f, %.1f)",
		pObj->GetPosition().x, pObj->GetPosition().y, pObj->GetPosition().z,
		(pObj->GetRenderType() == C3DObject::eRT_Wireframe ? "Wireframe" : "Solid"),
		pObj->GetRotation().x, pObj->GetRotation().y, pObj->GetRotation().z);
	m_pTxtInspectorTransform->SetText(szTransform);

	// 4. Синхронизируем слайдеры с реальным вращением объекта
	m_pSliderRotX->SetValue(pObj->GetRotation().x);
	m_pSliderRotY->SetValue(pObj->GetRotation().y);
	m_pSliderRotZ->SetValue(pObj->GetRotation().z);
}
