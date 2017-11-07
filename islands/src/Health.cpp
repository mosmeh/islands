#include "Health.h"

namespace islands {

Health::Health(HealthType maxHealth, double invincibleDuration) :
	health_(maxHealth),
	maxHealth_(maxHealth),
	invincibleDuration_(invincibleDuration) {}

void Health::update() {}

bool Health::isDead() const {
	return health_ <= 0;
}

bool Health::takeDamage(HealthType damage) {
	if (isInvincible()) {
		return false;
	}

	health_ -= damage;
	lastDamageTakenAt_ = glfwGetTime();
	return true;
}

Health::HealthType Health::get() const {
	return std::max(0, health_);
}

float Health::getNormalized() const {
	return static_cast<float>(health_) / maxHealth_;
}

Health::HealthType Health::getMaxHealth() const {
	return maxHealth_;
}

void Health::setMaxHealth(HealthType maxHealth) {
	maxHealth_ = maxHealth;
}

void Health::set(HealthType health) {
	health_ = health;
}

bool Health::isInvincible() const {
	return glfwGetTime() < lastDamageTakenAt_ + invincibleDuration_;
}

}
