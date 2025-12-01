#pragma once

#ifndef WORLD_H
#define WORLD_H

#include "primitive.h"
#include <vector>

// The world contains a list of primitives.
class World : public Primitive
{
	public:
		std::vector<shared_ptr<Primitive>> objects;

		World() {}

		World(shared_ptr<Primitive> object) { add(object); }

		void clear() { objects.clear(); }

		/// <summary>
		/// Add a primitive to the world.
		/// </summary>
		/// <param name="object">= the primitive in question</param>
		void add(shared_ptr<Primitive> object)
		{
			objects.push_back(object);
			primContainer = aabb(primContainer, object->hitBox());
		}

		aabb hitBox() const override
		{
			return primContainer;
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

	private:
		aabb primContainer;
};

#endif
