#include "SpecialObjects.h"
#include "Chunk.h"
#include "Health.h"

namespace islands {
namespace specialobj {

Curer::Curer(float radius) :
	radius_(radius),
	activated_(false) {}

void Curer::start() {
	getEntity().setSelfMask(Entity::Mask::StageObject);
	getEntity().setFilterMask(
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	playerEntity_ = getChunk().getEntityByName("Player");
}

void Curer::update() {
	if (!activated_) {
		if (glm::distance(getEntity().getPosition().xy(), playerEntity_->getPosition().xy()) < radius_) {
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
