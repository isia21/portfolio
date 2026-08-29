#include "stdafx.h"
#include "Tests.h"
#include "../Engine/Graphics.h"

#include "../Engine/Math/Vector3.h"
#include "../Engine/Math/Math.h"

#include <cmath>

namespace
{
	int g_nPassedTests = 0;
	int g_nFailedTests = 0;

#define TEST_CHECK(condition, message) \
		do { \
			if (!(condition)) { \
				Utils::ODS("[TEST_FAILED] Line %d: %s", __LINE__, message); \
				return false; \
			} \
		} while(0)

#define RUN_TEST(testFunc, testName) \
		do { \
			Utils::ODS("[TEST_RUN] Running: %s...", testName); \
			if (testFunc()) { \
				Utils::ODS("[TEST_OK] %s PASSED", testName); \
				g_nPassedTests++; \
			} else { \
				Utils::ODS("[TEST_ERROR] %s FAILED!", testName); \
				g_nFailedTests++; \
			} \
		} while(0)
}

// -----------------------------------------------------------------------------
// Тест 1: Математика плоскости SPlane
// -----------------------------------------------------------------------------
bool CUnitTester::TestPlaneMath()
{
	// Горизонтальная плоскость на высоте Y = 2.0 с нормалью вверх (0, 1, 0)

	Vector3 vM0(0.0f, 2.0f, 0.0f);
	Vector3 vN(0.0f, 1.0f, 0.0f);

	SPlane plane(vM0, vN);

	Vector3 ptAbove(0.0f, 5.0f, 0.0f);
	Vector3 ptBelow(0.0f, 0.0f, 0.0f);
	Vector3 ptOnPlane(10.0f, 2.0f, -5.0f);
	// Старые проверки
	{
		TEST_CHECK(fabs(plane.GetSignedDistance(ptAbove) - 3.0f) < 1e-5f, "[OLD] Distance above must be +3.0");
		TEST_CHECK(fabs(plane.GetSignedDistance(ptBelow) - (-2.0f)) < 1e-5f, "[OLD] Distance below must be -2.0");
		TEST_CHECK(fabs(plane.GetSignedDistance(ptOnPlane)) < 1e-5f, "[OLD] Distance on plane must be 0.0");

		TEST_CHECK(plane.GetSide(ptAbove) == 1, "[OLD] Side above must be +1");
		TEST_CHECK(plane.GetSide(ptBelow) == -1, "[OLD] Side below must be -1");
		TEST_CHECK(plane.GetSide(ptOnPlane) == 0, "[OLD] Side on plane must be 0");

		// Проверка поиска точки среза на отрезке [ptBelow, ptAbove]
		float d0 = plane.GetSignedDistance(ptBelow); // -2
		float d1 = plane.GetSignedDistance(ptAbove); // +3
		Vector3 intersect = plane.GetIntersectionPoint(ptBelow, ptAbove, d0, d1);

		TEST_CHECK(fabs(intersect.y - 2.0f) < 1e-5f, "[OLD] Intersection Y must be exactly 2.0");
	}
	// Новые
	{
		TEST_CHECK(fabs(PointPlaneDistance(vM0, vN, ptAbove) - 3.0f) < 1e-5f, "[NEW] Distance above must be +3.0");
		TEST_CHECK(fabs(PointPlaneDistance(vM0, vN, ptBelow) - (-2.0f)) < 1e-5f, "[NEW] Distance below must be -2.0");
		TEST_CHECK(fabs(PointPlaneDistance(vM0, vN, ptOnPlane)) < 1e-5f, "[NEW] Distance on plane must be 0.0");

		TEST_CHECK(PointSide(vM0, vN, ptAbove) == 1, "[NEW] Side above must be +1");
		TEST_CHECK(PointSide(vM0, vN, ptBelow) == -1, "[NEW] Side below must be -1");
		TEST_CHECK(PointSide(vM0, vN, ptOnPlane) == 0, "[NEW] Side on plane must be 0");

		// Проверка поиска точки среза на отрезке [ptBelow, ptAbove]
		float d0 = PointPlaneDistance(vM0, vN, ptBelow); // -2
		float d1 = PointPlaneDistance(vM0, vN, ptAbove); // +3
		Vector3 intersect = IntersectPlane(vM0, vN, ptBelow, ptAbove);

		TEST_CHECK(fabs(intersect.y - 2.0f) < 1e-5f, "[NEW] Intersection Y must be exactly 2.0");
	}
	return true;
}

// -----------------------------------------------------------------------------
// Тест 2: Промах плоскости (секущая плоскость вне объекта)
// -----------------------------------------------------------------------------
bool CUnitTester::TestPlaneMiss()
{
	// Куб размером 2x2x2 (от -1 до +1 по всем осям)

	// Плоскость лежит на высоте Y = 5.0 (выше куба)
	Vector3 vM0(0.0f, 5.0f, 0.0f);
	Vector3 vN(0.0f, 1.0f, 0.0f);

	SPlane highPlane(vM0, vN);

	{
		C3DObject* pCube = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.0f);
		TEST_CHECK(pCube != nullptr, "Failed to create cube");

		std::vector<C3DObject*> parts;
		bool bCutResultOld = CMeshSlicer::Slice(pCube, highPlane, parts, true);

		delete pCube;
		for (auto p : parts) delete p;

		TEST_CHECK(bCutResultOld == false, "[OLD] Slicer must return false on non-intersecting plane");
		TEST_CHECK(parts.empty(), "[OLD] Parts vector must remain empty on miss");
	}

	{
		C3DObject* pCube = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.0f);
		TEST_CHECK(pCube != nullptr, "Failed to create cube");

		int lNewPartsCount = SliceMesh(*pCube, pCube->GetPosition(), pCube->GetRotation(), pCube->GetScale(), vM0, vN);

		const std::vector<C3DObject*> & parts = pCube->GetChildren();
		//Если после слайсинга у нас появились сбамеши(дочерних > 0), значит плоскость пересекла наш объект
		bool bCutResultNew = lNewPartsCount > 0;
		

		delete pCube;
		for (auto p : parts) 
			delete p;

		TEST_CHECK(bCutResultNew == false, "[NEW] Slicer must return false on non-intersecting plane");
		TEST_CHECK(parts.empty(), "[NEW] Parts vector must remain empty on miss");

	}
	return true;
}

// -----------------------------------------------------------------------------
// Тест 3: Разрез куба ровно пополам
// -----------------------------------------------------------------------------
bool CUnitTester::TestCubeSliceSimple()
{
	C3DObject* pCube = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.0f);
	TEST_CHECK(pCube != nullptr, "Failed to create cube");

	// Горизонтальная плоскость ровно через центр куба Y = 0
	SPlane plane(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));

	std::vector<C3DObject*> parts;
	bool bCutResult = CMeshSlicer::Slice(pCube, plane, parts, false); // без разделения на BFS-островки

	TEST_CHECK(bCutResult == true, "Slice must succeed");
	TEST_CHECK(parts.size() == 2, "Expected exactly 2 parts (Upper and Lower)");

	C3DObject* pUpper = parts[0];
	C3DObject* pLower = parts[1];

	TEST_CHECK(pUpper->GetTriangleCount() > 0, "Upper part must contain triangles");
	TEST_CHECK(pLower->GetTriangleCount() > 0, "Lower part must contain triangles");
	TEST_CHECK(pUpper->GetIndices().size() % 3 == 0, "Upper indices must be multiple of 3");
	TEST_CHECK(pLower->GetIndices().size() % 3 == 0, "Lower indices must be multiple of 3");

	delete pCube;
	for (auto p : parts) delete p;
	return true;
}

// -----------------------------------------------------------------------------
// Тест 4: Невыпуклая синусоида (проверка разделения на N > 2 несвязных островков)
// -----------------------------------------------------------------------------
bool CUnitTester::TestSinePlaneMultipleIslands()
{
	// Создаем невыпуклую волнообразную поверхность (64x64 полигонов)
	C3DObject* pSine = C3DObject::CreatePrimitive(C3DObject::ePR_SinePlane, 10.0f);
	TEST_CHECK(pSine != nullptr, "Failed to create sine plane");

	// Плоскость режет гребни волн по высоте Y = 0.5
	SPlane cutPlane(Vector3(0.0f, 0.5f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));

	std::vector<C3DObject*> generatedIslands;
	bool bCutResult = CMeshSlicer::Slice(pSine, cutPlane, generatedIslands, true); // с разделением BFS

	TEST_CHECK(bCutResult == true, "Slicing non-convex wave must succeed");
	// При разрезе синусоиды на высоте Y=0.5 образуется множество изолированных верхушек волн
	TEST_CHECK(generatedIslands.size() > 2, "Complex wave slice must produce multiple (>2) disconnected sub-meshes");

	for (C3DObject* pIsland : generatedIslands)
	{
		TEST_CHECK(pIsland->GetVertexCount() > 0, "Each island must have vertices");
		TEST_CHECK(pIsland->GetTriangleCount() > 0, "Each island must have triangles");
	}

	delete pSine;
	for (auto p : generatedIslands) delete p;
	return true;
}

// -----------------------------------------------------------------------------
// Тест 5: Граничные условия (плоскость проходит строго через вершину)
// -----------------------------------------------------------------------------
bool CUnitTester::TestPlaneVertexBoundary()
{
	C3DObject* pCube = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.0f);

	// Плоскость лежит на высоте Y = 1.0 (касается верхней грани куба)
	SPlane tangentPlane(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));

	std::vector<C3DObject*> parts;
	bool bCutResult = CMeshSlicer::Slice(pCube, tangentPlane, parts, true);

	// При касании верхней грани все тело остается снизу, нож не делит меш на 2 половины
	TEST_CHECK(bCutResult == false, "Tangent plane touching boundary vertices should not produce invalid cuts");

	delete pCube;
	for (auto p : parts) delete p;
	return true;
}

// -----------------------------------------------------------------------------
// Тест 6: Корректность вращения ножа (CSlicer Rotation -> Plane Normal)
// -----------------------------------------------------------------------------
bool CUnitTester::TestSlicerRotationToNormal()
{
	CSlicer slicer(10.0f);

	// 1. Дефолтный нож смотрит строго вверх
	SPlane defaultPlane = slicer.GetPlane();
	TEST_CHECK(fabs(defaultPlane.Normal.x) < 1e-5f, "Default normal X must be 0");
	TEST_CHECK(fabs(defaultPlane.Normal.y - 1.0f) < 1e-5f, "Default normal Y must be 1");
	TEST_CHECK(fabs(defaultPlane.Normal.z) < 1e-5f, "Default normal Z must be 0");

	// 2. Поворот вокруг оси X на +90 градусов (нормаль должна повернуться в +Z)
	slicer.SetRotation(90.0f, 0.0f, 0.0f);
	SPlane rotatedPlaneX = slicer.GetPlane();

	TEST_CHECK(fabs(rotatedPlaneX.Normal.x) < 1e-5f, "Rotated X normal X must be 0");
	TEST_CHECK(fabs(rotatedPlaneX.Normal.y) < 1e-5f, "Rotated X normal Y must be 0");
	TEST_CHECK(fabs(rotatedPlaneX.Normal.z - 1.0f) < 1e-5f, "Rotated X normal Z must be +1");

	return true;
}

// -----------------------------------------------------------------------------
// Тест 7: Целостность экспорта и импорта Wavefront OBJ
// -----------------------------------------------------------------------------
bool CUnitTester::TestOBJExportImportIntegrity()
{
	C3DObject* pSphere = C3DObject::CreatePrimitive(C3DObject::ePR_Sphere, 2.0f);
	pSphere->SetName("Unit_Test_Sphere");

	const size_t origVertCount = pSphere->GetVertexCount();
	const size_t origTriCount = pSphere->GetTriangleCount();

	const char* pszTempFile = "temp_test_mesh.obj";
	TEST_CHECK(pSphere->ExportToOBJ(pszTempFile), "OBJ export must succeed");

	C3DObject* pLoaded = new C3DObject();
	bool bLoaded = pLoaded->LoadFromOBJ(pszTempFile);

	// Удаляем временный тестовый файл
	remove(pszTempFile);

	TEST_CHECK(bLoaded == true, "OBJ import must succeed");
	TEST_CHECK(pLoaded->GetVertexCount() == origVertCount, "Imported vertex count must match original");
	TEST_CHECK(pLoaded->GetTriangleCount() == origTriCount, "Imported triangle count must match original");

	delete pSphere;
	delete pLoaded;
	return true;
}

// -----------------------------------------------------------------------------
// Главная точка входа тестового модуля
// -----------------------------------------------------------------------------
bool CUnitTester::RunAllTests()
{
	g_nPassedTests = 0;
	g_nFailedTests = 0;

	Utils::ODS("=================================================");
	Utils::ODS("[TESTS] STARTING AUTOMATED UNIT TEST SUITE...");
	Utils::ODS("=================================================");

	RUN_TEST(TestPlaneMath, "1. SPlane Mathematical Primitives");
	RUN_TEST(TestPlaneMiss, "2. Non-intersecting Plane (Miss Condition)");
	RUN_TEST(TestCubeSliceSimple, "3. Cube Bisection (2 Slices Output)");
	RUN_TEST(TestSinePlaneMultipleIslands, "4. Non-Convex Wave Slicing (BFS Island Separation)");
	RUN_TEST(TestPlaneVertexBoundary, "5. Tangent & Boundary Vertices Robustness");
	RUN_TEST(TestSlicerRotationToNormal, "6. Slicer Euler Angles to Plane Normal");
	RUN_TEST(TestOBJExportImportIntegrity, "7. Wavefront OBJ Export/Import Integrity");

	Utils::ODS("=================================================");
	Utils::ODS("[TESTS] RESULTS: %d PASSED, %d FAILED (Total: %d)",
		g_nPassedTests, g_nFailedTests, g_nPassedTests + g_nFailedTests);
	Utils::ODS("=================================================");

	return (g_nFailedTests == 0);
}