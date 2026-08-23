#include "stdafx.h"
#include "../../../Graphics.h"
#include "../../../../App/Application.h"
#include "../../../World.h"

#include "ScenePage.h"
#include "ObjectInspectorPage.h"

const int lSceneWinWidth = 280;
const int lSceneWinHeight = 440;

CScenePage::CScenePage() : 
	CUIWindow::CUIWindow(
	10, 10, lSceneWinWidth, lSceneWinHeight,
	"Scene Outliner",
	0x1E1E1EF2, 0x282828FF, 0x454545FF,
	1)
{}

CScenePage::~CScenePage() {}

//==========================

void CScenePage::Init()
{
	const int lBtnY = 30;
	const int lBtnHeight = 22;
	const int lBtnWidth = 60;
	const int lBtnPadding = 5;
	int curBtnX = 6;

	// [New]
	CUIButton* pBtnNew = new CUIButton(curBtnX, lBtnY, lBtnWidth, lBtnHeight, "New", [this]() {SceneNew(); });
	AddChild(pBtnNew);
	curBtnX += lBtnWidth + lBtnPadding;

	// [Load]
	CUIButton* pBtnLoad = new CUIButton(curBtnX, lBtnY, lBtnWidth, lBtnHeight, "Load", [this]() {SceneLoad(); });
	AddChild(pBtnLoad);
	curBtnX += lBtnWidth + lBtnPadding;

	// [Save]
	CUIButton* pBtnSave = new CUIButton(curBtnX, lBtnY, lBtnWidth, lBtnHeight, "Save", [this]() {SceneSave(); });
	AddChild(pBtnSave);
	curBtnX += lBtnWidth + lBtnPadding;

	// [Export]
	CUIButton* pBtnExport = new CUIButton(curBtnX, lBtnY, 68, lBtnHeight, "Export", [this]() {SceneExport(); });
	curBtnX += lBtnWidth + lBtnPadding;
	AddChild(pBtnExport);

	// Smart Tree (Scene Outliner)
	const int lTreeY = 56;
	const int lTreeWidth = 268;
	const int lTreeHeight = 340;

	m_pSceneTree = new CUISmartTree(6, lTreeY, lTreeWidth, lTreeHeight, 20, 12);
	
	// Switch 3D Object visibility when toggling the checkbox in the tree
	m_pSceneTree->SetOnToggleVisibility([](SUITreeNode* pNode, bool bVisible) {
		if (pNode != nullptr && pNode->pUserData != nullptr)
		{
			C3DObject* pObj = static_cast<C3DObject*>(pNode->pUserData);
			pObj->SetVisible(bVisible);
			Utils::ODS("[SCENE_UI] Visibility of '%s' -> %s", pNode->sText.c_str(), bVisible ? "SHOWN" : "HIDDEN");
		}
		});

	// Log selection of a node in the tree
	m_pSceneTree->SetOnSelect([this](SUITreeNode* pNode) {
		CApplication* pApp = CApplication::GetInstance();
		if (pNode != nullptr && pNode->pUserData != nullptr)
			pApp->SetSelectedObject(static_cast<C3DObject*>(pNode->pUserData));
		else
			pApp->SetSelectedObject(nullptr);

		pApp->GetObjectInspectorPage()->UpdateInspector();
		});
	AddChild(m_pSceneTree);


	// --- Bottom slicer toolbar (Slicing Actions) ---
	const int lBottomY = lSceneWinHeight - 34;
	const int lBottomH = 26;
	int curBottomX = 6;

	// [Add Slicer]
	CUIButton* pBtnAddSlicer = new CUIButton(curBottomX, lBottomY, 78, lBottomH, "+ Slicer", [this]() { SlicerAdd();  });
	AddChild(pBtnAddSlicer);
	curBottomX += 78 + 4;

	// [Cut Scene]
	CUIButton* pBtnRunCut = new CUIButton(curBottomX, lBottomY, 96, lBottomH, "Cut Scene", [this]() { SlicerRun(); });
	pBtnRunCut->SetNormalColor(0x1B5E20EE);
	pBtnRunCut->SetHoverColor(0x2E7D32FF);
	pBtnRunCut->SetPressedColor(0x144017FF);
	AddChild(pBtnRunCut);
	curBottomX += 96 + 4;

	// [Reset Cuts]
	CUIButton* pBtnResetCuts = new CUIButton(curBottomX, lBottomY, 86, lBottomH, "Reset Cuts", [this]() { SlicerReset(); });
	AddChild(pBtnResetCuts);
}

//====================================
// Scene buttons callbacks
//====================================
void CScenePage::SceneNew()
{
	CApplication* pApp = CApplication::GetInstance();	
	CWorld* pWorld = pApp->GetWorld();

	if (pWorld == nullptr || pWorld->GetScene() == nullptr)
		return;

	CScene* pScene = pWorld->GetScene();

	// check if the scene has unsaved changes
	if (pScene->IsModified())
	{
		const int lResult = MessageBoxA(
			pApp->GetWindowHandle(),
			"The current scene has unsaved changes.\nAre you sure you want to create a new scene?",
			"New Scene Confirmation",
			MB_YESNO | MB_ICONQUESTION);

		if (lResult != IDYES)
			return; // Отмена действия
	}

	// reset the selected object to nullptr and update the inspector
	pApp->SetSelectedObject(nullptr);
	CApplication::GetInstance()->GetObjectInspectorPage()->UpdateInspector();

	// recreate the default scene
	pScene->CreateDefault();

	Utils::ODS("[UI] New default scene created.");
}

void CScenePage::SceneLoad()
{
	CApplication* pApp = CApplication::GetInstance();
	CWorld* pWorld = pApp->GetWorld();

	if (pWorld == nullptr || pWorld->GetScene() == nullptr)
		return;

	char szFilePath[MAX_PATH] = "";

	OPENFILENAMEA ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = pApp->GetWindowHandle();
	ofn.lpstrFilter = "CAD Assembly XML (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFilePath;
	ofn.nMaxFile = sizeof(szFilePath);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrTitle = "Open CAD Scene Assembly";

	if (GetOpenFileNameA(&ofn))
	{
		if (pWorld->GetScene()->LoadFromFile(szFilePath))
		{
			pApp->SetSelectedObject(nullptr);
			CApplication::GetInstance()->GetObjectInspectorPage()->UpdateInspector();
			Utils::ODS("[UI] Scene loaded from: %s", szFilePath);
		}
	}
}

void CScenePage::SceneSave()
{
	CApplication* pApp = CApplication::GetInstance();
	CWorld* pWorld = pApp->GetWorld();

	if (pWorld == nullptr || pWorld->GetScene() == nullptr)
		return;

	char szFilePath[MAX_PATH] = "Scene.xml";

	OPENFILENAMEA ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = pApp->GetWindowHandle();
	ofn.lpstrFilter = "CAD Assembly XML (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFilePath;
	ofn.nMaxFile = sizeof(szFilePath);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "xml";
	ofn.lpstrTitle = "Save CAD Scene Assembly";

	if (GetSaveFileNameA(&ofn))
	{
		CScene* pScene = pWorld->GetScene();
		if (pScene->SaveToFile(szFilePath))
			Utils::ODS("[UI] Scene successfully saved to: %s", szFilePath);
		else
			Utils::ODS("[UI_ERROR] Failed to save scene to: %s", szFilePath);
	}
}

void CScenePage::SceneExport()
{
	CApplication* pApp = CApplication::GetInstance();
	CWorld* pWorld = pApp->GetWorld();

	if (pWorld == nullptr || pWorld->GetScene() == nullptr)
		return;

	char szFilePath[MAX_PATH] = "Scene.obj";

	OPENFILENAMEA ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = pApp->GetWindowHandle();
	ofn.lpstrFilter = "Wavefront OBJ 3D Model (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFilePath;
	ofn.nMaxFile = sizeof(szFilePath);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "obj";
	ofn.lpstrTitle = "Export Scene to Wavefront OBJ";

	if (GetSaveFileNameA(&ofn))
	{
		if (pWorld->GetScene()->ExportToOBJ(szFilePath))
		{
			Utils::ODS("[UI] Scene successfully exported to OBJ: %s", szFilePath);
			MessageBoxA(pApp->GetWindowHandle(), "Scene exported successfully to OBJ!\nIndividual parts saved to 'ModelsOBJ/' folder.", "Export Complete", MB_OK | MB_ICONINFORMATION);
		}
		else
			Utils::ODS("[UI_ERROR] Failed to export scene to OBJ: %s", szFilePath);
	}
}

//====================================
// Slicer buttons callbacks
//====================================
void CScenePage::SlicerAdd()
{
	CApplication* pApp = CApplication::GetInstance();
	CWorld* pWorld = pApp->GetWorld();

	if (pWorld == nullptr || pWorld->GetScene() == nullptr)
		return;

	CScene* pScene = pWorld->GetScene();

	size_t slicerCount = 0;
	for (C3DObject* pObj : pScene->GetObjects())
	{
		if (pObj && pObj->GetObjectType() == C3DObject::eOT_Slicer)
			++slicerCount;
	}

	CSlicer* pNewSlicer = new CSlicer(12.0f, 0xFF005580);
	pNewSlicer->SetObjectType(C3DObject::eOT_Slicer);

	char szName[64] = {};
	sprintf_s(szName, sizeof(szName), "Slicer #%zu", slicerCount + 1);
	pNewSlicer->SetName(szName);

	pNewSlicer->SetPosition(0.0f, static_cast<float>(slicerCount) * 1.5f, 0.0f);

	pScene->AddObject(pNewSlicer);
	Utils::ODS("[UI] Added new slicing tool: %s", szName);
}

void CScenePage::SlicerRun()
{
	CApplication* pApp = CApplication::GetInstance();
	CWorld* pWorld = pApp->GetWorld();

	if (pWorld == nullptr || pWorld->GetScene() == nullptr)
		return;

	pWorld->GetScene()->ExecuteSlicingPipeline();
	pApp->SetSelectedObject(nullptr);
	CApplication::GetInstance()->GetObjectInspectorPage()->UpdateInspector();
}

void CScenePage::SlicerReset()
{
	CApplication* pApp = CApplication::GetInstance();
	CWorld* pWorld = pApp->GetWorld();

	if (pWorld == nullptr || pWorld->GetScene() == nullptr)
		return;

	CScene* pScene = pWorld->GetScene();
	pScene->ClearMeshParts();

	for (C3DObject* pObj : pScene->GetObjects())
	{
		if (pObj && pObj->GetObjectType() == C3DObject::eOT_SourceModel)
			pObj->SetVisible(true);
	}

	pScene->MarkStructureChanged();
	pApp->SetSelectedObject(nullptr);
	CApplication::GetInstance()->GetObjectInspectorPage()->UpdateInspector();
	Utils::ODS("[UI] Slices reset. Original models restored.");

}

//====================

void CScenePage::RebuildSceneTree()
{
	CApplication* pApp = CApplication::GetInstance();
	CWorld* pWorld = pApp->GetWorld();

	if (m_pSceneTree == nullptr || pWorld == nullptr || pWorld->GetScene() == nullptr)
		return;

	m_pSceneTree->Clear();

	CScene* pScene = pWorld->GetScene();
	const auto& vObjects = pScene->GetObjects();

	// --- 1. Source models and their sliced parts ---
	for (C3DObject* pObj : vObjects)
	{
		if (pObj && pObj->GetObjectType() == C3DObject::eOT_SourceModel)
		{
			SUITreeNode* pSourceNode = m_pSceneTree->AddRoot(pObj->GetName(), pObj);
			pSourceNode->bObjectVisible = pObj->IsVisible();

			for (C3DObject* pChild : pObj->GetChildren())
			{
				if (pChild != nullptr)
				{
					SUITreeNode* pChildNode = m_pSceneTree->AddChild(pSourceNode, pChild->GetName(), pChild);
					pChildNode->bObjectVisible = pChild->IsVisible();
				}
			}
		}
	}

	// --- 2. Separated group for slicers/cutters in the tree ---
	SUITreeNode* pSlicersGroup = nullptr;
	for (C3DObject* pObj : vObjects)
	{
		if (pObj && pObj->GetObjectType() == C3DObject::eOT_Slicer)
		{
			if (pSlicersGroup == nullptr)
				pSlicersGroup = m_pSceneTree->AddRoot("[ Slicers / Cutters ]", nullptr);

			SUITreeNode* pSlicerNode = m_pSceneTree->AddChild(pSlicersGroup, pObj->GetName(), pObj);
			pSlicerNode->bObjectVisible = pObj->IsVisible();
		}
	}
}