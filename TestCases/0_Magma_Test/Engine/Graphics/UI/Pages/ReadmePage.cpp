#include "stdafx.h"
#include "../../../Graphics.h"
#include "../../../../App/Application.h"
#include "ReadmePage.h"


const int lHelpWidth = 280;
const int lHelpHeight = 230;

const int lHelpX = 10;
const int lHelpY = 460; 


CReadmePage::CReadmePage() :
	CUIWindow(
		lHelpX, lHelpY,
		lHelpWidth, lHelpHeight,
		"Controls Cheat Sheet [F1]",
		0x1E1E1EF2, 0x282828FF, 0x454545FF, 1)
{}

CReadmePage::~CReadmePage() {}

//==========================

void CReadmePage::Init()
{
	const SShortcutHint hints[] = {
		{ "RMB + Drag",     "Orbit Camera (Rotate)",			0x00FFCCFF },
		{ "WASD + Shift",   "Fly / Move Focus Point",			0x00FFCCFF },
		{ "Q / E",          "Elevate Focus (Up/Down)",			0x00FFCCFF },
		{ "Mouse Wheel",    "Zoom In / Out",					0x00FFCCFF },
		{ "CTRL + S/O/N/E", "Scene Save/Open/New/Export",		0x00FFCCFF },
		{ "Ctrl + LMB Drag","Gesture Quick Cut",				0xFF5555FF },
		{ "Tree / Sliders", "Inspect & Rotate Normals",			0xFFD700FF },
		{ "F1",             "Toggle This Help Window",			0xAAAAAAFF },
	};

	int curY = 30;
	for (const auto& hint : hints)
	{
		CUITextBox* pKeyBox = new CUITextBox(
			6, curY, 100, 20,
			hint.szKey,
			0x141414AA, hint.dwColor, 11, TEXT_ALIGN_LEFT);
		m_vTxtKey.push_back(pKeyBox);
		AddChild(pKeyBox);

		CUITextBox* pDescBox = new CUITextBox(
			110, curY, 164, 20,
			hint.szDesc,
			0x00000000, 0xDDDDDDFF, 11, TEXT_ALIGN_LEFT);
		m_vTxtText.push_back(pKeyBox);
		AddChild(pDescBox);

		curY += 24;
	}
}
