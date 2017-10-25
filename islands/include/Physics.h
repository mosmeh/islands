#pragma once

#include "PhysicalBody.h"
#include "Collision.h"

namespace islands {

class Physics {
public:
	Physics();
	virtual ~Physics() = default;

	void update();

	template <class T, class... Args>
	std::enable_if_t<std::is_base_of<Collider, T>::value, std::shared_ptr<T>>
		createCollider(Args... args) {

		const auto collider = std::make_shared<T>(args...);
		colliders_.emplace_back(collider);
		return collider;
	}

	std::shared_ptr<PhysicalBody> createBody(std::shared_ptr<Collider> collider);

private:
	const glm::vec3 GRAVITY;
	const float FRICTION;
	std::vector<std::shared_ptr<Collider>> colliders_;
	std::vector<std::shared_ptr<PhysicalBody>> bodies_;
};

}