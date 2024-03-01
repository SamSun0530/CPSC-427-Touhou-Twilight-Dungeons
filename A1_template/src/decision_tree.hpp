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
	void (*action)(Entity& entity);
public:
	ActionNode(void (*action)(Entity& entity));

	IDecisionNode* process(Entity& entity) override;
	void setAction(void (*action)(Entity& entity));
};

// Conditional node for sensing environment
class ConditionalNode : public IDecisionNode {
	IDecisionNode* true_node;
	IDecisionNode* false_node;
	// Conditional function for entity sensing environment
	bool (*condition)(Entity& entity);
public:
	ConditionalNode(IDecisionNode* true_node, IDecisionNode* false_node, bool (*condition)(Entity& entity));

	IDecisionNode* process(Entity& entity) override;
	void setCondition(bool (*condition)(Entity& entity));
	void setTrue(IDecisionNode* true_node);
	void setFalse(IDecisionNode* false_node);
};

// Decision tree representation for updating entity
class DecisionTree
{
	IDecisionNode* root;
	IDecisionNode* current_node;
public:
	DecisionTree();
	DecisionTree(IDecisionNode* root);
	void update(Entity& entity);
	void setRoot(IDecisionNode* root);
};
