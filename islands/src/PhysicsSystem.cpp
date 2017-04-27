#include "PhysicsSystem.h"
#include "Entity.h"

namespace islands {

PhysicsSystem::PhysicsSystem() :
	GRAVITY(0, 0, -0.01f),
	FRICTION(0.05f) {}

void PhysicsSystem::update() {
	for (const auto body : bodies_) {
		auto v = body->getVelocity() + GRAVITY;

		const auto sphereCollider = std::dynamic_pointer_cast<SphereCollider>(body->getCollider());
		if (body->hasCollider() && sphereCollider) {
			body->advanceTime();
			sphereCollider->update();

			const auto& sphere = sphereCollider->getSphere();
			bool collide = false;
			for (int i = 0; i < 2; ++i) {
				for (const auto c : colliders_) {
					if (c == sphereCollider) {
						continue;
					}
					if (sphereCollider->intersects(c)) {
						collide = true;
						const auto m = std::dynamic_pointer_cast<MeshCollider>(c);
						if (m) {
							for (const auto& tri : sphereCollider->getCollidingTriangles(m)) {
								const auto n = glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0);
								if (glm::dot(n, v) < 0) {
									v -= (glm::dot(n, v) / glm::dot(n, n)) * n;
								}

								/*const auto a = tri.v0 - sphere.center;
								const auto b = tri.v1 - sphere.center;
								const auto c = tri.v2 - sphere.center;
								const auto v = glm::cross(b - a, c - a);
								const auto d = glm::dot(a, v);
								const auto e = glm::dot(v, v);
								body->getEntity().setPosition(body->getEntity().getPosition() +
									(sphere.radius - glm::abs(d) * glm::inversesqrt(e)) * glm::normalize(n));*/
							}
						} else {
							const auto s = std::dynamic_pointer_cast<SphereCollider>(c);
							assert(s);
							const auto n = sphere.center - s->getSphere().center;
							if (glm::dot(n, v) < 0) {
								v -= (glm::dot(n, v) / glm::dot(n, n)) * n;
							}
						}
					}
				}
			}
			if (collide) {
				for (glm::length_t i = 0; i < v.length(); ++i) {
					if (v[i] > FRICTION) {
						v[i] -= FRICTION;
					} else if (v[i] < -FRICTION) {
						v[i] += FRICTION;
					} else {
						v[i] = 0;
					}
				}
				sphereCollider->notifyCollision();
			}
			body->setVelocity(-body->getVelocity());
			body->advanceTime();
		}

		body->setVelocity(v);
		body->advanceTime();
	}
}

void PhysicsSystem::registerCollider(std::shared_ptr<Collider> collider) {
	colliders_.emplace_back(collider);
}

void PhysicsSystem::registerBody(std::shared_ptr<PhysicalBody> body) {
	bodies_.emplace_back(body);
}

bool PhysicsSystem::intersects(std::shared_ptr<Collider> collider) const {
	for (const auto c : colliders_) {
		if (c == collider) {
			continue;
		}
		if (collider->intersects(c)) {
			return true;
		}
	}

	return false;
}

}