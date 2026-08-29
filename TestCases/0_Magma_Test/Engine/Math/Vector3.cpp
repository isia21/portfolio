#include "stdafx.h"
#include "../Graphics.h"
#include "Vector3.h"

//-----------------------------------------------------------------------------
// Vector3
//-----------------------------------------------------------------------------
Vector3::Vector3()
	: x(0.0f)
	, y(0.0f)
	, z(0.0f)
{}

Vector3::Vector3(float x, float y, float z)
	: x(x)
	, y(y)
	, z(z)
{}

Vector3::Vector3(const Vertex3D& vertex)
	: x(vertex.x)
	, y(vertex.y)
	, z(vertex.z)
{}

//-----------------------------------------------------------------------------
// Set
//-----------------------------------------------------------------------------
void Vector3::Set(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


//-----------------------------------------------------------------------------
// Length
//-----------------------------------------------------------------------------
float Vector3::Length() const
{
	return sqrtf(x * x + y * y + z * z);
}

float Vector3::LengthSquared() const
{
	return x * x + y * y + z * z;
}


//-----------------------------------------------------------------------------
// Normalize
//-----------------------------------------------------------------------------
void Vector3::Normalize()
{
	const float fLength = Length();

	if (fLength <= 0.0f)
		return;

	x /= fLength;
	y /= fLength;
	z /= fLength;
}

Vector3 Vector3::Normalized() const
{
	const float fLength = Length();

	if (fLength <= 0.0f)
		return Vector3();

	return Vector3(
		x / fLength,
		y / fLength,
		z / fLength);
}


//-----------------------------------------------------------------------------
// Dot product
//-----------------------------------------------------------------------------
float Vector3::Dot(const Vector3& other) const
{
	return
		x * other.x +
		y * other.y +
		z * other.z;
}


//-----------------------------------------------------------------------------
// Cross product
//-----------------------------------------------------------------------------
Vector3 Vector3::Cross(const Vector3& other) const
{
	return Vector3(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x);
}


//-----------------------------------------------------------------------------
// Operators
//-----------------------------------------------------------------------------
Vector3 Vector3::operator+(const Vector3& other) const
{
	return Vector3(
		x + other.x,
		y + other.y,
		z + other.z);
}

Vector3 Vector3::operator-(const Vector3& other) const
{
	return Vector3(
		x - other.x,
		y - other.y,
		z - other.z);
}

Vector3 Vector3::operator*(float value) const
{
	return Vector3(
		x * value,
		y * value,
		z * value);
}

Vector3 Vector3::operator/(float value) const
{
	if (value == 0.0f)
		return Vector3();

	return Vector3(
		x / value,
		y / value,
		z / value);
}


//-----------------------------------------------------------------------------
// Compound operators
//-----------------------------------------------------------------------------
Vector3& Vector3::operator+=(const Vector3& other)
{
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

Vector3& Vector3::operator*=(const Vector3& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;

	return *this;
}

Vector3& Vector3::operator-=(const Vector3& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

Vector3& Vector3::operator*=(float value)
{
	x *= value;
	y *= value;
	z *= value;

	return *this;
}

Vector3& Vector3::operator/=(float value)
{
	if (value == 0.0f)
		return *this;

	x /= value;
	y /= value;
	z /= value;

	return *this;
}


//-----------------------------------------------------------------------------
// Comparison
//-----------------------------------------------------------------------------
bool Vector3::operator==(const Vector3& other) const
{
	return
		x == other.x &&
		y == other.y &&
		z == other.z;
}

bool Vector3::operator!=(const Vector3& other) const
{
	return !(*this == other);
}