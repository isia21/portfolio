#include "stdafx.h"

#include "../Engine/Graphics.h"
#include "../Engine/Input.h"
#include "../Engine/World.h"

#include "../Engine/Graphics/UI/Pages/ScenePage.h"
#include "../Engine/Graphics/UI/Pages/ObjectInspectorPage.h"
#include "../Engine/Graphics/UI/Pages/ReadmePage.h"

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
	//	m_pWorld->GetCamera()->SetDrunkMode(true);

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


		// --- Readme Page ---
		{
			m_pUIReadmePage = new CReadmePage();
			m_pUIReadmePage->Init();
			m_pUIManager->AddElement(m_pUIReadmePage);
		}

		// --- Create Scene Hierarchy & Slicing ---
		{
			m_pUIScenePage = new CScenePage();
			m_pUIScenePage->Init();
			m_pUIManager->AddElement(m_pUIScenePage);
		}

		// --- Inspector Window ---
		{
			m_pUIObjectInspectorPage = new CObjectInspectorPage();
			m_pUIObjectInspectorPage->Init();
			m_pUIManager->AddElement(m_pUIObjectInspectorPage);

			m_pUIObjectInspectorPage->UpdateInspector();
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

void CApplication::Update()
{
	// TODO:
	// --- Global App HotKey`s ---
	if (CKeyboard::GetInstance()->IsKeyPressed(VK_ESCAPE))
	{
		Shutdown(); // or DestroyWindow(m_hWnd);
		return;
	}

	if (CKeyboard::GetInstance()->IsKeyPressed(VK_F1))
		if (m_pUIReadmePage != nullptr)
			m_pUIReadmePage->SetVisible(!m_pUIReadmePage->IsVisible());

	if (CKeyboard::GetInstance()->IsKeyDown(VK_CONTROL))
	{
		bool bHooked = true;
		CScenePage* pScenePage = GetScenePage();
		if (CKeyboard::GetInstance()->IsKeyPressed('S'))
			pScenePage->SceneSave();
		else if (CKeyboard::GetInstance()->IsKeyPressed('O'))
			pScenePage->SceneLoad();
		else if (CKeyboard::GetInstance()->IsKeyPressed('N'))
			pScenePage->SceneNew();
		else if (CKeyboard::GetInstance()->IsKeyPressed('E'))
			pScenePage->SceneExport();
		else
			bHooked = false;
		
		if (bHooked) 
		{
			CKeyboard::GetInstance()->ConsumeKey('S');
			CKeyboard::GetInstance()->ConsumeKey('O');
			CKeyboard::GetInstance()->ConsumeKey('N');
			CKeyboard::GetInstance()->ConsumeKey('E');
		}
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
			GetScenePage()->RebuildSceneTree();
			pScene->ResetStructureChanged();
		}
	}

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

	// --- 2D Line of runtime slic ---
	if (m_pWorld != nullptr && m_pWorld->IsCuttingGestureActive())
	{
		int x1, y1, x2, y2;
		m_pWorld->GetCuttingLine(x1, y1, x2, y2);

		m_pRenderer->DrawLine(x1, y1, x2, y2, 0xFF0033FF, 3.0f);
	}


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

	if (m_pUIObjectInspectorPage != nullptr)
	{
		const int lInspX = m_lWidth - m_pUIObjectInspectorPage->GetWidth() - 10;
		m_pUIObjectInspectorPage->SetPosition(lInspX, 10);
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