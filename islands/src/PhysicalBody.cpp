#include "PhysicalBody.h"
#include "Entity.h"
#include "Window.h"

namespace islands {

PhysicalBody::PhysicalBody() :
	mass_(1.f),
	velocity_(0.f),
	collider_(nullptr),
	receiveGravity_(true) {}

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

void PhysicalBody::stepForward() const {
	getEntity().setPosition(getEntity().getPosition() + Window::getInstance().getDeltaTime() * velocity_);
}

void PhysicalBody::stepBackward() const {
	getEntity().setPosition(getEntity().getPosition() - Window::getInstance().getDeltaTime() * velocity_);
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

}