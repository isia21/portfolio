#include "stdafx.h"
#include "Application.h"


IMPLEMENT_SINGLETON(CApplication);

bool CApplication::Initialize(HINSTANCE hInstance)
{
	Utils::ODS("[INFO] Initializing application...");
	m_hInstance = hInstance;

	if (!RegisterWindowClass())
		return false;

	if (!CreateApplicationWindow())
		return false;

	if (!CreateOpenGLContext())
		return false;

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	m_bRunning = true;

	Utils::ODS("[INFO] Application initialized successfully.");
	return true;
}

int CApplication::Run()
{
	MSG message = {};
	Utils::ODS("[INFO] Entering main loop...");
	while (m_bRunning)
	{
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

		Update();
		Render();
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

bool CApplication::CreateOpenGLContext()
{
	m_hDC = GetDC(m_hWnd);
	if (m_hDC == nullptr)
		return false;

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {};

	pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 32;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cStencilBits = 8;
	pixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;

	const int pixelFormat = ChoosePixelFormat(m_hDC, &pixelFormatDescriptor);
	if (pixelFormat == 0)
		return false;

	if (!SetPixelFormat(m_hDC, pixelFormat, &pixelFormatDescriptor))
		return false;

	m_hGLRC = wglCreateContext(m_hDC);
	if (m_hGLRC == nullptr)
		return false;

	if (!wglMakeCurrent(m_hDC, m_hGLRC))
		return false;

	// Initial OpenGL state.
	glViewport(0, 0, m_lWidth, m_lHeight);
	glClearColor(0.08f, 0.08f, 0.10f, 1.0f);

	return true;
}


void CApplication::Update()
{
	// TODO:
	//
	// Input processing
	// Camera update
	// Object transforms
	// Animation
	//
	// This will later be separated from rendering.
}

void CApplication::Render()
{
	// Temporary renderer placeholder.
	//
	// The actual Renderer class will replace this code.

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SwapBuffers(m_hDC);
}

void CApplication::Resize(int width, int height)
{
	if (width <= 0 || height <= 0)
		return;

	m_lWidth = width;
	m_lHeight = height;

	if (m_hGLRC == nullptr)
		return;

	Utils::ODS("[INFO] Resizing window to %dx%d", m_lWidth, m_lHeight);
	glViewport(0, 0, m_lWidth, m_lHeight);
}

void CApplication::Shutdown()
{
	m_bRunning = false;

	Utils::ODS("[INFO] Shutting down application...");
	if (m_hGLRC != nullptr)
	{
		// A context must no longer be current
		// before it is destroyed.
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(m_hGLRC);
		m_hGLRC = nullptr;
	}

	if (m_hDC != nullptr && m_hWnd != nullptr)
	{
		ReleaseDC(m_hWnd, m_hDC);
		m_hDC = nullptr;
	}

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

	default:
		break;
	}

	return DefWindowProcW(hWnd, message, wParam, lParam);
}