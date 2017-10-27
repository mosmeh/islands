#include "Entity.h"
#include "Component.h"
#include "Camera.h"

namespace islands {

Entity::Entity(const std::string& name, Chunk& chunk) :
	Resource(name),
	name_(name),
	chunk_(chunk),
	position_(0),
	quaternion_(1, 0, 0, 0),
	scale_(1),
	selfMask_(0),
	filterMask_(0),
	destroyed_(false) {}

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

glm::mat4 Entity::calculateMVPMatrix() const {
	return Camera::getInstance().getProjectionViewMatrix() * modelMatrix_;
}

void Entity::update() {
	cleanComponents();
	for (size_t i = 0; i < components_.size(); ++i) {
		components_.at(i)->startAndUpdate();
	}
	cleanComponents();
}

void Entity::draw() const {
	for (const auto c : components_) {
		if (const auto drawable = std::dynamic_pointer_cast<Drawable>(c)) {
			drawable->draw();
		}
	}
}

Chunk& Entity::getChunk() const {
	return chunk_;
}

void Entity::setSelfMask(MaskType mask) {
	selfMask_ = mask;
}

Entity::MaskType Entity::getSelfMask() const {
	return selfMask_;
}

void Entity::setFilterMask(MaskType mask) {
	filterMask_ = mask;
}

Entity::MaskType Entity::getFilterMask() const {
	return filterMask_;
}

void Entity::destroy() {
	destroyed_ = true;
}

bool Entity::isDestroyed() {
	return destroyed_;
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

void Entity::cleanComponents() {
	components_.erase(std::remove_if(components_.begin(), components_.end(), [](std::shared_ptr<Component> c) {
		return c->isDestroyed();
	}), components_.end());

}

}