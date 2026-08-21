#include "stdafx.h"
#include "Renderer.h"
#include "3DObject.h"

#include <vector>

//-----------------------------------------------------------------------------
// Renderer
//-----------------------------------------------------------------------------
CRenderer::CRenderer()
	: m_hWnd(nullptr)
	, m_hDC(nullptr)
	, m_hGLRC(nullptr)
	, m_lWndWidth(0)
	, m_lWndHeight(0)
	, m_lViewportWidth(0)
	, m_lViewportHeight(0)
	, m_lWndPosX(0)
	, m_lWndPosY(0)
	, m_bFullScreen(false)
	, m_bLockFPS(false)
	, m_lFPSLock(60)
	, m_bInitialized(false)
	, m_fGridStep(1.0f)
{}

CRenderer::~CRenderer()
{
	Shutdown();
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CRenderer::Init(HWND hWnd, int lRenderViewportWidth, int lRenderViewportHeight, int bLockFPS, int lFPSLock)
{
	if (hWnd == nullptr)
		return false;

	if (lRenderViewportWidth <= 0 || lRenderViewportHeight <= 0)
		return false;

	m_hWnd = hWnd;

	m_lViewportWidth = lRenderViewportWidth;
	m_lViewportHeight = lRenderViewportHeight;

	RECT rect = {};

	if (GetClientRect(m_hWnd, &rect))
	{
		m_lWndWidth = rect.right - rect.left;
		m_lWndHeight = rect.bottom - rect.top;
	}

	m_bLockFPS = bLockFPS;
	m_lFPSLock = lFPSLock;

	if (!InitOpenGL())
	{
		ShutdownOpenGL();
		return false;
	}

	m_bInitialized = true;

	return true;
}

void CRenderer::Shutdown()
{
	if (!m_bInitialized && m_hGLRC == nullptr && m_hDC == nullptr)
		return;

	ClearRenderQueues();
	ShutdownOpenGL();

	m_hWnd = nullptr;

	m_lWndWidth = 0;
	m_lWndHeight = 0;

	m_lViewportWidth = 0;
	m_lViewportHeight = 0;

	m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// OpenGL initialization
//-----------------------------------------------------------------------------
bool CRenderer::InitOpenGL()
{
	if (m_hWnd == nullptr)
		return false;

	m_hDC = GetDC(m_hWnd);

	if (m_hDC == nullptr)
		return false;

	if (!InitPixelFormat())
		return false;

	if (!InitRenderContext())
		return false;

	glViewport(0, 0, m_lViewportWidth, m_lViewportHeight);

	glClearColor(0.08f, 0.08f, 0.10f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	return true;
}

bool CRenderer::InitPixelFormat()
{
	PIXELFORMATDESCRIPTOR pfd = {};

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	pfd.iPixelType = PFD_TYPE_RGBA;

	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;

	pfd.iLayerType = PFD_MAIN_PLANE;

	const int lPixelFormat = ChoosePixelFormat(m_hDC, &pfd);

	if (lPixelFormat == 0)
		return false;

	if (!SetPixelFormat(m_hDC, lPixelFormat, &pfd))
		return false;

	return true;
}

bool CRenderer::InitRenderContext()
{
	m_hGLRC = wglCreateContext(m_hDC);

	if (m_hGLRC == nullptr)
		return false;

	if (!wglMakeCurrent(m_hDC, m_hGLRC))
	{
		wglDeleteContext(m_hGLRC);
		m_hGLRC = nullptr;

		return false;
	}

	return true;
}

void CRenderer::ShutdownOpenGL()
{
	if (m_hGLRC != nullptr)
	{
		if (wglGetCurrentContext() == m_hGLRC)
			wglMakeCurrent(nullptr, nullptr);

		wglDeleteContext(m_hGLRC);
		m_hGLRC = nullptr;
	}

	if (m_hDC != nullptr && m_hWnd != nullptr)
	{
		ReleaseDC(m_hWnd, m_hDC);
		m_hDC = nullptr;
	}
}

//-----------------------------------------------------------------------------
// Frame
//-----------------------------------------------------------------------------
void CRenderer::Clear()
{
	if (!m_bInitialized)
		return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void CRenderer::Render()
{
	if (!m_bInitialized)
		return;

	RenderGrid();
	Render3D();
	Render2D();

	SwapBuffers(m_hDC);

	ClearRenderQueues();
}

//-----------------------------------------------------------------------------
// 3D
//-----------------------------------------------------------------------------
void CRenderer::Render3D()
{
	for (std::vector<C3DObject*>::const_iterator it = m_vObjects3D.begin();
		it != m_vObjects3D.end();
		++it)
	{
		C3DObject* pObject = *it;

		if (pObject == nullptr)
			continue;

		pObject->Render();
	}
}

//-----------------------------------------------------------------------------
// Grid
//-----------------------------------------------------------------------------
void CRenderer::SetGridStep(float fStep)
{
	if (fStep <= 0.0f)
		return;

	m_fGridStep = fStep;
}


void CRenderer::RenderGrid()
{
	if (!m_bInitialized)
		return;

	if (m_fGridStep <= 0.0f)
		return;

	const float fGridSize = 100.0f;

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.35f, 0.35f, 0.35f);
	
	glBegin(GL_LINES);

	for (float f = -fGridSize; f <= fGridSize; f += m_fGridStep)
	{
		// X direction
		glVertex3f(-fGridSize, 0.0f, f);
		glVertex3f(fGridSize, 0.0f, f);

		// Z direction
		glVertex3f(f, 0.0f, -fGridSize);
		glVertex3f(f, 0.0f, fGridSize);
	}
	glEnd();

	//-------------------------------------------------------------------------
	glBegin(GL_LINES);

	// X axis
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-fGridSize, 0.0f, 0.0f);
	glVertex3f(fGridSize, 0.0f, 0.0f);

	// Z axis
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, -fGridSize);
	glVertex3f(0.0f, 0.0f, fGridSize);

	glEnd();
}


//-----------------------------------------------------------------------------
// Draw requests
//-----------------------------------------------------------------------------

void CRenderer::Draw(C3DObject* pObject)
{
	if (pObject == nullptr)
		return;

	m_vObjects3D.push_back(pObject);
}


void CRenderer::DrawRect(int lX, int lY, int lWidth, int lHeight,
	unsigned int dwColor)
{
	RenderCommand2D command = {};

	command.eType = RENDER_COMMAND_RECT;

	command.Rect.lX = lX;
	command.Rect.lY = lY;
	command.Rect.lWidth = lWidth;
	command.Rect.lHeight = lHeight;
	command.Rect.dwColor = dwColor;

	m_vLayer2D.push_back(command);
}


void CRenderer::DrawText(int lX, int lY, const char* pszText,
	unsigned int dwColor, int lFontSize, ETextAlignment eAlignment)
{
	if (pszText == nullptr)
		return;

	RenderCommand2D command = {};

	command.eType = RENDER_COMMAND_TEXT;

	command.Text.lX = lX;
	command.Text.lY = lY;
	command.Text.dwColor = dwColor;
	command.Text.pszText = pszText;
	command.Text.lFontSize = lFontSize;
	command.Text.eAlignment = eAlignment;

	m_vLayer2D.push_back(command);
}


//-----------------------------------------------------------------------------
// 2D
//-----------------------------------------------------------------------------
void CRenderer::Render2D()
{
	if (m_vLayer2D.empty())
		return;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0.0, static_cast<double>(m_lViewportWidth), static_cast<double>(m_lViewportHeight), 0.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	for (std::vector<RenderCommand2D>::const_iterator it = m_vLayer2D.begin();
		it != m_vLayer2D.end();
		++it)
	{
		const RenderCommand2D& command = *it;

		switch (command.eType)
		{
		case RENDER_COMMAND_RECT:
			RenderRect(command);
			break;

		case RENDER_COMMAND_TEXT:
			RenderText(command);
			break;

		default:
			break;
		}
	}

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
}


void CRenderer::RenderRect(const RenderCommand2D& command)
{
	const float fR = static_cast<float>((command.Rect.dwColor >> 24) & 0xFF) / 255.0f;
	const float fG = static_cast<float>((command.Rect.dwColor >> 16) & 0xFF) / 255.0f;
	const float fB = static_cast<float>((command.Rect.dwColor >> 8) & 0xFF) / 255.0f;
	const float fA = static_cast<float>(command.Rect.dwColor & 0xFF) / 255.0f;
	glColor4f(fR, fG, fB, fA);

	glBegin(GL_QUADS);

	glVertex2i(command.Rect.lX, command.Rect.lY);
	glVertex2i(command.Rect.lX + command.Rect.lWidth, command.Rect.lY);
	glVertex2i(command.Rect.lX + command.Rect.lWidth, command.Rect.lY + command.Rect.lHeight);
	glVertex2i(command.Rect.lX, command.Rect.lY + command.Rect.lHeight);

	glEnd();
}


void CRenderer::RenderText(const RenderCommand2D& command)
{
	if (command.Text.pszText == nullptr)
		return;

	if (command.Text.lFontSize <= 0)
		return;

	HFONT hFont = CreateFontA(
		-command.Text.lFontSize,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		"Arial");

	if (hFont == nullptr)
		return;

	HFONT hOldFont = static_cast<HFONT>(SelectObject(m_hDC, hFont));

	const GLuint lFontBase = glGenLists(256);
	if (lFontBase == 0)
	{
		SelectObject(m_hDC, hOldFont);
		DeleteObject(hFont);

		return;
	}

	if (!wglUseFontBitmapsA(m_hDC, 0, 256, lFontBase))
	{
		glDeleteLists(lFontBase, 256);

		SelectObject(m_hDC, hOldFont);
		DeleteObject(hFont);

		return;
	}

	const float fR = static_cast<float>((command.Text.dwColor >> 24) & 0xFF) / 255.0f;
	const float fG = static_cast<float>((command.Text.dwColor >> 16) & 0xFF) / 255.0f;
	const float fB = static_cast<float>((command.Text.dwColor >> 8) & 0xFF) / 255.0f;
	const float fA = static_cast<float>(command.Text.dwColor & 0xFF) / 255.0f;
	glColor4f(fR, fG, fB, fA);

	SIZE textSize = {};
	GetTextExtentPoint32A(m_hDC, command.Text.pszText, static_cast<int>(strlen(command.Text.pszText)), &textSize);

	int lX = command.Text.lX;
	switch (command.Text.eAlignment)
	{
	case TEXT_ALIGN_CENTER:
		lX -= textSize.cx / 2;
		break;

	case TEXT_ALIGN_RIGHT:
		lX -= textSize.cx;
		break;

	case TEXT_ALIGN_LEFT:
	default:
		break;
	}

	glRasterPos2i(lX, command.Text.lY);

	glListBase(lFontBase);
	glCallLists(static_cast<GLsizei>(strlen(command.Text.pszText)), GL_UNSIGNED_BYTE, command.Text.pszText);
	glDeleteLists(lFontBase, 256);

	SelectObject(m_hDC, hOldFont);
	DeleteObject(hFont);
}


//-----------------------------------------------------------------------------
// Queue
//-----------------------------------------------------------------------------

void CRenderer::ClearRenderQueues()
{
	m_vObjects3D.clear();
	m_vLayer2D.clear();
}


//-----------------------------------------------------------------------------
// Resize
//-----------------------------------------------------------------------------

void CRenderer::Resize(int lWidth, int lHeight)
{
	if (lWidth <= 0 || lHeight <= 0)
		return;

	m_lWndWidth = lWidth;
	m_lWndHeight = lHeight;

	m_lViewportWidth = lWidth;
	m_lViewportHeight = lHeight;

	if (!m_bInitialized)
		return;

	glViewport(0, 0, m_lViewportWidth, m_lViewportHeight);
}