#pragma once

#include "common.hpp"

class Camera {
	// Camera position x,y
	vec2 position;

	// Offset for translating origin from top left to center
	const vec2 origin_offset;

	// Zoom
	float zoom;
	float zoom_min;
	float zoom_max;
	float zoom_increment;
public:
	// Determine if camera could be controlled freely
	bool isFreeCam;

	// Offset for following mouse cursor
	vec2 offset;
	vec2 offset_target;

	Camera() : origin_offset(window_px_half) {
		position = { 0.f, 0.f };
		isFreeCam = false;
		offset = { 0.f, 0.f };
		offset_target = { 0.f, 0.f };
		zoom = 1.0f;
		zoom_min = 0.5f;
		zoom_max = 2.0f;
		zoom_increment = 0.1f;
	}
	
	void setPosition(vec2 position);
	void addZoom(float scroll_offset_y);

	vec2& getPosition();

	mat3 createViewMatrix();
	void print(); // for debugging
};