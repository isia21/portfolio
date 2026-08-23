#pragma once
class CRenderer;
class C3DObject;
class CWorld;
class CUIManager;
class CUITextBox;
class CUIButton;
class CUISmartTree;
class CUIWindow;

class CScenePage;
class CObjectInspectorPage;


class CApplication
{
	DECLARE_SINGLETON(CApplication);

public:
	CApplication()
		: m_hInstance(nullptr), m_hWnd(nullptr),
		m_pRenderer(nullptr),
		m_pWorld(nullptr),

		m_pUIManager(nullptr),
		m_pUIScenePage(nullptr), m_pUIObjectInspectorPage(nullptr),

		m_bRunning(false),
		m_pSelectedObject(nullptr),
		m_lWidth(1280), m_lHeight(720),
		m_bLockFPS(true), m_lFPSLock(1200){};

	~CApplication();

public:
	bool Initialize(HINSTANCE hInstance);
	int Run();
	HWND GetWindowHandle() const { return m_hWnd; }

	int GetWidth() const { return m_lWidth; }
	int GetHeight() const { return m_lHeight; }

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

	// --- Current selected object ---
	C3DObject* m_pSelectedObject;

	// --- Render ring ---
	CRenderer* m_pRenderer;
	CWorld* m_pWorld;
	CUIManager* m_pUIManager;
	CUITextBox* m_pFPSTextBox;

	// --- UI Pages ---
	CScenePage* m_pUIScenePage;
	CObjectInspectorPage* m_pUIObjectInspectorPage;

public:
	CWorld* GetWorld() const { return m_pWorld; }

	C3DObject* GetSelectedObject() const { return m_pSelectedObject; }
	void SetSelectedObject(C3DObject* pObj) { /*if (pObj)*/ m_pSelectedObject = pObj; }

	CScenePage* GetScenePage() const { return m_pUIScenePage; }
	CObjectInspectorPage* GetObjectInspectorPage() const { return m_pUIObjectInspectorPage; }
};