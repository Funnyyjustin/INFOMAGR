#pragma once

namespace conf
{
	// Window configuration
	auto aspect_ratio = 16.0 / 9.0;
	unsigned int width = 1200;
	unsigned int height = int(width / aspect_ratio);
	sf::Vector2u const window_size = {width, height};
	sf::Vector2f const window_size_f = static_cast<sf::Vector2f>(window_size);
	uint32_t const max_framerate = 144;
	float const dt = 1.0f / static_cast<float>(max_framerate);

	// RT config
	int samples_per_pixel = 500;
	int max_depth = 50;
	double vfov = 20;
	double defocus_angle = 0.6;
	double focus_dist = 10.0;


}