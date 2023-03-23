#ifndef RBST_MATH_HPP
#define RBST_MATH_HPP

//fpm
#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

using num_det = fpm::fixed<std::int32_t, std::int64_t, 24>;
using int8 = std::int8_t;
using int16 = std::int16_t;

struct Vec2
{
	num_det x;
	num_det y;
};

namespace v2
{
	inline Vec2 zero()
	{
		return Vec2{ num_det{ 0 }, num_det{ 0 } };
	}

	inline Vec2 up()
	{
		return Vec2{ num_det{ 0 }, num_det{ 1 } };
	}

	inline Vec2 down()
	{
		return Vec2{ num_det{ 0 }, num_det{ -1 } };
	}

	inline Vec2 left()
	{
		return Vec2{ num_det{ -1 }, num_det{ 0 } };
	}

	inline Vec2 right()
	{
		return Vec2{ num_det{ 1 }, num_det{ 0 } };
	}

	inline Vec2 add(Vec2 a, Vec2 b)
	{
		return Vec2{ a.x + b.x, a.y + b.y };
	}

	inline Vec2 sub(Vec2 a, Vec2 b)
	{
		return Vec2{ a.x - b.x, a.y - b.y };
	}

	inline num_det dot(Vec2 a, Vec2 b)
	{
		return (a.x * b.x) + (a.y * b.y);
	}

	inline bool equal(Vec2 a, Vec2 b)
	{
		return (a.x == b.x) && (a.y == b.y);
	}

	inline Vec2 scalarMult(Vec2 v, num_det n)
	{
		return Vec2{ v.x * n, v.y * n };
	}

	inline Vec2 scalarDiv(Vec2 v, num_det n)
	{
		return Vec2{ v.x / n, v.y / n };
	}

	inline num_det length(Vec2 v)
	{
		return fpm::sqrt((v.x * v.x) + (v.y * v.y));
	}

	inline Vec2 normalize(Vec2 v)
	{
		return equal(v, zero()) ? v : scalarDiv(v, length(v));
	}

	//if you want to multiply the normal by a value
	inline Vec2 normalizeMult(Vec2 v, num_det n)
	{
		return equal(v, zero()) ? v : scalarMult(v, n / length(v));
	}

	inline Vec2 lerp(Vec2 zero, Vec2 one, num_det alpha)
	{
		return add(zero, scalarMult(sub(one, zero), alpha));
	}

	//angle in radians, please
	//positive rotates counter-clockwise
	Vec2 rotate(Vec2 v, num_det angle)
	{
		num_det cos = fpm::cos(angle);
		num_det sin = fpm::sin(angle);
		num_det x = (v.x * cos) - (v.y * sin);
		num_det y = (v.x * sin) + (v.y * cos);
		return Vec2{ x, y };
	}
	
	//projection of a on b
	Vec2 projection(Vec2 a, Vec2 b)
	{
		Vec2 b_normal = normalize(b);
		num_det relative = dot(a, b_normal);
		return scalarMult(b_normal, relative);
	}

	//rejection of a from b
	inline Vec2 rejection(Vec2 a, Vec2 b)
	{
		return sub(a, projection(a,b));
	}

	// distance between closest point in a ray and the center of something
	// won't check radius because them multiple occasions can account for things such as grazing
	num_det closest(Vec2 origin, Vec2 vector, Vec2 center)
	{
		Vec2 o2c = sub(center, origin);
		num_det relative = dot(o2c, vector);
		if (relative < num_det{ 0 })
		{
			// dunno how to define the highest value this type holds so this goes lmao
			return num_det{ 127 };
		}
		Vec2 projection = scalarMult(vector, relative);
		Vec2 closest = sub(o2c, projection);
		return length(closest);
	}
}

#endif