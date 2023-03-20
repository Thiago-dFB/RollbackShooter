//fpm
#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

using num8_24 = fpm::fixed_8_24;


struct Vec2
{
	num8_24 x;
	num8_24 y;
};

namespace v2
{
	inline Vec2 zero()
	{
		return Vec2{ num8_24{ 0 }, num8_24{ 0 } };
	}

	inline Vec2 up()
	{
		return Vec2{ num8_24{ 0 }, num8_24{ 1 } };
	}

	inline Vec2 down()
	{
		return Vec2{ num8_24{ 0 }, num8_24{ -1 } };
	}

	inline Vec2 left()
	{
		return Vec2{ num8_24{ -1 }, num8_24{ 0 } };
	}

	inline Vec2 right()
	{
		return Vec2{ num8_24{ 1 }, num8_24{ 0 } };
	}

	inline Vec2 add(Vec2 a, Vec2 b)
	{
		return Vec2{ a.x + b.x, a.y + b.y };
	}

	inline Vec2 sub(Vec2 a, Vec2 b)
	{
		return Vec2{ a.x - b.x, a.y - b.y };
	}

	inline num8_24 dot(Vec2 a, Vec2 b)
	{
		return (a.x * b.x) + (a.y * b.y);
	}

	inline bool equal(Vec2 a, Vec2 b)
	{
		return (a.x == b.x) && (a.y == b.y);
	}

	inline Vec2 scalarMult(Vec2 v, num8_24 n)
	{
		return Vec2{ v.x * n, v.y * n };
	}

	inline Vec2 scalarDiv(Vec2 v, num8_24 n)
	{
		return Vec2{ v.x / n, v.y / n };
	}

	inline num8_24 length(Vec2 v)
	{
		return fpm::sqrt((v.x * v.x) + (v.y * v.y));
	}

	inline Vec2 normalize(Vec2 v)
	{
		return equal(v, zero()) ? v : scalarDiv(v, length(v));
	}

	//if you want to multiply the normal by a value
	inline Vec2 normalizeMult(Vec2 v, num8_24 n)
	{
		return equal(v, zero()) ? v : scalarMult(v, n / length(v));
	}

	//angle in radians, please
	//positive rotates counter-clockwise
	Vec2 rotate(Vec2 v, num8_24 angle)
	{
		num8_24 cos = fpm::cos(angle);
		num8_24 sin = fpm::sin(angle);
		num8_24 x = (v.x * cos) - (v.y * sin);
		num8_24 y = (v.x * sin) + (v.y * cos);
		return Vec2{ x, y };
	}
	
	//projection of a on b
	Vec2 projection(Vec2 a, Vec2 b)
	{
		Vec2 b_normal = normalize(b);
		num8_24 relative = dot(a, b_normal);
		return scalarMult(b_normal, relative);
	}

	//rejection of a from b
	inline Vec2 rejection(Vec2 a, Vec2 b)
	{
		return sub(a, projection(a,b));
	}

	// distance between closest point in a ray and the center of something
	// won't check radius because them multiple occasions can account for things such as grazing
	num8_24 closest(Vec2 origin, Vec2 vector, Vec2 center)
	{
		Vec2 o2c = sub(center, origin);
		num8_24 relative = dot(o2c, vector);
		if (relative < num8_24{ 0 })
		{
			// dunno how to define the highest value this type holds so this goes lmao
			return num8_24{ 127 };
		}
		Vec2 projection = scalarMult(vector, relative);
		Vec2 closest = sub(o2c, projection);
		return length(closest);
	}
}