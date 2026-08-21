#pragma once
class CRenderer;

class CApplication
{
	DECLARE_SINGLETON(CApplication);

public:
	CApplication()
		: m_hInstance(nullptr), m_hWnd(nullptr),
		m_pRenderer(nullptr),
		m_bRunning(false),
		m_lWidth(1280), m_lHeight(720) {};

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
	HINSTANCE m_hInstance;
	CRenderer* m_pRenderer;

	HWND m_hWnd;

	bool m_bRunning;

	int m_lWidth;
	int m_lHeight;
};