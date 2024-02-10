#include "camera.hpp"

mat3 Camera::createViewMatrix()
{
	/*
	View matrix for camera - apply view matrix to entities that move
	For translating camera position in x, y direction (column-major ordering)
	Take the transpose of M_cam to get M_view
	1 0 x
	0 1 y
	0 0 1
	*/
	Transform transform;
	transform.translate(origin_offset);
	transform.translate(-position);
	transform.translate(-offset);
	return transform.mat;
}

void Camera::setPosition(vec2 position) {
	this->position = position;
};

void Camera::setOffset(vec2 offset) {
	this->offset = offset;
}

vec2& Camera::getPosition() {
	return this->position;
}

vec2& Camera::getOffset() {
	return this->offset;
}

void Camera::print() {
	printf("camera pos (x,y)=(%f,%f) ", position.x, position.y);
	printf("camera offset (x,y)=(%f,%f)\n", offset.x, offset.y);
}