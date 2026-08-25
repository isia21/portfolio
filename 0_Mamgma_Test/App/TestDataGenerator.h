#pragma once

class CTestDataGenerator
{
public:
	// --- Create test Meshes, Planes n .bar script to Artifacts/Data/
	static void GenerateAll();

private:
	static void EnsureDirectories();
	static void GenerateMeshes();
	static void GeneratePlanes();
	static void GenerateBatchScript();

	static void WritePlaneXML(const char* pszPath, float px, float py, float pz, float nx, float ny, float nz);
	static void WritePlaneTXT(const char* pszPath, float px, float py, float pz, float nx, float ny, float nz);
};