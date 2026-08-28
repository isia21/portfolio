#pragma once

struct Vector3;

struct Triangle {
	Vector3 vertices[3];
};

class C3DObject;

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
/// <param name="vAboveTris">out Буффер, куда запишем новые труги. НАД плоскостью</param>
/// <param name="vBelowTris">out Буффер, куда запишем новые труги. ПОД плоскостью</param>
/// <param name="fEpsilon">Эпсилон/Допуск</param>
void SliceTriangle(const Vector3& vM0, const Vector3& vN, const Triangle& tri,
    std::vector<Triangle>& vAboveTris, std::vector<Triangle>& vBelowTris,
    float fEpsilon = 1e-5f);

/// <summary>
/// Вспомогательный метод. Вращает целевой Vertex согласно заданым углам в градусах.
/// ПРИМЕЧАНИЕ: Наш обход - Z->Y->X или Roll->Pitch->Yaw.
/// </summary>
/// <param name="vVertex">IN/OUT целевая вершина для вращения</param>
/// <param name="vRotDegs">Значения в градусах для вращения по 3 осям</param>
void RotateVertexByDegs_ZYX(Vector3& vVertex, const Vector3& vRotDegs);

/// <summary>
/// Вспомогательный метод. Трансформирует точку в локальных координатах меша в Глобальные/мировые координаты.
/// </summary>
/// <param name="vIn">Исходная точка</param>
/// <param name="vTrs">Translation - Смещение</param>
/// <param name="vRot">Rotation - Вращение</param>
/// <param name="vScl">Scale - Размер</param>
/// <returns>Новую точку Vector3 в мировых коорд. исходя из согласно трансформам</returns>
Vector3 LocalToWorldPos(const Vector3& vIn, const Vector3& vTrs, const Vector3& vRot, const Vector3& vScl);


/// <summary>
/// Нарезает исходный меш pIn, вершины которого находятся в локальных координатах, но переводятся в глобальный координаты, согласно Transform data.
/// Считаем Якорь и Нормаль переданной в Мировых координатах.
/// </summary>
/// <param name="pIn">Исходный меш</param>
/// <param name="vMeshTrs">Mesh.Transform.Translation</param>
/// <param name="vMeshRot">Mesh.Transform.Rotation</param>
/// <param name="vMeshScl">Mesh.Transform.Scale</param>
/// <param name="vM0">Исходня точка плоскости M0 в глобальных координатах</param>
/// <param name="vN">Нормаль плоскости</param>
/// <returns>Возвращает массив новых мешей (если заданая плоскость разрезала/попала в вершины pIn)</returns>
std::vector<C3DObject*> SliceMesh(const C3DObject& pIn, const Vector3& vMeshTrs, const Vector3& vMeshRot, const Vector3& vMeshScl, const Vector3& vM0, const Vector3& vN);


/// <summary>
/// Нарезает исходный меш pIn, вершины которого находятся в локальных координатах, но переводятся в глобальный координаты, согласно Transform data.
/// Считаем Якорь заданым в локальных координатах (0,0,0)
/// Считаем Нормально заданой Строго вверх (0,1,0 -- Yup)
/// </summary>
/// <param name="pIn">Исходный меш</param>
/// <param name="vMeshTrs">Mesh.Transform.Translation</param>
/// <param name="vMeshRot">Mesh.Transform.Rotation</param>
/// <param name="vMeshScl">Mesh.Transform.Scale</param>
/// <param name="vAnchorTrs">Translation/Смещение якоря</param>
/// <param name="vNormalRot">Rotation/Вращение/Куда смотрит Yup нормаль (в градусах)</param>
/// <returns>Возвращает массив новых мешей (если заданая плоскость разрезала/попала в вершины pIn)</returns>
std::vector<C3DObject*> SliceMesh(const C3DObject& pIn, const Vector3& vMeshTrs, const Vector3& vMeshRot, const Vector3& vMeshScl, const Vector3& vAnchorTrs, const Vector3& vNormalRot);




