#include "stdafx.h"
#include "../../../Graphics.h"
#include "../../../../App/Application.h"
#include "../../../World.h"

#include "ScenePage.h"
#include "ObjectInspectorPage.h"


const int lInspWidth = 250;
const int lInspHeight = 350;
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

	AddChild(new CUISeparator(6, UI_AUTO, 238, 2, 0x454545FF));

	// ====================================================================
	// --- ROT n POS SLIDERS ---
	// common lamda for update text in Inspector
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
	{
		// ROT X
		m_pTxtRotX = new CUITextBox(6, UI_AUTO, 30, 22, "R.X", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
		m_pSliderRotX = new CUISlider(40, UI_AUTO, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
		m_pSliderRotX->SetOnValueChanged([this, updateTransformText](float fVal) {
			CApplication* pApp = CApplication::GetInstance();
			C3DObject* pObj = pApp->GetSelectedObject();
			if (pObj) {
				Vector3 rot = pObj->GetRotation();
				pObj->SetRotation(fVal, rot.y, rot.z);
				updateTransformText();
			}
			});
		AddChild({ m_pTxtRotX,m_pSliderRotX });

		// ROT Y
		m_pTxtRotY = new CUITextBox(6, UI_AUTO, 30, 22, "R.Y", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
		m_pSliderRotY = new CUISlider(40, UI_AUTO, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
		m_pSliderRotY->SetOnValueChanged([this, updateTransformText](float fVal) {
			CApplication* pApp = CApplication::GetInstance();
			C3DObject* pObj = pApp->GetSelectedObject();
			if (pObj) {
				Vector3 rot = pObj->GetRotation();
				pObj->SetRotation(rot.x, fVal, rot.z);
				updateTransformText();
			}
			});
		AddChild({ m_pTxtRotY,m_pSliderRotY });

		// ROT Z
		m_pTxtRotZ = new CUITextBox(6, UI_AUTO, 30, 22, "R.Z", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
		m_pSliderRotZ = new CUISlider(40, UI_AUTO, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
		m_pSliderRotZ->SetOnValueChanged([this, updateTransformText](float fVal) {
			CApplication* pApp = CApplication::GetInstance();
			C3DObject* pObj = pApp->GetSelectedObject();
			if (pObj) {
				Vector3 rot = pObj->GetRotation();
				pObj->SetRotation(rot.x, rot.y, fVal);
				updateTransformText();
			}
			});
		AddChild({ m_pTxtRotZ,m_pSliderRotZ });
	}

	AddChild(new CUISeparator(6, UI_AUTO, 238, 2, 0x454545FF));

	{
		// POS X
		m_pTxtPosX = new CUITextBox(6, UI_AUTO, 30, 22, "P.X", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
		m_pSliderPosX = new CUISlider(40, UI_AUTO, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
		m_pSliderPosX->SetOnValueChanged([this, updateTransformText](float fVal) {
			CApplication* pApp = CApplication::GetInstance();
			C3DObject* pObj = pApp->GetSelectedObject();
			if (pObj) {
				Vector3 pos = pObj->GetPosition();
				pObj->SetPosition(fVal, pos.y, pos.z);
				updateTransformText();
			}
			});
		AddChild({ m_pTxtPosX,m_pSliderPosX });

		// POS Y
		m_pTxtPosY = new CUITextBox(6, UI_AUTO, 30, 22, "P.Y", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
		m_pSliderPosY = new CUISlider(40, UI_AUTO, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
		m_pSliderPosY->SetOnValueChanged([this, updateTransformText](float fVal) {
			CApplication* pApp = CApplication::GetInstance();
			C3DObject* pObj = pApp->GetSelectedObject();
			if (pObj) {
				Vector3 pos = pObj->GetPosition();
				pObj->SetPosition(pos.x, fVal, pos.z);
				updateTransformText();
			}
			});
		AddChild({ m_pTxtPosY,m_pSliderPosY });

		// POS Z
		m_pTxtPosZ = new CUITextBox(6, UI_AUTO, 30, 22, "P.Z", 0, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
		m_pSliderPosZ = new CUISlider(40, UI_AUTO, 204, 22, -180.0f, 180.0f, 0.0f, 5.0f);
		m_pSliderPosZ->SetOnValueChanged([this, updateTransformText](float fVal) {
			CApplication* pApp = CApplication::GetInstance();
			C3DObject* pObj = pApp->GetSelectedObject();
			if (pObj) {
				Vector3 pos = pObj->GetPosition();
				pObj->SetPosition(pos.x, pos.y, fVal);
				updateTransformText();
			}
			});
		AddChild({ m_pTxtPosZ,m_pSliderPosZ });
	}
	// ====================================================================

	// note 210
	// 210 - 178 = 32 px offset by Y btw last slider n Controls section

	// --- Controls ---
	m_pBtnToggleWireframe = new CUIButton(6, UI_AUTO, 238, 26, "Toggle Wireframe", [this]() { ObjectToggleWireframe(); });
	AddChild(m_pBtnToggleWireframe);
	
	m_pBtnDeleteObject = new CUIButton(6, UI_AUTO, 238, 26, "Delete Entity", [this]() { ObjectDelete(); });
	m_pBtnDeleteObject->SetHoverColor(0x8B0000FF);
	AddChild(m_pBtnDeleteObject);

	// note 210
	// ====================================================================
	// --- Separator n OBJ loader---
	// 1. Hor Separator
	CUISeparator* pSep = new CUISeparator(6, UI_AUTO, 238, 2, 0x454545FF);
	AddChild(pSep);

	// 2. Button Create from .OBJ
	CUIButton* pBtnImportObj = new CUIButton(6, UI_AUTO, 238, 26, "Create from file (*.obj)", [this]() {
		CApplication* pApp = CApplication::GetInstance();
		CWorld* pWorld = pApp->GetWorld();
		
		if (pWorld == nullptr || pWorld->GetScene() == nullptr)
			return;

		char szFilePath[MAX_PATH] = "";

		OPENFILENAMEA ofn = {};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = pApp->GetWindowHandle();
		ofn.lpstrFilter = "Wavefront OBJ 3D Model (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = szFilePath;
		ofn.nMaxFile = sizeof(szFilePath);
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		ofn.lpstrTitle = "Import Wavefront OBJ Model";

		if (GetOpenFileNameA(&ofn))
		{
			C3DObject* pNewObj = new C3DObject();
			if (pNewObj->LoadFromOBJ(szFilePath))
			{
				// add new mesh to scene
				pWorld->GetScene()->AddObject(pNewObj);

				// select created mesh as current
				pApp->SetSelectedObject(pNewObj);
				pApp->GetScenePage()->RebuildSceneTree();
				UpdateInspector();

				Utils::ODS("[UI] Successfully imported OBJ model: %s", szFilePath);
			}
			else
			{
				delete pNewObj;
				MessageBoxA(pApp->GetWindowHandle(), "Failed to parse OBJ file geometry.", "Import Error", MB_OK | MB_ICONERROR);
			}
		}
		});

	pBtnImportObj->SetNormalColor(0x204060FF);
	pBtnImportObj->SetHoverColor(0x2E5B88FF);
	pBtnImportObj->SetPressedColor(0x14283CFF);
	AddChild(pBtnImportObj);
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


	// hide extra elem if selected object is nukk
	if (pObj == nullptr)
	{
		m_pTxtInspectorName->SetText("No Selection");
		m_pTxtInspectorStats->SetText("Verts: 0 | Tris: 0");
		m_pTxtInspectorTransform->SetText("Pos: (0.0, 0.0, 0.0)\nRot: (0.0, 0.0, 0.0)");

		m_pBtnToggleWireframe->SetEnabled(false);
		m_pBtnDeleteObject->SetEnabled(false);

		// hide sliders
		m_pTxtRotX->SetVisible(false); m_pSliderRotX->SetVisible(false);
		m_pTxtRotY->SetVisible(false); m_pSliderRotY->SetVisible(false);
		m_pTxtRotZ->SetVisible(false); m_pSliderRotZ->SetVisible(false);
		m_pTxtPosX->SetVisible(false); m_pSliderPosX->SetVisible(false);
		m_pTxtPosY->SetVisible(false); m_pSliderPosY->SetVisible(false);
		m_pTxtPosZ->SetVisible(false); m_pSliderPosZ->SetVisible(false);
		return;
	}

	m_pBtnToggleWireframe->SetEnabled(true);
	m_pBtnDeleteObject->SetEnabled(true);

	m_pTxtRotX->SetVisible(true); m_pSliderRotX->SetVisible(true);
	m_pTxtRotY->SetVisible(true); m_pSliderRotY->SetVisible(true);
	m_pTxtRotZ->SetVisible(true); m_pSliderRotZ->SetVisible(true);

	m_pTxtPosX->SetVisible(true); m_pSliderPosX->SetVisible(true);
	m_pTxtPosY->SetVisible(true); m_pSliderPosY->SetVisible(true);
	m_pTxtPosZ->SetVisible(true); m_pSliderPosZ->SetVisible(true);

	// reset cur obj name lable
	m_pTxtInspectorName->SetText(pObj->GetName());

	// cur obj stats
	char szStats[64] = {};
	sprintf_s(szStats, sizeof(szStats), "Verts: %zu | Tris: %zu",
		pObj->GetVertexCount(), pObj->GetTriangleCount());
	m_pTxtInspectorStats->SetText(szStats);

	// cur obj transform
	char szTransform[128] = {};
	sprintf_s(szTransform, sizeof(szTransform), "Pos: (%.1f, %.1f, %.1f) | %s\nRot: (%.1f, %.1f, %.1f)",
		pObj->GetPosition().x, pObj->GetPosition().y, pObj->GetPosition().z,
		(pObj->GetRenderType() == C3DObject::eRT_Wireframe ? "Wireframe" : "Solid"),
		pObj->GetRotation().x, pObj->GetRotation().y, pObj->GetRotation().z);
	m_pTxtInspectorTransform->SetText(szTransform);

	// sync cur obj transform with sliders
	m_pSliderRotX->SetValue(pObj->GetRotation().x);
	m_pSliderRotY->SetValue(pObj->GetRotation().y);
	m_pSliderRotZ->SetValue(pObj->GetRotation().z);

	m_pSliderPosX->SetValue(pObj->GetPosition().x);
	m_pSliderPosY->SetValue(pObj->GetPosition().y);
	m_pSliderPosZ->SetValue(pObj->GetPosition().z);
}
