#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
private:
	RenderSystem* renderer;

public:
	void step(float elapsed_ms);
	void init(RenderSystem* renderer_arg);

	PhysicsSystem()
	{
	}
};