#include "PlayerController.h"
#include "Camera.h"
#include "Input.h"
#include "Chunk.h"
#include "ResourceSystem.h"
#include "NameGenerator.h"

namespace islands {

PlayerController::PlayerController() : attacking_(false) {}

void PlayerController::start() {
	body_ = getEntity().getFirstComponent<PhysicalBody>();
	drawer_ = getEntity().getFirstComponent<ModelDrawer>();
	drawer_->enableAnimation("Walk.002", false);
	drawer_->update();

	ball_ = std::make_shared<Entity>("ball");
	ball_->setScale(glm::vec3(0.9f));

	const auto model = ResourceSystem::getInstance().createOrGet<Model>("ball", "sphere.obj");
	const auto drawer = std::make_shared<ModelDrawer>(model);
	drawer->setVisible(false);
	ball_->attachComponent(drawer);

	ballBody_ = std::make_shared<PhysicalBody>();
	const auto collider = std::make_shared<SphereCollider>(model);
	collider->registerCallback([this] (std::shared_ptr<Collider>) {
		ball_->getFirstComponent<ModelDrawer>()->setVisible(false);
	});
	ballBody_->setCollider(collider);

	ball_->attachComponent(collider);

	ballBody_->setReceiveGravity(false);
	ball_->attachComponent(ballBody_);
}

void PlayerController::update() {
	Camera::getInstance().lookAt(getEntity().getPosition());

	static const float SPEED = 8.f;
	static const auto THETA = 5 * glm::quarter_pi<float>();
	static const auto ROTATION = glm::mat2(glm::cos(THETA), glm::sin(-THETA),
		glm::sin(THETA), glm::cos(THETA)) * glm::mat2(-1, 0, 0, 1);

	glm::vec3 v = body_->getVelocity();
	const auto dir = ROTATION * Input::getInstance().getDirection();
	if (dir.x != 0) {
		v.x = SPEED * dir.x;
	}
	if (dir.y != 0) {
		v.y = SPEED * dir.y;
	}
	if (std::abs(v.z) < glm::epsilon<float>() &&
		Input::getInstance().isCommandActive(Input::Command::Jump)) {

		v.z = 10.f;
	}
	body_->setVelocity(v);

	static auto w = glm::vec3(1.f, 0, 0);
	static auto start = INFINITY;
	static bool playing = false;
	if (Input::getInstance().isCommandActive(Input::Command::Attack)) {
		attacking_ = true;
		start = static_cast<float>(glfwGetTime());
		playing = true;
		drawer_->enableAnimation("Armature|Attack", false);
	}

	const auto u = glm::normalize(glm::vec3(v.xy, 0));
	if (glm::length(u) > glm::epsilon<float>()) {
		w = u;
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

	if (attacking_ && glfwGetTime() > start + 60.f / 72 && playing) {
		playing = false;
		start = INFINITY;
		ball_->setPosition(getEntity().getPosition() + 1.3f * w + glm::vec3(0, 0, 2.f));
		ball_->getFirstComponent<ModelDrawer>()->setVisible(true);
		ballBody_->setVelocity(12.f * w);
	}
}

void PlayerController::updateParentChunk() {
	getChunk().addEntity(ball_);
	auto& ps = getChunk().getPhysicsSystem();
	ps.registerBody(ballBody_);
	ps.registerBody(getEntity().getFirstComponent<PhysicalBody>());
	ps.registerCollider(getEntity().getFirstComponent<Collider>());
}

}
