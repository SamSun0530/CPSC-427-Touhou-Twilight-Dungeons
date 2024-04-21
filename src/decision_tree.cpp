#include "decision_tree.hpp"

// ========== Action Node ==========
ActionNode::ActionNode(std::function<void(Entity& entity)> action = nullptr) : action(action) {}

IDecisionNode* ActionNode::process(Entity& entity) {
	assert(this->action != nullptr && "No function set for an action node");
	// Do action and return null since terminating/leaf node
	this->action(entity);
	return nullptr;
}

void ActionNode::setAction(std::function<void(Entity& entity)> action) {
	this->action = action;
}

// ========== Conditional Node ==========
ConditionalNode::ConditionalNode(std::function<bool(Entity& entity)> condition) : ConditionalNode(nullptr, nullptr, condition) {}
ConditionalNode::ConditionalNode(IDecisionNode* true_node = nullptr,
	IDecisionNode* false_node = nullptr,
	std::function<bool(Entity& entity)> condition = nullptr) : true_node(true_node), false_node(false_node), condition(condition) {}
ConditionalNode::~ConditionalNode() {
	delete true_node;
	delete false_node;
};

IDecisionNode* ConditionalNode::process(Entity& entity) {
	assert(this->true_node != nullptr && "No decision node set for true node");
	assert(this->false_node != nullptr && "No decision node set for false node");
	assert(this->condition != nullptr && "No function set for an condition node");

	return this->condition(entity) ? this->true_node : this->false_node;
}

void ConditionalNode::setCondition(std::function<bool(Entity& entity)> condition) {
	this->condition = condition;
}

void ConditionalNode::setTrue(IDecisionNode* true_node) {
	this->true_node = true_node;
}

void ConditionalNode::setFalse(IDecisionNode* false_node) {
	this->false_node = false_node;
}

// ========== Decision Tree ==========
DecisionTree::~DecisionTree() {
	delete root;
};

void DecisionTree::update(Entity& entity) {
	// reset current node to root for next entity
	this->current_node = this->root;
	// continue down tree until hit an action node
	while (this->current_node != nullptr) {
		this->current_node = this->current_node->process(entity);
	}
}

void DecisionTree::setRoot(IDecisionNode* root) {
	this->root = root;
	this->current_node = root;
}
