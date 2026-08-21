#include "stdafx.h"

#include "App/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int)
{
	CApplication application(hInstance);

	if (!application.Initialize())
	{
		MessageBox(nullptr, "Failed to initialize application.", "Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	return application.Run();
}