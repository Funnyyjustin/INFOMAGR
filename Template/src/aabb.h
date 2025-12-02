#ifndef RAYTRACER_AABB_H
#define RAYTRACER_AABB_H
#include <algorithm>

#include "interval.h"
#include "ray.h"
using namespace std;

class aabb {
    public:
        Interval x, y, z;

        aabb() {}

        aabb(const Interval& x, const Interval& y, const Interval& z)
            : x(x), y(y), z(z) {}

        aabb(const Point3& a, const Point3& b)
        {
            if (a[0] <= b[0]) x = Interval(a[0], b[0]);
            else x = Interval(b[0], a[0]);

            if (a[1] <= b[1]) y = Interval(a[1], b[1]);
            else y = Interval(b[1], a[1]);

            if (a[2] <= b[2]) z = Interval(a[2], b[2]);
            else z = Interval(b[2], a[2]);
        }

        aabb(const aabb& box0, const aabb& box1)
        {
            x = Interval(box0.x, box1.x);
            y = Interval(box0.y, box1.y);
            z = Interval(box0.z, box1.z);

        }

        bool hit(const Ray& ray, Interval ray_t) const
        {
            return
                single_axis_hit(ray, ray_t, x, 0) &&
                single_axis_hit(ray, ray_t, y, 1) &&
                single_axis_hit(ray, ray_t, z, 2);
        }

        bool single_axis_hit(const Ray& ray, Interval ray_t, Interval axis, int axIndex) const
        {
            // get intersection with edges of the aabb
            auto intersect1 = (axis.min - ray.origin()[axIndex]) / ray.direction()[axIndex];
            auto intersect2 = (axis.max - ray.origin()[axIndex]) / ray.direction()[axIndex];

            // set new mins and maxs for hit interval of ray
            auto newMin = min(intersect1, intersect2);
            auto newMax = max(intersect1, intersect2);

            if (newMin < axis.min) axis.min = newMin;
            if (newMax > axis.max) axis.max = newMax;

            return ray_t.max > ray_t.min;
        }

        bool boxes_overlap(const aabb& box)
        {
            bool x_overlap = x.overlap(box.x);
            bool y_overlap = y.overlap(box.y);
            bool z_overlap = z.overlap(box.z);
            return x_overlap || y_overlap || z_overlap;
        }

        std::tuple<aabb, aabb> split(int axis)
        {
            if (axis == 0) // We split along the x-axis
            {
                int mid = (x.max - x.min) / 2;
                aabb left = aabb(Interval(x.min, mid), y, z);
                aabb right = aabb(Interval(mid, x.max), y, z);
                return { left, right };
            }
            else if (axis == 1) // We split along the y-axis
            {
                int mid = (y.max - y.min) / 2;
                aabb left = aabb(x, Interval(y.min, mid), z);
                aabb right = aabb(x, Interval(mid, y.max), z);
                return { left, right };
            }
            else if (axis == 2) // We split along the z-axis
            {
                int mid = (z.max - z.min) / 2;
                aabb left = aabb(x, y, Interval(z.min, mid));
                aabb right = aabb(x, y, Interval(mid, z.max));
                return { left, right };
            }
        }
};

#endif //RAYTRACER_AABB_H