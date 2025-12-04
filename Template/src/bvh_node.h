//
// Created by joost on 04/12/2025.
//

#ifndef RAYTRACER_BVH_NODE_H
#define RAYTRACER_BVH_NODE_H
#include <memory>
#include <algorithm>

#include "aabb.h"
#include "primitive.h"
#include "world.h"
using namespace std;

class bvh_node : public Primitive
{
    public:
        bvh_node(World world) : bvh_node(world.objects, 0, world.objects.size()) {}

        bvh_node(vector<shared_ptr<Primitive>>& objects, size_t start, size_t end)
        {
            bbox = aabb(Interval::empty, Interval::empty, Interval::empty);

            for (size_t i = start; i < end; i++)
                bbox = aabb(bbox, objects[i]->hitBox());

            int axis = bbox.longest_axis();

            auto comparison = (axis == 0) ? compareX
                                     : (axis == 1) ? compareY
                                                   : compareZ;

            size_t span = end - start;

            switch (span) {
                case 1:
                    left = objects[start];
                    right = objects[start];
                    break;
                case 2:
                    left = objects[start];
                    right = objects[start + 1];
                    break;
                default:
                    sort(begin(objects) + start, begin(objects) + end, comparison);

                    auto middle = start + span / 2;
                    left = make_shared<bvh_node>(objects, start, middle);
                    right = make_shared<bvh_node>(objects, middle, end);
            }

        }

        static bool compare(const shared_ptr<Primitive>& a, const shared_ptr<Primitive>& b, int axis)
        {
            auto intervalA = a->hitBox().axis_by_index(axis);
            auto intervalB = b->hitBox().axis_by_index(axis);
            return intervalA.min < intervalB.min;
        }


        static bool compareX(const shared_ptr<Primitive>& a, const shared_ptr<Primitive>& b)
            { return compare(a,b,0); }

        static bool compareY(const shared_ptr<Primitive>& a, const shared_ptr<Primitive>& b)
            { return compare(a,b,1); }

        static bool compareZ(const shared_ptr<Primitive>& a, const shared_ptr<Primitive>& b)
            { return compare(a,b,2); }



        bool hit(const Ray &r, Interval ray_t, Hit_record &rec) const override
        {
            if (!bbox.hit(r, ray_t)) return false;

            bool hit_left = left->hit(r, ray_t, rec);
            bool hit_right;
            if (hit_left) hit_right = right->hit(r, Interval(ray_t.min, rec.t), rec);
            else hit_right = right->hit(r, ray_t, rec);

            return hit_left || hit_right;
        }

        aabb hitBox() const override { return bbox; }

    private:
        shared_ptr<Primitive> left, right;
        aabb bbox;
};

#endif //RAYTRACER_BVH_NODE_H