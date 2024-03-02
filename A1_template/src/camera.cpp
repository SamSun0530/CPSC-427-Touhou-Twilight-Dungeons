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

void Camera::addZoom(float scroll_offset_y) {
	zoom = min(zoom_max, max(zoom_min, zoom + scroll_offset_y * zoom_increment));
}

vec2& Camera::getPosition() {
	return this->position;
}

void Camera::print() {
	printf("===============\n");
	printf("camera pos (x,y)=(%f,%f) ", position.x, position.y);
	printf("camera offset (x,y)=(%f,%f)\n", offset.x, offset.y);
	printf("camera top: %f, bottom: %f, left: %f, right: %f\n", top, bottom, left, right);
	printf("zoom: %f\n", zoom);
}

// Set camera world coordinates to edges to cull entities during draw call
// We do this in create view matrix function, as after this call, the draw happens
// Extra padding on the edges allow entities to render before entering the screen
void Camera::setCameraAABB() {
	vec2 camera_center = position + offset;
	float extra_padding = world_tile_size / 2.f;
	vec2 offset_from_center = origin_offset * (1.f / zoom) + extra_padding;
	top = camera_center.y - offset_from_center.y;
	bottom = camera_center.y + offset_from_center.y;
	left = camera_center.x - offset_from_center.x;
	right = camera_center.x + offset_from_center.x;
}

bool Camera::isInCameraView(vec2 position) {
	return position.y >= top && position.y <= bottom && position.x >= left && position.x <= right;
}