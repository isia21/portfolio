#include "stdafx.h"
#include "Vector3.h"
#include "Math.h"

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

Vector3 IntersectPlane(const Vector3& vM0, const Vector3& vN, const Vector3& vP0, const Vector3& vP1) 
{
	float fDist0 = PointPlaneDistance(vM0, vN, vP0);
	float fDist1 = PointPlaneDistance(vM0, vN, vP1);

	// Параметр t пропорционального деления отрезка [0..1]
	float t = fDist0 / (fDist0 - fDist1);

	return vP0 + (vP1 - vP0) * t;
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
	std::vector<Vector3> frontPoly;
	std::vector<Vector3> backPoly;
	for (int i = 0; i < 3; i++)
	{
		int lCurVertexOffset = i;
		int lNextVertexOffset = (i + 1) % 3;

		// Определяем текущую и след точку/вершину
		const Vector3& pA = tri.vertices[lCurVertexOffset];
		const Vector3& pB = tri.vertices[lNextVertexOffset];
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
			Vector3 pNewDot = IntersectPlane(vM0, vN, pA, pB);

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