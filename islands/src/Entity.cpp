#include "Entity.h"
#include "Component.h"

namespace islands {

Entity::Entity(const std::string& name, Chunk* chunk) :
	Resource(name),
	name_(name),
	chunk_(chunk),
	position_(0),
	quaternion_(1, 0, 0, 0),
	scale_(1) {}

bool Entity::isLoaded() const {
	for (const auto c : components_) {
		const auto resource = std::dynamic_pointer_cast<Resource>(c);
		if (resource && !resource->isLoaded()) {
			return false;
		}
	}
	return true;
}

void Entity::setPosition(const glm::vec3& position) {
	position_ = position;
	updateModelMatrix();
}

const glm::vec3& Entity::getPosition() const {
	return position_;
}

void Entity::setQuaternion(const glm::quat& quaternion) {
	quaternion_ = quaternion;
	updateModelMatrix();
}

const glm::quat& Entity::getQuaternion() const {
	return quaternion_;
}

void Entity::setScale(const glm::vec3& scale) {
	scale_ = scale;
	updateModelMatrix();
}

const glm::vec3& Entity::getScale() const {
	return scale_;
}

const glm::mat4& Entity::getModelMatrix() const {
	return modelMatrix_;
}

void Entity::update() const {
	for (const auto c : components_) {
		c->startOnce();
		c->update();
	}
}

void Entity::draw() const {
	for (const auto c : components_) {
		if (const auto drawable = std::dynamic_pointer_cast<Drawable>(c)) {
			drawable->draw();
		}
	}
}

Chunk& Entity::getChunk() const {
	return *chunk_;
}

void Entity::attachComponent(std::shared_ptr<Component> component) {
	component->setEntity(this);
	components_.emplace_back(component);
}

void Entity::loadImpl() {
	for (const auto c : components_) {
		if (const auto resource = std::dynamic_pointer_cast<Resource>(c)) {
			resource->loadAsync();
		}
	}
}

void Entity::updateModelMatrix() {
	modelMatrix_ = glm::scale(
		glm::translate(glm::mat4(1.f), position_) * glm::mat4_cast(quaternion_), scale_);
	/* above does the same as below
	const auto translation = glm::translate(glm::mat4(1.f), position_);
	const auto rotation = glm::mat4_cast(quaternion_);
	const auto scaling = glm::scale(glm::mat4(1.f), scale_);
	modelMatrix_ = translation * rotation * scaling;*/
}

}