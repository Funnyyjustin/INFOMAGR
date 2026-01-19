#pragma once

namespace conf
{
	// Window configuration
	auto aspect_ratio = 16.0 / 9.0;
	unsigned int width = 720;
	unsigned int height = int(width / aspect_ratio);
	sf::Vector2u const window_size = {width, height};
	sf::Vector2f const window_size_f = static_cast<sf::Vector2f>(window_size);
	uint32_t const max_framerate = 144;
	float const dt = 1.0f / static_cast<float>(max_framerate);
	double voxels_on_x = 50;

	// RT config
	int samples_per_pixel = 100;
	int max_depth = 10;
	double vfov = 80;
	double defocus_angle = 1;
	double focus_dist = 10;

	// Adaptive sampling config
	int first_samples = 20;
	int second_samples = 10;
	float threshold = 0.01;
	int num_samples = 100;
}