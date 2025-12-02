#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <iomanip>
#include <chrono>

#include "interval.h"
#include "kdtree.h"
#include "material.h"
#include "primitive.h"
#include "world.h"

/// <summary>
/// Function that writes the progress to the console. The progress is defined as the number of columns of the window that have been rendered so far.
/// </summary>
/// <param name="x">= the current column</param>
inline void progress(int x)
{   
    auto width = conf::width;
    auto progress_percentage = (double(x) / width) * 100.0;
    std::cout << "Rendering: " << std::fixed << std::setprecision(1) << progress_percentage << "%   (" << x << "/" << conf::width << " columns rendered) \n";
}

class Camera
{
    public:
        Point3 cam_pos = Point3(0, 0, 0);
        Point3 cam_dir = Point3(0, 0, -1);
        Vec3 v_up = Vec3(0, 1, 0);

        sf::VertexArray render(World& world, bool rendered)
        {
            initialize();

            auto start = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::cout << "Started render at: " << std::ctime(&start) << "\n";

            // Array of pixels
            auto arr = sf::VertexArray(sf::PrimitiveType::Points, conf::window_size.x * conf::window_size.y);

            KdTree tree = KdTree();
            KdNode* root = tree.buildTree(world.objects);
            this->world = world;

            // Draw function
            for (int x = 0; x < conf::window_size.x; x++)
            {
                // Don't output the progress (again) if the screen is rendered already.
                if (!rendered)
                    progress(x);

                for (int y = 0; y < conf::window_size.y; y++)
                {
                    // Calculate current pixel in vertex array and set its position in the VertexArray
                    auto currentPixel = y * conf::width + x;
                    arr[currentPixel].position = sf::Vector2f(x, y);

                    // Calculate position and direction of the ray
                    auto pos = pixel00_loc + (x * pixel_delta_u) + (y * pixel_delta_v);
                    auto dir = pos - camera_center;
                    Ray r(camera_center, dir);

                    Vec3 color(0, 0, 0); // Starting color is always black; if we hit nothing this is the result
                    
                    // Anti-aliasing
                    for (int sample = 0; sample < conf::samples_per_pixel; sample++)
                    {
                        Ray r = get_ray(x, y);
                        World subset = tree.traverseTree(r, root);
                        color += ray_color(r, conf::max_depth, subset); // Track the ray a certain amount of times
                    }

                    // Set color of current pixel on the screen and apply gamma correction
                    color *= pixel_samples_scale;
                    color = to_gamma(color);
                    arr[currentPixel].color = convert_to_color(color);
                }
            }

            auto end = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::cout << "Finished render at: " << std::ctime(&end) << "\n";

            float total = end - start;
            std::cout << "Total elapsed time: " << total << " seconds" << "\n";

            return arr;
        }

    private:
        Point3 camera_center;
        Point3 pixel00_loc;
        Vec3 pixel_delta_u;
        Vec3 pixel_delta_v;
        double pixel_samples_scale;
        Vec3 u, v, w; // Cam frame basis vectors
        Vec3 defocus_disk_u;
        Vec3 defocus_disk_v;

        World world;

        void initialize()
        {
            // Camera configuration
            //auto focal_length = (cam_pos - cam_dir).length();
            auto theta = degrees_to_radians(conf::vfov);
            auto h = std::tan(theta / 2);
            auto viewport_height = 2 * h * conf::focus_dist;
            auto viewport_width = viewport_height * (double(conf::width) / conf::height);

            camera_center = cam_pos;

            w = unit_vector(cam_pos - cam_dir);
            u = unit_vector(cross(v_up, w));
            v = cross(w, u);

            auto viewport_u = viewport_width * u;
            auto viewport_v = viewport_height * -v;

            pixel_delta_u = viewport_u / conf::width;
            pixel_delta_v = viewport_v / conf::height;

            auto viewport_upper_left = camera_center - (conf::focus_dist * w) - viewport_u / 2 - viewport_v / 2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

            pixel_samples_scale = 1.0 / conf::samples_per_pixel;

            // Depth of field stuff
            auto defocus_radius = conf::focus_dist * std::tan(degrees_to_radians(conf::defocus_angle / 2));
            defocus_disk_u = u * defocus_radius;
            defocus_disk_v = v * defocus_radius;
        }

        /// <summary>
        /// Gets a 3D vector containing the RGB values of the color.
        /// This is a recursive function that traces the ray up to a certain amount of bounces.
        /// </summary>
        /// <param name="r">= The ray that is being traced.</param>
        /// <param name="depth">= The current depth.</param>
        /// <param name="world">= The world of primitives.</param>
        /// <returns>A 3D vector containing the RGB values of the resulting color.</returns>
        Vec3 ray_color(const Ray& r, int depth, const World subset) const
        {
            if (depth <= 0)
                return Vec3(0, 0, 0);

            Hit_record rec;
            if (subset.hit(r, Interval(0.001, infinity), rec))
            {
                Ray scat;
                Vec3 att;

                if (rec.mat->scatter(r, rec, att, scat))
                {
                    //auto tree = KdTree();
                    //auto root = tree.buildTree(world.objects);
                    //std::cout << world.objects.size();
                    //return att * ray_color(scat, depth - 1, tree.traverseTree(scat, root));
                    return att * ray_color(scat, depth - 1, subset);
                }

                //Vec3 dir = rec.normal + random_unit_vector(); //random_on_hs(rec.normal);
                //return 0.5 * ray_color(Ray(rec.p, dir), depth - 1, world);
                return Vec3(0, 0, 0);
            }

            Vec3 unit_dir = unit_vector(r.direction());
            auto a = 0.5 * (unit_dir.y() + 1.0);
            return (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
        }

        /// <summary>
        /// Applies gamma correction to one of the values within a RGB vector.
        /// </summary>
        /// <param name="comp">= The RGB component.</param>
        /// <returns></returns>
        Vec3 to_gamma(Vec3 v)
        {
            double x, y, z;
            if (v.x() > 0)
                x = std::sqrt(v.x());
            else x = 0;

            if (v.y() > 0)
                y = std::sqrt(v.y());
            else y = 0;

            if (v.z() > 0)
                z = std::sqrt(v.z());
            else z = 0;

            return Vec3(x, y, z);
        }

        /// <summary>
        /// Converts a Vec3 color to a Color color.
        /// </summary>
        /// <param name="color">= A color of the type Vec3</param>
        /// <returns>A color of the type Color</returns>
        sf::Color convert_to_color(const Vec3 color)
        {
            static const Interval intensity(0.000, 0.999);
            return sf::Color(256 * intensity.clamp(color.x()), 256 * intensity.clamp(color.y()), 256 * intensity.clamp(color.z()));
        }

        /// <summary>
        /// Gets a ray, based on the viewport and the camera position.
        /// </summary>
        /// <param name="x">= x-coordinate of the current pixel</param>
        /// <param name="y">= y-coordinate of the current pixel</param>
        /// <returns>A ray that starts from the camera and goes through the viewport.</returns>
        Ray get_ray(int x, int y) const
        {
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc + ((x + offset.x()) * pixel_delta_u) + ((y + offset.y()) * pixel_delta_v);

            auto r_org = (conf::defocus_angle <= 0) ? camera_center : defocus_disk_sample();
            auto r_dir = pixel_sample - r_org;

            return Ray(r_org, r_dir);
        }

        /// <summary>
        /// Gets a random sample square.
        /// </summary>
        /// <returns>A random sample square.</returns>
        Vec3 sample_square() const
        {
            return Vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }

        /// <summary>
        /// Function for the depth of field functionality.
        /// </summary>
        /// <returns></returns>
        Point3 defocus_disk_sample() const
        {
            auto p = random_in_unit_disk();
            return camera_center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }
};

#endif
