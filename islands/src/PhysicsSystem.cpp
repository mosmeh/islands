#include "PhysicsSystem.h"
#include "Entity.h"
#include "Window.h"

namespace islands {

PhysicsSystem::PhysicsSystem() :
	GRAVITY(0, 0, -36.f),
	FRICTION(3.f) {}

void PhysicsSystem::update() {
	constexpr size_t MAX_NUM_ITERATIONS = 32;

	for (const auto body : bodies_) {
		auto v = body->getVelocity();
		if (body->getReceiveGravity()) {
			v += GRAVITY * Window::getInstance().getDeltaTime();
		}

		if (body->hasCollider()) {
			const auto collider = std::dynamic_pointer_cast<SphereCollider>(body->getCollider());
			assert(collider);

			body->stepForward();
			collider->update();

			static const auto rectifyVelocity = [&v](const glm::vec3& normal) {
				if (glm::dot(normal, v) < 0) {
					v -= (glm::dot(normal, v) / glm::dot(normal, normal)) * normal;
				}
			};

			bool collide = false;
			auto prevV = glm::vec3(INFINITY);
			for (size_t count = 0; count < MAX_NUM_ITERATIONS; ++count) {
				if (glm::distance2(prevV, v) < glm::epsilon<float>()) {
					break;
				}
				prevV = v;

				for (const auto c : colliders_) {
					if (c == collider) {
						continue;
					}
					if (collider->intersects(c)) {
						collide = true;
						if (const auto meshCollider = std::dynamic_pointer_cast<MeshCollider>(c)) {
							for (const auto& tri : collider->getCollidingTriangles(meshCollider)) {
								rectifyVelocity(glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0));

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
							rectifyVelocity(collider->getSphere().center - sphereCollider->getSphere().center);
						} else if (const auto planeCollider = std::dynamic_pointer_cast<PlaneCollider>(c)) {
							rectifyVelocity(planeCollider->getPlane().normal);
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
			body->stepBackward();
		}

		body->setVelocity(v);
		body->stepForward();
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