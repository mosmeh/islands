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

float Health::getNormalized() const {
	return static_cast<float>(health_) / maxHealth_;
}

}