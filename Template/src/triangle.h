//
// Created by joost on 26/11/2025.
//

#ifndef ADVANCED_GRAPHICS_TRIANGLE_H

#define ADVANCED_GRAPHICS_TRIANGLE_H

#include <memory>

#include "primitive.h"
#include "common.h"

class Triangle : public Primitive
{
    public:
        Triangle(const Point3& Q, const Vec3& u, const Vec3& v, std::shared_ptr<Material> mat)
        : Q(Q), u(u), v(v), mat(mat)
        {
            auto n = cross(u, v);
            normal = unit_vector(n);
            D = dot(normal, Q);
            w = n / dot(n, n);
        }

        bool hit(const Ray& ray, Interval ray_t, Hit_record& rec) const override
        {
            // if ray is parallel to tringle (enough), dont hit
            auto denom = dot(normal, ray.direction());
            if (std::fabs(denom) < 1e-8) {
                return false;
            }


            auto dist = (D-dot(normal, ray.origin()))/ denom;


            if (!ray_t.contains(dist)) {
                return false;
            }


            auto intersection = ray.at(dist);
            Vec3 hit_vec = intersection - Q;


            // alpha and beta are like u and v from Q to intersection point
            auto alpha = dot(w, cross(hit_vec, v));
            auto beta = dot(w, cross(u, hit_vec));

            if (!is_interior(alpha, beta)) {
                return false;
            }


            rec.t = dist;
            rec.p = intersection;
            rec.mat = mat;
            rec.set_face_normal(ray, normal);

            return true;
        }

        virtual bool is_interior(double alpha, double beta) const
        {
            Interval interval = Interval(0, 1);

            if (alpha > 0 && beta > 0 && alpha + beta < 1)
                return true;

            return false;
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