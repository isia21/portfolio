#pragma once

class CUnitTester
{
public:
	// --- Run all tests w out in ODS (n log file) ---
	static bool RunAllTests();

private:
	// --- Test Cases --
	static bool TestPlaneMath();
	static bool TestPlaneMiss();
	static bool TestCubeSliceSimple();
	static bool TestSinePlaneMultipleIslands();
	static bool TestPlaneVertexBoundary();
	static bool TestSlicerRotationToNormal();
	static bool TestOBJExportImportIntegrity();
};