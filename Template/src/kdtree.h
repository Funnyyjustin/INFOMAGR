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
		int maxDepth = 10;
		int minLeafSize = 1;
		std::vector<shared_ptr<Primitive>> primitives;

		KdTree() { }

		/// <summary>
		/// Builds a kd-tree of the scene
		/// </summary>
		/// <param name="objects"> = the objects in the scene</param>
		/// <returns>A KdNode*, which is the root of the kd-tree</returns>
		KdNode* buildTree(std::vector<shared_ptr<Primitive>> objects)
		{
			// Create root node of the entire tree
			KdNode* root = new KdNode();
			root->primitives = objects;

			std::vector<int> bounds = getBounds(objects);

			aabb scee = aabb(
				Interval(bounds[0], bounds[1]),
				Interval(bounds[2], bounds[3]),
				Interval(bounds[4], bounds[5]));

			int num = 500;
			aabb scene = aabb(Interval(-num, num), Interval(-num, num), Interval(-num, num));
			root->boundingbox = scene;

			return buildTreeRec(root, 0);
		}

		/// <summary>
		/// Gets max bounds of the scene, based on all objects in the scene
		/// </summary>
		/// <param name="objects"> = the objects in the scene</param>
		/// <returns>A 1x6 vector with the min and max bounds of the three axes.</returns>
		std::vector<int> getBounds(std::vector<shared_ptr<Primitive>> objects)
		{
			int min_x = 9999999999;
			int min_y = 9999999999;
			int min_z = 9999999999;
			int max_x = -9999999999;
			int max_y = -9999999999;
			int max_z = -9999999999;

			for (const auto& obj : objects)
			{
				auto primitive = obj.get();
				auto box = primitive->hitBox();
				auto x = box.x;
				auto y = box.y;
				auto z = box.z;

				min_x = std::min(min_x, round(x.min));
				max_x = std::max(max_x, round(x.max));

				min_y = std::min(min_y, round(y.min));
				max_y = std::max(max_y, round(y.max));

				min_z = std::min(min_z, round(z.min));
				max_z = std::max(max_z, round(z.max));
			}

			// We clamp the values to the biggest nearest 100-value
			// So, 101 --> 200, -101 --> -200
			// Otherwise, bounding boxes get goofy for some reason
			//return std::vector<int> { min_x, max_x, min_y, max_y, min_z, max_z };
			return std::vector<int> { clamp(min_x), clamp(max_x), clamp(min_y), clamp(max_y), clamp(min_z), clamp(max_z) };
		}

		/// <summary>
		/// Rounds a number to the nearest biggest int
		/// So, 1.2 --> 2.0, -1.2 --> -2.0
		/// </summary>
		/// <param name="x"> = float</param>
		/// <returns></returns>
		int round(double x)
		{
			if (x > 0.0)
				return static_cast<int>(std::ceil(x));
			else
				return static_cast<int>(std::floor(x));
		}

		/// <summary>
		/// Clamps a number to the nearest 100-value
		/// So, 101 --> 200, -101 --> -200
		/// </summary>
		/// <param name="x"> = int</param>
		/// <returns></returns>
		int clamp(int x)
		{
			// If x already is equal to 0, no rounding is needed
			if (x == 0)
				return x;

			// We take the amount of times that the input number fits inside 100
			// Then, we round based on the value of mod
			int div = int(x / 100);
			int mod = x % 100;

			float res = div;

			// If mod > 0, it means that the input number is positive, and we should round up
			// If mod < 0, it means that the input number is negative, and we should round down
			// If mod == 0, no rounding is needed
			if (mod > 0)
				res += 1;
			else if (mod < 0)
				res -= 1;

			return res * 100;
		}

		/// <summary>
		/// Recursively builds the kd-tree based on some root node and a current depth
		/// </summary>
		/// <param name="root"> = the root node of the tree</param>
		/// <param name="depth"> = the current depth of the tree</param>
		/// <returns></returns>
		KdNode* buildTreeRec(KdNode* root, int depth)
		{
			aabb currentspace = root->boundingbox;

			// Make root node a leaf if we reach max depth of the tree
			// It can also be a leaf if the box contains no primitives
			// It can also be a leaf if the box contains a minimum number of primitives
			if (depth == maxDepth || root->primitives.size() == 0 || root->primitives.size() <= minLeafSize)
			{
				root->isLeaf = true;
				return root;
			}

			// Divide space --> not done with SAH currently, just midway point
			// This can also be done with SAH, which could improve performance even more

			// 1. Get axis along which we should split
			// Currently, we just switch between the three different axes, starting with the x-axis
			//int axis = depth % 3;
			int axis = getLongestAxis(currentspace);

			// 2. Then, create two new (child) bounding boxes
			// We also get the primitives that overlap into these new bounding boxes, and give them to our children
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

		int getLongestAxis(aabb box)
		{
			auto x = box.x.size();
			auto y = box.y.size();
			auto z = box.z.size();

			int min = std::max({x, y, z});

			if (min == x) return 0;
			if (min == y) return 1;
			return 2;
		}

		/// <summary>
		/// Assigns primitives of the parent node to the child node if the primitive is within the AABB of the child
		/// </summary>
		/// <param name="objs"> = the primitives that (partly) reside within the AABB of the parent node</param>
		/// <param name="box"> = the AABB of the child node</param>
		/// <returns>A list of primitives that (partly) reside within the AABB of the child node</returns>
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

		/// <summary>
		/// Traverses the ray through a kd-tree
		/// </summary>
		/// <param name="ray"> = The ray that is traversing through the tree</param>
		/// <param name="tree"> = The tree that is being traversed through; the root node of the tree should be given here</param>
		/// <returns>A World, containing all primitives that the ray might intersect with</returns>
		World traverseTree(Ray& ray, KdNode* tree)
		{
			// Check if we hit the bounding box of the current node of the tree
			auto [hit, entry, exit] = tree->boundingbox.boxRayIntersection(ray, Interval(0.001, INFINITY));
			//auto hit = tree->boundingbox.hit(ray, Interval(0.001, infinity));

			// No hit with the bounding box, so no primitives to check within the box / subtree
			if (!hit)
				return World();

			KdNode* leaf = findLeaf(entry, tree);

			// We do not find the leaf, so we return an empty list
			if (leaf == nullptr)
				return World();

			auto primitives = leaf->primitives;
			World world = World();

			// For each primitive in the leaf, check if there is an intersection with the ray
			for (const std::shared_ptr<Primitive> p : primitives)
			{
				Hit_record h;
				if (p.get()->hit(ray, Interval(0.001, infinity), h))
				{
					world.objects.push_back(p);
				}
			}

			// Intersections within the leaf have been found, we can stop traversing
			if (world.objects.size() > 0)
				return world;

			// Otherwise, if no intersections are found, we traverse the ray through the tree until we hit the next box
			Ray ray_new = Ray(exit + ray.direction() * 0.001, ray.direction());
			return traverseTree(ray_new, tree);
		}

		/// <summary>
		/// Finds the leaf in the kd-tree that contains a certain point
		/// </summary>
		/// <param name="point"> = the point that is located somewhere in the kd-tree</param>
		/// <param name="tree"> = the root node of the kd-tree</param>
		/// <returns>The leaf node that contains the point. Returns a nullptr if none of the leaves contain the point</returns>
		KdNode* findLeaf(Point3 point, KdNode* tree)
		{
			// Tree does not exist or the entire (sub)tree does not contain the point, so we can stop searching
			if (!tree || !tree->boundingbox.contains(point))
				return nullptr;

			// If we are a leaf, then the point is in the leaf, so we return it
			if (tree->isLeaf && tree->boundingbox.contains(point))
				return tree;

			// If the left child node of the current node contains the point, we check this subtree
			if (tree->left->boundingbox.contains(point))
			{
				KdNode* res = findLeaf(point, tree->left);
				if (res) return res;
			}

			// If the right child node of the current node contains the point, we check this subtree
			if (tree->right->boundingbox.contains(point))
			{
				KdNode* res = findLeaf(point, tree->right);
				if (res) return res;
			}

			return nullptr;
		}

		/// <summary>
		/// Gets the number of leaves in the tree.
		/// </summary>
		/// <param name="node"></param>
		/// <returns></returns>
		int numLeaves(KdNode* node)
		{
			if (node->isLeaf)
				return 1;

			return numLeaves(node->left) + numLeaves(node->right);
		}

		/// <summary>
		/// Gets the number of primitives in the tree
		/// </summary>
		/// <param name="node"></param>
		/// <returns></returns>
		int numPrims(KdNode* node)
		{
			if (node->isLeaf)
				return node->primitives.size();

			return numPrims(node->left) + numPrims(node->right);
		}
};

#endif