#include "stdafx.h"
#include "Vector3.h"
#include "Math.h"
#include "../Graphics.h"


#include <queue>

float PointPlaneDistance(const Vector3& vM0, const Vector3& vN, const Vector3& vP)
{
	/*
	* https://foxford.ru/wiki/matematika/uravnenie-ploskosti
	* https://www.youtube.com/watch?v=cVfy6ioxUhA
	Согласно уравнению плоскости.
	M0 = {x0,y0,z0} -- Где точка/якорь задается 
	N  = {a, b, c}  -- Нормаль, определяющая вращение/куда смотрит плоскость
	P  = {x, y, z}  -- Точка, положение которой относительно плоскости нужно определеить 

	a(x-x0) + b(y-y0) + c(z-z0) == 0 (0 говорит о том что точка лежит строго на плоскости)
	*/

	return vN.x * (vP.x - vM0.x)
		+  vN.y * (vP.y - vM0.y)
		+  vN.z * (vP.z - vM0.z);
}

enum ePointSide : int
{
	ePS_Below = -1,
	ePS_OnPlane, 
	ePS_Above
};

int PointSide(const Vector3& vM0, const Vector3& vN, const Vector3& vP, float fEpsilon)
{
	/*
	fEpsilon нам действительно необходим, т.к. у результата будет погрешность, не будет абсоютного нуля.
	т.е. Эпсилон в данном случае становится range`ом/Допуском/Рамками, значение попадающее в допуск считается Нулем.
	*/

	float fDist = PointPlaneDistance(vM0, vN, vP);
	if (fDist > fEpsilon)
		return 1;
	else if (fDist < -fEpsilon)
		return -1;
	else
		return 0;
}

#define D3DCOLOR_ARGB(a,r,g,b) \
    ((unsigned long)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))


Vector3 IntersectPlane(const Vector3& vM0, const Vector3& vN, const Vector3& vP0, const Vector3& vP1){

	float fDist0 = PointPlaneDistance(vM0, vN, vP0);
	float fDist1 = PointPlaneDistance(vM0, vN, vP1);

	// Параметр t пропорционального деления отрезка [0..1]
	float t = fDist0 / (fDist0 - fDist1);

	// Рассчет новой промежуточной 3D точки
	Vector3 v3Out = vP0 + (vP1 - vP0) * t;

	return v3Out;
}

Vertex3D IntersectPlane(const Vector3& vM0, const Vector3& vN, const Vertex3D& vP0, const Vertex3D& vP1)
{
	Vector3 v3P0 = vP0;
	Vector3 v3P1 = vP1;

	float fDist0 = PointPlaneDistance(vM0, vN, v3P0);
	float fDist1 = PointPlaneDistance(vM0, vN, v3P1);

	// Параметр t пропорционального деления отрезка [0..1]
	float t = fDist0 / (fDist0 - fDist1);

	// Рассчет новой промежуточной 3D точки
	Vector3 v3Out = v3P0 + (v3P1 - v3P0) * t;

	// Рассчет нового промежуточного цвета для созданной точки
	
	/*	Интерполяция цвета.
	Данный способ работы с DWORD ARGB перенесен из моего legacy-проекта, где цвет вершины/материала исторически представлен как 32-bit DWORD в формате D3DCOLOR_ARGB.
	
	Аналогичная логика использовалась в AnimColor: ARGB DWORD -> извлечение каналов через mask/shift -> интерполяция каналов -> сборка обратно в DWORD.
	
	При необходимости могу предоставить соответствующий фрагмент legacy-кода для подтверждения происхождения данной реализации.
	*/
	DWORD dwFrameAColor = vP0.dwColor;
	DWORD dwFrameBColor = vP1.dwColor;

	float fAA = (float)((dwFrameAColor & 0xFF000000) >> 24);
	float fRA = (float)((dwFrameAColor & 0x00FF0000) >> 16);
	float fGA = (float)((dwFrameAColor & 0x0000FF00) >> 8);
	float fBA = (float)(dwFrameAColor & 0x000000FF);

	float fAB = (float)((dwFrameBColor & 0xFF000000) >> 24);
	float fRB = (float)((dwFrameBColor & 0x00FF0000) >> 16);
	float fGB = (float)((dwFrameBColor & 0x0000FF00) >> 8);
	float fBB = (float)(dwFrameBColor & 0x000000FF);

	DWORD dwA = (DWORD)(fAA + (fAB - fAA) * t);
	if (dwA > 255) dwA = 255;
	DWORD dwR = (DWORD)(fRA + (fRB - fRA) * t);
	if (dwR > 255) dwR = 255;
	DWORD dwG = (DWORD)(fGA + (fGB - fGA) * t);
	if (dwG > 255) dwG = 255;
	DWORD dwB = (DWORD)(fBA + (fBB - fBA) * t);
	if (dwB > 255) dwB = 255;
	DWORD dwNewColor = D3DCOLOR_ARGB(dwA, dwR, dwG, dwB);

	// Создаем новую вершину из найденной позиции от t и найденного цвета от t
	Vertex3D v3DOut = { v3Out.x, v3Out.y, v3Out.z, dwNewColor };

	return v3DOut;
}

void SliceTriangle(const Vector3& vM0, const Vector3& vN, const Triangle& tri, std::vector<Triangle>& vAboveTris, std::vector<Triangle>& vBelowTris, float fEpsilon)
{
	// 1. Быстрая сортировка (случаи, когда очевидно, что все вершины ВЫШЕ/НИЖЕ плоскости.
	// 
	// --- Определяем расстояние точек/вершин треугольника относительно плоскости ---
	float fDists[3] = {
		PointPlaneDistance(vM0, vN, tri.vertices[0]),
		PointPlaneDistance(vM0, vN, tri.vertices[1]),
		PointPlaneDistance(vM0, vN, tri.vertices[2])
	};
	// --- Определяем сторону точек треугольники относительно плоскости ---
	ePointSide eSides[3] = {
		(ePointSide)PointSide(vM0, vN, tri.vertices[0], fEpsilon),
		(ePointSide)PointSide(vM0, vN, tri.vertices[1], fEpsilon),
		(ePointSide)PointSide(vM0, vN, tri.vertices[2], fEpsilon)
	};

	// Быстрый выход: нет ни одной вершины снизу -> целиком наверх
	if (eSides[0] != ePointSide::ePS_Below && eSides[1] != ePointSide::ePS_Below && eSides[2] != ePointSide::ePS_Below)
	{
		vAboveTris.push_back(tri);
		return;
	}
	// Быстрый выход: нет ни одной вершины сверху -> целиком вниз
	if (eSides[0] != ePointSide::ePS_Above && eSides[1] != ePointSide::ePS_Above && eSides[2] != ePointSide::ePS_Above)
	{
		vBelowTris.push_back(tri);
		return;
	}

	//	// --- Если при нашей "быстрой сортировке" мы уже определили треугольник целиком как верхнюю/нижнюю часть - можем выходить
	//	if (!vAboveTris.empty() || !vBelowTris.empty())
	//		return;

	// 2. Если у нас остались пограничные случаи, когда 2 точки ребра треугольника находятся по разные стороны или лежат в пределах допуска fEpsilon НА плоскости
	// То попробуем их разрезать
	// 
	// --- Временные буферы "новых" полигонов
	std::vector<Vertex3D> frontPoly;
	std::vector<Vertex3D> backPoly;
	for (int i = 0; i < 3; i++)
	{
		int lCurVertexOffset = i;
		int lNextVertexOffset = (i + 1) % 3;

		// Определяем текущую и след точку/вершину
		const Vertex3D& pA = { 
			tri.vertices[lCurVertexOffset].x,
			tri.vertices[lCurVertexOffset].y,
			tri.vertices[lCurVertexOffset].z,
			tri.colors[lCurVertexOffset] 
		};
		const Vertex3D& pB = { 
			tri.vertices[lNextVertexOffset].x,
			tri.vertices[lNextVertexOffset].y,
			tri.vertices[lNextVertexOffset].z,
			tri.colors[lNextVertexOffset] 
		};
		// Фиксируем их дистанцию от нашей плоскости
		float fDistA = fDists[lCurVertexOffset];
		float fDistB = fDists[lNextVertexOffset];
		// Фиксируем определенную сторону от нашей плоскости
		ePointSide eSideA = eSides[lCurVertexOffset];
		ePointSide eSideB = eSides[lNextVertexOffset];

		// Добавляем текущую вершину A, сли точка НЕ ниже (Above или OnPlane) -> кладем в front
		if (eSideA != ePointSide::ePS_Below)
			frontPoly.push_back(pA);

		// Добавляем текущую вершину A, если точка НЕ выше (Below или OnPlane) -> кладем в back
		if (eSideA != ePointSide::ePS_Above)
			backPoly.push_back(pA);

		// Если ребро AB пересекает плоскость (один конец Above, другой Below)
		// МЫ НАМЕРЕННО НЕ СВЕРЯЕМ ePS_OnPlane, чтоб не создавать дубликаты точек, формирующий "вырожденный треугольник" с нулевой площадью (2 вершины которого совпадают)
		if ((eSideA == ePointSide::ePS_Above && eSideB == ePointSide::ePS_Below) ||
			(eSideA == ePointSide::ePS_Below && eSideB == ePointSide::ePS_Above))
		{
			// Находим точную точку пересечения на ребре AB
			Vertex3D pNewDot = IntersectPlane(vM0, vN, pA, pB);

			// Точка пересечения pNewDot находится строго на плоскости, добавляем в оба полигона
			frontPoly.push_back(pNewDot);
			backPoly.push_back(pNewDot);
		}
	}

	// 3. Триангуляция полученных полигонов веерным
	// --- Собираем новые треуг., что ВЫШЕ плоскости ---
	if (frontPoly.size() >= 3) {
		for (size_t i = 1; i + 1 < frontPoly.size(); ++i) {
			vAboveTris.push_back({ frontPoly[0], frontPoly[i], frontPoly[i + 1] });
		}
	}
	// --- Собираем новые треуг., что НИЖЕ плоскости ---
	if (backPoly.size() >= 3) {
		for (size_t i = 1; i + 1 < backPoly.size(); ++i) {
			vBelowTris.push_back({ backPoly[0], backPoly[i], backPoly[i + 1] });
		}
	}
}


void RotateVertexByDegs_ZYX(Vector3& vVertex, const Vector3& vRotDegs)
{
	// Вращение в 3D сводится к последовательному 2D-повороту координат в перпендикулярной оси плоскости:
	// 
	// Математическая база 2D-поворота на угол theta:
	//   new_u = u * cos(theta) - v * sin(theta)
	//   new_v = u * sin(theta) + v * cos(theta)
	
	// 1. Вокруг Z: меняются (X, Y), координата Z константа.
	if(vRotDegs.z != 0.0f)
	{
		const float fRotRadsZ = vRotDegs.z * MATH_DEG2RAD;
		const float fRotCosZ = cosf(fRotRadsZ);
		const float fRotSinZ = sinf(fRotRadsZ);
		const float fNewPosX = (vVertex.x * fRotCosZ) - (vVertex.y * fRotSinZ);
		const float fNewPosY = (vVertex.x * fRotSinZ) + (vVertex.y * fRotCosZ);
		vVertex.x = fNewPosX;
		vVertex.y = fNewPosY;
	}
	// 2. Вокруг Y: меняются (X, Z), координата Y константа.
	if (vRotDegs.y != 0.0f)
	{
		const float fRotRadsY = vRotDegs.y * MATH_DEG2RAD;
		const float fRotCosY = cosf(fRotRadsY);
		const float fRotSinY = sinf(fRotRadsY);
		const float fNewPosX = (vVertex.x * fRotCosY) - (vVertex.z * fRotSinY);
		const float fNewPosZ = (vVertex.x * fRotSinY) + (vVertex.z * fRotCosY);
		vVertex.x = fNewPosX;
		vVertex.z = fNewPosZ;
	}
	// 3. Вокруг X: меняются (Y, Z), координата X константа.
	if (vRotDegs.x != 0.0f)
	{
		const float fRotRadsX = vRotDegs.x * MATH_DEG2RAD;
		const float fRotCosX = cosf(fRotRadsX);
		const float fRotSinX = sinf(fRotRadsX);
		const float fNewPosY = (vVertex.y * fRotCosX) - (vVertex.z * fRotSinX);
		const float fNewPosZ = (vVertex.y * fRotSinX) + (vVertex.z * fRotCosX);
		vVertex.y = fNewPosY;
		vVertex.z = fNewPosZ;
	}
}

Vertex3D LocalToWorldPos(const Vertex3D& vIn, const Vector3& vTrs, const Vector3& vRot, const Vector3& vScl)
{
	//Порядок трансформации SRT
	Vector3 vWorldPos = { vIn.x, vIn.y, vIn.z };
	
	//1. Scale - масштабируем точку
	//	vWorldPos.x *= vScl.x;
	//	vWorldPos.y *= vScl.y;
	//	vWorldPos.z *= vScl.z;
	//Или используем НОВЫЙ перегруженный оператор Vector3& operator*=(const Vector3& other)
	vWorldPos *= vScl;

	//2. Rotate - вращаем тчоку
	RotateVertexByDegs_ZYX(vWorldPos, vRot);

	//3. Translate - смещаем точку
	//	vWorldPos.x += vTrs.x;
	//	vWorldPos.y += vTrs.y;
	//	vWorldPos.z += vTrs.z;
	//Или используем СТАРЫЙ перегруженный оператор Vector3& operator+=(const Vector3& other)
	vWorldPos += vTrs;

	return { vWorldPos.x,vWorldPos.y, vWorldPos.z, vIn.dwColor };
}


Triangle::Triangle(const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2)
{
	vertices[0] = { v0.x, v0.y, v0.z };
	vertices[1] = { v1.x, v1.y, v1.z };
	vertices[2] = { v2.x, v2.y, v2.z };

	colors[0] = v0.dwColor;
	colors[1] = v1.dwColor;
	colors[2] = v2.dwColor;
}

//	Triangle::Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2)
//	{
//		vertices[0] = v0; vertices[1] = v1; vertices[2] = v2;
//	}




int SliceMesh(C3DObject& pIn, const Vector3& vMeshTrs, const Vector3& vMeshRot, const Vector3& vMeshScl, const Vector3& vM0, const Vector3& vN)
{
	const float fEpsilon = 1e-5f;

	//1. Для начала мы должны очистить возможный предыдуший результат нарезки
	pIn.ClearChildren();

	//2. Собираем массив треугольников исходного меша
	std::vector<Triangle> vSrcTres;
	const std::vector<Vertex3D>& vSrcVertices = pIn.GetVertices();
	const std::vector<unsigned int>& vSrcIndnices = pIn.GetIndices();
	for (int lTrsIdx = 0; lTrsIdx < pIn.GetTriangleCount(); lTrsIdx++)
	{
		unsigned int i0 = vSrcIndnices[(lTrsIdx * 3) + 0];
		unsigned int i1 = vSrcIndnices[(lTrsIdx * 3) + 1];
		unsigned int i2 = vSrcIndnices[(lTrsIdx * 3) + 2];
		
		//Сразу приводим Локальные коорд к Мировым
		const Vertex3D& v0 = LocalToWorldPos(vSrcVertices[i0], vMeshTrs, vMeshRot, vMeshScl);
		const Vertex3D& v1 = LocalToWorldPos(vSrcVertices[i1], vMeshTrs, vMeshRot, vMeshScl);
		const Vertex3D& v2 = LocalToWorldPos(vSrcVertices[i2], vMeshTrs, vMeshRot, vMeshScl);

		//emplace_back, вместо push_back, т.к. emplace_back вызывает конструктор структуры
		vSrcTres.emplace_back(v0, v1, v2);
	}

	//3. Нарезаем исходные треугольники плоскостью
	std::vector<Triangle> vAboveTres;
	std::vector<Triangle> vBelowTres;

	for (const auto& tSrcTriangle : vSrcTres)
		SliceTriangle(vM0, vN, tSrcTriangle, vAboveTres, vBelowTres, fEpsilon);
		

	//4. Собираем нарезанные вершины в саб меши
	std::vector<std::vector<Triangle>> vAboveMeshes = CreateIslands(vAboveTres);
	std::vector<std::vector<Triangle>> vBelowMeshes = CreateIslands(vBelowTres);


	auto MakeNewObject = [](const std::vector<Triangle>& vObjTres) {
		C3DObject *pObj = nullptr;

		if (!vObjTres.empty())
		{
			pObj = new C3DObject();

			int lTrIdx = 0;
			for (auto &stT : vObjTres)
			{
				for (int lVertPos = 0; lVertPos < 3; lVertPos++) {
					pObj->AddVertex({ stT.vertices[lVertPos].x,stT.vertices[lVertPos].y,stT.vertices[lVertPos].z,stT.colors[lVertPos] });
					pObj->AddIndices(lTrIdx * 3 + lVertPos);
				}
				lTrIdx++;
			}
		}
		return pObj;
	};
	int lRetTotalSubMeshesCount = 0;
	auto PushSideNewObjects = [&](std::vector<std::vector<Triangle>> &vTres, /*std::string strPrefix*/const char * pszPrefix) {
		static char szNewNameBuff[512];
		int lCurSideMesh = 0;
		for (const auto &vCurNewUpperMeshTres : vTres)
		{
			C3DObject* pObj = MakeNewObject(vCurNewUpperMeshTres);
			if (pObj)
			{
				pObj->SetModelColor(pIn.GetModelColor());
				//TODO: т.к. система координат внутри меша - локальная, нам необходимо создать 
				//	Новый Translation оффсет, чтоб он совпадал со смешением исходного
				//	Путем рассчета нового центроида = sum(v3[]) / count(v3[]);
				
				//sprintf(szNewNameBuff, "%s_%s_%d", pIn.GetName(), strPrefix.c_str(), lCurSideMesh);
				sprintf_s(szNewNameBuff, sizeof(szNewNameBuff), "%s_%s_%d", pIn.GetName(), pszPrefix, lCurSideMesh);
				pObj->SetName(szNewNameBuff);

				//Выделяем меш визуально, путем его отрисовки через линии/грани, без заливки
				pObj->SetRenderType(C3DObject::eRT_Wireframe);
				pObj->SetVisible(true);

				//Настраиваем связи дочернего саб меша к родительскому
				pObj->SetParent(&pIn);
				pIn.AddChild(pObj);

				lRetTotalSubMeshesCount++;
				lCurSideMesh++;
			}
		}
	};

	PushSideNewObjects(vAboveMeshes, "UP");
	PushSideNewObjects(vBelowMeshes, "DN");
	
	if (lRetTotalSubMeshesCount)
		pIn.SetVisible(false);

	return lRetTotalSubMeshesCount;
}

int SliceMeshYup(C3DObject& pIn, const Vector3& vMeshTrs, const Vector3& vMeshRot, const Vector3& vMeshScl, const Vector3& vAnchorTrs, const Vector3& vNormalRot)
{
	//Якорь - в абсолютно нуле
	Vector3 vM0(0, 0, 0);
	//Нормаль - строго вверх Yup
	Vector3 vN(0, 1, 0);

	//Смещаям якорь на заданный Translation
	vM0 += vAnchorTrs;
	//Вращаем Yup нормаль на заданные градусы вращения
	RotateVertexByDegs_ZYX(vN, vNormalRot);

	return SliceMesh(pIn, vMeshTrs, vMeshRot, vMeshScl, vM0, vN);
}

std::vector<std::vector<Triangle>> CreateIslands(const std::vector<Triangle>& vInTres)
{
	// BFS - обход графа в ширину (т.к. мы ищем связи)
	//	https://www.youtube.com/watch?v=4iDv8Zu8L3I
	//	https://habr.com/ru/articles/969450/
	//	>https://en.wikipedia.org/wiki/Disjoint-set_data_structure#:~:text=%5B20%5D-,Applications,edit,-A%20demo%20for
	//	https://en.wikipedia.org/wiki/Component_(graph_theory)


	if (vInTres.empty()) 
		return {};

	const float fEpsilon = 1e-5f;

	// Описание ребра из его 2-ух точек
	struct stEdge {
		Vector3 v0, v1;
	};
	// Проверка, являются ли 2 точки одной и той же с учетом допуска по точности
	auto IsSamePoint = [fEpsilon](const Vector3& p1, const Vector3& p2) {
		return 
			(std::abs(p1.x - p2.x) <= fEpsilon) &&
			(std::abs(p1.y - p2.y) <= fEpsilon) &&
			(std::abs(p1.z - p2.z) <= fEpsilon);
		};
	// Проверка, являются ли 2 ребра одним и тем же
	auto IsSameEdge = [&](const stEdge& e1, const stEdge& e2) {
		return 
			(IsSamePoint(e1.v0, e2.v0) && IsSamePoint(e1.v1, e2.v1)) ||
			(IsSamePoint(e1.v0, e2.v1) && IsSamePoint(e1.v1, e2.v0));
		};

	// =========================================================================
	// СТАДИЯ 1: СБОР УНИКАЛЬНЫХ РЕБЕР И СВЯЗЕЙ (Ребро -> Треугольники)
	// =========================================================================
	struct stEdgeEntry {
		stEdge tEdge;
		std::vector<int> vTrsIndices; // Индексы треугольников с этим ребром
	};

	std::vector<stEdgeEntry> edgeMap;

	auto RegisterEdge = [&](const stEdge& newEdge, int triIndex) {
		// Ищем, встречалось ли уже такое ребро
		for (auto& entry : edgeMap) {
			if (IsSameEdge(entry.tEdge, newEdge)) {
				entry.vTrsIndices.push_back(triIndex);
				return;
			}
		}
		// Если ребро новое — создаем новую запись
		edgeMap.push_back({ newEdge, { triIndex } });
		};

	// Проходим по всем треугольникам и регистрируем их стороны
	for (int lCurTr = 0; lCurTr < static_cast<int>(vInTres.size()); ++lCurTr) {
		const Triangle& t = vInTres[lCurTr];
		RegisterEdge({ t.vertices[0], t.vertices[1]}, lCurTr);
		RegisterEdge({ t.vertices[1], t.vertices[2]}, lCurTr);
		RegisterEdge({ t.vertices[2], t.vertices[0]}, lCurTr);
	}

	// Хелпер: быстрый поиск соседей треугольника по построенной карте
	auto GetNeighbors = [&](int triIdx) -> std::vector<int> {
		std::vector<int> vNeighb;
		const Triangle& t = vInTres[triIdx];
		stEdge triEdges[3] = {
			{ t.vertices[0], t.vertices[1] },
			{ t.vertices[1], t.vertices[2] },
			{ t.vertices[2], t.vertices[0] }
		};

		for (const auto& tCurEdge : triEdges) {
			for (const auto& tCurEdgeEntry : edgeMap) {
				if (IsSameEdge(tCurEdgeEntry.tEdge, tCurEdge)) {
					for (int nIdx : tCurEdgeEntry.vTrsIndices) {
						if (nIdx != triIdx) {
							vNeighb.push_back(nIdx);
						}
					}
					break;
				}
			}
		}
		return vNeighb;
		};

	// =========================================================================
	// СТАДИЯ 2: ОБХОД И ПОИСК СВЯЗАННЫХ ОСТРОВОВ (BFS)
	// =========================================================================
	int totalTriangles = static_cast<int>(vInTres.size());
	std::vector<bool> visited(totalTriangles, false);
	std::vector<std::vector<Triangle>> islands;

	for (int i = 0; i < totalTriangles; ++i)
	{
		if (visited[i]) continue;

		// =====================================================================
		// СТАДИЯ 3: СБОРКА ТРЕУГОЛЬНИКОВ ТЕКУЩЕГО ОСТРОВА
		// =====================================================================
		std::vector<Triangle> currentIsland;
		std::queue<int> q;

		q.push(i);
		visited[i] = true;

		while (!q.empty())
		{
			int curTriIdx = q.front();
			q.pop();

			currentIsland.push_back(vInTres[curTriIdx]);

			// Добавляем всех не посещенных соседей через ребра
			for (int neighborIdx : GetNeighbors(curTriIdx))
			{
				if (!visited[neighborIdx])
				{
					visited[neighborIdx] = true;
					q.push(neighborIdx);
				}
			}
		}

		// Остров полностью изолирован и собран
		islands.push_back(std::move(currentIsland));
	}

	return islands;
}