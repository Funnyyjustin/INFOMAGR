#pragma once

#ifndef SPHERE_H
#define SPHERE_H

#include "primitive.h"

class Sphere : public Primitive
{
	public:
		Sphere(const Point3& center, double radius, shared_ptr<Material> mat) : center(center), radius(std::fmax(0, radius)),mat(mat)
		{
			auto rvec = Vec3(radius, radius, radius);
			boundingbox = aabb(center - rvec, center + rvec);
		}

		/// <summary>
		/// Calculates if a ray hits this primitive.
		/// </summary>
		/// <param name="r">= The ray that is being traced.</param>
		/// <param name="ray_t">= The interval of distances in which a ray can intersect with a primitive.</param>
		/// <param name="rec">The hit record of the ray.</param>
		/// <returns>true if the ray hits this sphere, false if not.</returns>
		bool hit(const Ray& r, Interval ray_t, Hit_record& rec) const override
		{
			// Calculate the discriminant
			Vec3 oc = center - r.origin();
			auto a = r.direction().length_sq();
			auto h = dot(r.direction(), oc);
			auto c = oc.length_sq() - radius * radius;
			auto d = h * h - a * c;

			// If the discriminant is negative, then there is no intersection between the ray and the sphere
			if (d < 0)
				return false;

			auto sd = std::sqrt(d);

			// Check if the intersection is in the interval of the distances
			auto root = (h - sd) / a;
			if (!ray_t.contains(root))
			{
				root = (h + sd) / a;
				if (!ray_t.contains(root))
					return false;
			}

			// At this point, there is a valid intersection between the ray and the primitive
			// Set face normal and return true
			rec.t = root;
			rec.p = r.at(rec.t);
			Vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_normal(r, outward_normal);
			rec.mat = mat;

			return true;
		}

		aabb hitBox() const override { return boundingbox; }

	private:
		Point3 center;
		double radius;
		shared_ptr<Material> mat;
		aabb boundingbox;
};

#endif
