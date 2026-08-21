#pragma once

class CApplication
{
	DECLARE_SINGLETON(CApplication);

public:
	CApplication()
		: m_hInstance(nullptr), m_hWnd(nullptr), m_hDC(nullptr),
		m_hGLRC(nullptr), 
		m_bRunning(false),
		m_lWidth(1280), m_lHeight(720) {}

	~CApplication() { Shutdown(); }

public:
	bool Initialize(HINSTANCE hInstance);
	int Run();
	HWND GetWindowHandle() const { return m_hWnd; }
private:
	bool RegisterWindowClass();
	bool CreateApplicationWindow();

	bool CreateOpenGLContext();
	void Update();
	void Render();

	void Resize(int width, int height);
	void Shutdown();

private:
	static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE m_hInstance;

	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hGLRC;

	bool m_bRunning;

	int m_lWidth;
	int m_lHeight;
};