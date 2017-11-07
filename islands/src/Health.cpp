#include "Health.h"

namespace islands {

Health::Health(HealthType maxHealth) :
	health_(maxHealth),
	maxHealth_(maxHealth) {}

void Health::update() {}

bool Health::isDead() const {
	return health_ <= 0;
}

void Health::takeDamage(HealthType damage) {
	health_ -= damage;
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

}
