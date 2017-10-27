#include "Enemy.h"
#include "ResourceSystem.h"
#include "Chunk.h"

namespace islands {

Slime::Slime() : status_(State::Moving) {}

void Slime::start() {
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	static constexpr auto MESH_FILENAME = "slime.dae";
	const auto model = ResourceSystem::getInstance().createOrGet<Model>(MESH_FILENAME, MESH_FILENAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		if (status_ != State::Dead) {
			const auto& entity = opponent->getEntity();
			if (entity.getSelfMask() & Entity::Mask::Player) {
				entity.getFirstComponent<Health>()->takeDamage(1);
			}
			if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
				drawer_->setVisible(false);
				damageEffect_->activate();
			}
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	health_ = getEntity().createComponent<Health>(30);

	playerEntity_ = getChunk().getEntityByName("Player");
}

void Slime::update() {
	static constexpr float SPEED = 2.f;
	static constexpr double MOVE_DURATION_FACTOR = 5.0;
	static constexpr double CHANGE_DIR_DURATION = 1.0;

	if (health_->isDead()) {
		status_ = State::Dying;
	}

	const auto delta = glfwGetTime() - moveStartedAt_;
	switch (status_) {
	case State::Moving: {
		const auto factor = static_cast<float>(std::max(0.0, (1.0 - std::cos(MOVE_DURATION_FACTOR * delta)) / 2.0));
		if (delta > glm::two_pi<double>() / MOVE_DURATION_FACTOR) {
			const auto& playerPos = playerEntity_->getPosition();
			direction_ = glm::normalize(playerPos - getEntity().getPosition());
			direction_.z = 0.f;

			if (glm::dot(direction_, glm::vec3(0, 1.f, 0)) < 1.f - glm::epsilon<float>()) {
				targetQuat_ = glm::rotation(glm::vec3(0, -1.f, 0), direction_);
			} else {
				targetQuat_ = glm::angleAxis(glm::pi<float>(), glm::vec3(0, 0, 1.f));
			}

			drawer_->stopAnimation();

			moveStartedAt_ = glfwGetTime();
			initPos_ = getEntity().getPosition();
			status_ = State::ChangingDirection;
		} else {
			body_->setVelocity(SPEED * factor * direction_);
			drawer_->enableAnimation("", false, 3.f);
		}
		break;
	}
	case State::ChangingDirection: {
 		if (delta <= CHANGE_DIR_DURATION) {
			const auto factor = static_cast<float>(delta / CHANGE_DIR_DURATION / glm::two_pi<double>());
			getEntity().setQuaternion(glm::slerp(getEntity().getQuaternion(), targetQuat_, factor));
			const auto d = std::fmod(delta, CHANGE_DIR_DURATION / 2);
			getEntity().setPosition(initPos_ - glm::vec3(0, 0, 10.f * d * (d - CHANGE_DIR_DURATION / 2)));
		} else {
			moveStartedAt_ = glfwGetTime();
			status_ = State::Moving;
		}
		break;
	case State::Dying: {
		// TODO
		status_ = State::Moving;
		break;
	}
	}
	}
}

}