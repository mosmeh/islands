#include "PhysicalBody.h"
#include "Window.h"

namespace islands {

PhysicalBody::PhysicalBody(std::shared_ptr<Collider> collider = nullptr) :
	mass_(1.f),
	velocity_(0.f),
	collider_(collider),
	receiveGravity_(true),
	isGhost_(false) {}

void PhysicalBody::update() {}

std::shared_ptr<Collider> PhysicalBody::getCollider() const {
	assert(hasCollider());
	return collider_;
}

bool PhysicalBody::hasCollider() const {
	return collider_ != nullptr;
}

void PhysicalBody::setMass(float mass) {
	mass_ = mass;
}

void PhysicalBody::setVelocity(const glm::vec3& velocity) {
	velocity_ = velocity;
}

void PhysicalBody::addVelocity(const glm::vec3& velocity) {
	velocity_ += velocity;
}

const glm::vec3& PhysicalBody::getVelocity() const {
	return velocity_;
}

void PhysicalBody::moveBy(const glm::vec3& offset) const {
	getEntity().setPosition(getEntity().getPosition() + offset);
}

void PhysicalBody::stepForward() const {
	moveBy(Window::getInstance().getDeltaTime() * velocity_);
}

void PhysicalBody::applyImpulse(const glm::vec3& impulse) {
	velocity_ += impulse / mass_;
}

void PhysicalBody::setReceiveGravity(bool receive) {
	receiveGravity_ = receive;
}

bool PhysicalBody::getReceiveGravity() const {
	return receiveGravity_;
}

void PhysicalBody::setGhost(bool isGhost) {
	isGhost_ = isGhost;
}

bool PhysicalBody::isGhost() const {
	return isGhost_;
}

}
