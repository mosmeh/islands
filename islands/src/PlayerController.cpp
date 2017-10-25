#include "PlayerController.h"
#include "Camera.h"
#include "Input.h"
#include "Chunk.h"
#include "ResourceSystem.h"
#include "Health.h"

namespace islands {

PlayerController::PlayerController() : attacking_(false) {}

void PlayerController::start() {
	getEntity().setScale({0.00485f, 0.00485f, 0.006525f});

	constexpr auto PLAYER_MESH_NAME = "player.fbx";
	const auto model = ResourceSystem::getInstance().createOrGet<Model>(PLAYER_MESH_NAME, PLAYER_MESH_NAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model);
	drawer_->enableAnimation("Walk.002", false);
	drawer_->update();

	const auto collider = getChunk().getPhysics().createCollider<SphereCollider>(model);
	collider->setSelfMask(Collider::Mask::Player);
	collider->setFilterMask(
		Collider::Mask::StaticObject |
		Collider::Mask::Enemy |
		Collider::Mask::EnemyAttack
	);
	getEntity().attachComponent(collider);

	body_ = getChunk().getPhysics().createBody(collider);
	getEntity().attachComponent(body_);

	getEntity().createComponent<Health>(100);

	fireBall_ = getEntity().createComponent<FireBall>();
}

void PlayerController::update() {
	Camera::getInstance().lookAt(getEntity().getPosition());

	if (getEntity().getFirstComponent<Health>()->isDead()) {
		std::exit(0);
	}

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

		fireBall_->fire();
	}
}

}
