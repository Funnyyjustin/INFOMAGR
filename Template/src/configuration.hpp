#pragma once

namespace conf
{
	// Window configuration
	auto aspect_ratio = 16.0 / 9.0;
	unsigned int width = 1980;
	unsigned int height = 1080; //int(width / aspect_ratio);
	sf::Vector2u const window_size = {width, height};
	sf::Vector2f const window_size_f = static_cast<sf::Vector2f>(window_size);
	uint32_t const max_framerate = 144;
	float const dt = 1.0f / static_cast<float>(max_framerate);
}