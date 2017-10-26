#include "Physics.h"
#include "Entity.h"
#include "Window.h"

namespace islands {

Physics::Physics() :
	GRAVITY(0, 0, -36.f),
	FRICTION(3.f) {}

void Physics::update() {
	for (const auto body : bodies_) {
		auto v = body->getVelocity();
		if (body->getReceiveGravity()) {
			v += GRAVITY * Window::getInstance().getDeltaTime();
		}
		body->setVelocity(v);
		body->stepForward();
	}

	for (const auto collider : colliders_) {
		collider->update();
	}

	for (const auto collider : colliders_) {
		for (const auto c : colliders_) {
			if (&c->getEntity() == &collider->getEntity()) {
				continue;
			}
			if (collider->intersects(c)) {
				collider->notifyCollision(c);
			}
		}
	}

	for (const auto body : bodies_) {
		if (body->hasCollider()) {
			bool collide = false;
			const auto collider = body->getCollider();
			for (const auto c : colliders_) {
				if (&c->getEntity() == &collider->getEntity()) {
					continue;
				}
				if (collider->intersects(c)) {
					collide = true;
					body->moveBy(c->getSinkingCorrector(collider));
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

std::shared_ptr<PhysicalBody> Physics::createBody(std::shared_ptr<Collider> collider) {
	const auto body = std::make_shared<PhysicalBody>(collider);
	bodies_.emplace_back(body);
	return body;
}

}