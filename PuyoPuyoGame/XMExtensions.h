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

	// TODO:
	// - Add More of these as needed

}