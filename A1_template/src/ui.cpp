#include "ui.hpp"

mat3 UI::createViewMatrix()
{
	/*
	View matrix for camera - apply view matrix to entities that move
	For translating camera position in x, y direction (column-major ordering)
	1 0 x
	0 1 y
	0 0 1
	*/
	Transform transform;
	//transform.translate(-position);
	transform.translate(origin_offset);
	return transform.mat;
}

void UI::setPosition(vec2 position) {
	this->position = position;
};

vec2& UI::getPosition() {
	return this->position;
}

void UI::print() {
}