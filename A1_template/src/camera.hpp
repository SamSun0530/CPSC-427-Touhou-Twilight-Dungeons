#pragma once

#include "common.hpp"

class Camera {
	// Camera position x,y
	vec2 position;

	// Offset for translating origin from top left to center
	const vec2 origin_offset;

	// Offset for following mouse cursor
	vec2 offset;
public:
	// Determine if camera could be controlled freely
	bool isFreeCam;

	Camera() : origin_offset(window_px_half) {
		position = { 0.f, 0.f };
		isFreeCam = false;
		offset = { 0.f, 0.f };
	}

	void setPosition(vec2 position);
	void setOffset(vec2 offset);
	vec2& getPosition();
	vec2& getOffset();
	mat3 createViewMatrix();
	void print(); // for debugging
};