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
	ComponentContainer<RoomHitbox> roomHitbox;
	ComponentContainer<Kinematic> kinematics;
	ComponentContainer<Collidable> collidables;
	ComponentContainer<AiTimer> aitimers;
	ComponentContainer<FollowPath> followpaths;
	ComponentContainer<EntityAnimation> animation;
	ComponentContainer<BeeEnemy> beeEnemies;
	ComponentContainer<BomberEnemy> bomberEnemies;
	ComponentContainer<WolfEnemy> wolfEnemies;
	ComponentContainer<SubmachineGunEnemy> submachineGunEnemies;
	ComponentContainer<Coin> coins;
	ComponentContainer<Product> products;
	ComponentContainer<MaxHPIncrease> maxhpIncreases;
	ComponentContainer<AttackUp> attackUps;
	ComponentContainer<Chest> chests;
	ComponentContainer<Key> keys;
	ComponentContainer<FollowFlowField> followFlowField;
	ComponentContainer<CircleCollidable> circleCollidables;
	ComponentContainer<EntityAnimation> alwaysplayAni;
	ComponentContainer<BezierCurve> bezierCurves;
	ComponentContainer<FocusDot> focusdots; // only for rendering dot for reimu
	ComponentContainer<RenderText> texts;
	ComponentContainer<RenderTextPermanent> textsPerm;

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
		registry_list.push_back(&beeEnemies);
		registry_list.push_back(&bomberEnemies);
		registry_list.push_back(&wolfEnemies);
		registry_list.push_back(&submachineGunEnemies);
		registry_list.push_back(&coins);
		registry_list.push_back(&products);
		registry_list.push_back(&maxhpIncreases);
		registry_list.push_back(&attackUps);
		registry_list.push_back(&chests);
		registry_list.push_back(&keys);
		registry_list.push_back(&followFlowField);
		registry_list.push_back(&circleCollidables);
		registry_list.push_back(&alwaysplayAni);
		registry_list.push_back(&bezierCurves);
		registry_list.push_back(&focusdots);
		registry_list.push_back(&texts);
		registry_list.push_back(&textsPerm);
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