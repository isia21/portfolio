#pragma once
class C3DObject;

//-----------------------------------------------------------------------------
// Render layer
//-----------------------------------------------------------------------------
enum ERenderLayer
{
	RENDER_LAYER_3D = 0,
	RENDER_LAYER_2D
};


//-----------------------------------------------------------------------------
// Render commands for 2D layer / UI layer
//-----------------------------------------------------------------------------
enum ERenderCommand2DType
{
	RENDER_COMMAND_RECT = 0,
	RENDER_COMMAND_TEXT
};

enum ETextAlignment
{
	TEXT_ALIGN_LEFT = 0,
	TEXT_ALIGN_CENTER,
	TEXT_ALIGN_RIGHT
};

struct RenderCommand2D
{
	ERenderCommand2DType eType;

	union
	{
		struct
		{
			int lX;
			int lY;
			int lWidth;
			int lHeight;
			unsigned int dwColor;
		} Rect;

		struct
		{
			int lX;
			int lY;
			unsigned int dwColor;
			const char* pszText;
			int lFontSize;
			ETextAlignment eAlignment;
		} Text;
	};
};
//-----------------------------------------------------------------------------
// Renderer
//-----------------------------------------------------------------------------
class CRenderer
{
public:
	CRenderer();
	~CRenderer();

public:
	bool Init(HWND hWnd, int lRenderViewportWidth, int lRenderViewportHeight, int bLockFPS, int lFPSLock = 60);

	void Shutdown();

private:
	void RenderGrid();

	void RenderRect(const RenderCommand2D& command);
	void RenderText(const RenderCommand2D& command);

public:
	void Clear();
	void Render();

	void Render3D();
	void Render2D();

	void Draw(C3DObject* pObject);
	void DrawRect(int lX, int lY, int lWidth, int lHeight, unsigned int dwColor);
	void DrawText(int lX, int lY, const char* pszText, unsigned int dwColor, int lFontSize = 14, ETextAlignment eAlignment = TEXT_ALIGN_LEFT);

	void SetGridStep(float fStep);

	void Resize(int lWidth, int lHeight);

	bool IsInitialized() const { return m_bInitialized; }

private:
	void ClearRenderQueues();

	bool InitOpenGL();
	void ShutdownOpenGL();

	bool InitPixelFormat();
	bool InitRenderContext();

private:
	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hGLRC;

	int m_lWndWidth;
	int m_lWndHeight;

	int m_lViewportWidth;
	int m_lViewportHeight;

	int m_lWndPosX;
	int m_lWndPosY;

	bool m_bFullScreen;
	bool m_bLockFPS;

	int m_lFPSLock;

	bool m_bInitialized;

	float m_fGridStep;

	std::vector<C3DObject*> m_vObjects3D;
	std::vector<RenderCommand2D> m_vLayer2D;
};