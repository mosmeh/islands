#include "Player.h"
#include "Camera.h"
#include "Input.h"
#include "Chunk.h"
#include "ResourceSystem.h"
#include "Health.h"

namespace islands {

Player::Player() :
	status_(State::Idling),
	attackAnimStartedAt_(INFINITY) {}

void Player::start() {
	getEntity().setScale({0.00485f, 0.00485f, 0.006525f});
	getEntity().setSelfMask(Entity::Mask::Player);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Enemy |
		Entity::Mask::EnemyAttack
	);

	constexpr auto PLAYER_MESH_NAME = "player.fbx";
	const auto model = ResourceSystem::getInstance().createOrGet<Model>(PLAYER_MESH_NAME, PLAYER_MESH_NAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model);
	drawer_->enableAnimation("Walk.002", false);
	drawer_->update();
	drawer_->stopAnimation();

	const auto collider = getChunk().getPhysics().createCollider<SphereCollider>(model);
	getEntity().attachComponent(collider);

	body_ = getChunk().getPhysics().createBody(collider);
	getEntity().attachComponent(body_);

	getEntity().createComponent<Health>(100);

	fireBall_ = getEntity().createComponent<FireBall>();
}

void Player::update() {
	Camera::getInstance().lookAt(getEntity().getPosition());

	if (getEntity().getFirstComponent<Health>()->isDead()) {
		std::exit(0);
	}

	static constexpr float SPEED = 8.f;
	static constexpr auto THETA = 5 * glm::quarter_pi<float>();
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

	const auto u = glm::normalize(glm::vec3(v.xy, 0));
	if (glm::length(u) > glm::epsilon<float>()) {
		status_ = State::Walking;
		drawer_->enableAnimation("Walk.002", true, 3.0 * 24);
	} else if (status_ == State::Walking) {
		status_ = State::Idling;
		drawer_->stopAnimation();
	}

	static constexpr float ATTACK_ANIM_SPEED = 1.5f * 24;
	switch (status_) {
	case State::Idling: {
		if (Input::getInstance().isCommandActive(Input::Command::Attack)) {
			status_ = State::AnimatingPreFire;
			attackAnimStartedAt_ = glfwGetTime();
			drawer_->enableAnimation("Armature|Attack", false, ATTACK_ANIM_SPEED);
		}
		break;
	}
	case State::Walking:
		if (glm::dot(u, glm::vec3(-1.f, 0, 0)) < 1.f - glm::epsilon<float>()) {
			getEntity().setQuaternion(glm::rotation(glm::vec3(1.f, 0, 0), u));
		} else {
			getEntity().setQuaternion(glm::angleAxis(glm::pi<float>(), glm::vec3(0, 0, 1.f)));
		}
		break;
	case State::AnimatingPreFire:
		if (glfwGetTime() > attackAnimStartedAt_ + 20.0 / ATTACK_ANIM_SPEED) {
			status_ = State::AnimatingPostFire;
			fireBall_->fire();
		}
		break;
	case State::AnimatingPostFire:
		if (glfwGetTime() > attackAnimStartedAt_ + 35.0 / ATTACK_ANIM_SPEED) {
			status_ = State::Idling;
			drawer_->stopAnimation();
		}
		break;
	}
}

}
