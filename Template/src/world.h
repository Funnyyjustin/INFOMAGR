#pragma once

#ifndef WORLD_H
#define WORLD_H

#include "primitive.h"
#include <vector>

#include "BVH.h"

// The world contains a list of primitives.
class World : public Primitive
{
	public:
		std::vector<shared_ptr<Primitive>> objects;
		Node* bvh;

		World() {bvh = new Node();}

		World(shared_ptr<Primitive> object) { add(object); }

		void clear() { objects.clear(); }

		/// <summary>
		/// Add a primitive to the world.
		/// </summary>
		/// <param name="object">= the primitive in question</param>
		void add(shared_ptr<Primitive> object)
		{
			objects.push_back(object);
			bvh->objects.push_back(object);
		}

		/// <summary>
		/// Checks if a ray hits some primitive in the world.
		/// </summary>
		/// <param name="r">= The ray that is being traced.</param>
		/// <param name="ray_t">= The interval of distances where the intersection is valid.</param>
		/// <param name="rec">= The hit record of the ray.</param>
		/// <returns></returns>
		bool hit(const Ray& r, Interval ray_t, Hit_record& rec) const override
		{
			Hit_record temp_rec;
			bool hit_anything = false;
			auto closest_so_far = ray_t.max;

			for (const auto& object : objects)
			{
				if (object->hit(r, Interval(ray_t.min, closest_so_far), temp_rec))
				{
					hit_anything = true;
					closest_so_far = temp_rec.t;
					rec = temp_rec;
				}
			}

			return hit_anything;
		}

		aabb hitBox() const override { return bvh->hitbox;}

		Point3 center() const override {return hitBox().center();}
};

/*
bool bvh_hit(const Ray& r, Interval ray_t, Hit_record& rec, Node node) const
		{
			/// traverse bvh structure until no further children
			/// if node.triangles is empty
			///		if ray hits node.child1
			///			call this function with node.child1 as node
			///		if ray hits node.child2
			///			call this function with node.child2 as node
			///
			///	if node.triangles is not empty, is it a leaf

			if (node.triangles.empty()) return false;

			if (node.child1->triangles.empty())
			{
				if (!node.child1->hitbox.hit(r, ray_t))
					return bvh_hit(r, ray_t, rec, *node.child1);
			}
			else if (node.child2->triangles.empty())
			{
				if (!node.child2->hitbox.hit(r, ray_t))
					return bvh_hit(r, ray_t, rec, *node.child2);
			}
			else
			{
				Hit_record temp_rec;
				bool hit_anything = false;
				auto closest_so_far = ray_t.max;

				for (const auto& triangle : node.triangles)
				{
					if (triangle->hit(r, Interval(ray_t.min, closest_so_far), temp_rec))
					{
						hit_anything = true;
						closest_so_far = temp_rec.t;
						rec = temp_rec;
					}
				}

				return hit_anything;
			}

		}

 */

#endif
