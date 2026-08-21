#pragma once

class CApplication
{
public:
	CApplication(HINSTANCE hInstance)
		: m_hInstance(hInstance), m_hWnd(nullptr), m_hDC(nullptr), 
		m_hGLRC(nullptr), 
		m_bRunning(false),
		m_width(1280), m_height(720) {}

	~CApplication() { Shutdown(); }

public:
	bool Initialize();
	int Run();

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

	int m_width;
	int m_height;
};