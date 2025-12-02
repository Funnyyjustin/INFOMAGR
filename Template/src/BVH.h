//
// Created by joost on 02/12/2025.
//

#ifndef RAYTRACER_NODE_H
#define RAYTRACER_NODE_H
#include <random>
#include <vector>

#include "aabb.h"
#include "triangle.h"

class Node
{
public:
    aabb hitbox;
    vector<shared_ptr<Triangle>> triangles;
    Node* child1;
    Node* child2;

    Node(vector<shared_ptr<Triangle>> triangles)
    {
        hitbox = aabb();
        for (auto triangle: triangles)
        {
            auto points = triangle->points();
            hitbox.include(get<0>(points));
            hitbox.include(get<1>(points));
            hitbox.include(get<2>(points));
        }

        DivideTriangles(triangles, 0);
    }

    Node(vector<shared_ptr<Triangle>> triangles, int depth)
    {
        hitbox = aabb();
        for (auto triangle: triangles)
        {
            auto points = triangle->points();
            hitbox.include(get<0>(points));
            hitbox.include(get<1>(points));
            hitbox.include(get<2>(points));
        }

        if (depth < 6)
            DivideTriangles(triangles, depth);
    }

    void add(shared_ptr<Triangle> triangle)
    {
        triangles.push_back(triangle);

    }

    tuple<vector<Triangle>, vector<Triangle>> DivideTriangles(vector<shared_ptr<Triangle>> ts, int depth)
    {
        vector<shared_ptr<Triangle>> forChild1;
        vector<shared_ptr<Triangle>> forChild2;

        for (auto triangle: ts)
        {
            auto corners = triangle->points();
            if (depth % 2 == 0)
            {
                auto middleX = (
                    get<0>(corners).x() +
                    get<1>(corners).x() +
                    get<2>(corners).x()) / 3;

                if (middleX < (hitbox.x.max + hitbox.x.min)/2)
                    forChild1.push_back(triangle);
                else
                    forChild2.push_back(triangle);
            }
            else
            {
                auto middleY = (
                    get<0>(corners).y() +
                    get<1>(corners).y() +
                    get<2>(corners).y()) / 3;

                if (middleY < (hitbox.y.max + hitbox.y.min)/2)
                    forChild1.push_back(triangle);
                else
                    forChild2.push_back(triangle);
            }
        }

        child1 = new Node(forChild1, depth + 1);
        child2 = new Node(forChild2, depth + 1);
    }


};



#endif //RAYTRACER_NODE_H