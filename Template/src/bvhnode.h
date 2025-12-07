#ifndef BVH_H
#define BVH_H

#include "aabb.h"
#include "primitive.h"
#include "world.h"

class bvh_node : public Primitive {
public:
    bvh_node(World list) : bvh_node(list.objects, 0, list.objects.size())
    {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list, which we will modify. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy.
    }

    bvh_node(std::vector<shared_ptr<Primitive>>& objects, size_t start, size_t end)
    {
        bbox = aabb::empty;
        for (size_t object_index = start; object_index < end; object_index++)
            bbox = aabb(bbox, objects[object_index]->hitBox());

        int axis = bbox.longest_axis();

        //int axis = random_int(0, 2);

        auto comparator = (axis == 0) ? box_x_compare
            : (axis == 1) ? box_y_compare
            : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1)
        {
            left = right = objects[start];
        }
        else if (object_span == 2)
        {
            left = objects[start];
            right = objects[start + 1];
        }
        else
        {
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

            auto mid = start + object_span / 2;
            left = make_shared<bvh_node>(objects, start, mid);
            right = make_shared<bvh_node>(objects, mid, end);
        }

        //bbox = aabb(left->hitBox(), right->hitBox());
    }

    bool hit(const Ray& r, Interval ray_t, Hit_record& rec) const override
    {
        rec.traversal_steps++;

        if (!bbox.hit(r, ray_t))
            return false;

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, Interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    aabb hitBox() const override { return bbox; }

private:
    shared_ptr<Primitive> left;
    shared_ptr<Primitive> right;
    aabb bbox;

    static bool box_compare(
        const shared_ptr<Primitive> a, const shared_ptr<Primitive> b, int axis_index
    ) {
        auto a_axis_interval = a->hitBox().axis_interval(axis_index);
        auto b_axis_interval = b->hitBox().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare(const shared_ptr<Primitive> a, const shared_ptr<Primitive> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const shared_ptr<Primitive> a, const shared_ptr<Primitive> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const shared_ptr<Primitive> a, const shared_ptr<Primitive> b) {
        return box_compare(a, b, 2);
    }
};

#endif