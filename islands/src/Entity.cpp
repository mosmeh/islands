#include "Entity.h"
#include "Component.h"

namespace islands {

Entity::Entity(const std::string& name) :
	Resource(name),
	name_(name),
	chunk_(nullptr),
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

void Entity::update() {
	for (const auto c : components_) {
		c->startAndUpdate();
	}
}

void Entity::draw() const {
	for (const auto c : components_) {
		if (const auto drawable = std::dynamic_pointer_cast<Drawable>(c)) {
			drawable->draw();
		}
	}
}

void Entity::setChunk(Chunk* chunk) {
	chunk_ = chunk;
}

Chunk& Entity::getChunk() const {
	assert(chunk_);
	return *chunk_;
}

void Entity::attachComponent(std::shared_ptr<Component> component) {
	component->setEntity(this);
	components_.emplace_back(component);
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