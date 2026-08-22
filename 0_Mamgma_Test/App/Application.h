#pragma once
class CRenderer;
class C3DObject;
class CWorld;
class CUIManager;
class CUITextBox;
class CUIButton;
class CUISmartTree;
class CUIWindow;

class CApplication
{
	DECLARE_SINGLETON(CApplication);

public:
	CApplication()
		: m_hInstance(nullptr), m_hWnd(nullptr),
		m_pRenderer(nullptr),
		m_pWorld(nullptr),
		m_pUIManager(nullptr),
		m_bRunning(false),
		
		m_pSelectedObject(nullptr),
		m_pInspectorWindow(nullptr),
		m_pTxtInspectorName(nullptr),
		m_pTxtInspectorStats(nullptr),
		m_pTxtInspectorTransform(nullptr),
		m_pBtnToggleWireframe(nullptr),
		m_pBtnDeleteObject(nullptr),

		m_lWidth(1280), m_lHeight(720),
		m_bLockFPS(true), m_lFPSLock(1200){};

	~CApplication();

public:
	bool Initialize(HINSTANCE hInstance);
	int Run();
	HWND GetWindowHandle() const { return m_hWnd; }

	void RebuildSceneTree();
	void UpdateInspector();

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
	CUISmartTree* m_pSceneTree;

	// --- Inspector Window elements ---
	CUIWindow* m_pInspectorWindow;
	CUITextBox* m_pTxtInspectorName;
	CUITextBox* m_pTxtInspectorStats;
	CUITextBox* m_pTxtInspectorTransform;
	CUIButton* m_pBtnToggleWireframe;
	CUIButton* m_pBtnDeleteObject;
};