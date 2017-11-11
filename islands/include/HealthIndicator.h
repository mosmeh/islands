#pragma once

#include "Sprite.h"
#include "Health.h"

namespace islands {

class HealthIndicator {
public:
	HealthIndicator();
	virtual ~HealthIndicator() = default;

	void draw(std::shared_ptr<Health> health);

private:
	Sprite filledHeart_, emptyHeart_;
};

}
