#include "SpecialObjects.h"
#include "Chunk.h"
#include "Health.h"

namespace islands {
namespace specialobj {

TotemPoll::TotemPoll() : activated_(false) {}

void TotemPoll::start() {
	getEntity().setSelfMask(Entity::Mask::StageObject);
	getEntity().setFilterMask(
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	playerEntity_ = getChunk().getEntityByName("Player");
}

void TotemPoll::update() {
	if (!activated_) {
		static constexpr auto RADIUS = 6.f;
		if (glm::distance(getEntity().getPosition().xy(), playerEntity_->getPosition().xy()) < RADIUS) {
			activated_ = true;

			const auto health = playerEntity_->getFirstComponent<Health>();
			health->setMaxHealth(15);
			health->set(15);

			Sound::createOrGet("cure.ogg")->createInstance()->play();
		}
	}
}

}
}
