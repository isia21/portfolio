#include "stdafx.h"
#include "TestDataGenerator.h"
#include "../Engine/Graphics.h"

#include <direct.h>
#include <fstream>

void CTestDataGenerator::EnsureDirectories()
{
	_mkdir("Artifacts");
	_mkdir("Artifacts/Data");
}

void CTestDataGenerator::WritePlaneXML(const char* pszPath, float px, float py, float pz, float nx, float ny, float nz)
{
	pugi::xml_document doc;
	pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";

	pugi::xml_node planeNode = doc.append_child("Plane");

	pugi::xml_node pt = planeNode.append_child("Point");
	pt.append_attribute("x") = px;
	pt.append_attribute("y") = py;
	pt.append_attribute("z") = pz;

	pugi::xml_node norm = planeNode.append_child("Normal");
	norm.append_attribute("x") = nx;
	norm.append_attribute("y") = ny;
	norm.append_attribute("z") = nz;

	doc.save_file(pszPath, "    ");
}

void CTestDataGenerator::WritePlaneTXT(const char* pszPath, float px, float py, float pz, float nx, float ny, float nz)
{
	std::ofstream file(pszPath, std::ios::out | std::ios::trunc);
	if (file.is_open())
	{
		file << px << " " << py << " " << pz << " " << nx << " " << ny << " " << nz << "\n";
		file.close();
	}
}

void CTestDataGenerator::GenerateMeshes()
{
	// 1. Cube 2x2x2
	{
		C3DObject* pCube = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.0f);
		if (pCube)
		{
			pCube->SetName("Cube_Test");
			pCube->SaveToFile("Artifacts/Data/01_Cube.model.xml");
			pCube->ExportToOBJ("Artifacts/Data/01_Cube.obj");
			delete pCube;
		}
	}

	// 2. Sphere
	{
		C3DObject* pSphere = C3DObject::CreatePrimitive(C3DObject::ePR_Sphere, 2.0f);
		if (pSphere)
		{
			pSphere->SetName("Sphere_Test");
			pSphere->SaveToFile("Artifacts/Data/02_Sphere.model.xml");
			pSphere->ExportToOBJ("Artifacts/Data/02_Sphere.obj");
			delete pSphere;
		}
	}

	// 3. Sine Wave Surface
	{
		C3DObject* pSine = C3DObject::CreatePrimitive(C3DObject::ePR_SinePlane, 10.0f);
		if (pSine)
		{
			pSine->SetName("SineWave_NonConvex");
			pSine->SaveToFile("Artifacts/Data/03_SineWave_NonConvex.model.xml");
			pSine->ExportToOBJ("Artifacts/Data/03_SineWave_NonConvex.obj");
			delete pSine;
		}
	}
}

void CTestDataGenerator::GeneratePlanes()
{
	// Slicer Plane 1: Hor div (Y = 0.0, Normal = Up)
	WritePlaneXML("Artifacts/Data/Plane_Y_Zero.xml", 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Slicer Plane 2: Cut sin wave (Y = 0.6, Normal = Up) -> make N sep submeshes
	WritePlaneXML("Artifacts/Data/Plane_Wave_Cutter.xml", 0.0f, 0.6f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Slicer Plane 3: cut by 45 deg
	WritePlaneXML("Artifacts/Data/Plane_Diagonal_45Deg.xml", 0.0f, 0.0f, 0.0f, 0.7071f, 0.7071f, 0.0f);

	// Slicer Plane 4: Miss (plane to hight over object Y = 50.0)
	WritePlaneXML("Artifacts/Data/Plane_Miss.xml", 0.0f, 50.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Slicer Plane 5: (6 floats: px py pz nx ny nz)
	WritePlaneTXT("Artifacts/Data/Plane_Simple_Text.txt", 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void CTestDataGenerator::GenerateBatchScript()
{
	std::ofstream bat("Artifacts/Data/run_cli_tests.bat", std::ios::out | std::ios::trunc);
	if (!bat.is_open()) return;

	bat << "@echo off\n";
	bat << "set EXEPATH=..\\..\\..\\x64\\Debug\\0_Mamgma_Test.exe\n";
	bat << "if not exist %EXEPATH% set EXEPATH=..\\..\\..\\Debug\\0_Mamgma_Test.exe\n";
	bat << "if not exist %EXEPATH% set EXEPATH=..\\..\\..\\x64\\Release\\0_Mamgma_Test.exe\n";
	bat << "if not exist %EXEPATH% set EXEPATH=..\\..\\..\\Release\\0_Mamgma_Test.exe\n";
	bat << "if not exist %EXEPATH% set EXEPATH=0_Mamgma_Test.exe\n\n";

	bat << "echo =======================================================\n";
	bat << "echo  AUTOMATED CLI TEST BATCH SUITE\n";
	bat << "echo =======================================================\n\n";

	bat << "echo [TEST 1] Slicing Cube with Plane Y=0 (OBJ -> OBJ)...\n";
	bat << "%EXEPATH% 01_Cube.obj Plane_Y_Zero.xml out_cube_piece_\n\n";

	bat << "echo [TEST 2] Slicing Non-Convex Sine Wave into Multiple Islands...\n";
	bat << "%EXEPATH% 03_SineWave_NonConvex.obj Plane_Wave_Cutter.xml out_wave_island_\n\n";

	bat << "echo [TEST 3] Diagonal 45-degree cut on Sphere...\n";
	bat << "%EXEPATH% 02_Sphere.model.xml Plane_Diagonal_45Deg.xml out_sphere_diag_\n\n";

	bat << "echo [TEST 4] Plane Miss Test (No intersection)...\n";
	bat << "%EXEPATH% 01_Cube.obj Plane_Miss.xml out_cube_miss_\n\n";

	bat << "echo [TEST 5] Plain text plane format test...\n";
	bat << "%EXEPATH% 01_Cube.obj Plane_Simple_Text.txt out_cube_txt_plane_\n\n";

	bat << "echo =======================================================\n";
	bat << "echo  ALL BATCH TESTS FINISHED!\n";
	bat << "echo =======================================================\n";
	bat << "pause\n";

	bat.close();
}

void CTestDataGenerator::GenerateAll()
{
	EnsureDirectories();
	GenerateMeshes();
	GeneratePlanes();
	GenerateBatchScript();

	Utils::ODS("[DATA_GEN] Generated test datasets and batch runner in 'Artifacts/Data/'");
}