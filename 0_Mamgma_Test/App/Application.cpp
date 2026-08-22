#include "stdafx.h"

#include "../Engine/Renderer.h"
#include "../Engine/Camera.h"
#include "../Engine/World.h"
#include "../Engine/UI.h"

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


		// --- Create Buttons panel ---
		const int lBtnWidth = 80;
		const int lBtnHeight = 24;
		const int lBtnPadding = 6;
		int lCurrentX = 10;
		const int lBtnY = 8;

		// Load button
		CUIButton* pBtnLoad = new CUIButton(
			lCurrentX, lBtnY,
			lBtnWidth, lBtnHeight,
			"Load",
			[]() {
				Utils::ODS("[UI_CALLBACK] Button 'Load' was clicked!");
			});

		pBtnLoad->SetNormalColor(0x353535CC);
		pBtnLoad->SetHoverColor(0x505050FF);
		pBtnLoad->SetPressedColor(0x2060A0FF);
		m_pUIManager->AddElement(pBtnLoad);

		lCurrentX += lBtnWidth + lBtnPadding;

		// Save button
		CUIButton* pBtnSave = new CUIButton(
			lCurrentX, lBtnY,
			lBtnWidth, lBtnHeight,
			"Save",
			[]() {
				Utils::ODS("[UI_CALLBACK] Button 'Save' was clicked!");
			});

		pBtnSave->SetNormalColor(0x353535CC);
		pBtnSave->SetHoverColor(0x505050FF);
		pBtnSave->SetPressedColor(0x2060A0FF);
		m_pUIManager->AddElement(pBtnSave);

		lCurrentX += lBtnWidth + lBtnPadding;

		// Export button
		CUIButton* pBtnExport = new CUIButton(
			lCurrentX, lBtnY,
			lBtnWidth, lBtnHeight,
			"Export",
			[]() {
				Utils::ODS("[UI_CALLBACK] Button 'Export' was clicked!");
			});

		pBtnExport->SetNormalColor(0x353535CC);
		pBtnExport->SetHoverColor(0x505050FF);
		pBtnExport->SetPressedColor(0x2060A0FF);
		m_pUIManager->AddElement(pBtnExport);
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
	//
	// Input processing
	
	// --- World update ---
	if (m_pWorld != nullptr)
		m_pWorld->Update(static_cast<float>(m_fFrameTime));

	// Object transforms
	// Animation
	//
	// This will later be separated from rendering.
}

void CApplication::Render()
{
	// --- Clear screen and set camera ---
	m_pRenderer->Clear();

	if (m_pWorld != nullptr)
		m_pRenderer->SetCamera(m_pWorld->GetCamera());


	// --- Render World Objects ---
	for (C3DObject* pObject : m_pWorld->GetObjects())
		m_pRenderer->Draw(pObject);

	
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
}

void CApplication::Shutdown()
{
	m_bRunning = false;

	Utils::ODS("[INFO] Shutting down application...");

	SAFEDELETE(m_pUIManager);
	SAFEDELETE(m_pFPSTextBox);//	 = nullptr;

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

	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
			DestroyWindow(hWnd);

		return 0;
	}

	// --- Mouse Event Processing ---
	case WM_MOUSEMOVE:
	{
		const int mouseX = LOWORD(lParam);
		const int mouseY = HIWORD(lParam);
		if (m_pUIManager != nullptr)
			m_pUIManager->ProcessMouseMove(mouseX, mouseY);
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		const int mouseX = LOWORD(lParam);
		const int mouseY = HIWORD(lParam);
		if (m_pUIManager != nullptr)
			m_pUIManager->ProcessMouseDown(mouseX, mouseY, 0);
		return 0;
	}

	case WM_LBUTTONUP:
	{
		const int mouseX = LOWORD(lParam);
		const int mouseY = HIWORD(lParam);
		if (m_pUIManager != nullptr)
			m_pUIManager->ProcessMouseUp(mouseX, mouseY, 0);
		return 0;
	}

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}