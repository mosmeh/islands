#include "Physics.h"
#include "Collision.h"
#include "PhysicalBody.h"
#include "Entity.h"
#include "Window.h"

namespace islands {

namespace physics {

void update(const Chunk& chunk) {
	static const glm::vec3 GRAVITY(0, 0, -36.f);
	static constexpr float FRICTION = 3.f;

	std::vector<std::shared_ptr<PhysicalBody>> bodies;
	std::vector<std::shared_ptr<Collider>> colliders;
	for (const auto entity : chunk.getEntities()) {
		for (const auto body : entity->getComponents<PhysicalBody>()) {
			bodies.emplace_back(body);
		}
		for (const auto collider : entity->getComponents<Collider>()) {
			colliders.emplace_back(collider);
		}
	}

	for (const auto body : bodies) {
		auto v = body->getVelocity();
		if (body->getReceiveGravity()) {
			v += GRAVITY * Window::getInstance().getDeltaTime();
		}
		body->setVelocity(v);
		body->stepForward();
	}

	for (const auto collider : colliders) {
		collider->update();
	}

	for (const auto collider : colliders) {
		for (const auto c : colliders) {
			if (&c->getEntity() == &collider->getEntity()) {
				continue;
			}
			if (collider->intersects(c)) {
				collider->notifyCollision(c);
			}
		}
	}

	for (const auto body : bodies) {
		if (body->hasCollider() && !body->isGhost()) {
			bool frictionCollide = false;
			const auto collider = body->getCollider();
			for (const auto c : colliders) {
				if (&c->getEntity() == &collider->getEntity()) {
					continue;
				}
				if (!c->isGhost() && collider->intersects(c)) {
					body->moveBy(c->getSinkageCorrector(collider));

					if (c->getEntity().getSelfMask() != Entity::Mask::CollisionWall) {
						frictionCollide = true;
					}
				}
			}
			if (frictionCollide) {
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

}

}
