#pragma once

#include "PhysicalBody.h"
#include "Collision.h"

namespace islands {

class PhysicsSystem {
public:
	PhysicsSystem();
	virtual ~PhysicsSystem() = default;

	void update();
	bool registerCollider(std::shared_ptr<Collider> collider);
	bool unregisterCollider(std::shared_ptr<Collider> collider);
	bool registerBody(std::shared_ptr<PhysicalBody> body);
	bool unregisterBody(std::shared_ptr<PhysicalBody> body);

private:
	const glm::vec3 GRAVITY;
	const float FRICTION;
	std::vector<std::shared_ptr<Collider>> colliders_;
	std::vector<std::shared_ptr<PhysicalBody>> bodies_;
};

}