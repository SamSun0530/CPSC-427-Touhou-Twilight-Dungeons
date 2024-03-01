#include "ai_system.hpp"

void AISystem::init() {

	ConditionalNode* is_near_player = new ConditionalNode([](Entity& entity) {
		float distance_to_attack = 90000.f; // sqrt(90000)=300 pixels
		Motion& motion = registry.motions.get(entity);
		// asusme there is only one player
		for (Entity& player_entity : registry.players.entities) {
			Motion& player_motion = registry.motions.get(player_entity);
			vec2 dp = player_motion.position - motion.position;
			if (dot(dp, dp) < distance_to_attack) return true;
		}
		return false;
		});
	ActionNode* stop_firing = new ActionNode([](Entity& entity) { registry.bulletFireRates.remove(entity); });
	ActionNode* fire_at_player = new ActionNode([](Entity& entity) {
		if (!registry.bulletFireRates.has(entity)) {
			auto& fire_rate = registry.bulletFireRates.emplace(entity);
			fire_rate.is_firing = true;
			fire_rate.fire_rate = 3;
		}
		});
	is_near_player->setTrue(fire_at_player);
	is_near_player->setFalse(stop_firing);
	this->ghost.setRoot(is_near_player);
}