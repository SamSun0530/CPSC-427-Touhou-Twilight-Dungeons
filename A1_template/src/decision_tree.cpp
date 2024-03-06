#include "decision_tree.hpp"

// ========== Action Node ==========
ActionNode::ActionNode(void (*action)(Entity& entity) = nullptr) : action(action) {}

IDecisionNode* ActionNode::process(Entity& entity) {
	assert(this->action != nullptr && "No function set for an action node");
	// Do action and return null since terminating/leaf node
	this->action(entity);
	return nullptr;
}

void ActionNode::setAction(void (*action)(Entity& entity)) {
	this->action = action;
}

// ========== Conditional Node ==========
ConditionalNode::ConditionalNode(bool (*condition)(Entity& entity)) : ConditionalNode(nullptr, nullptr, condition) {}
ConditionalNode::ConditionalNode(IDecisionNode* true_node = nullptr,
	IDecisionNode* false_node = nullptr,
	bool (*condition)(Entity& entity) = nullptr) : true_node(true_node), false_node(false_node), condition(condition) {}
ConditionalNode::~ConditionalNode() { delete true_node; delete false_node; };

IDecisionNode* ConditionalNode::process(Entity& entity) {
	assert(this->true_node != nullptr && "No decision node set for true node");
	assert(this->false_node != nullptr && "No decision node set for false node");
	assert(this->condition != nullptr && "No function set for an condition node");

	return this->condition(entity) ? this->true_node : this->false_node;
}

void ConditionalNode::setCondition(bool (*condition)(Entity& entity)) {
	this->condition = condition;
}

void ConditionalNode::setTrue(IDecisionNode* true_node) {
	this->true_node = true_node;
}

void ConditionalNode::setFalse(IDecisionNode* false_node) {
	this->false_node = false_node;
}

// ========== Decision Tree ==========
DecisionTree::DecisionTree() : DecisionTree(nullptr) {}
DecisionTree::DecisionTree(IDecisionNode* root = nullptr) : root(root), current_node(root) {}
//DecisionTree::~DecisionTree() { delete root; }; // TODO: memory leak
DecisionTree::~DecisionTree() {}; // TODO: memory leak

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
