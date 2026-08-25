#include "stdafx.h"

#include "3DObject.h"
#include "../../Math/Vector3.h"

#include "Slicer.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>

//-----------------------------------------------------------------------------
// CSlicer
//-----------------------------------------------------------------------------
CSlicer::CSlicer(float fVisualSize, unsigned int dwColor)
	: m_fVisualSize(fVisualSize)
{
	SetName("Slicing Plane (Tool)");

	SetModelColor(dwColor);
	SetVisible(true);

	RebuildVisualMesh();
}

void CSlicer::SetVisualSize(float fSize)
{
	if (fSize <= 0.0f)
		return;

	m_fVisualSize = fSize;
	RebuildVisualMesh();
}

void CSlicer::RebuildVisualMesh()
{
	std::vector<Vertex3D>& vVrts = GetVertices();
	std::vector<unsigned int>& vInds = GetIndices();

	vVrts.clear();
	vInds.clear();

	const float fHalf = m_fVisualSize / 2.0f;
	const unsigned int dwWhite = 0xFFFFFFFF;

	// 1. Создаем 4 вершины квадрата в локальной плоскости XZ (нормаль смотрит вверх вдоль оси Y)
	Vertex3D v0 = { -fHalf, 0.0f, -fHalf, dwWhite }; // Top-Left
	Vertex3D v1 = { fHalf, 0.0f, -fHalf, dwWhite }; // Top-Right
	Vertex3D v2 = { fHalf, 0.0f,  fHalf, dwWhite }; // Bottom-Right
	Vertex3D v3 = { -fHalf, 0.0f,  fHalf, dwWhite }; // Bottom-Left

	vVrts.push_back(v0);
	vVrts.push_back(v1);
	vVrts.push_back(v2);
	vVrts.push_back(v3);

	// 2. Двусторонняя триангуляция (чтобы нож было видно сверху и снизу):

	// Лицевая сторона (Top Face)
	vInds.push_back(0);
	vInds.push_back(1);
	vInds.push_back(2);

	vInds.push_back(0);
	vInds.push_back(2);
	vInds.push_back(3);

	// Обратная сторона (Bottom Face)
	vInds.push_back(0);
	vInds.push_back(2);
	vInds.push_back(1);

	vInds.push_back(0);
	vInds.push_back(3);
	vInds.push_back(2);
}

SPlane CSlicer::GetPlane() const
{
	// 1. Базовая локальная нормаль нашего горизонтального плейна — строго вверх вдоль Y
	Vector3 normal = { 0.0f, 1.0f, 0.0f };

	// 2. Поворачиваем нормаль в мировое пространство в том же порядке, что и в рендере (Z -> Y -> X)

	// Вращение вокруг Z
	if (m_vRotation.z != 0.0f)
	{
		const float radZ = m_vRotation.z * MATH_DEG2RAD;
		const float cosZ = cosf(radZ);
		const float sinZ = sinf(radZ);
		const float rx = normal.x * cosZ - normal.y * sinZ;
		const float ry = normal.x * sinZ + normal.y * cosZ;
		normal.x = rx;
		normal.y = ry;
	}

	// Вращение вокруг Y
	if (m_vRotation.y != 0.0f)
	{
		const float radY = m_vRotation.y * MATH_DEG2RAD;
		const float cosY = cosf(radY);
		const float sinY = sinf(radY);
		const float rx = normal.x * cosY + normal.z * sinY;
		const float rz = -normal.x * sinY + normal.z * cosY;
		normal.x = rx;
		normal.z = rz;
	}

	// Вращение вокруг X
	if (m_vRotation.x != 0.0f)
	{
		const float radX = m_vRotation.x * MATH_DEG2RAD;
		const float cosX = cosf(radX);
		const float sinX = sinf(radX);
		const float ry = normal.y * cosX - normal.z * sinX;
		const float rz = normal.y * sinX + normal.z * cosX;
		normal.y = ry;
		normal.z = rz;
	}

	// 3. Собираем математическую плоскость: Точка = Позиция слайсера, Нормаль = Повернутый вектор
	return SPlane(m_vPosition, normal);
}

// Вспомогательная функция: перевод локальной вершины в мировой базис (TRS)
static Vertex3D TransformToWorld(const Vertex3D& v, const Vector3& pos, const Vector3& rot, const Vector3& scale)
{
	Vector3 res = { v.x * scale.x, v.y * scale.y, v.z * scale.z };

	// Z
	if (rot.z != 0.0f)
	{
		const float radZ = rot.z * MATH_DEG2RAD;
		const float cosZ = cosf(radZ), sinZ = sinf(radZ);
		const float rx = res.x * cosZ - res.y * sinZ;
		const float ry = res.x * sinZ + res.y * cosZ;
		res.x = rx; res.y = ry;
	}
	// Y
	if (rot.y != 0.0f)
	{
		const float radY = rot.y * MATH_DEG2RAD;
		const float cosY = cosf(radY), sinY = sinf(radY);
		const float rx = res.x * cosY + res.z * sinY;
		const float rz = -res.x * sinY + res.z * cosY;
		res.x = rx; res.z = rz;
	}
	// X
	if (rot.x != 0.0f)
	{
		const float radX = rot.x * MATH_DEG2RAD;
		const float cosX = cosf(radX), sinX = sinf(radX);
		const float ry = res.y * cosX - res.z * sinX;
		const float rz = res.y * sinX + res.z * cosX;
		res.y = ry; res.z = rz;
	}

	// Translation
	Vertex3D worldVert = v;
	worldVert.x = res.x + pos.x;
	worldVert.y = res.y + pos.y;
	worldVert.z = res.z + pos.z;
	return worldVert;
}

// Вспомогательная функция: интерполяция вершины на ребре среза
static Vertex3D InterpolateEdgeVertex(const Vertex3D& v0, const Vertex3D& v1, float fDist0, float fDist1)
{
	const float t = fDist0 / (fDist0 - fDist1);

	Vertex3D v = {};
	v.x = v0.x + t * (v1.x - v0.x);
	v.y = v0.y + t * (v1.y - v0.y);
	v.z = v0.z + t * (v1.z - v0.z);

	// Интерполяция цвета вершины (RGBA)
	const unsigned int r0 = (v0.dwColor >> 24) & 0xFF, r1 = (v1.dwColor >> 24) & 0xFF;
	const unsigned int g0 = (v0.dwColor >> 16) & 0xFF, g1 = (v1.dwColor >> 16) & 0xFF;
	const unsigned int b0 = (v0.dwColor >> 8) & 0xFF, b1 = (v1.dwColor >> 8) & 0xFF;
	const unsigned int a0 = v0.dwColor & 0xFF, a1 = v1.dwColor & 0xFF;

	const unsigned int r = static_cast<unsigned int>(r0 + t * (static_cast<int>(r1) - static_cast<int>(r0)));
	const unsigned int g = static_cast<unsigned int>(g0 + t * (static_cast<int>(g1) - static_cast<int>(g0)));
	const unsigned int b = static_cast<unsigned int>(b0 + t * (static_cast<int>(b1) - static_cast<int>(b0)));
	const unsigned int a = static_cast<unsigned int>(a0 + t * (static_cast<int>(a1) - static_cast<int>(a0)));

	v.dwColor = (r << 24) | (g << 16) | (b << 8) | a;
	return v;
}

// Вспомогательный метод добавления треугольника в буфер
static void AddTriangle(std::vector<Vertex3D>& vOutVerts, std::vector<unsigned int>& vOutIndices,
	const Vertex3D& a, const Vertex3D& b, const Vertex3D& c)
{
	const unsigned int idx = static_cast<unsigned int>(vOutVerts.size());
	vOutVerts.push_back(a);
	vOutVerts.push_back(b);
	vOutVerts.push_back(c);

	vOutIndices.push_back(idx);
	vOutIndices.push_back(idx + 1);
	vOutIndices.push_back(idx + 2);
}

//-----------------------------------------------------------------------------
// CMeshSlicer Implementation
//-----------------------------------------------------------------------------
void CMeshSlicer::SplitMeshByPlane(
	const C3DObject* pSource,
	const SPlane& plane,
	std::vector<Vertex3D>& outUpperVerts, std::vector<unsigned int>& outUpperIndices,
	std::vector<Vertex3D>& outLowerVerts, std::vector<unsigned int>& outLowerIndices)
{
	const auto& srcVerts = pSource->GetVertices();
	const auto& srcIndices = pSource->GetIndices();
	const Vector3 pos = pSource->GetPosition();
	const Vector3 rot = pSource->GetRotation();
	const Vector3 scale = pSource->GetScale();

	// 1. Предварительно переводим все вершины в мировой базис
	std::vector<Vertex3D> worldVerts(srcVerts.size());
	std::vector<float> distances(srcVerts.size());

	for (size_t i = 0; i < srcVerts.size(); ++i)
	{
		worldVerts[i] = TransformToWorld(srcVerts[i], pos, rot, scale);
		Vector3 p(worldVerts[i].x, worldVerts[i].y, worldVerts[i].z);
		distances[i] = plane.GetSignedDistance(p);
	}

	// 2. Обрабатываем каждый треугольник сетки
	for (size_t i = 0; i < srcIndices.size(); i += 3)
	{
		if (i + 2 >= srcIndices.size())
			break;

		const unsigned int i0 = srcIndices[i];
		const unsigned int i1 = srcIndices[i + 1];
		const unsigned int i2 = srcIndices[i + 2];

		const Vertex3D& v0 = worldVerts[i0];
		const Vertex3D& v1 = worldVerts[i1];
		const Vertex3D& v2 = worldVerts[i2];

		const float d0 = distances[i0];
		const float d1 = distances[i1];
		const float d2 = distances[i2];

		const float fEps = 1e-5f;
		const int s0 = (d0 > fEps) ? 1 : ((d0 < -fEps) ? -1 : 0);
		const int s1 = (d1 > fEps) ? 1 : ((d1 < -fEps) ? -1 : 0);
		const int s2 = (d2 > fEps) ? 1 : ((d2 < -fEps) ? -1 : 0);

		// СЛУЧАЙ 1: Треугольник целиком снизу (или касается плоскости снизу)
		if (s0 <= 0 && s1 <= 0 && s2 <= 0)
		{
			AddTriangle(outLowerVerts, outLowerIndices, v0, v1, v2);
			continue;
		}

		// СЛУЧАЙ 2: Треугольник целиком сверху (или касается плоскости сверху)
		if (s0 >= 0 && s1 >= 0 && s2 >= 0)
		{
			AddTriangle(outUpperVerts, outUpperIndices, v0, v1, v2);
			continue;
		}

		const bool b0 = d0 >= 0.0f;
		const bool b1 = d1 >= 0.0f;
		const bool b2 = d2 >= 0.0f;

		// СЛУЧАЙ 3: Треугольник пересекается плоскостью (1 вершина с одной стороны, 2 с другой)
		// Находим "одиночную" вершину (lone vertex)
		if (b0 != b1 && b0 != b2)
		{
			// v0 одиночная
			Vertex3D i01 = InterpolateEdgeVertex(v0, v1, d0, d1);
			Vertex3D i02 = InterpolateEdgeVertex(v0, v2, d0, d2);

			if (b0) // v0 выше
			{
				AddTriangle(outUpperVerts, outUpperIndices, v0, i01, i02);
				AddTriangle(outLowerVerts, outLowerIndices, v1, v2, i02);
				AddTriangle(outLowerVerts, outLowerIndices, v1, i02, i01);
			}
			else // v0 ниже
			{
				AddTriangle(outLowerVerts, outLowerIndices, v0, i01, i02);
				AddTriangle(outUpperVerts, outUpperIndices, v1, v2, i02);
				AddTriangle(outUpperVerts, outUpperIndices, v1, i02, i01);
			}
		}
		else if (b1 != b0 && b1 != b2)
		{
			// v1 одиночная
			Vertex3D i12 = InterpolateEdgeVertex(v1, v2, d1, d2);
			Vertex3D i10 = InterpolateEdgeVertex(v1, v0, d1, d0);

			if (b1) // v1 выше
			{
				AddTriangle(outUpperVerts, outUpperIndices, v1, i12, i10);
				AddTriangle(outLowerVerts, outLowerIndices, v2, v0, i10);
				AddTriangle(outLowerVerts, outLowerIndices, v2, i10, i12);
			}
			else // v1 ниже
			{
				AddTriangle(outLowerVerts, outLowerIndices, v1, i12, i10);
				AddTriangle(outUpperVerts, outUpperIndices, v2, v0, i10);
				AddTriangle(outUpperVerts, outUpperIndices, v2, i10, i12);
			}
		}
		else
		{
			// v2 одиночная
			Vertex3D i20 = InterpolateEdgeVertex(v2, v0, d2, d0);
			Vertex3D i21 = InterpolateEdgeVertex(v2, v1, d2, d1);

			if (b2) // v2 выше
			{
				AddTriangle(outUpperVerts, outUpperIndices, v2, i20, i21);
				AddTriangle(outLowerVerts, outLowerIndices, v0, v1, i21);
				AddTriangle(outLowerVerts, outLowerIndices, v0, i21, i20);
			}
			else // v2 ниже
			{
				AddTriangle(outLowerVerts, outLowerIndices, v2, i20, i21);
				AddTriangle(outUpperVerts, outUpperIndices, v0, v1, i21);
				AddTriangle(outUpperVerts, outUpperIndices, v0, i21, i20);
			}
		}
	}
}
// Структура для квантования координат (поиск совпадающих точек с точностью до 0.001)
struct SPointKey
{
	int x, y, z;
	bool operator==(const SPointKey& o) const { return x == o.x && y == o.y && z == o.z; }
};

struct SPointKeyHash
{
	size_t operator()(const SPointKey& k) const
	{
		return ((std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1)) >> 1) ^ (std::hash<int>()(k.z) << 1);
	}
};

static inline SPointKey QuantizePoint(const Vertex3D& v, float fScale = 1000.0f)
{
	return {
		static_cast<int>(std::round(v.x * fScale)),
		static_cast<int>(std::round(v.y * fScale)),
		static_cast<int>(std::round(v.z * fScale))
	};
}

void CMeshSlicer::SeparateDisconnectedIslands(
	const std::vector<Vertex3D>& vertices,
	const std::vector<unsigned int>& indices,
	const std::string& sBaseName,
	unsigned int dwColor,
	std::vector<C3DObject*>& outParts)
{
	if (vertices.empty() || indices.empty())
		return;

	const size_t numTriangles = indices.size() / 3;

	// 1. Строим граф смежности по РЕАЛЬНЫМ КООРДИНАТАМ (X, Y, Z), а не по индексам!
	std::unordered_map<SPointKey, std::vector<size_t>, SPointKeyHash> pointToTriangles;
	for (size_t t = 0; t < numTriangles; ++t)
	{
		const Vertex3D& v0 = vertices[indices[t * 3]];
		const Vertex3D& v1 = vertices[indices[t * 3 + 1]];
		const Vertex3D& v2 = vertices[indices[t * 3 + 2]];

		pointToTriangles[QuantizePoint(v0)].push_back(t);
		pointToTriangles[QuantizePoint(v1)].push_back(t);
		pointToTriangles[QuantizePoint(v2)].push_back(t);
	}

	std::vector<bool> visitedTriangles(numTriangles, false);
	size_t islandIndex = 0;

	// 2. Поиск связных компонент (BFS)
	for (size_t t = 0; t < numTriangles; ++t)
	{
		if (visitedTriangles[t])
			continue;

		std::vector<size_t> islandTriangles;
		std::queue<size_t> queue;

		queue.push(t);
		visitedTriangles[t] = true;

		while (!queue.empty())
		{
			const size_t currTri = queue.front();
			queue.pop();
			islandTriangles.push_back(currTri);

			// Находим соседей через физически совпадающие вершины (X, Y, Z)
			for (int j = 0; j < 3; ++j)
			{
				const Vertex3D& v = vertices[indices[currTri * 3 + j]];
				const SPointKey key = QuantizePoint(v);

				const auto& neighborTris = pointToTriangles[key];
				for (size_t neighbor : neighborTris)
				{
					if (!visitedTriangles[neighbor])
					{
						visitedTriangles[neighbor] = true;
						queue.push(neighbor);
					}
				}
			}
		}

		// 3. Собираем цельный саб-меш
		if (!islandTriangles.empty())
		{
			C3DObject* pPart = new C3DObject();
			char szName[512] = {};
			sprintf_s(szName, sizeof(szName), "%s_Part_%zu", sBaseName.c_str(), ++islandIndex);
			pPart->SetName(szName);
			pPart->SetModelColor(dwColor);
			pPart->SetVisible(true);
			pPart->SetRenderType(C3DObject::ERenderType::eRT_Wireframe);

			// 3.1. Геометрический центр островка (Centroid)
			Vector3 centroid = { 0.0f, 0.0f, 0.0f };
			size_t totalPoints = islandTriangles.size() * 3;
			for (size_t triIdx : islandTriangles)
			{
				centroid.x += vertices[indices[triIdx * 3]].x;
				centroid.y += vertices[indices[triIdx * 3]].y;
				centroid.z += vertices[indices[triIdx * 3]].z;

				centroid.x += vertices[indices[triIdx * 3 + 1]].x;
				centroid.y += vertices[indices[triIdx * 3 + 1]].y;
				centroid.z += vertices[indices[triIdx * 3 + 1]].z;

				centroid.x += vertices[indices[triIdx * 3 + 2]].x;
				centroid.y += vertices[indices[triIdx * 3 + 2]].y;
				centroid.z += vertices[indices[triIdx * 3 + 2]].z;
			}
			const float fInvCount = 1.0f / static_cast<float>(totalPoints);
			centroid.x *= fInvCount;
			centroid.y *= fInvCount;
			centroid.z *= fInvCount;

			pPart->SetPosition(centroid.x, centroid.y, centroid.z);

			// 3.2. Перенос в локальные координаты со склейкой вершин
			std::unordered_map<SPointKey, unsigned int, SPointKeyHash> pointToNewIndex;
			for (size_t triIdx : islandTriangles)
			{
				for (int j = 0; j < 3; ++j)
				{
					const Vertex3D& origVert = vertices[indices[triIdx * 3 + j]];
					const SPointKey key = QuantizePoint(origVert);

					auto it = pointToNewIndex.find(key);
					if (it == pointToNewIndex.end())
					{
						const unsigned int newIdx = static_cast<unsigned int>(pPart->GetVertices().size());
						pointToNewIndex[key] = newIdx;

						Vertex3D localVert = origVert;
						localVert.x -= centroid.x;
						localVert.y -= centroid.y;
						localVert.z -= centroid.z;

						pPart->AddVertex(localVert);
						pPart->AddIndices(newIdx);
					}
					else
					{
						pPart->AddIndices(it->second);
					}
				}
			}

			outParts.push_back(pPart);
		}
	}
}

bool CMeshSlicer::Slice(
	C3DObject* pSourceMesh,
	const SPlane& cuttingPlane,
	std::vector<C3DObject*>& outGeneratedParts,
	bool bSeparateIslands)
{
	if (pSourceMesh == nullptr || pSourceMesh->GetVertices().empty() || pSourceMesh->GetIndices().empty())
		return false;

	outGeneratedParts.clear();

	std::vector<Vertex3D> upperVerts, lowerVerts;
	std::vector<unsigned int> upperIndices, lowerIndices;

	// 1. Разрезаем геометрию плоскостью
	SplitMeshByPlane(pSourceMesh, cuttingPlane, upperVerts, upperIndices, lowerVerts, lowerIndices);

	// Если нож не задел объект (все полигоны остались с одной стороны)
	if (upperIndices.empty() || lowerIndices.empty())
	{
		Utils::ODS("[SLICER] Plane does not intersect mesh '%s'. Slicing aborted.", pSourceMesh->GetName());
		return false;
	}

	const std::string sBaseName = pSourceMesh->GetName();
	const unsigned int dwColor = pSourceMesh->GetModelColor();

	// 2. Разделение на независимые невыпуклые островки (SubMeshes)
	if (bSeparateIslands)
	{
		SeparateDisconnectedIslands(upperVerts, upperIndices, sBaseName + "_Upper", dwColor, outGeneratedParts);
		SeparateDisconnectedIslands(lowerVerts, lowerIndices, sBaseName + "_Lower", dwColor, outGeneratedParts);
	}
	else
	{
		// Режим "просто 2 половины":
		C3DObject* pUpper = new C3DObject();
		std::vector<Vertex3D>& vVrtsUP = pUpper->GetVertices();
		std::vector<unsigned int>& vIndsUP = pUpper->GetIndices();
		pUpper->SetName((sBaseName + "_Upper").c_str());
		vVrtsUP = upperVerts;
		vIndsUP = upperIndices;
		pUpper->SetModelColor(dwColor);
		//pUpper->SetRenderType(C3DObject::ERenderType::eRT_Wireframe);
		outGeneratedParts.push_back(pUpper);

		C3DObject* pLower = new C3DObject();
		pLower->SetName((sBaseName + "_Lower").c_str());
		std::vector<Vertex3D>& vVrtsDN = pLower->GetVertices();
		std::vector<unsigned int>& vIndsDN = pLower->GetIndices();
		vVrtsDN = lowerVerts;
		vIndsDN = lowerIndices;
		pLower->SetModelColor(dwColor);
		//pUpper->SetRenderType(C3DObject::ERenderType::eRT_Wireframe);
		outGeneratedParts.push_back(pLower);
	}

	Utils::ODS("[SLICER] Mesh '%s' sliced into %zu independent sub-meshes!", sBaseName.c_str(), outGeneratedParts.size());
	return true;
}