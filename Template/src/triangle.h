//
// Created by joost on 26/11/2025.
//

#ifndef ADVANCED_GRAPHICS_TRIANGLE_H

#define ADVANCED_GRAPHICS_TRIANGLE_H

#include <cmath>
#include <memory>

#include "primitive.h"
#include "Vec3.h"

class Triangle : public Primitive
{
    public:
        Triangle(const Point3& Q, const Vec3& u, const Vec3& v, shared_ptr<Material> mat)
        : Q(Q), u(u), v(v), mat(mat)
        {
            auto n = cross(u, v);
            normal = unit_vector(n);
            D = dot(normal, Q);
            w = n / dot(n, n);
        }

        bool hit(const Ray& r, Interval ray_t, Hit_record& rec) const override
        {
            auto denom = dot(normal, r.direction());

            if (std::fabs(denom) < 1e-8) return false;

            auto t = (D-dot(normal, r.origin()))/ denom;
            if (!ray_t.contains(t)) return false;

        }

    private:
        Point3 Q;
        Vec3 u,v;
        Vec3 w;
        shared_ptr<Material> mat;
        Vec3 normal;
        double D;
};


#endif //ADVANCED_GRAPHICS_TRIANGLE_H