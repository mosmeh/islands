#include "Player.h"
#include "Camera.h"
#include "Input.h"
#include "Chunk.h"
#include "ResourceSystem.h"
#include "Scene.h"
#include "Sound.h"

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
	model_ = ResourceSystem::getInstance().createOrGet<Model>(PLAYER_MESH_NAME, PLAYER_MESH_NAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);
	drawer_->enableAnimation("Walk.002", false);
	drawer_->update();
	drawer_->stopAnimation();

	const auto collider = getEntity().createComponent<SphereCollider>(model_, 0.7f);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		const auto& entity = opponent->getEntity();
		if (entity.getSelfMask() & (Entity::Mask::Enemy | Entity::Mask::EnemyAttack)) {
			if (!health_->isInvincible()) {
				drawer_->setVisible(false);
				damageEffect_->activate();
				ResourceSystem::getInstance().createOrGet<Sound>("PlayerDamageSound", "player_damage.ogg")->createInstance()->play();
			}
		}
	});
	body_ = getEntity().createComponent<PhysicalBody>(collider);

	health_ = getEntity().createComponent<Health>(10, 2.0);
	damageEffect_ = getEntity().createComponent<DamageEffect>(model_, 2.0);
}

void Player::update() {
	Camera::getInstance().lookAt(getEntity().getPosition());

	if (status_ == State::Dead) {
		if (dyingEffect_->isFinished()) {
			SceneManager::getInstance().changeScene(SceneKey::GameOver, false);
		}
		return;
	} else if (health_->isDead()) {
		status_ = State::Dead;
		drawer_->destroy();
		damageEffect_->destroy();
		dyingEffect_ = getEntity().createComponent<ScatterEffect>(model_);
		return;
	}

	if (!damageEffect_->isActive()) {
		drawer_->setVisible(true);
	}

	static constexpr float SPEED = 8.f;
	static constexpr auto THETA = 5 * glm::quarter_pi<float>();
	static const auto ROTATION = glm::mat2(glm::cos(THETA), glm::sin(-THETA),
		glm::sin(THETA), glm::cos(THETA)) * glm::mat2(-1, 0, 0, 1);

	const auto dir = ROTATION * Input::getInstance().getDirection();
	glm::vec3 velocity(SPEED * dir, body_->getVelocity().z);

	if (std::abs(velocity.z) < glm::epsilon<float>() &&
		Input::getInstance().isCommandActive(Input::Command::Jump)) {

		velocity.z = 10.f;
	}

	body_->setVelocity(velocity);

	const auto u = glm::normalize(glm::vec3(velocity.xy, 0));
	if (glm::length(u) > glm::epsilon<float>()) {
		status_ = State::Walking;
		drawer_->enableAnimation("Walk.002", true, 3.0 * 24);
	} else if (status_ == State::Walking) {
		status_ = State::Idling;
		drawer_->stopAnimation();
	}

	static constexpr float ATTACK_ANIM_SPEED = 1.5 * 24;
	switch (status_) {
	case State::Idling: {
		if (Input::getInstance().isCommandActive(Input::Command::Attack)) {
			status_ = State::PreFire;
			attackAnimStartedAt_ = glfwGetTime();
			drawer_->enableAnimation("Armature|Attack", false, ATTACK_ANIM_SPEED);
		}
		break;
	}
	case State::Walking:
		getEntity().setQuaternion(geometry::directionToQuaternion(u, {1.f, 0, 0}));
		break;
	case State::PreFire:
		if (glfwGetTime() > attackAnimStartedAt_ + 20.0 / ATTACK_ANIM_SPEED) {
			status_ = State::PostFire;
			getChunk().createEntity("FireBall")->createComponent<FireBall>(
				getEntity().getPosition(), getEntity().getQuaternion());
		}
		break;
	case State::PostFire:
		if (glfwGetTime() > attackAnimStartedAt_ + 35.0 / ATTACK_ANIM_SPEED) {
			status_ = State::Idling;
			drawer_->stopAnimation();
		}
		break;
	}
}

}
