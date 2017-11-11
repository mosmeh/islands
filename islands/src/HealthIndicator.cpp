#include "HealthIndicator.h"
#include "ResourceSystem.h"

namespace islands {

HealthIndicator::HealthIndicator() :
	filledHeart_(ResourceSystem::getInstance().createOrGet<Texture2D>(
		"FilledHeartImage", "heart_filled.png")),
	emptyHeart_(ResourceSystem::getInstance().createOrGet<Texture2D>(
		"EmptyHeartImage", "heart_empty.png")) {

	filledHeart_.setSize({0.04, 0.065});
	emptyHeart_.setSize({0.04, 0.065});
}

void HealthIndicator::draw(std::shared_ptr<Health> health) {
	static const glm::vec2 MARGIN(0.04, 0.05);
	static const float STRIDE = 0.042f;

	float xPos = 0;
	for (Health::HealthType i = 0; i < health->getMaxHealth(); ++i) {
		auto& heart = (i < health->get()) ? filledHeart_ : emptyHeart_;
		heart.setPosition(glm::vec2(xPos, 0) + MARGIN);
		heart.draw();

		xPos += STRIDE;
	}
}

}
