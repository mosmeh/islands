#pragma once

#include "Resource.h"

namespace islands {

class Component;
class Chunk;

class Entity : public Resource {
public:
	Entity(const std::string& name, Chunk* chunk);
	virtual ~Entity() = default;

	bool isLoaded() const override;

	void setPosition(const glm::vec3& position);
	const glm::vec3& getPosition() const;
	void setQuaternion(const glm::quat& quaternion);
	const glm::quat& getQuaternion() const;
	void setScale(const glm::vec3& scale);
	const glm::vec3& getScale() const;

	const glm::mat4& getModelMatrix() const;

	void update() const;
	void draw() const;
	Chunk& getChunk() const;

	void attachComponent(std::shared_ptr<Component> component);

	template <class Real>
	std::enable_if_t<std::is_base_of<Component, Real>::value, bool>
	hasComponent() const;

	template <class Real>
	std::enable_if_t<std::is_base_of<Component, Real>::value, std::shared_ptr<Real>>
	getComponent() const;

private:
	const std::string name_;
	Chunk* chunk_;
	glm::vec3 position_, scale_;
	glm::quat quaternion_;
	glm::mat4 modelMatrix_;
	std::vector<std::shared_ptr<Component>> components_;

	void loadImpl() override;
	void updateModelMatrix();
};

template<class Real>
inline std::enable_if_t<std::is_base_of<Component, Real>::value, bool>
Entity::hasComponent() const {
	for (const auto c : components_) {
		if (const auto t = std::dynamic_pointer_cast<Real>(c)) {
			return true;
		}
	}
	return false;
}

template<class Real>
inline std::enable_if_t<std::is_base_of<Component, Real>::value, std::shared_ptr<Real>>
Entity::getComponent() const {
	for (const auto c : components_) {
		if (const auto t = std::dynamic_pointer_cast<Real>(c)) {
			return t;
		}
	}
	throw std::invalid_argument("not found");
}

}