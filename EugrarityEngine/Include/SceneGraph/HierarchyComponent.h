#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;
class Entity;

class
	HierarchyComponent : public Component {
public:
	HierarchyComponent() : Component(ComponentType::HIERARCHY) {}
	~HierarchyComponent() = default;

	void
		init() override {}

	void
		update(float) override {}

	void
		render(DeviceContext& deviceContext) override {}

	void
		destroy() override {
		m_children.clear();
		m_parent = nullptr;
	}

	// API SceneGraph
	void
		setParent(Entity* parent) {
		m_parent = parent;
	}

	bool
		isRoot() const {
		return m_parent == nullptr;
	}

	bool
		hasChildren() const {
		return !m_children.empty();
	}

	void
		addChild(Entity* child) {
		if (!child) {
			return;
		}

		if (std::find(m_children.begin(), m_children.end(), child) != m_children.end()) {
			return;
		}
		m_children.push_back(child);
	}

	void
		removeChild(Entity* child) {
		if (!child) return;

		m_children.erase(
			std::remove(m_children.begin(), m_children.end(), child),
			m_children.end()
		);
	}

public:
	Entity* m_parent = nullptr;
	std::vector<Entity*> m_children;
};