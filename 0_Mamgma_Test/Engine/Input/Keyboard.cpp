#include "stdafx.h"
#include "Keyboard.h"

IMPLEMENT_SINGLETON(CKeyboard);

CKeyboard::CKeyboard()
{
	for (int i = 0; i < 256; ++i)
	{
		m_bCurrentState[i] = false;
		m_bPreviousState[i] = false;
	}
}

CKeyboard::~CKeyboard()
{}

void CKeyboard::Update()
{
	// --- Copy cur state as prev state (for check state switch: btn.pressed | btn.up => hold | click)
	for (int i = 0; i < 256; ++i)
		m_bPreviousState[i] = m_bCurrentState[i];
}

void CKeyboard::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (wParam < 0 || wParam > 255)
		return;

	switch (message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		m_bCurrentState[wParam] = true;
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		m_bCurrentState[wParam] = false;
		break;
	}
}

bool CKeyboard::IsKeyDown(int key) const
{
	if (key < 0 || key > 255) return false;
	return m_bCurrentState[key];
}

bool CKeyboard::IsKeyPressed(int key) const
{
	if (key < 0 || key > 255) return false;
	return m_bCurrentState[key] && !m_bPreviousState[key];
}

bool CKeyboard::IsKeyReleased(int key) const
{
	if (key < 0 || key > 255) return false;
	return !m_bCurrentState[key] && m_bPreviousState[key];
}