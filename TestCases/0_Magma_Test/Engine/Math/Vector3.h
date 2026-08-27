#pragma once

//-----------------------------------------------------------------------------
// 3D vector
//-----------------------------------------------------------------------------
struct Vector3
{
	float x;
	float y;
	float z;

public:
	Vector3();
	Vector3(float x, float y, float z);

public:
	void Set(float x, float y, float z);

	float Length() const;
	float LengthSquared() const;

	void Normalize();

	Vector3 Normalized() const;

	float Dot(const Vector3& other) const;
	Vector3 Cross(const Vector3& other) const;

public:
	Vector3 operator+(const Vector3& other) const;
	Vector3 operator-(const Vector3& other) const;
	Vector3 operator*(float value) const;
	Vector3 operator/(float value) const;

	Vector3& operator+=(const Vector3& other);
	Vector3& operator-=(const Vector3& other);
	Vector3& operator*=(const Vector3& other);
	Vector3& operator*=(float value);
	Vector3& operator/=(float value);

	bool operator==(const Vector3& other) const;
	bool operator!=(const Vector3& other) const;
};