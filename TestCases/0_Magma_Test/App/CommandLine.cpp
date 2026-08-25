#include "stdafx.h"
#include "CommandLine.h"

#include "../Engine/Graphics.h"

#include <iostream>

void CCommandLine::PrintUsage()
{
	printf("\n=======================================================\n");
	printf(" MeshCut CLI - Headless 3D Mesh Slicing Tool\n");
	printf("=======================================================\n");
	printf("Usage:\n");
	printf("  MeshCut.exe <mesh_file> <plane_file> [output_prefix]\n\n");
	printf("Supported Mesh Formats:\n");
	printf("  - XML Model (*.model.xml)\n");
	printf("  - Wavefront OBJ (*.obj)\n\n");
	printf("Supported Plane Formats:\n");
	printf("  - XML Plane (<Plane><Point x=.. /><Normal x=.. /></Plane>)\n");
	printf("  - Plain text (6 floats: px py pz nx ny nz)\n\n");
	printf("Example:\n");
	printf("  MeshCut.exe Models/Cube.model.xml Plane.xml output_piece_\n");
	printf("=======================================================\n\n");
}

bool CCommandLine::ParsePlane(const char* pszFilePath, SPlane& outPlane)
{
	// 1. Try to parse as XML
	pugi::xml_document doc;
	if (doc.load_file(pszFilePath))
	{
		pugi::xml_node planeNode = doc.child("Plane");
		if (planeNode)
		{
			pugi::xml_node pt = planeNode.child("Point");
			pugi::xml_node norm = planeNode.child("Normal");

			if (pt && norm)
			{
				outPlane.Point = Vector3(
					pt.attribute("x").as_float(0.0f),
					pt.attribute("y").as_float(0.0f),
					pt.attribute("z").as_float(0.0f));

				outPlane.Normal = Vector3(
					norm.attribute("x").as_float(0.0f),
					norm.attribute("y").as_float(1.0f),
					norm.attribute("z").as_float(0.0f));

				outPlane.Normalize();
				return true;
			}
		}
	}

	// 2. Try to pars as TXT (6 float values as divided by \s or \t: px py pz nx ny nz)
	std::ifstream txtFile(pszFilePath);
	if (txtFile.is_open())
	{
		float px, py, pz, nx, ny, nz;
		if (txtFile >> px >> py >> pz >> nx >> ny >> nz)
		{
			outPlane = SPlane(Vector3(px, py, pz), Vector3(nx, ny, nz));
			return true;
		}
	}

	return false;
}

int CCommandLine::Execute(int argc, char* argv[])
{
	_mkdir("Out");

	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
	{
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
	}

	if (argc < 3)
	{
		PrintUsage();
		return EXIT_FAILURE;
	}

	const char* pszMeshPath = argv[1];
	const char* pszPlanePath = argv[2];
	const char* pszOutPrefix = (argc >= 4) ? argv[3] : "cut_part_";

	printf("\n[CLI] Starting headless slicing pipeline...\n");
	printf("[CLI] Loading mesh:  %s\n", pszMeshPath);
	printf("[CLI] Loading plane: %s\n", pszPlanePath);

	// 1. Load object Mesh (XML or OBJ)
	C3DObject sourceMesh;
	if (!sourceMesh.LoadFromFile(pszMeshPath))
	{
		printf("[CLI_ERROR] Failed to load mesh file: %s\n", pszMeshPath);
		return EXIT_FAILURE;
	}

	printf("[CLI] Mesh loaded successfully (%zu verts, %zu triangles)\n",
		sourceMesh.GetVertexCount(), sourceMesh.GetTriangleCount());

	// 2. Load slicer params
	SPlane cuttingPlane;
	if (!ParsePlane(pszPlanePath, cuttingPlane))
	{
		printf("[CLI_ERROR] Failed to parse cutting plane from: %s\n", pszPlanePath);
		return EXIT_FAILURE;
	}

	printf("[CLI] Cutting Plane: Point(%.2f, %.2f, %.2f) | Normal(%.2f, %.2f, %.2f)\n",
		cuttingPlane.Point.x, cuttingPlane.Point.y, cuttingPlane.Point.z,
		cuttingPlane.Normal.x, cuttingPlane.Normal.y, cuttingPlane.Normal.z);

	// 3. Run slicer n divide on separated islands (BFS)
	std::vector<C3DObject*> vCutParts;
	bool bSuccess = CMeshSlicer::Slice(&sourceMesh, cuttingPlane, vCutParts, true);

	if (!bSuccess || vCutParts.empty())
	{
		printf("[CLI_WARN] Plane did not intersect mesh or no parts generated.\n");
		printf("[CLI] Saving 1 original intact mesh: %s1.obj\n", pszOutPrefix);

		char szOutName[MAX_PATH] = {};
		sprintf_s(szOutName, sizeof(szOutName), "Out/%s1.obj", pszOutPrefix);
		sourceMesh.ExportToOBJ(szOutName);
		return EXIT_SUCCESS;
	}

	// 4. Export every divided sub mesh in self file 
	printf("[CLI_SUCCESS] Mesh cut into %zu disconnected pieces!\n", vCutParts.size());
	for (size_t i = 0; i < vCutParts.size(); ++i)
	{
		char szOutName[MAX_PATH] = {};
		sprintf_s(szOutName, sizeof(szOutName), "Out/%s%zu.obj", pszOutPrefix, i + 1);

		vCutParts[i]->ExportToOBJ(szOutName);
		printf("  -> Saved: %s (%zu vertices, %zu triangles)\n",
			szOutName, vCutParts[i]->GetVertexCount(), vCutParts[i]->GetTriangleCount());

		delete vCutParts[i];
	}

	printf("[CLI] Batch slicing completed successfully.\n\n");
	return EXIT_SUCCESS;
}