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

            if (newMin < ray_t.min) ray_t.min = newMin;
            if (newMax > ray_t.max) ray_t.max = newMax;

            return ray_t.max > ray_t.min;
        }

        bool boxes_overlap(const aabb& box)
        {
            bool x_overlap = x.overlap(box.x);
            bool y_overlap = y.overlap(box.y);
            bool z_overlap = z.overlap(box.z);
            return x_overlap && y_overlap && z_overlap;
        }

        bool contains(Point3 point)
        {
            bool x_contains = point.x() >= x.min && point.x() <= x.max;
            bool y_contains = point.y() >= y.min && point.y() <= y.max;
            bool z_contains = point.z() >= z.min && point.z() <= z.max;
            return x_contains && y_contains && z_contains;
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

        std::tuple<bool, Point3, Point3> boxRayIntersection(Ray ray)
        {
            float tEnter = -INFINITY;
            float tEnd = INFINITY;

            const Interval axes[3] = { x, y, z };
            const float origin[3] = { ray.origin().x(), ray.origin().y(), ray.origin().z() };
            const float dir[3] = { ray.direction().x(), ray.direction().y(), ray.direction().z() };

            for (int i = 0; i < 3; i++)
            {
                float o = origin[i];
                float d = dir[i];
                float minb = axes[i].min;
                float maxb = axes[i].max;

                if (std::abs(d) < 1e-8f)
                {
                    if (o < minb || o > maxb)
                        return { false, Point3(0, 0, 0), Point3(0, 0, 0) };
                }
                else
                {
                    float t1 = (minb - o) / d;
                    float t2 = (maxb - o) / d;

                    if (t1 > t2) std::swap(t1, t2);

                    tEnter = std::max(tEnter, t1);
                    tEnd = std::min(tEnd, t2);

                    if (tEnter > tEnd)
                        return { false, Point3(0, 0, 0), Point3(0, 0, 0) };
                }
            }

            if (tEnd < 0)
                return { false, Point3(0, 0, 0), Point3(0, 0, 0) };

            Point3 entryPoint = ray.origin() + ray.direction() * tEnter;
            Point3 exitPoint = ray.origin() + ray.direction() * tEnd;
            return { true, entryPoint, exitPoint };
        }
};

#endif //RAYTRACER_AABB_H