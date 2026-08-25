#include "stdafx.h"

#include "App/Application.h"
#include "App/CommandLine.h"
#include "App/TestDataGenerator.h"
#include "App/Tests.h"
#include "Engine/Input.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int)
{

#ifdef _DEBUG
	// --- 1. Create test data in Artifacts/Data/ ---
	//	CTestDataGenerator::GenerateAll();

#endif
	Utils::ODS(" =================");
	Utils::ODS(" === APP START ===");
	Utils::ODS(" =================\n\n");
	// --- 2. Run all unit tests ---
	CUnitTester::RunAllTests();

	// --- 3. Try to process data passed as command line args ---
	// In case passed args is: app.exe mesh.obj plane.xml [prefix]
	if (__argc >= 3)
		return CCommandLine::Execute(__argc, __argv);


	// --- 3. Init app sub systems ---
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

	int result = pApp->Run();

	CApplication::ExitInstance();
	CMouse::ExitInstance();
	CKeyboard::ExitInstance();

	return result;
}