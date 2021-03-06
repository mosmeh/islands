#include "Player.h"
#include "Camera.h"
#include "Input.h"
#include "Chunk.h"
#include "Scene.h"
#include "Sound.h"
#include "FireBall.h"
#include "Effect.h"
#include "NameGenerator.h"

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

	const auto model = Model::createOrGet("player.fbx");
	drawer_ = getEntity().createComponent<ModelDrawer>(model);
	drawer_->enableAnimation("Walk.002", false);
	drawer_->update();
	drawer_->stopAnimation();

	const auto material = std::make_shared<Material>();
	material->setTexture(Texture2D::createOrGet("player-ao.png"));
	material->setFragmentShader(Shader::createOrGet("ao.frag", Shader::Type::Fragment));
	drawer_->pushMaterial(material);

	const auto collider = getEntity().createComponent<SphereCollider>(model, 0.7f);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		const auto& entity = opponent->getEntity();
		if (entity.getSelfMask() & (Entity::Mask::Enemy | Entity::Mask::EnemyAttack)) {
			if (!health_->isInvincible()) {
				if (entity.hasComponent<Health>()) {
					if (entity.getFirstComponent<Health>()->isDead()) {
						return;
					}
				}
				getEntity().createComponent<effect::Damage>(2.0);
				Sound::createOrGet("player_damage.ogg")->createInstance()->play();
			}
		}
	});
	body_ = getEntity().createComponent<PhysicalBody>(collider);

	health_ = getEntity().createComponent<Health>(10, 2.0);
}

void Player::update() {
	Camera::getInstance().lookAt(getEntity().getPosition());

	if (status_ == State::Dead) {
		return;
	} else if (health_->isDead()) {
		status_ = State::Dead;
		if (getEntity().hasComponent<effect::Damage>()) {
			getEntity().getFirstComponent<effect::Damage>()->destroy();
		}
		getEntity().createComponent<effect::Scatter>([this] {
			getEntity().destroy();
			SceneManager::getInstance().changeScene<GameOverScene>(false);
		});
		return;
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
			getChunk().createEntity(
				NameGenerator::generate("FireBall"))->createComponent<FireBall>(
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
