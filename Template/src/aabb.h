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


    // includes point p in the intervals of the aabb
    void include(Point3& p)
    {
        x.max = max(p.x(), x.max);
        y.max = max(p.y(), y.max);
        z.max = max(p.z(), z.max);

        x.min = min(p.x(), x.min);
        y.min = min(p.y(), y.min);
        z.min = min(p.z(), z.min);
    }

    bool hit(const Ray& ray, Interval& ray_t)
    {
        return
            single_axis_hit(ray, x, ray_t, 0) &&
            single_axis_hit(ray, y, ray_t, 1) &&
            single_axis_hit(ray, z, ray_t, 2);
    }

    Point3 center()
    {
        return {(x.min + x.max) * 0.5, (y.min + y.max) * 0.5, (z.min + z.max) * 0.5};
    }

private:
    bool single_axis_hit(const Ray& ray, Interval& axis, Interval& ray_t, int axIndex)
    {
        // get intersection with edges of the aabb
        auto intersect1 = (axis.min - ray.origin()[axIndex]) / ray.direction()[axIndex];
        auto intersect2 = (axis.max - ray.origin()[axIndex]) / ray.direction()[axIndex];

        // set new mins and maxs for hit interval of ray
        auto newMin = min(intersect1, intersect2);
        auto newMax = max(intersect1, intersect2);

        return newMax > newMin;
        /*
        if (newMin < ray_t.min) ray_t.min = newMin;
        if (newMax > ray_t.max) ray_t.max = newMax;

        return ray_t.max > ray_t.min;
        */
    }


};

#endif //RAYTRACER_AABB_H