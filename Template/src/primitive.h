#pragma once

#ifndef PRIMITIVE_H
#define PRIMITIVE_H

class Material;

// Keep track of the hits of a ray
class Hit_record
{
	public:
		Point3 p;
		Vec3 normal;
		shared_ptr<Material> mat;
		double t;
		bool front_face;
		int traversal_steps;
		int intersection_tests;

		/// <summary>
		/// Sets the normal inward or outward.
		/// </summary>
		/// <param name="r">= The ray that is being traced.</param>
		/// <param name="outward_normal">= The normal on the surface of the intersection between the ray and the primitive.</param>
		void set_face_normal(const Ray& r, const Vec3& outward_normal)
		{
			front_face = dot(r.direction(), outward_normal) < 0;
			normal = front_face ? outward_normal : -outward_normal;
		}
};

class Primitive
{
	public:
		virtual ~Primitive() = default;

		virtual bool hit(const Ray& r, Interval ray_t, Hit_record& rec) const = 0;

		virtual aabb hitBox() const = 0;
};

#endif
