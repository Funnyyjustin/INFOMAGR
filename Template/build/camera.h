#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"

class camera {
public:
    sf::VertexArray render(const hittable& world)
    {
        initialize();

        // Array of pixels
        auto arr = sf::VertexArray(sf::PrimitiveType::Points, conf::window_size.x * conf::window_size.y);

        // Draw function
        for (int x = 0; x < conf::window_size.x; x++)
        {
            for (int y = 0; y < conf::window_size.y; y++)
            {
                // Calculate current pixel in vertex array
                auto currentPixel = y * conf::width + x;
                arr[currentPixel].position = sf::Vector2f(x, y);

                // Calculate position and direction of the ray
                auto pos = pixel00_loc + (x * pixel_delta_u) + (y * pixel_delta_v);
                auto dir = pos - camera_center;
                ray r(camera_center, dir);

                // Set position and color of current pixel on the screen
                arr[currentPixel].color = ray_color(r, world);
            }
        }

        return arr;
    }

private:
    point3 camera_center;
    point3 pixel00_loc;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;

    void initialize()
    {
        // Camera configuration
        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        auto viewport_width = viewport_height * (double(conf::width) / conf::height);
        camera_center = point3(0, 0, 0);

        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        pixel_delta_u = viewport_u / conf::width;
        pixel_delta_v = viewport_v / conf::height;

        auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

    sf::Color ray_color(const ray& r, const hittable& world) const
    {
        hit_record rec;
        if (world.hit(r, interval(0, infinity), rec))
        {
            vec3 res = 0.5 * (rec.normal + vec3(1, 1, 1));
            return sf::Color(int(res.x() * 255), int(res.y() * 255), int(res.z() * 255));
        }

        vec3 unit_dir = unit_vector(r.direction());
        auto a = 0.5 * (unit_dir.y() + 1.0);
        vec3 res = (1.0 - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5, 0.7, 1.0);
        sf::Color color(int(res.x() * 255), int(res.y() * 255), int(res.z() * 255));
        return color;
    }
};

#endif
