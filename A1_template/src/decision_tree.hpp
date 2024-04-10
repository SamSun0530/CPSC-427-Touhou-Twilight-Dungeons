#pragma once

#include "tiny_ecs_registry.hpp"

// Decision node interface
class IDecisionNode {
public:
	virtual IDecisionNode* process(Entity& entity) = 0;
};

// Action leaf node for executing actions
class ActionNode : public IDecisionNode {
	// Action function for entity acting
	std::function<void(Entity& entity)> action;
public:
	ActionNode(std::function<void(Entity& entity)>);

	IDecisionNode* process(Entity& entity) override;
	void setAction(std::function<void(Entity& entity)>);
};

// Conditional node for sensing environment
class ConditionalNode : public IDecisionNode {
	IDecisionNode* true_node = nullptr;
	IDecisionNode* false_node = nullptr;
	// Conditional function for entity sensing environment
	std::function<bool(Entity& entity)> condition;

public:
	ConditionalNode(std::function<bool(Entity& entity)> condition);
	ConditionalNode(IDecisionNode* true_node, IDecisionNode* false_node, std::function<bool(Entity& entity)> condition);
	~ConditionalNode();

	IDecisionNode* process(Entity& entity) override;
	void setCondition(std::function<bool(Entity& entity)> condition);
	void setTrue(IDecisionNode* true_node);
	void setFalse(IDecisionNode* false_node);
};

// Decision tree representation for updating entity
class DecisionTree
{
	IDecisionNode* root = nullptr;
	IDecisionNode* current_node = nullptr;
public:
	~DecisionTree();

	void update(Entity& entity);
	void setRoot(IDecisionNode* root);
};
