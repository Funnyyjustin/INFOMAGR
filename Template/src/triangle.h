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

            set_bounding_box();
        }

        virtual void set_bounding_box()
        {
            //auto d1 = aabb(Q, Q + u + v);
            //auto d2 = aabb(Q + u, Q + v);
            //box = aabb(d1, d2);

            Vec3 p1 = Q + u;
            Vec3 p2 = Q + v;

            double minx = std::min({ Q.x(), p1.x(), p2.x() });
            double miny = std::min({ Q.y(), p1.y(), p2.y() });
            double minz = std::min({ Q.z(), p1.z(), p2.z() });

            double maxx = std::max({ Q.x(), p1.x(), p2.x() });
            double maxy = std::max({ Q.y(), p1.y(), p2.y() });
            double maxz = std::max({ Q.z(), p1.z(), p2.z() });

            box = aabb(Interval(minx, maxx), Interval(miny, maxy), Interval(minz, maxz));
        }

        aabb hitBox() const override { return box; }

        bool hit(const Ray& r, Interval ray_t, Hit_record& rec) const override
        {
            rec.intersection_tests++;
            auto denom = dot(normal, r.direction());

            if (std::fabs(denom) < 1e-8) return false;

            auto t = (D - dot(normal, r.origin())) / denom;
            if (!ray_t.contains(t)) return false;

            auto intersection = r.at(t);
            Vec3 planar_hitpt_vector = intersection - Q;
            auto a = dot(w, cross(planar_hitpt_vector, v));
            auto b = dot(w, cross(u, planar_hitpt_vector));

            if (!is_interior(a, b, rec))
                return false;

            rec.t = t;
            rec.p = intersection;
            rec.mat = mat;
            rec.set_face_normal(r, normal);
            rec.traversal_steps++;

            return true;
        }

        virtual bool is_interior(double a, double b, Hit_record& rec) const
        {
            Interval unit_interval = Interval(0, 1);

            if ((a < 0) || (b < 0) || (a + b > 1))
                return false;

            return true;
        }

    private:
        Point3 Q;
        Vec3 u,v;
        Vec3 w;
        shared_ptr<Material> mat;
        Vec3 normal;
        double D;

        aabb box;
};


#endif //ADVANCED_GRAPHICS_TRIANGLE_H