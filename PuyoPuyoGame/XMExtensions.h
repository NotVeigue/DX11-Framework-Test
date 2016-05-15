#pragma once
#include <DirectXMath.h>

using namespace DirectX;

// Just a precaution to avoid possibly conflicting with other libraries
namespace XMExtensions
{

	inline bool operator==(const XMFLOAT2& a, const XMFLOAT2& b)
	{
		return a.x == b.x && a.y == b.y;
	}

	inline bool operator!=(const XMFLOAT2& a, const XMFLOAT2& b)
	{
		return a.x != b.x || a.y != b.y;
	}

	inline XMFLOAT2 operator-(const XMFLOAT2& a, const XMFLOAT2& b)
	{
		return XMFLOAT2(a.x - b.x, a.y - b.y);
	}

	inline XMFLOAT2 operator-(const XMFLOAT2& a)
	{
		return XMFLOAT2(-a.x, -a.y);
	}

	inline XMFLOAT2 operator+(const XMFLOAT2& a, const XMFLOAT2& b)
	{
		return XMFLOAT2(a.x + b.x, a.y + b.y);
	}

	inline XMFLOAT2 operator*(const XMFLOAT2& a, float n)
	{
		return XMFLOAT2(a.x * n, a.y * n);
	}

	inline XMFLOAT3 operator*(const XMFLOAT3& a, float n)
	{
		return XMFLOAT3(a.x * n, a.y * n, a.z * n);
	}

	inline XMFLOAT4 operator*(const XMFLOAT4& a, float n)
	{
		return XMFLOAT4(a.x * n, a.y * n, a.z * n, a.w * n);
	}

	inline XMFLOAT2 operator/(const XMFLOAT2& a, float n)
	{
		return XMFLOAT2(a.x / n, a.y / n);
	}

	inline XMFLOAT3 operator/(const XMFLOAT3& a, float n)
	{
		return XMFLOAT3(a.x / n, a.y / n, a.z / n);
	}

	inline XMFLOAT4 operator/(const XMFLOAT4& a, float n)
	{
		return XMFLOAT4(a.x / n, a.y / n, a.z / n, a.w/n);
	}

	inline XMFLOAT2 normalize(const XMFLOAT2& v)
	{
		return v/sqrtf(v.x * v.x + v.y * v.y);
	}

	inline XMFLOAT3 normalize(const XMFLOAT3& v)
	{
		return v/sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	inline XMFLOAT4 normalize(const XMFLOAT4& v)
	{
		return v/sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	}

	// TODO:
	// - Add More of these as needed

}