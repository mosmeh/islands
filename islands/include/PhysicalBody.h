#pragma once

#include "Component.h"
#include "Collision.h"

namespace islands {

class PhysicalBody : public Component {
public:
	PhysicalBody();
	virtual ~PhysicalBody() = default;

	void update() override;

	void setCollider(std::shared_ptr<Collider> collider);
	std::shared_ptr<Collider> getCollider() const;
	bool hasCollider() const;

	void setMass(float mass);

	void setVelocity(const glm::vec3& velocity);
	void addVelocity(const glm::vec3& velocity);
	const glm::vec3& getVelocity() const;
	void stepForward() const;
	void stepBackward() const;
	void applyImpulse(const glm::vec3& impulse);

private:
	float mass_;
	glm::vec3 velocity_;
	std::shared_ptr<Collider> collider_;
};

}