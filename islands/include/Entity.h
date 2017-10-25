#pragma once

#include "Resource.h"

namespace islands {

class Component;
class Chunk;

class Entity : public Resource {
public:
	Entity(const std::string& name, Chunk& chunk);
	virtual ~Entity() = default;

	bool isLoaded() const override;

	void setPosition(const glm::vec3& position);
	const glm::vec3& getPosition() const;
	void setQuaternion(const glm::quat& quaternion);
	const glm::quat& getQuaternion() const;
	void setScale(const glm::vec3& scale);
	const glm::vec3& getScale() const;

	const glm::mat4& getModelMatrix() const;

	void update();
	void draw() const;

	Chunk& getChunk() const;

	void attachComponent(std::shared_ptr<Component> component);

	template <class T, class... Args>
	std::enable_if_t<std::is_base_of<Component, T>::value, std::shared_ptr<T>>
	createComponent(Args... args);

	template <class T>
	std::enable_if_t<std::is_base_of<Component, T>::value, bool>
	hasComponent() const;

	template <class T>
	std::enable_if_t<std::is_base_of<Component, T>::value, std::shared_ptr<T>>
	getFirstComponent() const;

	template <class T>
	std::enable_if_t<std::is_base_of<Component, T>::value, std::vector<std::shared_ptr<T>>>
	getComponents() const;

private:
	const std::string name_;
	Chunk& chunk_;
	glm::vec3 position_, scale_;
	glm::quat quaternion_;
	glm::mat4 modelMatrix_;
	std::vector<std::shared_ptr<Component>> components_;

	void updateModelMatrix();
};

template<class T, class ...Args>
inline std::enable_if_t<std::is_base_of<Component, T>::value, std::shared_ptr<T>>
Entity::createComponent(Args ...args) {
	const auto component = std::make_shared<T>(args...);
	attachComponent(component);
	return component;
}

template<class T>
inline std::enable_if_t<std::is_base_of<Component, T>::value, bool>
Entity::hasComponent() const {
	for (const auto c : components_) {
		if (const auto t = std::dynamic_pointer_cast<T>(c)) {
			return true;
		}
	}
	return false;
}

template<class T>
inline std::enable_if_t<std::is_base_of<Component, T>::value, std::shared_ptr<T>>
Entity::getFirstComponent() const {
	for (const auto c : components_) {
		if (const auto t = std::dynamic_pointer_cast<T>(c)) {
			return t;
		}
	}
	throw std::invalid_argument("not found");
}

template<class T>
inline std::enable_if_t<std::is_base_of<Component, T>::value, std::vector<std::shared_ptr<T>>>
Entity::getComponents() const {
	std::vector<std::shared_ptr<T>> comps;
	for (const auto c : components_) {
		if (const auto t = std::dynamic_pointer_cast<T>(c)) {
			comps.emplace_back(t);
		}
	}
	return comps;
}

}