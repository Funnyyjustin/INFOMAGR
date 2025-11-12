#include <SFML/Graphics.hpp>
#include "events.hpp"
#include "configuration.hpp"

#include "../build/common.h"
#include "../build/camera.h"
#include "../build/hittable.h"
#include "../build/hittable_list.h"
#include "../build/sphere.h"

int main()
{
	std::cout << "Starting Ray Tracer..";

	auto window = sf::RenderWindow{ { conf::window_size.x, conf::window_size.y }, "RayTracer" };

	// Camera
	camera cam;

	// World
	hittable_list world;
	world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

	while (window.isOpen())
	{
		processEvents(window);

		window.clear();

		sf::VertexArray res = cam.render(world);

		// Draw and display
		window.draw(res);
		window.display();
	}
}