#include "PhysicsSystem.h"
#include "Entity.h"

namespace islands {

PhysicsSystem::PhysicsSystem() :
	GRAVITY(0, 0, -0.01f),
	FRICTION(0.05f) {}

void PhysicsSystem::update() {
	for (const auto body : bodies_) {
		auto v = body->getVelocity() + GRAVITY;

		const auto collider = std::dynamic_pointer_cast<SphereCollider>(body->getCollider());
		if (body->hasCollider() && collider) {
			body->advanceTime();
			collider->update();

			const auto& sphere = collider->getSphere();
			bool collide = false;
			auto prevV = glm::vec3(NAN);
			for (size_t count = 0; prevV != v && count < 10; ++count) {
				prevV = v;
				for (const auto c : colliders_) {
					if (c == collider) {
						continue;
					}
					if (collider->intersects(c)) {
						collide = true;
						if (const auto meshCollider = std::dynamic_pointer_cast<MeshCollider>(c)) {
							for (const auto& tri : collider->getCollidingTriangles(meshCollider)) {
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
						} else if (const auto sphereCollider = std::dynamic_pointer_cast<SphereCollider>(c)) {
							const auto n = sphere.center - sphereCollider->getSphere().center;
							if (glm::dot(n, v) < 0) {
								v -= (glm::dot(n, v) / glm::dot(n, n)) * n;
							}
						} else if (const auto planeCollider = std::dynamic_pointer_cast<PlaneCollider>(c)) {
							const auto n = planeCollider->getPlane().normal;
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
				collider->notifyCollision();
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