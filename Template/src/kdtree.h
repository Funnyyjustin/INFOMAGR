#ifndef KDTREE
#define KDTREE

#include <vector>
#include <algorithm>
#include <limits>


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
		std::vector<shared_ptr<Primitive>> primitives;

		KdTree(std::vector<shared_ptr<Primitive>> objects)
		{
			primitives = objects;
		}

		KdNode* buildTree(std::vector<shared_ptr<Primitive>> objects)
		{
			// Create root node of the entire tree
			KdNode* root = new KdNode();
			root->primitives = objects;
			aabb scene = aabb(Interval::universe, Interval::universe, Interval::universe);
			root->boundingbox = scene;
			return buildTreeRec(root, 0);
		}

		KdNode* buildTreeRec(KdNode* root, int depth)
		{
			// Make root node a leaf if we reach max depth of the tree
			if (depth == maxDepth)
			{
				root->isLeaf = true;
				return root;
			}

			// Divide space --> not done with SAH currently, just midway point
			// This can also be done with SAH, which could improve performance even more
			aabb currentspace = root->boundingbox;

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

		std::vector<shared_ptr<Primitive>> splitPrimitives(std::vector<shared_ptr<Primitive>> objs, aabb box)
		{
			std::vector<shared_ptr<Primitive>> res;
			for (const std::shared_ptr<Primitive> obj : objs)
			{
				auto p = obj.get();
				if (p->hitBox().boxes_overlap(box))
					res.push_back(obj);
			}

			return res;
		}
};

#endif