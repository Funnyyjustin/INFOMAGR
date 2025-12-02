#ifndef Vec3_H
#define Vec3_H

#include "common.h"

class Vec3
{
	public:
		double e[3];

		Vec3() : e{ 0, 0, 0 } {}
		Vec3(double e0, double e1, double e2) : e{ e0, e1, e2 } {}

		double x() const { return e[0]; }
		double y() const { return e[1]; }
		double z() const { return e[2]; }

		Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }
		double operator[] (int i) const { return e[i]; }
		double& operator[] (int i) { return e[i]; }

		/// <summary>
		/// Adds two vectors to each other.
		/// </summary>
		/// <param name="v">:: Vec3</param>
		/// <returns>A single vector.</returns>
		Vec3& operator+=(const Vec3& v)
		{
			e[0] += v.e[0];
			e[1] += v.e[1];
			e[2] += v.e[2];
			return *this;
		}

		/// <summary>
		/// Multiplies the vector with some value t.
		/// </summary>
		/// <param name="t">:: double</param>
		/// <returns>A single vector.</returns>
		Vec3& operator*=(double t)
		{
			e[0] *= t;
			e[1] *= t;
			e[2] *= t;
			return *this;
		}

		/// <summary>
		/// Multiples the vector with the inverse of some value t.
		/// </summary>
		/// <param name="t">:: double</param>
		/// <returns>A single vector.</returns>
		Vec3 operator/=(double t)
		{
			return *this *= 1 / t;
		}

		/// <summary>
		/// Gets the length of the vector.
		/// </summary>
		/// <returns></returns>
		double length() const
		{
			return std::sqrt(length_sq());
		}

		/// <summary>
		/// Gets the length of the vector, squared.
		/// </summary>
		/// <returns></returns>
		double length_sq() const
		{
			return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
		}

		/// <summary>
		/// Returns true if the vector is very close to zero in all dimensions.
		/// </summary>
		/// <returns></returns>
		bool near_zero() const
		{
			auto s = 1e-8;
			return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
		}

		/// <summary>
		/// Gets a random vector.
		/// </summary>
		/// <returns></returns>
		static Vec3 random()
		{
			return Vec3(random_double(), random_double(), random_double());
		}

		/// <summary>
		/// Gets a random vector.
		/// </summary>
		/// <param name="min">= The minimum value any of the scalars of the random vector can have.</param>
		/// <param name="max">= The maximum value any of the scalars of the random vector can have.</param>
		/// <returns></returns>
		static Vec3 random(double min, double max)
		{
			return Vec3(random_double(min, max), random_double(min, max), random_double(min, max));
		}
};

using Point3 = Vec3;

/// <summary>
/// Prints the vector to the screen.
/// </summary>
/// <param name="out"></param>
/// <param name="v"></param>
/// <returns></returns>
inline std::ostream& operator<<(std::ostream& out, const Vec3& v)
{
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

/// <summary>
/// Adds two vectors to each other
/// </summary>
/// <param name="u">:: Vec3</param>
/// <param name="v">:: Vec3</param>
/// <returns>A single Vec3 (u + v)</returns>
inline Vec3 operator+(const Vec3& u, const Vec3& v)
{
	return Vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

/// <summary>
/// Subtracts two vectors u and v from each other.
/// </summary>
/// <param name="u">:: Vec3</param>
/// <param name="v">:: Vec3</param>
/// <returns>A single vector (u - v)</returns>
inline Vec3 operator-(const Vec3& u, const Vec3& v)
{
	return Vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

/// <summary>
/// Multiplies two vectors with each other.
/// </summary>
/// <param name="u">:: Vec3</param>
/// <param name="v">:: Vec3</param>
/// <returns>A single vector (u * v)</returns>
inline Vec3 operator*(const Vec3& u, const Vec3& v)
{
	return Vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

/// <summary>
/// Multiplies a value with a vector.
/// </summary>
/// <param name="t">:: double</param>
/// <param name="v">:: Vec3</param>
/// <returns>A single vector (t * v)</returns>
inline Vec3 operator*(double t, const Vec3& v)
{
	return Vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

/// <summary>
/// Multiplies a vector with a value.
/// </summary>
/// <param name="v">:: Vec3</param>
/// <param name="t">:: double</param>
/// <returns>A single vector (v * t)</returns>
inline Vec3 operator*(const Vec3& v, double t)
{
	return t * v;
}

/// <summary>
/// Multiplies the inverse of a value with a vector.
/// </summary>
/// <param name="v">:: Vec3</param>
/// <param name="t">:: double</param>
/// <returns></returns>
inline Vec3 operator/(const Vec3& v, double t)
{
	return (1 / t) * v;
}

/// <summary>
/// Gets the dot product between two vectors.
/// </summary>
/// <param name="u">:: Vec3</param>
/// <param name="v">:: Vec3</param>
/// <returns>A single vector</returns>
inline double dot(const Vec3& u, const Vec3& v)
{
	return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

/// <summary>
/// Gets the cross product between two vectors.
/// </summary>
/// <param name="u">:: Vec3</param>
/// <param name="v">:: Vec3</param>
/// <returns></returns>
inline Vec3 cross(const Vec3& u, const Vec3& v)
{
	return Vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1], u.e[2] * v.e[0] - u.e[0] * v.e[2], u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

/// <summary>
/// Transforms a vector into a unit vector.
/// </summary>
/// <param name="v">:: Vec3</param>
/// <returns>The unit vector of v</returns>
inline Vec3 unit_vector(const Vec3& v)
{
	return v / v.length();
}

/// <summary>
/// Gets a random (unit) vector
/// </summary>
/// <returns></returns>
inline Vec3 random_unit_vector()
{
	while (true)
	{
		auto p = Vec3::random(-1, 1);
		auto lensq = p.length_sq();

		if (1e-160 < lensq && lensq <= 1)
			return p / sqrt(lensq);
	}
}

/// <summary>
/// Gets a unit vector that is on the hemisphere of a sphere.
/// </summary>
/// <param name="n">= The surface normal of the sphere</param>
/// <returns></returns>
inline Vec3 random_on_hs(const Vec3& n)
{
	Vec3 on_sphere = random_unit_vector();

	if (dot(on_sphere, n) > 0.0)
		return on_sphere;

	return -on_sphere;
}

/// <summary>
/// Calculates the vector that is reflected, based on some incoming vector and the surface normal.
/// </summary>
/// <param name="v">:: Vec3</param>
/// <param name="n">:: Vec3</param>
/// <returns></returns>
inline Vec3 reflect(const Vec3& v, const Vec3& n)
{
	return v - 2 * dot(v, n) * n;
}

/// <summary>
/// Calculates the vector that is refracted, based on some incoming vector, the surface normal and some goofy sign I don't understand.
/// </summary>
/// <param name="uv">:: Vec3</param>
/// <param name="n">:: Vec3</param>
/// <param name="sign">:: double</param>
/// <returns></returns>
inline Vec3 refract(const Vec3& uv, const Vec3& n, double sign)
{
	auto cos_theta = std::fmin(dot(-uv, n), 1.0);
	Vec3 r_perp = sign * (uv + cos_theta * n);
	Vec3 r_par = -std::sqrt(std::fabs(1.0 - r_perp.length_sq())) * n;
	return r_perp + r_par;
}

/// <summary>
/// Some other utility function that gets a random unit vector in two dimensions.
/// </summary>
/// <returns></returns>
inline Vec3 random_in_unit_disk()
{
	while (true)
	{
		auto p = Vec3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_sq() < 1)
			return p;
	}
}

#endif