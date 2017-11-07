#pragma once
#include "Component.h"

namespace islands {

class Health : public Component {
public:
	using HealthType = unsigned int;
	Health(HealthType maxHealth, double invincibleDuration = 0.0);
	virtual ~Health() = default;

	void update() override;
	bool isDead() const;
	bool takeDamage(HealthType damage);
	HealthType get() const;
	float getNormalized() const;
	HealthType getMaxHealth() const;
	void setMaxHealth(HealthType maxHealth);
	void set(HealthType health);
	bool isInvincible() const;

private:
	using SignedHealthType = std::make_signed<HealthType>::type;
	const double invincibleDuration_;
	HealthType maxHealth_;
	SignedHealthType health_;
	double lastDamageTakenAt_;
};

}
