#include "stdafx.h"
#include "Mouse.h"

IMPLEMENT_SINGLETON(CMouse);

CMouse::CMouse()
	: m_lX(0)
	, m_lY(0)
	, m_lPreviousX(0)
	, m_lPreviousY(0)
	, m_lDeltaX(0)
	, m_lDeltaY(0)
	, m_lWheelDelta(0)
	, m_lWheelDeltaAccum(0)
{
	for (int i = 0; i < Button_Count; ++i)
	{
		m_bCurrentState[i] = false;
		m_bPreviousState[i] = false;
	}
}

CMouse::~CMouse()
{}

void CMouse::Update()
{
	// --- 1. Copy cur state as prev state ---
	for (int i = 0; i < Button_Count; ++i)
	{
		m_bPreviousState[i] = m_bCurrentState[i];
		m_bConsumed[i] = false;
	}
	m_bWheelConsumed = false;

	// --- 2. Calc mouse pos delta ---
	m_lDeltaX = m_lX - m_lPreviousX;
	m_lDeltaY = m_lY - m_lPreviousY;

	// --- 3. Update prev POINT to cur POINT, for use in next frame ---
	m_lPreviousX = m_lX;
	m_lPreviousY = m_lY;
	
	// --- 4. Reset n Accum Wheel Delta
	m_lWheelDelta = m_lWheelDeltaAccum;
	m_lWheelDeltaAccum = 0;
}

void CMouse::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		m_lX = static_cast<short>(LOWORD(lParam));
		m_lY = static_cast<short>(HIWORD(lParam));
		break;

	case WM_LBUTTONDOWN:
		m_bCurrentState[Button_Left] = true;
		break;
	case WM_LBUTTONUP:
		m_bCurrentState[Button_Left] = false;
		break;

	case WM_RBUTTONDOWN:
		m_bCurrentState[Button_Right] = true;
		break;
	case WM_RBUTTONUP:
		m_bCurrentState[Button_Right] = false;
		break;

	case WM_MBUTTONDOWN:
		m_bCurrentState[Button_Middle] = true;
		break;
	case WM_MBUTTONUP:
		m_bCurrentState[Button_Middle] = false;
		break;

	case WM_MOUSEWHEEL:
		m_lWheelDeltaAccum += GET_WHEEL_DELTA_WPARAM(wParam);
		break;
	}
}

bool CMouse::IsButtonDown(EMouseButton button) const 
{
	if (button < 0 || button >= Button_Count) 
		return false;
	return m_bCurrentState[button] 
		&& !m_bConsumed[button];
}

bool CMouse::IsButtonPressed(EMouseButton button) const 
{
	if (button < 0 || button >= Button_Count) 
		return false;
	return 
		m_bCurrentState[button]
		&& !m_bPreviousState[button] 
		&& !m_bConsumed[button];
}

bool CMouse::IsButtonReleased(EMouseButton button) const 
{
	if (button < 0 || button >= Button_Count) 
		return false;
	return !m_bCurrentState[button] 
		&& m_bPreviousState[button] 
		&& !m_bConsumed[button];
}

bool CMouse::IsButtonDownRaw(EMouseButton button) const 
{
	if (button < 0 || button >= Button_Count) 
		return false;
	return m_bCurrentState[button];
}

void CMouse::ConsumeButton(EMouseButton button)
{
	if (button >= 0 && button < Button_Count)
		m_bConsumed[button] = true;
}

void CMouse::ConsumeWheel()
{
	m_bWheelConsumed = true;
}

int CMouse::GetWheelDelta() const {
	return m_bWheelConsumed ? 0 : m_lWheelDelta;
}