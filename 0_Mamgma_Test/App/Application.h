#pragma once
class CRenderer;
class CCamera;

class CApplication
{
	DECLARE_SINGLETON(CApplication);

public:
	CApplication()
		: m_hInstance(nullptr), m_hWnd(nullptr),
		m_pRenderer(nullptr),
		m_pCamera(nullptr),
		m_bRunning(false),
		m_lWidth(1280), m_lHeight(720),
		m_bLockFPS(true), m_lFPSLock(1200){};

	~CApplication();

public:
	bool Initialize(HINSTANCE hInstance);
	int Run();
	HWND GetWindowHandle() const { return m_hWnd; }
private:
	bool RegisterWindowClass();
	bool CreateApplicationWindow();

	void Update();
	void Render();

	void Resize(int width, int height);
	void Shutdown();

private:
	static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	// --- Window ---
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	// --- Render ring ---
	CRenderer* m_pRenderer;
	CCamera* m_pCamera;

	// --- Application states ---
	bool m_bRunning;

	// --- Window size ---
	int m_lWidth;
	int m_lHeight;

	// --- FPS Limiter ---
	bool m_bLockFPS;
	int m_lFPSLock;
	LARGE_INTEGER m_liPerformanceFrequency;
	LARGE_INTEGER m_liFrameStart;

	// --- UI FPS Counter ---
	double m_fFrameTime;
	double m_fFPS;
	double m_fFPSTime;
	int m_lFPSFrames;
};