#include "PlayerController.h"
#include "Camera.h"
#include "InputSystem.h"
#include "Chunk.h"
#include "ResourceSystem.h"
#include "NameGenerator.h"

namespace islands {

PlayerController::PlayerController() : attacking_(false) {}

void PlayerController::start() {
	body_ = getEntity().getFirstComponent<PhysicalBody>();
	drawer_ = getEntity().getFirstComponent<ModelDrawer>();

	ball = std::make_shared<Entity>("ball");
	ball->setScale(glm::vec3(0.9f));

	const auto model = ResourceSystem::getInstance().createOrGet<Model>("ball", "sphere.obj");
	const auto drawer = std::make_shared<ModelDrawer>(model);
	drawer->setVisible(false);
	ball->attachComponent(drawer);

	body = std::make_shared<PhysicalBody>();
	const auto collider = std::make_shared<SphereCollider>(model, 0.f);
	collider->registerCallback([this] {
		ball->getComponent<ModelDrawer>()->setVisible(false);
	});
	body->setCollider(collider);

	ball->attachComponent(collider);

	body->setReceiveGravity(false);
	getChunk().getPhysicsSystem().registerBody(body);
	ball->attachComponent(body);

	getChunk().addEntity(ball);
}

void PlayerController::update() {
	Camera::getInstance().lookAt(getEntity().getPosition());

	static const float SPEED = 12.f;
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
