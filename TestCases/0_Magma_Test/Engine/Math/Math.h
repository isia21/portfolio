#pragma once

struct Vector3;

struct Triangle {
	Vector3 vertices[3];
};

/// <summary>
/// Реализация уравнения плоскости. 
/// </summary>
/// <param name="vM0">Исходня точка плоскости M0</param>
/// <param name="vN">Нормаль, задающая направление плоскости</param>
/// <param name="vP">Точка, положение которой относительно плоскости нужно проверить</param>
/// <returns> 
/// == 0 - P на плоскости
///  > 0 - P НАД ПЛОСКОСТЬЮ (относительно заданного направления через vN
///  < 0 - P ПОД ПЛОСКОСТЬЮ (относительно заданного направления через vN
/// </returns>
float PointPlaneDistance(const Vector3& vM0, const Vector3& vN, const Vector3& vP);

/// <summary>
/// Реализация уравнения плоскости. 
/// </summary>
/// <param name="vM0">Исходня точка плоскости M0</param>
/// <param name="vN">Нормаль, задающая направление плоскости</param>
/// <param name="vP">Точка, положение которой относительно плоскости нужно проверить</param>
/// <param name="fEpsilon">Порог погрешности (эпсилон). Определяет допустимый интервал [-fEpsilon, fEpsilon], при попадании в который точка считается лежащей строго на плоскости (для компенсации погрешностей float).</param>
/// <returns> 
///  0 - P на плоскости
///  1 - P НАД ПЛОСКОСТЬЮ (относительно заданного направления через vN)
/// -1 - P ПОД ПЛОСКОСТЬЮ (относительно заданного направления через vN)
/// </returns>
int PointSide(const Vector3& vM0, const Vector3& vN, const Vector3& vP, float fEpsilon = 1e-5f);

/// <summary>
/// Предикат: находятся ли две точки по одну сторону от плоскости.
/// </summary>
/// <param name="vM0">Исходная точка плоскости M0</param>
/// <param name="vN">Нормаль плоскости</param>
/// <param name="vP0">Первая точка</param>
/// <param name="vP1">Вторая точка</param>
/// <param name="fEpsilon">Допустимая погрешность</param>
/// <returns>true, если обе точки имеют идентичный статус классификации</returns>
//	bool IsPointsSameSide(const Vector3& vM0, const Vector3& vN, const Vector3& vP0, const Vector3& vP1, float fEpsilon = 1e-5f) {
//	    return PointSide(vM0, vN, vP0, fEpsilon) == PointSide(vM0, vN, vP1, fEpsilon);
//	}

/// <summary>
/// Находит точку пересечения ребра P0-P1 с плоскостью.
/// Вызывать только если P0 и P1 находятся по разные стороны относительно fEpsilon.
/// </summary>
Vector3 IntersectPlane(const Vector3& vM0, const Vector3& vN, const Vector3& vP0, const Vector3& vP1);


/// <summary>
/// Принимает на вход исходный треуг., данные о плоскости, и буфферы, куда будут записаны новые треуги после резки.
/// </summary>
/// <param name="vM0">Исходня точка плоскости M0</param>
/// <param name="vN">Нормаль плоскости</param>
/// <param name="tri">Треугольник (3 вершины)</param>
/// <param name="vAboveTris">out Буффер, куда запишем новые труги. ПОД плоскостью</param>
/// <param name="vBelowTris">out Буффер, куда запишем новые труги. НАД плоскостью</param>
/// <param name="fEpsilon">Эпсилон/Допуск</param>
void SliceTriangle(const Vector3& vM0, const Vector3& vN, const Triangle& tri,
    std::vector<Triangle>& vAboveTris, std::vector<Triangle>& vBelowTris,
    float fEpsilon = 1e-5f);