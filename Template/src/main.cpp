#include <SFML/Graphics.hpp>
#include "events.hpp"
#include "configuration.hpp"

#include "aabb.h"
#include "bvhnode.h"
#include "common.h"
#include "camera.h"
#include "parseobj.h"
#include "primitive.h"
#include "material.h"
#include "sphere.h"
#include "triangle.h"
#include "world.h"

int main()
{
    bool rendered = false;

	int render = 1; // 1 is small world, 2 is big world
	cout << "Select render: "
		<< "\n 1: Small world"
		<< "\n 2: Big world"
		<< endl;
	cin >> render;

	int structure = 1; // 1 is none, 2 is grid
	cout << "Select acceleration structure: "
		<< "\n 1: None"
		<< "\n 2: Grid"
		<< endl;
	cin >> structure;

	int device = 1; // 1 is cpu, 2 is gpu
	cout << "Select device: "
		<< "\n 1: CPU"
		<< "\n 2: GPU"
		<< endl;
	cin >> device;

	std::cout << "Starting render..\n";

	auto window = sf::RenderWindow{ { conf::window_size.x, conf::window_size.y}, "RayTracer" };

	// Camera
	Camera cam;
	cam.v_up = Vec3(0, 1, 0);

    // World
    World world;

	// Get correct world to render
	if (render == 1)
	{
		cam.cam_pos = Vec3(0, 0, 0);
		cam.cam_dir = Vec3(0, 0, -1);
		conf::defocus_angle = 1.0;
		conf::focus_dist = 10.0;
		conf::vfov = 90;

		auto material_ground = make_shared<Lambertian>(Vec3(0.8, 0.8, 0.0));
		auto material_center = make_shared<Lambertian>(Vec3(0.1, 0.2, 0.5));
		auto material_left = make_shared<Dielectric>(1.50);
		auto material_bubble = make_shared<Dielectric>(1.00 / 1.50);
		auto material_right = make_shared<Metal>(Vec3(0.8, 0.6, 0.2), 1.0);

		world.add(make_shared<Sphere>(Point3(0.0, -100.5, -1.0), 100.0, material_ground));
		world.add(make_shared<Sphere>(Point3(0.0, 0.0, -1.2), 0.5, material_center));
		world.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.5, material_left));
		world.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.4, material_bubble));
		world.add(make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, material_right));
	}
	else if (render == 2)
	{
		// Nice render but takes a while
		cam.cam_pos = Vec3(13, 2, 3);
		cam.cam_dir = Vec3(0, 0, 0);
		conf::defocus_angle = 0.6;
		conf::focus_dist = 10.0;
		conf::vfov = 20;

		auto ground_material = make_shared<Lambertian>(Vec3(0.5, 0.5, 0.5));
		world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

		for (int a = -11; a < 11; a++) {
			for (int b = -11; b < 11; b++) {
				auto choose_mat = random_double();
				Point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

				if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
					shared_ptr<Material> sphere_material;

					if (choose_mat < 0.8) {
						// diffuse
						auto albedo = Vec3::random() * Vec3::random();
						sphere_material = make_shared<Lambertian>(albedo);
						world.add(make_shared<Sphere>(center, 0.2, sphere_material));
					}
					else if (choose_mat < 0.95) {
						// metal
						auto albedo = Vec3::random(0.5, 1);
						auto fuzz = random_double(0, 0.5);
						sphere_material = make_shared<Metal>(albedo, fuzz);
						world.add(make_shared<Sphere>(center, 0.2, sphere_material));
					}
					else {
						// glass
						sphere_material = make_shared<Dielectric>(1.5);
						world.add(make_shared<Sphere>(center, 0.2, sphere_material));
					}
				}
			}
		}

		auto material1 = make_shared<Dielectric>(1.5);
		world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

		auto material2 = make_shared<Lambertian>(Vec3(0.4, 0.2, 0.1));
		world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

		auto material3 = make_shared<Metal>(Vec3(0.7, 0.6, 0.5), 0.0);
		world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));
	}

	sf::VertexArray res;

	while (window.isOpen())
	{
        // Process inputs
		processEvents(window);

        // Render screen
		if (!rendered)
		{
			// CPU
			if (device == 1)
			{
				if (structure == 1)
					res = cam.render(world, rendered, Camera::NONE);
				else if (structure == 2)
					res = cam.render(world, rendered, Camera::GRID);
			}
			// GPU
			else if (device == 2)
			{
				if (structure == 1)
					res = cam.render_gpu(world, rendered, Camera::NONE);
				else if (structure == 2)
					std::cout << "Grid is not yet implemented on the GPU!\n";
			}
			
			rendered = true;
			std::cout << "Render finished. \n";
		}

		// Draw and display
		window.clear();
		window.draw(res);
		window.display();
	}
}