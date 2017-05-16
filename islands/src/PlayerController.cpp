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

	const float speed = 0.2f;
	glm::vec3 v = body_->getVelocity();
	if (InputSystem::getInstance().isKeyPressed(GLFW_KEY_LEFT)) {
		v.x = -speed;
	} else if (InputSystem::getInstance().isKeyPressed(GLFW_KEY_RIGHT)) {
		v.x = speed;
	}
	if (InputSystem::getInstance().isKeyPressed(GLFW_KEY_UP)) {
		v.y = speed;
	} else if (InputSystem::getInstance().isKeyPressed(GLFW_KEY_DOWN)) {
		v.y = -speed;
	}
	if (InputSystem::getInstance().isKeyPressed(GLFW_KEY_X)) {
		v.z = 0.1f;
	}
	body_->setVelocity(v);

	if (InputSystem::getInstance().isKeyPressed(GLFW_KEY_Z)) {
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
