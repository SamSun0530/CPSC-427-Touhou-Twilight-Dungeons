#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<HitTimer> hitTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<RenderRequest> renderRequestsForeground;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Pickupable> pickupables;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Floor> floors;
	ComponentContainer<Wall> walls;
	ComponentContainer<EnemyBullet> enemyBullets;
	ComponentContainer<InvulnerableTimer> invulnerableTimers;
	ComponentContainer<HP> hps;
	ComponentContainer<PlayerBullet> playerBullets;
	ComponentContainer<IdleMoveAction> idleMoveActions;
	ComponentContainer<BulletFireRate> bulletFireRates;
	ComponentContainer<DeathTimer> realDeathTimers;
	ComponentContainer<Kinematic> kinematics;
	ComponentContainer<Collidable> collidables;
	ComponentContainer<AiTimer> aitimers;
	ComponentContainer<FollowPath> followpaths;
	ComponentContainer<EntityAnimation> animation;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&hitTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&renderRequestsForeground);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&pickupables);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&floors);
		registry_list.push_back(&walls);
		registry_list.push_back(&enemyBullets);
		registry_list.push_back(&invulnerableTimers);
		registry_list.push_back(&hps);
		registry_list.push_back(&playerBullets);
		registry_list.push_back(&idleMoveActions);
		registry_list.push_back(&bulletFireRates);
		registry_list.push_back(&realDeathTimers);
		registry_list.push_back(&kinematics);
		registry_list.push_back(&collidables);
		registry_list.push_back(&aitimers);
		registry_list.push_back(&followpaths);
		registry_list.push_back(&animation);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;