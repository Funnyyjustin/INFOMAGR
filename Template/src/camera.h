#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <iomanip>
#include <chrono>

#include "interval.h"
#include "kdtree.h"
#include "material.h"
#include "primitive.h"
#include "Grid.h"
#include "world.h"

// Include various GPU-related libraries/files/etc.
#include <CL/cl.h>
#include "gpu_structs.h"
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

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

        enum AccelStruct {
            NONE,
            BVH,
            KDtree,
            GRID
        };

        enum AntiAliasing {
            FIXED,
            ADAPTIVE
        };

        sf::VertexArray render(World& world, bool rendered, AccelStruct axl, AntiAliasing aa)
        {
            initialize();

            // Array of pixels
            auto arr = sf::VertexArray(sf::PrimitiveType::Points, conf::window_size.x * conf::window_size.y);

            KdTree tree = KdTree();
            KdNode* root = tree.buildTree({});
            Grid grid = Grid();
            if (axl == KDtree) root = tree.buildTree(world.objects);
            if (axl == BVH) world = World(make_shared<bvh_node>(world));
            if (axl == GRID) grid = Grid(world);
            this->world = world;

            auto start = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::cout << "Started render at: " << std::ctime(&start) << "\n";

            int num_rays_shot = 0;

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

                    Vec3 color(0, 0, 0); // Starting color is always black; if we hit nothing this is the result
                    
                    // Anti-aliasing
                    if (aa == FIXED)
                    {
                        for (int sample = 0; sample < conf::samples_per_pixel; sample++)
                        {
                            num_rays_shot++;
                            Ray r = get_ray(x, y);
                            if (axl == KDtree)
                            {
                                Hit_record rec;
                                World subset = tree.traverseTree(r, root, rec);
                                //color += kdTraverse(r, conf::max_depth, subset, tree, root, rec); // Track the ray a certain amount of times
                                //intersection_tests.push_back(rec.intersection_tests);
                                //traversal_steps.push_back(rec.traversal_steps);
                            }
                            else if (axl == GRID)
                            {
                                //color += gridTraverse(r, conf::max_depth, grid);
                            }
                            else
                            {
                                color += noAccelTraverse(r, conf::max_depth, world);
                            }
                        }

                        // Set color of current pixel on the screen and apply gamma correction
                        color *= pixel_samples_scale;
                        color = to_gamma(color);
                        arr[currentPixel].color = convert_to_color(color);
                    }
                    else if (aa == ADAPTIVE)
                    {
                        int num_samples = 0;

                        vector<Vec3> colors;
                        for (int sample = 0; sample < conf::first_samples; sample++)
                        {
                            num_rays_shot++;
                            Ray r = get_ray(x, y);

                            if (axl == NONE || axl == BVH)
                                colors.push_back(noAccelTraverse(r, conf::max_depth, world));
                            else if (axl == GRID)
                                colors.push_back(gridTraverse(r, conf::max_depth, grid));
                            else if (axl == KDtree)
                            {
                                std::cout << "KDtree not implemented for adaptive sampling. Please try another structure!\n";
                                break;
                            }
                        }

                        Vec3 mean = Vec3(0, 0, 0);
                        Vec3 M2 = Vec3(0, 0, 0);
                        for (Vec3 sample : colors)
                        {
                            num_samples++;
                            Vec3 delta = sample - mean;
                            mean += delta / num_samples;
                            Vec3 delta2 = sample - mean;
                            M2 += delta * delta2;
                        }

                        Vec3 variance = M2 / (num_samples - 1);

                        // Calculate error
                        float error_sq =
                            0.2126 * 0.2126 * variance.x() +
                            0.7152 * 0.7152 * variance.y() +
                            0.0722 * 0.0722 * variance.z();

                        float error = sqrt(error_sq / num_samples);

                        bool satisfies = error <= conf::threshold;

                        if (satisfies)
                        {
                            for (Vec3 sample : colors)
                                color += sample;

                            // Set color of current pixel on the screen and apply gamma correction
                            color *= (1.0 / num_samples);
                            color = to_gamma(color);
                            arr[currentPixel].color = convert_to_color(color);
                        }
                        else
                        {
                            while (!satisfies && num_samples <= conf::num_samples)
                            {
                                int new_num_samples = 0;
                                for (int samples_new = 0; samples_new < conf::second_samples; samples_new++)
                                {
                                    num_rays_shot++;
                                    Ray r = get_ray(x, y);

                                    if (axl == NONE || axl == BVH)
                                        colors.push_back(noAccelTraverse(r, conf::max_depth, world));
                                    else if (axl == GRID)
                                        colors.push_back(gridTraverse(r, conf::max_depth, grid));
                                    else if (axl == KDtree)
                                    {
                                        std::cout << "KDtree not implemented for adaptive sampling. Please try another structure!\n";
                                        break;
                                    }

                                    num_samples++;
                                }

                                Vec3 mean = Vec3(0, 0, 0);
                                Vec3 M2 = Vec3(0, 0, 0);
                                for (Vec3 sample : colors)
                                {
                                    new_num_samples++;
                                    Vec3 delta = sample - mean;
                                    mean += delta / num_samples;
                                    Vec3 delta2 = sample - mean;
                                    M2 += delta * delta2;
                                }

                                Vec3 variance = M2 / (num_samples - 1);

                                // Calculate error
                                float error_sq =
                                    0.2126 * 0.2126 * variance.x() +
                                    0.7152 * 0.7152 * variance.y() +
                                    0.0722 * 0.0722 * variance.z();

                                float error = sqrt(error_sq / num_samples);

                                satisfies = error <= conf::threshold;
                            }

                            for (Vec3 sample : colors)
                                color += sample;

                            // Set color of current pixel on the screen and apply gamma correction
                            color *= (1.0 / num_samples);
                            color = to_gamma(color);
                            arr[currentPixel].color = convert_to_color(color);
                        }
                    }
                }
            }

            auto end = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::cout << "Finished render at: " << std::ctime(&end) << "\n";

            float total = end - start;
            std::cout << "Total elapsed time: " << total << " seconds" << "\n";

            std::cout << "Total number of rays shot through the scene: " << num_rays_shot << "\n";

            return arr;
        }

        sf::VertexArray render_gpu(World& world, bool rendered, AccelStruct axl, AntiAliasing aa)
        {
            // Initialize camera + configuration for GPU
            initialize();

            int width = conf::window_size.x;
            int size = width * conf::window_size.y;
            int sphere_count = 2;

            Configuration* conf = new Configuration();
            conf->camera_center = Point3toFloat4(this->camera_center);
            conf->pixel00_loc = Point3toFloat4(this->pixel00_loc);
            conf->pixel_delta_u = Vec3toFloat4(this->pixel_delta_u);
            conf->pixel_delta_v = Vec3toFloat4(this->pixel_delta_v);
            conf->sphere_count = sphere_count;
            conf->screen_width = width;

            // Array of pixels
            auto arr = sf::VertexArray(sf::PrimitiveType::Points, size);

            cl_float4* arr_temp = new cl_float4[size];
            SphereNew* spheres = new SphereNew[sphere_count];
            MaterialNew* materials = new MaterialNew[sphere_count];

            // Initialize image array
            for (int i = 0; i < size; i++)
            {
                arr_temp[i] = { 0, 0, 0, 0 };
            }

            // Small sphere
            spheres[0].center = { 0, 0, -1.2f, 0 };
            spheres[0].radius = { 0.5f, 0, 0, 0 };
            materials[0].type = { 1, 0, 0, 0 };
            materials[0].info = { 0.8f, 0.8f, 0.8f, 0.3f };

            // Big sphere
            spheres[1].center = { 0, -100.5f, -1.0f, 0 };
            spheres[1].radius = { 100.0f, 0, 0, 0 };
            materials[1].type = { 0, 0, 0, 0 };
            materials[1].info = { 0.8f, 0.8f, 0.0f, 0 };

            auto start = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::cout << "Started render at: " << std::ctime(&start) << "\n";

            // Rendering with OpenCL

            cl_int err;

            // Get platforms
            cl_platform_id platform;
            err = clGetPlatformIDs(1, &platform, nullptr);
            std::cout << "Get platform ID: " << err << "\n";

            // Get devices
            cl_device_id device;
            err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
            std::cout << "Get device ID: " << err << "\n";

            // Create context
            cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
            std::cout << "Create context: " << err << "\n";

            // Create command queue
            cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
            std::cout << "Create queue: " << err << "\n";

            // Create memory buffers
            cl_mem image_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float4) * size, arr_temp, &err);
            std::cout << "Create image buffer: " << err << "\n";

            cl_mem conf_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Configuration), conf, &err);
            std::cout << "Create configuration buffer: " << err << "\n";

            cl_mem spheres_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(SphereNew) * sphere_count, spheres, &err);
            std::cout << "Create sphere buffer: " << err << "\n";

            cl_mem materials_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(MaterialNew) * sphere_count, materials, &err);
            std::cout << "Create material buffer: " << err << "\n";

            // Write buffers to GPU
            err = clEnqueueWriteBuffer(queue, image_buffer, CL_TRUE, 0, sizeof(cl_float4) * size, arr_temp, 0, nullptr, nullptr);
            std::cout << "Write image buffer: " << err << "\n";

            err = clEnqueueWriteBuffer(queue, conf_buffer, CL_TRUE, 0, sizeof(Configuration), conf, 0, nullptr, nullptr);
            std::cout << "Write configuration buffer: " << err << "\n";

            err = clEnqueueWriteBuffer(queue, spheres_buffer, CL_TRUE, 0, sizeof(SphereNew) * sphere_count, spheres, 0, nullptr, nullptr);
            std::cout << "Write sphere buffer: " << err << "\n";

            err = clEnqueueWriteBuffer(queue, materials_buffer, CL_TRUE, 0, sizeof(MaterialNew) * sphere_count, materials, 0, nullptr, nullptr);
            std::cout << "Write material buffer: " << err << "\n";

            // Read kernel source
            std::string sourcecode = loadKernel(lookUpDir() + "shader.cl");
            const char* source = sourcecode.c_str();

            // Create program
            cl_program program = clCreateProgramWithSource(context, 1, &source, nullptr, &err);
            std::cout << "Create program: " << err << "\n";

            // Build program
            err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
            std::cout << "Build program: " << err << "\n";

            // Create kernel
            cl_kernel kernel = clCreateKernel(program, "get_color", &err);
            std::cout << "Create kernel: " << err << "\n";

            // Set kernel arguments
            err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &image_buffer);
            std::cout << "Set kernel argument 0: " << err << "\n";

            err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &conf_buffer);
            std::cout << "Set kernel argument 1: " << err << "\n";

            err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &spheres_buffer);
            std::cout << "Set kernel argument 2: " << err << "\n";

            err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &materials_buffer);
            std::cout << "Set kernel argument 3: " << err << "\n";

            // Run kernel
            size_t globalsize = size;
            size_t localsize = 1;
            err = clEnqueueNDRangeKernel(queue, kernel, 1, 0, &globalsize, &localsize, 0, nullptr, nullptr);
            std::cout << "Kernel enqueue: " << err << "\n";

            // Write back from GPU
            err = clEnqueueReadBuffer(queue, image_buffer, CL_TRUE, 0, sizeof(cl_float4) * size, arr_temp, 0, nullptr, nullptr);
            std::cout << "Read image buffer: " << err << "\n";

            clFinish(queue);

            // Clean up
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            clReleaseMemObject(image_buffer);
            clReleaseMemObject(conf_buffer);
            clReleaseMemObject(spheres_buffer);
            clReleaseMemObject(materials_buffer);
            clReleaseCommandQueue(queue);
            clReleaseContext(context);

            // Write from temp array to actual array
            for (int id = 0; id < size; id++)
            {
                arr[id].position = sf::Vector2f(id % conf::window_size.x, id / conf::window_size.x);
                arr[id].color = convert_to_color(to_gamma(Vec3(arr_temp[id].x, arr_temp[id].y, arr_temp[id].z)));
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
        Vec3 kdTraverse(const Ray& r, int depth, const World subset, KdTree tree, KdNode* root, Hit_record record) const
        {
            if (depth <= 0)
                return Vec3(0, 0, 0);

            Hit_record rec;
            if (subset.hit(r, Interval(0.001, infinity), rec))
            {
                Ray scat;
                Vec3 att;

                //intersection_tests.push_back(record.intersection_tests);
                //traversal_steps.push_back(record.traversal_steps);

                if (rec.mat->scatter(r, rec, att, scat))
                {
                    Hit_record r;
                    auto prims = tree.traverseTree(scat, root, r);
                    //intersection_tests.push_back(r.intersection_tests);
                    //traversal_steps.push_back(r.traversal_steps);
                    return att * kdTraverse(scat, depth - 1, prims, tree, root, record);
                }


                return Vec3(0, 0, 0);
            }

            Vec3 unit_dir = unit_vector(r.direction());
            auto a = 0.5 * (unit_dir.y() + 1.0);
            return (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
        }

        /// <summary>
        /// Gets a 3D vector containing the RGB values of the color.
        /// This is a recursive function that traces the ray up to a certain amount of bounces.
        /// </summary>
        /// <param name="r">= The ray that is being traced.</param>
        /// <param name="depth">= The current depth.</param>
        /// <param name="world">= The world of primitives.</param>
        /// <returns>A 3D vector containing the RGB values of the resulting color.</returns>
        Vec3 noAccelTraverse(const Ray& r, int depth, const World& world) const
        {
            if (depth <= 0)
                return Vec3(0, 0, 0);

            Hit_record rec;
            if (world.hit(r, Interval(0.001, infinity), rec))
            {
                Ray scat;
                Vec3 att;

                //intersection_tests.push_back(rec.intersection_tests);
                //traversal_steps.push_back(rec.traversal_steps);

                if (rec.mat->scatter(r, rec, att, scat))
                    return att * noAccelTraverse(scat, depth - 1, world);

                return Vec3(0, 0, 0);
            }

            Vec3 unit_dir = unit_vector(r.direction());
            auto a = 0.5 * (unit_dir.y() + 1.0);
            return (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
        }

        /// <summary>
        /// Gets a 3D vector containing the RGB values of the color.
        /// This is a recursive function that traces the ray up to a certain amount of bounces.
        /// </summary>
        /// <param name="r">= The ray that is being traced.</param>
        /// <param name="depth">= The current depth.</param>
        /// <param name="world">= The world of primitives.</param>
        /// <returns>A 3D vector containing the RGB values of the resulting color.</returns>
        Vec3 gridTraverse(const Ray& r, int depth, const Grid& grid) const
        {
            if (depth <= 0)
                return Vec3(0, 0, 0);

            Hit_record rec;
            if (grid.hit(r, Interval(0.001, infinity), rec))
            {
                //std::cout << "Hit found" << std::endl;
                Ray scat;
                Vec3 att;

                //intersection_tests.push_back(rec.intersection_tests);
                //traversal_steps.push_back(rec.traversal_steps);

                if (rec.mat->scatter(r, rec, att, scat))
                    return att * gridTraverse(scat, depth - 1, grid);

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
            //return sf::Color(255 * color.x(), 255 * color.y(), 255 * color.z());
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

            //auto r_org = (conf::defocus_angle <= 0) ? camera_center : defocus_disk_sample();
            auto r_org = camera_center;
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

        string lookUpDir()
        {
            string curr = std::filesystem::current_path().generic_string();

            auto splits = split(curr, "/");

            while (true)
            {
                int index = splits.size() - 1;
                //print(splits[index]);
                if (splits[index] == "Template") break;

                splits.erase(splits.begin() + index);
            }

            std::vector<string> newpathloose;
            for (int i = 0; i < splits.size() - 1; i++)
            {
                newpathloose.push_back(splits[i]);
                newpathloose.push_back("/");
            }

            string newpath;
            for (string elem : newpathloose)
                newpath += elem;
            newpath += "Template/src/";
            return newpath;
        }

        std::vector<string> split(const string& s, string delimiter)
        {
            std::vector<string> elems;
            size_t pos = s.find(delimiter);
            string word;

            string temp = s;
            while (pos != string::npos)
            {
                word = temp.substr(0, pos);
                elems.push_back(word);
                temp.erase(0, pos + delimiter.length());
                pos = temp.find(delimiter);
            }
            elems.push_back(temp);
            return elems;
        }

        std::string loadKernel(const std::string& path) {
            std::ifstream file(path);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open kernel file: " + path);
            }

            std::ostringstream ss;
            ss << file.rdbuf();
            return ss.str();
        }

        cl_float4 Point3toFloat4(Point3 p)
        {
            cl_float4 v = cl_float4();
            v.s[0] = (float)p.e[0];
            v.s[1] = (float)p.e[1];
            v.s[2] = (float)p.e[2];
            v.s[3] = 0.0f;
            return v;
        }

        cl_float4 Vec3toFloat4(Vec3 vec)
        {
            cl_float4 v = cl_float4();
            v.s[0] = (float)vec.e[0];
            v.s[1] = (float)vec.e[1];
            v.s[2] = (float)vec.e[2];
            v.s[3] = 0.0f;
            return v;
        }
};

#endif
