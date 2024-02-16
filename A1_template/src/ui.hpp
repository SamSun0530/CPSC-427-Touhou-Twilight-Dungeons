#pragma once

#include "common.hpp"

class UI {
	// UI position x,y
	vec2 position;

	// Offset for translating origin from top left to center
	const vec2 origin_offset;
public:

	UI() : origin_offset(window_px_half) {
		position = { 0.f, 0.f };
	}

	void setPosition(vec2 position);

	vec2& getPosition();

	mat3 createViewMatrix();
	void print(); // for debugging
};