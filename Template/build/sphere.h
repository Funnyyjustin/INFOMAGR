#pragma once

#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable
{
	public:
		sphere(const point3& center, double radius) : center(center), radius(std::fmax(0, radius)) {}

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override
		{
			vec3 oc = center - r.origin();
			auto a = r.direction().length_sq();
			auto h = dot(r.direction(), oc);
			auto c = oc.length_sq() - radius * radius;
			auto d = h * h - a * c;

			if (d < 0)
				return false;

			auto sd = std::sqrt(d);

			auto root = (h - sd) / a;
			if (root <= ray_t.min || ray_t.max <= root)
			{
				root = (h + sd) / a;
				if (root <= ray_t.min || ray_t.max <= root)
					return false;
			}

			rec.t = root;
			rec.p = r.at(rec.t);
			vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_normal(r, outward_normal);

			return true;
		}

	private:
		point3 center;
		double radius;
};

#endif
