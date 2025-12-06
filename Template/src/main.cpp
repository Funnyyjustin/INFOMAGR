#include <SFML/Graphics.hpp>
#include "events.hpp"
#include "configuration.hpp"

#include "aabb.h"
#include "bvhnode.h"
#include "common.h"
#include "camera.h"
#include "kdtree.h"
#include "parseobj.h"
#include "primitive.h"
#include "material.h"
#include "sphere.h"
#include "triangle.h"
#include "world.h"

int main()
{
    bool rendered = false;

	std::cout << "Starting render..\n";

	auto window = sf::RenderWindow{ { conf::window_size.x, conf::window_size.y }, "RayTracer" };

	// Camera
	Camera cam;
	cam.cam_pos = Point3(0, 0, 0);
	cam.cam_dir = Point3(0, 0, 1);
	cam.v_up = Vec3(0, 1, 0);


	// World
	World world;

    auto material_ground = make_shared<Lambertian>(Vec3(0.8, 0.8, 0.0));
    auto material_center = make_shared<Lambertian>(Vec3(0.1, 0.2, 0.5));
    auto material_left = make_shared<Dielectric>(1.50);
    auto material_bubble = make_shared<Dielectric>(1.00 / 1.50);
    auto material_right = make_shared<Metal>(Vec3(0.8, 0.6, 0.2), 1.0);

    //world.add(make_shared<Sphere>(Point3(0.0, -100.5, -1.0), 100.0, material_ground));
    //world.add(make_shared<Sphere>(Point3(0.0, 0.0, -1.2), 0.5, material_center));
    //world.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.5, material_left));
    //world.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.4, material_bubble));
    //world.add(make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, material_right));

    Parser parser;
    auto [vertices, vertex_normals, faces]
		= parser.parse("Chicken.obj", Point3(50.0, -100.0, 150.0));
        //= parser.parse("forg.obj", Point3(0.0, 0.0, 0.75));

	//std::cout << "Number of vertices: " << vertices.size() << std::endl;

    // Load all triangles in the mesh
    for (int face_index = 0; face_index < faces.size(); face_index++)
    {
        Point3 face = faces[face_index];
        double p_index_a = face.x() - 1;
        double p_index_b = face.y() - 1;
        double p_index_c = face.z() - 1;

        Point3 a = vertices[p_index_a];
        Point3 b = vertices[p_index_b];
        Point3 c = vertices[p_index_c];

        // To calculate the normal of the face, we take two edges of the triangle A and B
        // The normal of the face is then the cross product between the two edges A and B
        Vec3 A = b - a;
        Vec3 B = c - a;
        Vec3 normal = cross(A, B);

        world.add(make_shared<Triangle>(a, A, B, material_center));
    }

	std::cout << "Number of primitives: " << world.objects.size() << std::endl;
    // Nice render but takes a while
    /*auto ground_material = make_shared<Lambertian>(Vec3(0.5, 0.5, 0.5));
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
    world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));*/



	sf::VertexArray res;

	vector<int> traversal_steps;
	vector<int> intersection_tests;
	while (window.isOpen())
	{
        // Process inputs
		processEvents(window);

        // Render screen
		if (!rendered)
		{
			res = cam.render(world, rendered, Camera::GRID, traversal_steps, intersection_tests);
			rendered = true;
			std::cout << "Render finished. \n";
			std::cout << "Median traversal steps: " << median(traversal_steps) << std::endl;
			std::cout << "Average traversal steps: " << average(traversal_steps) << std::endl;
			std::cout << "Median intersection tests: " << median(intersection_tests) << std::endl;
			std::cout << "Average intersection tests: " << average(intersection_tests) << std::endl;
		}

		// Draw and display
		window.clear();
		window.draw(res);
		window.display();
	}
}