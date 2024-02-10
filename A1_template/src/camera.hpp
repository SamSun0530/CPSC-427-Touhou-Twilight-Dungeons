#pragma once

#include "common.hpp"

class Camera {
	// Camera position x,y
	vec2 position;

	// Camera speed
	float speed;

	// Determine if camera could be controlled freely
	bool isFreeCam;

	// Offset for translating origin from top left to center
	const vec2 origin_offset;
public:
	Camera() : origin_offset({ window_width_px / 2, window_height_px / 2 }) {
		position = { 0.f, 0.f };
		speed = 2.f;
		isFreeCam = false;
	}

	void setPosition(vec2 position);

	vec2& getPosition();

	mat3 createViewMatrix();
};