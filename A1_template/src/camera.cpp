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
	transform.scale(vec2(zoom));
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

void Camera::addZoom(float scroll_offset_y) {
	float new_zoom = zoom + scroll_offset_y * zoom_increment;
	if (new_zoom >= zoom_min && new_zoom <= zoom_max) {
		zoom = new_zoom;
	}
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