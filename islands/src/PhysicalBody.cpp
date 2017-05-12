#include "PhysicalBody.h"
#include "Entity.h"

namespace islands {

PhysicalBody::PhysicalBody(float mass) :
	mass_(mass),
	velocity_(0.f),
	collider_(nullptr) {}

void PhysicalBody::update() {}

void PhysicalBody::setCollider(std::shared_ptr<Collider> collider) {
	collider_ = collider;
}

std::shared_ptr<Collider> PhysicalBody::getCollider() const {
	return collider_;
}

bool PhysicalBody::hasCollider() const {
	return collider_ != nullptr;
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

void PhysicalBody::stepForward() const {
	getEntity().setPosition(getEntity().getPosition() + velocity_);
}

void PhysicalBody::stepBackward() const {
	getEntity().setPosition(getEntity().getPosition() - velocity_);
}

void PhysicalBody::applyImpulse(const glm::vec3& impulse) {
	velocity_ += impulse / mass_;
}

}