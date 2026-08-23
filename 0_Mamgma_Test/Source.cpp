#include "stdafx.h"

#include "App/Application.h"
#include "Engine/Input.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int)
{
	CKeyboard::InitInstance();
	CMouse::InitInstance();
	CApplication::InitInstance();
	CApplication* pApp = CApplication::GetInstance();
	Utils::Init(pApp->GetWindowHandle());

	if (!pApp->Initialize(hInstance))
	{
		Utils::Message("[ERROR] Failed to initialize application.");
		return EXIT_FAILURE;
	}

	return pApp->Run();


	//return S_OK;
}