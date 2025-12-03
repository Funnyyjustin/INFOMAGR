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
    bool leaf;
    vector<shared_ptr<Primitive>> objects;
    Node* child1;
    Node* child2;
    int depth = 0;

    Node() {}

    Node(vector<shared_ptr<Primitive>>& objectsIn, int depth) : depth(depth)
    {
        MakeNode(objectsIn);
    }



    void MakeNode(vector<shared_ptr<Primitive>>& objectsIn)
    {
        vector<shared_ptr<Primitive>> forChild1, forChild2;
        if (depth >= 10 || objectsIn.empty())
        {
            leaf = true;
            objects = objectsIn;
            return;
        }

        for (auto object: objectsIn)
        {
            hitbox = aabb(hitbox, object->hitBox());
            if (depth % 2 == 0)
            {
                if (object->center().x() < (hitbox.x.max + hitbox.x.min)/2)
                    forChild1.push_back(object);
                else
                    forChild2.push_back(object);
            }
            else
            {
                if (object->center().y() < (hitbox.y.max + hitbox.y.min)/2)
                    forChild1.push_back(object);
                else
                    forChild2.push_back(object);
            }
        }

        child1 = new Node(forChild1, depth + 1);
        child2 = new Node(forChild2, depth + 1);
    }

    void MakeNode() 
    {
        MakeNode(objects);
    }

    vector<shared_ptr<Primitive>> getPrimitives(const Ray& r, Interval ray_t)
    {
        if (!hitbox.hit(r, ray_t)) return {};
        if (leaf) return objects;


        vector<shared_ptr<Primitive>> out;

        auto prims1 = child1->getPrimitives(r, ray_t);
        auto prims2 = child2->getPrimitives(r, ray_t);

        //std::cout << prims1.size() << std::endl;
        //std::cout << prims2.size() << std::endl;

        for (const shared_ptr<Primitive>& object: prims1) {
            out.push_back(object);
            //std::cout << "child1 dive ";
        }

        //std:cout << std::endl;

        for (const shared_ptr<Primitive>& object: prims2) {
            out.push_back(object);
            //std::cout << "child2 dive ";
        }

        //std::cout << std::endl;

        return out;
    }




    private:
        void add(shared_ptr<Primitive> triangle)
        {
            objects.push_back(triangle);
            hitbox = aabb(hitbox, triangle->hitBox());
        }

};



#endif //RAYTRACER_NODE_H