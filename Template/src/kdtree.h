#ifndef KDTREE
#define KDTREE

#include <vector>
#include <algorithm>
#include <limits>
#include "primitive.h"
#include "aabb.h"
#include "world.h"

class KdNode
{
	public:
		std::vector<shared_ptr<Primitive>> primitives;
		aabb boundingbox;
		KdNode* parent;
		KdNode* left;
		KdNode* right;
		bool isLeaf = false;

		KdNode() {}

		KdNode(std::vector<shared_ptr<Primitive>> p, aabb box, KdNode* parent) { primitives = p; boundingbox = box; this->parent = parent; }
};

class KdTree
{
	public:
		int maxDepth = 5;
		int minLeafSize = 10;
		std::vector<shared_ptr<Primitive>> primitives;

		KdTree() { }

		KdNode* buildTree(std::vector<shared_ptr<Primitive>> objects)
		{
			// Create root node of the entire tree
			KdNode* root = new KdNode();
			root->primitives = objects;
			aabb scene = aabb(Interval::universe, Interval::universe, Interval::universe);
			root->boundingbox = scene;
			maxDepth = 15;
			return buildTreeRec(root, 0);
		}

		KdNode* buildTreeRec(KdNode* root, int depth)
		{
			aabb currentspace = root->boundingbox;

			// Make root node a leaf if we reach max depth of the tree
			// It can also be a leaf if the box contains no primitives
			if (depth == maxDepth || root->primitives.size() == 0 || root->primitives.size() <= minLeafSize)
			{
				root->isLeaf = true;
				return root;
			}

			// Divide space --> not done with SAH currently, just midway point
			// This can also be done with SAH, which could improve performance even more

			// 1. Get axis along which we should split
			int axis = depth % 3;

			// 2. Then, create two new (child) bounding boxes
			// We also get the primitives that overlap into these new bounding boxes
			auto [leftnode, rightnode] = currentspace.split(axis);
			KdNode* left = new KdNode(splitPrimitives(root->primitives, leftnode), leftnode, root);
			KdNode* right = new KdNode(splitPrimitives(root->primitives, rightnode), rightnode, root);

			// 3. Call buildTreeRec on both child nodes and increase depth
			// We assign the results of the recursion to the left and right nodes of the root
			root->left = buildTreeRec(left, depth + 1);
			root->right = buildTreeRec(right, depth + 1);

			// 4. Return values is the original root, now assigned with children
			// This one node contains the entire tree, which can be traversed through recursively
			return root;
		}

		std::vector<shared_ptr<Primitive>> splitPrimitives(const std::vector<shared_ptr<Primitive>>& objs, aabb box)
		{
			std::vector<shared_ptr<Primitive>> res;
			for (auto obj : objs)
			{
				auto p = obj.get();
				if (p->hitBox().boxes_overlap(box))
					res.push_back(obj);
			}

			return res;
		}

		World traverseTree(Ray& ray, KdNode* tree)
		{
			// Check if we hit the bounding box of the current node of the tree
			auto [hit, entry, exit] = tree->boundingbox.boxRayIntersection(ray);

			// No hit with the bounding box, so no primitives to check within the box / subtree
			if (!hit)
				return World();

			KdNode* leaf = findLeaf(entry, tree);
			auto primitives = leaf->primitives;
			World world = World();

			// For each primitive in the leaf, check if there is an intersection with the ray
			for (const std::shared_ptr<Primitive> p : primitives)
			{
				Hit_record h;
				if (p.get()->hit(ray, Interval(0.001, infinity), h))
					world.objects.push_back(p);
			}

			// Intersections within the leaf have been found, we can stop traversing
			if (world.objects.size() > 0)
				return world;

			// Otherwise, if no intersections are found, we traverse the ray through the tree until we hit the next box
			Point3 offset = Point3(0.001, 0.001, 0.001);
			Ray ray_new = Ray(exit + offset, ray.direction());
			return traverseTree(ray_new, tree);
		}

		KdNode* findLeaf(Point3 point, KdNode* tree)
		{
			if (!tree || !tree->boundingbox.contains(point))
				return nullptr;

			if (tree->isLeaf)
				return tree;

			if (tree->left->boundingbox.contains(point))
			{
				KdNode* res = findLeaf(point, tree->left);
				if (res) return res;
			}

			if (tree->right->boundingbox.contains(point))
			{
				KdNode* res = findLeaf(point, tree->right);
				if (res) return res;
			}

			return nullptr;
		}
};

#endif