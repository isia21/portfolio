#include "stdafx.h"

#include "../Engine/Graphics.h"
#include "../Engine/Input.h"
#include "../Engine/World.h"

#include "Application.h"

IMPLEMENT_SINGLETON(CApplication);

CApplication::~CApplication()
{
	SAFEDELETE(m_pRenderer);
	SAFEDELETE(m_pWorld);
}

bool CApplication::Initialize(HINSTANCE hInstance)
{
	Utils::ODS("[INFO] Initializing application...");
	m_hInstance = hInstance;

	if (!RegisterWindowClass())
		return false;

	if (!CreateApplicationWindow())
		return false;

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	QueryPerformanceFrequency(&m_liPerformanceFrequency);
	QueryPerformanceCounter(&m_liFrameStart);

	// --- Init renderer core ---
	m_pRenderer = new CRenderer();
	m_pRenderer->SetVSync(false);
	if (!m_pRenderer->Init(m_hWnd, m_lWidth, m_lHeight, false))
		return false;

	// --- Init world and camera ---
	m_pWorld = new CWorld();
	if (m_pWorld == nullptr || !m_pWorld->Init())
		return false;
	m_pWorld->GetCamera()->SetViewport(m_lWidth, m_lHeight);
	m_pWorld->GetCamera()->SetDrunkMode(true);

	// --- Init UI ---
	m_pUIManager = new CUIManager();
	{
		// --- Create FPS TextBox ---
		const int lFpsWidth = 160;
		const int lFpsHeight = 22;
		const int lFpsX = (m_lWidth / 2) - (lFpsWidth / 2);
		const int lFpsY = 8;

		m_pFPSTextBox = new CUITextBox(
			lFpsX, lFpsY,
			lFpsWidth, lFpsHeight,
			"FPS --.-- --.---ms",
			0x80808080,
			0xFFFFFFFF,
			14,
			TEXT_ALIGN_CENTER);

		m_pUIManager->AddElement(m_pFPSTextBox);


		// --- Create Scene Hierarchy & Slicing ---
		{
			const int lSceneWinWidth = 280;
			const int lSceneWinHeight = 440;

			CUIWindow* pSceneWindow = new CUIWindow(
				10, 10,
				lSceneWinWidth, lSceneWinHeight,
				"Scene Outliner",
				0x1E1E1EF2,
				0x282828FF,
				0x454545FF,
				1);

			const int lBtnY = 30;
			const int lBtnHeight = 22;
			const int lBtnWidth = 60;
			const int lBtnPadding = 5;
			int curBtnX = 6;

			// [New]
			CUIButton* pBtnNew = new CUIButton(curBtnX, lBtnY, lBtnWidth, lBtnHeight, "New", [this]() {
				if (m_pWorld == nullptr || m_pWorld->GetScene() == nullptr)
					return;

				CScene* pScene = m_pWorld->GetScene();

				// check if the scene has unsaved changes
				if (pScene->IsModified())
				{
					const int lResult = MessageBoxA(
						m_hWnd,
						"The current scene has unsaved changes.\nAre you sure you want to create a new scene?",
						"New Scene Confirmation",
						MB_YESNO | MB_ICONQUESTION);

					if (lResult != IDYES)
						return; // Отмена действия
				}

				// reset the selected object to nullptr and update the inspector
				m_pSelectedObject = nullptr;
				UpdateInspector();

				// recreate the default scene
				pScene->CreateDefault();

				Utils::ODS("[UI] New default scene created.");
				});
			pSceneWindow->AddChild(pBtnNew);
			curBtnX += lBtnWidth + lBtnPadding;

			// [Load]
			CUIButton* pBtnLoad = new CUIButton(curBtnX, lBtnY, lBtnWidth, lBtnHeight, "Load", [this]() {
				if (m_pWorld == nullptr || m_pWorld->GetScene() == nullptr)
					return;

				char szFilePath[MAX_PATH] = "";

				OPENFILENAMEA ofn = {};
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = m_hWnd;
				ofn.lpstrFilter = "CAD Assembly XML (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
				ofn.lpstrFile = szFilePath;
				ofn.nMaxFile = sizeof(szFilePath);
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
				ofn.lpstrTitle = "Open CAD Scene Assembly";

				if (GetOpenFileNameA(&ofn))
				{
					if (m_pWorld->GetScene()->LoadFromFile(szFilePath))
					{
						m_pSelectedObject = nullptr;
						UpdateInspector();
						Utils::ODS("[UI] Scene loaded from: %s", szFilePath);
					}
				}
				});
			pSceneWindow->AddChild(pBtnLoad);
			curBtnX += lBtnWidth + lBtnPadding;

			// [Save]
			CUIButton* pBtnSave = new CUIButton(curBtnX, lBtnY, lBtnWidth, lBtnHeight, "Save", [this]() {
				if (m_pWorld == nullptr || m_pWorld->GetScene() == nullptr)
					return;

				char szFilePath[MAX_PATH] = "Scene.xml";

				OPENFILENAMEA ofn = {};
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = m_hWnd;
				ofn.lpstrFilter = "CAD Assembly XML (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
				ofn.lpstrFile = szFilePath;
				ofn.nMaxFile = sizeof(szFilePath);
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
				ofn.lpstrDefExt = "xml";
				ofn.lpstrTitle = "Save CAD Scene Assembly";

				
				if (GetSaveFileNameA(&ofn))
				{
					CScene* pScene = m_pWorld->GetScene();
					if (pScene->SaveToFile(szFilePath))
					{
						Utils::ODS("[UI] Scene successfully saved to: %s", szFilePath);
					}
					else
					{
						Utils::ODS("[UI_ERROR] Failed to save scene to: %s", szFilePath);
					}
				}
				});
			pSceneWindow->AddChild(pBtnSave);
			curBtnX += lBtnWidth + lBtnPadding;

			// [Export]
			CUIButton* pBtnExport = new CUIButton(curBtnX, lBtnY, 68, lBtnHeight, "Export", [this]() {
				if (m_pWorld == nullptr || m_pWorld->GetScene() == nullptr)
					return;

				char szFilePath[MAX_PATH] = "Scene.obj";

				OPENFILENAMEA ofn = {};
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = m_hWnd;
				ofn.lpstrFilter = "Wavefront OBJ 3D Model (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
				ofn.lpstrFile = szFilePath;
				ofn.nMaxFile = sizeof(szFilePath);
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
				ofn.lpstrDefExt = "obj";
				ofn.lpstrTitle = "Export Scene to Wavefront OBJ";

				if (GetSaveFileNameA(&ofn))
				{
					if (m_pWorld->GetScene()->ExportToOBJ(szFilePath))
					{
						Utils::ODS("[UI] Scene successfully exported to OBJ: %s", szFilePath);
						MessageBoxA(m_hWnd, "Scene exported successfully to OBJ!\nIndividual parts saved to 'ModelsOBJ/' folder.", "Export Complete", MB_OK | MB_ICONINFORMATION);
					}
					else
					{
						Utils::ODS("[UI_ERROR] Failed to export scene to OBJ: %s", szFilePath);
					}
				}
				});
			curBtnX += lBtnWidth + lBtnPadding;
			pSceneWindow->AddChild(pBtnExport);

			// Smart Tree (Scene Outliner)
			const int lTreeY = 56;
			const int lTreeWidth = 268;
			const int lTreeHeight = 340;

			m_pSceneTree = new CUISmartTree(6, lTreeY, lTreeWidth, lTreeHeight, 20, 12);

			const std::vector<C3DObject*>& vWorldObjects = m_pWorld->GetObjects();

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
				if (pNode != nullptr && pNode->pUserData != nullptr)
					m_pSelectedObject = static_cast<C3DObject*>(pNode->pUserData);
				else
					m_pSelectedObject = nullptr;

				UpdateInspector();
				});

			pSceneWindow->AddChild(m_pSceneTree);


			// --- Bottom slicer toolbar (Slicing Actions) ---
			const int lBottomY = lSceneWinHeight - 34;
			const int lBottomH = 26;
			int curBottomX = 6;

			// [Add Slicer]
			CUIButton* pBtnAddSlicer = new CUIButton(curBottomX, lBottomY, 78, lBottomH, "+ Slicer", [this]() {
				if (m_pWorld == nullptr || m_pWorld->GetScene() == nullptr)
					return;

				CScene* pScene = m_pWorld->GetScene();

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
				});
			pSceneWindow->AddChild(pBtnAddSlicer);
			curBottomX += 78 + 4;

			// [Cut Scene]
			CUIButton* pBtnRunCut = new CUIButton(curBottomX, lBottomY, 96, lBottomH, "Cut Scene", [this]() {
				if (m_pWorld == nullptr || m_pWorld->GetScene() == nullptr)
					return;

				m_pWorld->GetScene()->ExecuteSlicingPipeline();
				m_pSelectedObject = nullptr;
				UpdateInspector();
				});
			pBtnRunCut->SetNormalColor(0x1B5E20EE);
			pBtnRunCut->SetHoverColor(0x2E7D32FF);
			pBtnRunCut->SetPressedColor(0x144017FF);
			pSceneWindow->AddChild(pBtnRunCut);
			curBottomX += 96 + 4;

			// [Reset Cuts]
			CUIButton* pBtnResetCuts = new CUIButton(curBottomX, lBottomY, 86, lBottomH, "Reset Cuts", [this]() {
				if (m_pWorld == nullptr || m_pWorld->GetScene() == nullptr)
					return;

				CScene* pScene = m_pWorld->GetScene();
				pScene->ClearMeshParts();

				for (C3DObject* pObj : pScene->GetObjects())
				{
					if (pObj && pObj->GetObjectType() == C3DObject::eOT_SourceModel)
						pObj->SetVisible(true);
				}

				pScene->MarkStructureChanged();
				m_pSelectedObject = nullptr;
				UpdateInspector();
				Utils::ODS("[UI] Slices reset. Original models restored.");
				});
			pSceneWindow->AddChild(pBtnResetCuts);

			m_pUIManager->AddElement(pSceneWindow);
		}

		// --- Inspector Window ---
		{
			const int lInspWidth = 250;
			const int lInspHeight = 230;
			const int lInspX = m_lWidth - lInspWidth - 10;

			m_pInspectorWindow = new CUIWindow(
				lInspX, 10,
				lInspWidth, lInspHeight,
				"Object Inspector",
				0x1E1E1EF2, 0x282828FF, 0x454545FF, 1);

			m_pTxtInspectorName = new CUITextBox(6, 30, 238, 22, "No Selection", 0x141414AA, 0x00FF88FF, 12, TEXT_ALIGN_LEFT);
			m_pInspectorWindow->AddChild(m_pTxtInspectorName);

			m_pTxtInspectorStats = new CUITextBox(6, 56, 238, 22, "Verts: 0 | Tris: 0", 0x141414AA, 0xAAAAAAFF, 12, TEXT_ALIGN_LEFT);
			m_pInspectorWindow->AddChild(m_pTxtInspectorStats);

			m_pTxtInspectorTransform = new CUITextBox(6, 82, 238, 40, "Pos: (0.0, 0.0, 0.0)", 0x141414AA, 0xCCCCCCFF, 11, TEXT_ALIGN_LEFT);
			m_pInspectorWindow->AddChild(m_pTxtInspectorTransform);

			m_pBtnToggleWireframe = new CUIButton(6, 130, 238, 26, "Toggle Wireframe", [this]() {
				if (m_pSelectedObject != nullptr)
				{

					m_pSelectedObject->SetRenderType(
						(m_pSelectedObject->GetRenderType()  == C3DObject::eRT_Wireframe)
						? C3DObject::eRT_Poligon
						: C3DObject::eRT_Wireframe
						);

					UpdateInspector();
				}
				});
			m_pInspectorWindow->AddChild(m_pBtnToggleWireframe);


			m_pBtnDeleteObject = new CUIButton(6, 162, 238, 26, "Delete Entity", [this]() {
				if (m_pSelectedObject != nullptr && m_pWorld != nullptr && m_pWorld->GetScene() != nullptr)
				{
					m_pWorld->GetScene()->RemoveObject(m_pSelectedObject);
					m_pSelectedObject = nullptr;
					RebuildSceneTree();
					UpdateInspector();
				}
				});
			m_pBtnDeleteObject->SetHoverColor(0x8B0000FF);
			m_pInspectorWindow->AddChild(m_pBtnDeleteObject);

			m_pUIManager->AddElement(m_pInspectorWindow);

			UpdateInspector();
		}
	}
	m_bRunning = true;

	Utils::ODS("[INFO] Application initialized successfully.");
	return true;
}

int CApplication::Run()
{
	MSG message = {};

	QueryPerformanceCounter(&m_liFrameStart);

	Utils::ODS("[INFO] Entering main loop...");

	while (m_bRunning)
	{

		// --- Update Input Sys ---
		if (CKeyboard::GetInstance()) CKeyboard::GetInstance()->Update();
		if (CMouse::GetInstance()) CMouse::GetInstance()->Update();

		// --- Message pump --- 
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				m_bRunning = false;
				break;
			}

			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (!m_bRunning)
			break;

		// --- Frame timing ---	
		LARGE_INTEGER liCurrent;
		QueryPerformanceCounter(&liCurrent);

		const double fDeltaTime = static_cast<double>(liCurrent.QuadPart - m_liFrameStart.QuadPart) / static_cast<double>(m_liPerformanceFrequency.QuadPart);

		m_liFrameStart = liCurrent;

		m_fFrameTime = fDeltaTime;
		m_fFPSTime += fDeltaTime;
		++m_lFPSFrames;

		if (m_fFPSTime >= 0.25)
		{
			m_fFPS = static_cast<double>(m_lFPSFrames) / m_fFPSTime;

			m_lFPSFrames = 0;
			m_fFPSTime = 0.0;

			// --- ОБНОВЛЕНИЕ ТЕКСТА FPS ---
			if (m_pFPSTextBox != nullptr)
			{
				char szFpsBuffer[64] = {};
				sprintf_s(szFpsBuffer, sizeof(szFpsBuffer), "FPS %.2f %.3fms", m_fFPS, m_fFrameTime * 1000.0);
				m_pFPSTextBox->SetText(szFpsBuffer);
			}

		}

		// --- Process frame and render ---
		Update();
		Render();

		// --- FPS limiter ---
		if (m_bLockFPS && m_lFPSLock > 0)
		{
			const LONGLONG llTargetFrameTicks = m_liPerformanceFrequency.QuadPart / m_lFPSLock;

			LARGE_INTEGER liCurrent;
			do
			{
				QueryPerformanceCounter(&liCurrent);
			}
			while ((liCurrent.QuadPart - m_liFrameStart.QuadPart) < llTargetFrameTicks);
		}

	}
	return static_cast<int>(message.wParam);
}


bool CApplication::RegisterWindowClass()
{
	WNDCLASSEX windowClass = {};

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = &CApplication::StaticWindowProc;
	windowClass.hInstance = m_hInstance;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = "MeshCutOpenGLWindow";

	if (!RegisterClassEx(&windowClass))
	{
		return false;
	}

	return true;
}


bool CApplication::CreateApplicationWindow()
{
	RECT windowRect = { 0,0,m_lWidth,m_lHeight};

	if (!AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE))
		return false;
	
	const int windowWidth = windowRect.right - windowRect.left;
	const int windowHeight = windowRect.bottom - windowRect.top;

	m_hWnd = CreateWindowEx(
		0,
		"MeshCutOpenGLWindow", "Mesh Cutting - OpenGL",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidth, windowHeight,
		nullptr, nullptr, m_hInstance, this);

	if (m_hWnd == nullptr)
		return false;

	return true;
}

void CApplication::UpdateInspector()
{
	if (m_pInspectorWindow == nullptr)
		return;

	// Если объект не выбран:
	if (m_pSelectedObject == nullptr)
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
	m_pTxtInspectorName->SetText(m_pSelectedObject->GetName());

	// 2. Статистика геометрии
	char szStats[64] = {};
	sprintf_s(szStats, sizeof(szStats), "Verts: %zu | Tris: %zu",
		m_pSelectedObject->GetVertexCount(),
		m_pSelectedObject->GetTriangleCount());
	m_pTxtInspectorStats->SetText(szStats);

	// 3. Координаты
	char szTransform[128] = {};
	sprintf_s(szTransform, sizeof(szTransform), "Pos: (%.1f, %.1f, %.1f) | %s",
		m_pSelectedObject->GetPosition().x,
		m_pSelectedObject->GetPosition().y,
		m_pSelectedObject->GetPosition().z,
		(m_pSelectedObject->GetRenderType() == C3DObject::eRT_Wireframe ? "Wireframe" : "Solid"));
	m_pTxtInspectorTransform->SetText(szTransform);
}

void CApplication::RebuildSceneTree()
{
	if (m_pSceneTree == nullptr || m_pWorld == nullptr || m_pWorld->GetScene() == nullptr)
		return;

	m_pSceneTree->Clear();

	CScene* pScene = m_pWorld->GetScene();
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

void CApplication::Update()
{
	// TODO:
	// --- Global App HotKey`s ---
	if (CKeyboard::GetInstance()->IsKeyPressed(VK_ESCAPE))
	{
		Shutdown(); // or DestroyWindow(m_hWnd);
		return;
	}

	// --- Update UI ---
	// (can reset input states)
	if (m_pUIManager != nullptr)
		m_pUIManager->Update();

	// --- World update ---
	if (m_pWorld != nullptr)
	{
		m_pWorld->Update(static_cast<float>(m_fFrameTime));

		// --- Check for scene structure changes and rebuild the scene tree if necessary ---
		CScene* pScene = m_pWorld->GetScene();
		if (pScene != nullptr && pScene->HasStructureChanged())
		{
			RebuildSceneTree();
			pScene->ResetStructureChanged();
		}
	}

	// Object transforms
	// Animation
	//
	// This will later be separated from rendering.
}

void CApplication::Render()
{
	// --- Clear screen and set camera ---
	m_pRenderer->Clear();

	// --- Render World ---
	if (m_pWorld != nullptr)
	{
		m_pRenderer->SetCamera(m_pWorld->GetCamera());

		// --- Render World Objects ---
		for (C3DObject* pObject : m_pWorld->GetObjects())
			m_pRenderer->Draw(pObject);
	}

	// --- Render UI Canvas ---
	if (m_pUIManager != nullptr)
		m_pUIManager->Render(m_pRenderer);

	// --- Render all scene/frame data -- 
	m_pRenderer->Render();

#ifdef _DEBUG
	static unsigned long s_lFrame = 0;
	Utils::ODS("[APP] Frame %lu", ++s_lFrame);
#endif
}

void CApplication::Resize(int width, int height)
{
	if (width <= 0 || height <= 0)
		return;

	m_lWidth = width;
	m_lHeight = height;

	Utils::ODS("[INFO] Resizing window to %dx%d", m_lWidth, m_lHeight);

	if (m_pRenderer != nullptr)
		m_pRenderer->Resize(m_lWidth, m_lHeight);

	if (m_pWorld != nullptr && m_pWorld->GetCamera() != nullptr)
		m_pWorld->GetCamera()->SetViewport(m_lWidth, m_lHeight);

	// --- Fix FPS box position after window resize ---
	if (m_pFPSTextBox != nullptr)
	{
		const int lFpsX = (m_lWidth / 2) - (m_pFPSTextBox->GetWidth() / 2);
		m_pFPSTextBox->SetPosition(lFpsX, 8);
	}

	if (m_pInspectorWindow != nullptr)
	{
		const int lInspX = m_lWidth - m_pInspectorWindow->GetWidth() - 10;
		m_pInspectorWindow->SetPosition(lInspX, 10);
	}
}

void CApplication::Shutdown()
{
	m_bRunning = false;

	Utils::ODS("[INFO] Shutting down application...");

	SAFEDELETE(m_pUIManager);
	//SAFEDELETE(m_pFPSTextBox);//	 = nullptr;

	if (m_pRenderer != nullptr)
		m_pRenderer->Shutdown();

	if (m_pWorld != nullptr)
		m_pWorld->Shutdown();

	if (m_hWnd != nullptr)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}
}


LRESULT CALLBACK CApplication::StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CApplication* application = nullptr;

	if (message == WM_NCCREATE)
	{
		const CREATESTRUCT* createStruct = reinterpret_cast<const CREATESTRUCT*>(lParam);
		application = static_cast<CApplication*>(createStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
	}
	else
		application =reinterpret_cast<CApplication*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));


	if (application != nullptr)
		return application->WindowProc(hWnd, message, wParam, lParam);

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// --- Push raw data to Input ---
	if (CKeyboard::GetInstance()) 
		CKeyboard::GetInstance()->ProcessMessage(message, wParam, lParam);
	if (CMouse::GetInstance()) 
		CMouse::GetInstance()->ProcessMessage(message, wParam, lParam);

	switch (message)
	{
	case WM_SIZE:
	{
		const int width = LOWORD(lParam);
		const int height = HIWORD(lParam);
		Resize(width, height);
		return 0;
	}

	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
		return 0;
	}

	case WM_DESTROY:
	{
		m_bRunning = false;
		PostQuitMessage(0);
		return 0;
	}
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}