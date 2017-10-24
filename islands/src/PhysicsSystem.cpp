#include "PhysicsSystem.h"
#include "Entity.h"
#include "Window.h"

namespace islands {

PhysicsSystem::PhysicsSystem() :
	GRAVITY(0, 0, -36.f),
	FRICTION(3.f) {}

void PhysicsSystem::update() {
	for (const auto body : bodies_) {
		auto v = body->getVelocity();
		if (body->getReceiveGravity()) {
			v += GRAVITY * Window::getInstance().getDeltaTime();
		}
		body->setVelocity(v);
		body->stepForward();
		if (body->hasCollider()) {
			body->getCollider()->update();
		}
	}

	for (const auto body : bodies_) {
		if (body->hasCollider()) {
			bool collide = false;
			const auto collider = body->getCollider();
			for (const auto c : colliders_) {
				if (c == collider) {
					continue;
				}
				if (collider->intersects(c)) {
					collide = true;
					body->moveBy(c->getSinkingCorrector(collider));
					collider->notifyCollision(c);
				}
			}
			if (collide) {
				auto v = body->getVelocity();
				for (glm::length_t i = 0; i < v.length(); ++i) {
					if (v[i] > FRICTION) {
						v[i] -= FRICTION;
					} else if (v[i] < -FRICTION) {
						v[i] += FRICTION;
					} else {
						v[i] = 0;
					}
				}
				body->setVelocity(v);
			}
		}
	}
}

bool PhysicsSystem::registerCollider(std::shared_ptr<Collider> collider) {
	if (std::find(colliders_.begin(), colliders_.end(), collider) == colliders_.end()) {
		colliders_.emplace_back(collider);
		return true;
	}
	return false;
}

bool PhysicsSystem::unregisterCollider(std::shared_ptr<Collider> collider) {
	const auto iter = std::find(colliders_.begin(), colliders_.end(), collider);
	if (iter != colliders_.end()) {
		colliders_.erase(iter);
		return true;
	}
	return false;
}

bool PhysicsSystem::registerBody(std::shared_ptr<PhysicalBody> body) {
	if (std::find(bodies_.begin(), bodies_.end(), body) == bodies_.end()) {
		bodies_.emplace_back(body);
		return true;
	}
	return false;
}

bool PhysicsSystem::unregisterBody(std::shared_ptr<PhysicalBody> body) {
	const auto iter = std::find(bodies_.begin(), bodies_.end(), body);
	if (iter != bodies_.end()) {
		bodies_.erase(iter);
		return true;
	}
	return false;
}

}