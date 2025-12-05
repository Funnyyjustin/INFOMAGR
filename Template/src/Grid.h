//
// Created by joost on 04/12/2025.
//

#ifndef RAYTRACER_VOXEL_H
#define RAYTRACER_VOXEL_H
#include <vector>

#include "aabb.h"
#include "configuration.hpp"
#include "primitive.h"
#include "world.h"


class Voxel {
    public:
        vector<int> objects;
};

class Grid : public Primitive
{
    public:
        vector<Voxel> voxels;
        vector<shared_ptr<Primitive>> primitives;

        Grid() = default;

        explicit Grid(const World& world)
        {
            primitives = world.objects;

            worldMin = {world.hitBox().x.min, world.hitBox().y.min, world.hitBox().z.min};
            worldMax = {world.hitBox().x.max, world.hitBox().y.max, world.hitBox().z.max};

            auto worldDimensions = worldMax - worldMin;

            boxesAlongX = conf::voxels_on_x;
            auto voxel_size = worldDimensions.x() / boxesAlongX;
            boxesAlongY = ceil(worldDimensions.y() / voxel_size);
            boxesAlongZ = ceil(worldDimensions.z() / voxel_size);
			std::cout << boxesAlongX << " boxes along X." << std::endl;
			std::cout << boxesAlongY << " boxes along Y." << std::endl;
			std::cout << boxesAlongZ << " boxes along Z." << std::endl;

            cellDimensions = {
                worldDimensions.x() / boxesAlongX,
                worldDimensions.y() / boxesAlongY,
                worldDimensions.z() / boxesAlongZ
            };

			std::cout << boxesAlongX * boxesAlongY * boxesAlongZ << " voxels to be created." << std::endl;
            voxels.resize(boxesAlongX * boxesAlongY * boxesAlongZ);

            for (int i = 0; i < world.objects.size(); i++)
                insertPrimitiveIndex(i, world.objects[i]->hitBox());


            /*
            int boxes_along_x = conf::boxes_along_x;
            int boxes_along_y = boxes_along_x * (int) ceil(world.hitBox().y.size() / world.hitBox().x.size());
            int boxes_along_z = boxes_along_x * (int) ceil(world.hitBox().z.size() / world.hitBox().x.size());

            voxels.reserve(boxes_along_x * boxes_along_y * boxes_along_z);

            double sizeX = world.hitBox().x.size() / boxes_along_x;
            double sizeY = world.hitBox().y.size() / boxes_along_y;
            double sizeZ = world.hitBox().z.size() / boxes_along_z;

            for (int x = 0; x < boxes_along_x - 1; x++)
                for (int y = 0; y < boxes_along_y - 1; y++)
                    for (int z = 0; z < boxes_along_z - 1; z++)
                    {
                        auto interX = Interval(world.hitBox().x.min + x * sizeX, world.hitBox().x.max + (x + 1) * sizeX);
                        auto interY = Interval(world.hitBox().y.min + y * sizeY, world.hitBox().y.max + (y + 1) * sizeY);
                        auto interZ = Interval(world.hitBox().z.min + z * sizeZ, world.hitBox().z.max + (z + 1) * sizeZ);
                        voxels.push_back(Voxel(aabb(interX, interY, interZ)));
                    }

            for (auto vox: voxels)
            {
                std::cout << "X: " << vox.bbox.x.min << " - " << vox.bbox.x.max << std::endl;
                std::cout << "Y: " << vox.bbox.y.min << " - " << vox.bbox.y.max << std::endl;
                std::cout << "Z: " << vox.bbox.z.min << " - " << vox.bbox.z.max << std::endl;
                std::cout << std::endl;
            }
            */

        }

        aabb hitBox() const override {return aabb();}

        bool hit(const Ray &r, Interval ray_t, Hit_record &rec) const override
        {
            double entry_t = ray_t.min;
            double exit_t = ray_t.max;
            Interval temp = {entry_t, exit_t};

            // ray never hits grid
            if (!aabb(worldMin, worldMax).hit(r, temp))
            {
                //std::cout << "Grid miss" << std::endl;
                return false;
            }


            entry_t = max(ray_t.min, entry_t);
            exit_t = max(ray_t.max, exit_t);

            if (entry_t > exit_t)
            {
                std::cout << "Impossible entry-exit miss" << std::endl;
                return false;
            }


            // starting point of ray in grid
            Point3 entry = r.at(entry_t);
            auto entryVoxel = getVoxelIndex(entry);
            clamp(entryVoxel);

            int stepX = (r.direction().x() > 0) ? 1 : -1;
            int stepY = (r.direction().y() > 0) ? 1 : -1;
            int stepZ = (r.direction().z() > 0) ? 1 : -1;
            Vec3 stepV = {double(stepX), double(stepY), double(stepZ)};

            double deltaX = abs(cellDimensions.x() / r.direction().x());
            double deltaY = abs(cellDimensions.y() / r.direction().y());
            double deltaZ = abs(cellDimensions.z() / r.direction().z());
            Vec3 deltaV = {deltaX, deltaY, deltaZ};

            double maxT_x = nextBoundaryT(entryVoxel.x(), r.origin().x(), r.direction().x(), worldMin.x(), cellDimensions.x(), stepX);
            double maxT_y = nextBoundaryT(entryVoxel.y(), r.origin().y(), r.direction().y(), worldMin.y(), cellDimensions.y(), stepY);
            double maxT_z = nextBoundaryT(entryVoxel.z(), r.origin().y(), r.direction().z(), worldMin.z(), cellDimensions.z(), stepZ);
            Vec3 maxV = {maxT_x, maxT_y, maxT_z};

            goTo(maxT_x, deltaX, entry_t);
            goTo(maxT_y, deltaY, entry_t);
            goTo(maxT_z, deltaZ, entry_t);


            return traverse(r, entryVoxel, stepV, maxV, deltaV, exit_t, rec, ray_t);
        }

    private:
        Point3 worldMin, worldMax;
		Vec3 cellDimensions;
		int boxesAlongX, boxesAlongY, boxesAlongZ;


        bool traverse(const Ray& r, Point3 voxindex, Vec3 stepV, Vec3 maxV, Vec3 deltaV, double exit, Hit_record& rec, Interval ray_t) const
        {

            if (voxindex.x() < 0 || voxindex.x() >= boxesAlongX ||
                voxindex.y() < 0 || voxindex.y() >= boxesAlongY ||
                voxindex.z() < 0 || voxindex.z() >= boxesAlongZ)
            {
                //std::cout << "Bounds check miss" << std::endl;
                return false;
            }


            bool hitAnything = false;
            double closest = exit;
            Hit_record temp;

            clamp(voxindex);
			auto i = index3(voxindex.x(), voxindex.y(), voxindex.z());
            Voxel v = voxels[i];

            //std::cout << "voxel owns " << v.objects.size() << " objects" << std::endl;
            for (int index : v.objects)
            {
                shared_ptr<Primitive> prim = primitives[index];
                if (prim->hit(r, Interval(ray_t.min, closest), temp))
                {
                    hitAnything = true;
                    closest = temp.t;
                    rec = temp;
                }
            }

            double next = min({maxV.x(), maxV.y(), maxV.z()});
            if (hitAnything && closest <= next)
                return true;

            //expand voxel if ray did not hot anything (before next voxel)
            if (maxV.x() < maxV.y())
            {
                if (maxV.x() < maxV.z())
                {
                    return traverse(
                        r,
                        {voxindex.x() + stepV.x(), voxindex.y(), voxindex.z()},
                        stepV,
                        {maxV.x() + deltaV.x(), maxV.z(), maxV.z()},
                        deltaV, exit, rec, ray_t);
                }
                return traverse(
                    r,
                    {voxindex.x(), voxindex.y(), voxindex.z() + stepV.z()},
                    stepV,
                    {maxV.x(), maxV.z(), maxV.z() + deltaV.z()},
                    deltaV, exit, rec, ray_t);
            }

            if (maxV.y() < maxV.z())
            {
                return traverse(
                    r,
                    {voxindex.x(), voxindex.y() + stepV.y(), voxindex.z()},
                    stepV,
                    {maxV.x(), maxV.y() + deltaV.y(), maxV.z()},
                    deltaV, exit, rec, ray_t);
            }

            return traverse(
                r,
                {voxindex.x(), voxindex.y(), voxindex.z() + deltaV.z()},
                stepV,
                {maxV.x(), maxV.y(), maxV.z() + deltaV.z()},
                deltaV, exit, rec, ray_t);
        }

        void goTo(double& maxT, double delta, double entry_t) const
        {
            if (maxT == numeric_limits<double>::infinity()) return;

            if (maxT < entry_t)
            {
                double d = entry_t - maxT;
                double num_steps = ceil(d / delta);
                maxT += num_steps * delta;
            }
        }

        double nextBoundaryT(int start, double r_origin, double r_dir, double min, double cellDim, int step) const
        {
            // ray will not hit current axis
            if (r_dir == 0) return numeric_limits<double>::infinity();

            int step_;
            if (step > 0) step_ = 1;
            else step_ = 0;

            double boundary = min + (start + step_) * cellDim;
            return (boundary - r_origin) / r_dir;
        }

        Vec3 getVoxelIndex(const Point3& p) const
        {
			auto offset = p - worldMin;
            Point3 a = {floor(offset.x() / cellDimensions.x()),
                        floor(offset.y() / cellDimensions.y()),
                        floor(offset.z() / cellDimensions.z())};
            return a;
		}

        void insertPrimitiveIndex(int index, const aabb& box) const
        {
            auto min = getVoxelIndex({box.x.min, box.y.min, box.z.min});
            auto max = getVoxelIndex({box.x.max, box.y.max, box.z.max});

            clamp(min);
            clamp(max);

            for (int x = min.x(); x < max.x(); x++)
                for (int y = min.y(); y < max.y(); y++)
                    for (int z = min.z(); z < max.z(); z++)
                    {
                        auto i = index3(x,y,z);
                        auto v = voxels[i];
                        v.objects.push_back(index);
                    }

        }

        int index3(int x, int y, int z) const
        {
            return x + boxesAlongX * (y + boxesAlongY * z);
        }

        void clamp(Point3& p) const
        {
            p = {
                std::clamp(p.x(), 0.0, double(boxesAlongX) - 1),
                std::clamp(p.y(), 0.0, double(boxesAlongY) - 1),
                std::clamp(p.z(), 0.0, double(boxesAlongZ) - 1)
            };
        }

};

#endif //RAYTRACER_VOXEL_H