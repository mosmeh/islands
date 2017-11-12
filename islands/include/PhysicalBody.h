#pragma once

#include "Component.h"
#include "Collision.h"

namespace islands {

class PhysicalBody : public Component {
public:
	PhysicalBody(std::shared_ptr<Collider> collider);
	virtual ~PhysicalBody() = default;

	void update() override;

	std::shared_ptr<Collider> getCollider() const;
	bool hasCollider() const;

	void setMass(float mass);

	void setVelocity(const glm::vec3& velocity);
	void addVelocity(const glm::vec3& velocity);
	const glm::vec3& getVelocity() const;
	void moveBy(const glm::vec3& offset) const;
	void stepForward() const;
	void applyImpulse(const glm::vec3& impulse);

	void setReceiveGravity(bool receive);
	bool getReceiveGravity() const;

private:
	std::shared_ptr<Collider> collider_;
	float mass_;
	glm::vec3 velocity_;
	bool receiveGravity_;
};

}
