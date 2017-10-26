#pragma once
#include "Component.h"

namespace islands {

class Health : public Component {
public:
	using HealthType = unsigned int;
	Health(HealthType maxHealth);
	virtual ~Health() = default;

	void update() override;
	bool isDead() const;
	void takeDamage(HealthType damage);
	float getNormalized() const;

private:
	using SignedHealthType = std::make_signed<HealthType>::type;
	const HealthType maxHealth_;
	SignedHealthType health_;
};

}