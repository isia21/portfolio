#pragma once
//-----------------------------------------------------------------------------
// SPlane: Pure Mathematical 3D Plane (Point + Normal)
//-----------------------------------------------------------------------------
struct SPlane
{
	Vector3 Point;  // Точка прохождения плоскости (P0)
	Vector3 Normal; // Единичный вектор нормали плоскости (N)

	SPlane()
		: Point(0.0f, 0.0f, 0.0f)
		, Normal(0.0f, 1.0f, 0.0f) // По умолчанию горизонтальная плоскость (нормаль вверх)
	{}

	SPlane(const Vector3& vPoint, const Vector3& vNormal)
		: Point(vPoint)
		, Normal(vNormal)
	{
		Normalize();
	}

	// Нормализация вектора нормали к единичной длине
	void Normalize()
	{
		const float fLenSq = Normal.x * Normal.x + Normal.y * Normal.y + Normal.z * Normal.z;
		if (fLenSq > 1e-8f)
		{
			const float fInvLen = 1.0f / sqrtf(fLenSq);
			Normal.x *= fInvLen;
			Normal.y *= fInvLen;
			Normal.z *= fInvLen;
		}
		else
			Normal = Vector3(0.0f, 1.0f, 0.0f);
	}

	// Знаковое расстояние от точки до плоскости: Dot(V - P0, N)
	// > 0  — точка выше (положительная сторона)
	// < 0  — точка ниже (отрицательная сторона)
	// == 0 — точка лежит ровно на плоскости
	float GetSignedDistance(const Vector3& vTestPoint) const
	{
		return (vTestPoint.x - Point.x) * Normal.x +
			(vTestPoint.y - Point.y) * Normal.y +
			(vTestPoint.z - Point.z) * Normal.z;
	}

	// Определение стороны: +1 (Верх), -1 (Низ), 0 (На плоскости)
	int GetSide(const Vector3& vTestPoint, float fEpsilon = 1e-5f) const
	{
		const float fDist = GetSignedDistance(vTestPoint);
		if (fDist > fEpsilon)  return 1;
		if (fDist < -fEpsilon) return -1;
		return 0;
	}

	// Вычисление точной точки пересечения отрезка [vA, vB] с плоскостью
	Vector3 GetIntersectionPoint(const Vector3& vA, const Vector3& vB, float fDistA, float fDistB) const
	{
		// Интерполяционный фактор t = dA / (dA - dB)
		const float fT = fDistA / (fDistA - fDistB);

		return Vector3(
			vA.x + fT * (vB.x - vA.x),
			vA.y + fT * (vB.y - vA.y),
			vA.z + fT * (vB.z - vA.z)
		);
	}
};


//-----------------------------------------------------------------------------
// CSlicer: Interactive 3D Cutting Tool Entity (Scene Object)
//-----------------------------------------------------------------------------
class CSlicer : public C3DObject
{
public:
	CSlicer(float fVisualSize = 12.0f, unsigned int dwColor = 0xFF004488);
	virtual ~CSlicer() override = default;

public:
	// Сборка математической плоскости из текущего положения (Pos) и ориентации (Rot)
	SPlane GetPlane() const;

	// Размер визуального отображения ножа
	float GetVisualSize() const { return m_fVisualSize; }
	void SetVisualSize(float fSize);

	// Перегенерация визуальной сетки ножа (полупрозрачный квадрат + рамка)
	void RebuildVisualMesh();

private:
	float m_fVisualSize;
};


//-----------------------------------------------------------------------------
// CMeshSlicer: Core Mesh Slicing & Sub-mesh Separation Engine
//-----------------------------------------------------------------------------
class CMeshSlicer
{
public:
	// Главный метод нарезки: принимает меш и плоскость, возвращает список новых частей
	static bool Slice(
		C3DObject* pSourceMesh,
		const SPlane& cuttingPlane,
		std::vector<C3DObject*>& outGeneratedParts,
		bool bSeparateIslands = true);

	// Перегрузка под наш 3D-слайсер напрямую:
	static bool Slice(
		C3DObject* pSourceMesh,
		const CSlicer* pSlicer,
		std::vector<C3DObject*>& outGeneratedParts,
		bool bSeparateIslands = true)
	{
		if (pSlicer == nullptr) return false;
		return Slice(pSourceMesh, pSlicer->GetPlane(), outGeneratedParts, bSeparateIslands);
	}

private:
	// Внутренние этапы алгоритма:

	// 1. Разрезание исходной геометрии на Верхнюю и Нижнюю половины
	static void SplitMeshByPlane(
		const C3DObject* pSource,
		const SPlane& plane,
		std::vector<Vertex3D>& outUpperVerts, std::vector<unsigned int>& outUpperIndices,
		std::vector<Vertex3D>& outLowerVerts, std::vector<unsigned int>& outLowerIndices);

	// 2. Разделение несвязных кусков геометрии (островков/холмов) на самостоятельные C3DObject
	static void SeparateDisconnectedIslands(
		const std::vector<Vertex3D>& vertices,
		const std::vector<unsigned int>& indices,
		const std::string& sBaseName,
		unsigned int dwColor,
		std::vector<C3DObject*>& outParts);
};