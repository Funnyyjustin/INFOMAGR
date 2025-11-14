#pragma once

#ifndef RAY_H
#define RAY_H

#include "Vec3.h"

class Ray
{
	public:
		Ray() {}

		Ray(const Point3& origin, const Vec3& direction) : orig(origin), dir(direction) {}

		const Point3& origin() const { return orig; }
		const Vec3& direction() const { return dir; }

		/// <summary>
		/// Gets the point from some distance t from the origin of the ray.
		/// </summary>
		/// <param name="t">= The distance from the origin of the ray.</param>
		/// <returns></returns>
		Point3 at(double t) const
		{
			return orig + t * dir;
		}

	private:
		Point3 orig;
		Vec3 dir;
};

#endif
