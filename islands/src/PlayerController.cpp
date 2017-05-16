#include "PlayerController.h"
#include "Camera.h"
#include "InputSystem.h"

namespace islands {

PlayerController::PlayerController() : attacking_(false) {}

void PlayerController::start() {
	body_ = getEntity().getComponent<PhysicalBody>();
	drawer_ = getEntity().getComponent<ModelDrawer>();
}

void PlayerController::update() {
	Camera::getInstance().lookAt(getEntity().getPosition());

	static const float SPEED = 0.2f;
	static const auto THETA = 5 * glm::quarter_pi<float>();
	static const auto ROTATION = glm::mat2(glm::cos(THETA), glm::sin(-THETA),
		glm::sin(THETA), glm::cos(THETA)) * glm::mat2(-1, 0, 0, 1);

	glm::vec3 v = body_->getVelocity();
	const auto dir = ROTATION * InputSystem::getInstance().getDirection();
	if (dir.x != 0) {
		v.x = SPEED * dir.x;
	}
	if (dir.y != 0) {
		v.y = SPEED * dir.y;
	}
	if (InputSystem::getInstance().isCommandActive(InputSystem::Command::Jump)) {
		v.z = 0.1f;
	}
	body_->setVelocity(v);

	if (InputSystem::getInstance().isCommandActive(InputSystem::Command::Attack)) {
		attacking_ = true;
		drawer_->enableAnimation("Armature|Attack", false);
	}

	const auto u = glm::normalize(glm::vec3(v.xy, 0));
	if (glm::length(u) > glm::epsilon<float>()) {
		drawer_->enableAnimation("Walk.002", true, 24 * 3);
		attacking_ = false;

		if (glm::dot(u, glm::vec3(-1.f, 0, 0)) < 1.f - glm::epsilon<float>()) {
			getEntity().setQuaternion(glm::rotation(glm::vec3(1.f, 0, 0), u));
		} else {
			getEntity().setQuaternion(glm::angleAxis(glm::pi<float>(), glm::vec3(0, 0, 1.f)));
		}
	} else {
		if (!(attacking_ && drawer_->isPlayingAnimation())) {
			drawer_->stopAnimation();
			attacking_ = false;
		}
	}
}

}
