#include "camera.hpp"

mat3 Camera::createViewMatrix()
{
	/*
	View matrix for camera - apply view matrix to entities that move
	For translating camera position in x, y direction (column-major ordering)
	1 0 x
	0 1 y
	0 0 1
	*/
	Transform transform;
	transform.translate(origin_offset);
	transform.translate(-position);
	return transform.mat;
}

void Camera::setPosition(vec2 position) {
	this->position = position;
};
