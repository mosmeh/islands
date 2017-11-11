#include "Enemy.h"
#include "ResourceSystem.h"
#include "Chunk.h"
#include "NameGenerator.h"
#include "Scene.h"
#include "Sound.h"

namespace islands {

Slime::Slime() :
	status_(State::Moving),
	stateChangedAt_(-HUGE_VAL) {}

void Slime::start() {
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	static constexpr auto MESH_FILENAME = "slime.dae";
	model_ = ResourceSystem::getInstance().createOrGet<Model>(MESH_FILENAME, MESH_FILENAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		if (status_ != State::Dead) {
			const auto& entity = opponent->getEntity();
			if (entity.getSelfMask() & Entity::Mask::Player) {
				entity.getFirstComponent<Health>()->takeDamage(1);
			}
			if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
				getEntity().createComponent<DamageEffect>();
			}
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	health_ = getEntity().createComponent<Health>(3);

	playerEntity_ = getChunk().getEntityByName("Player");
}

void Slime::update() {
	static constexpr float SPEED = 2.f;
	static constexpr double MOVE_DURATION_FACTOR = 5.0;
	static constexpr double CHANGE_DIR_DURATION = 1.0;

	if (status_ != State::Dead && health_->isDead()) {
		status_ = State::Dead;
		if (getEntity().hasComponent<DamageEffect>()) {
			getEntity().getFirstComponent<DamageEffect>()->destroy();
		}
		getEntity().createComponent<ScatterEffect>([this] {
			getEntity().destroy();
		});
		ResourceSystem::getInstance().get<Sound>("EnemyDieSound")->createInstance()->play();
		return;
	}

	const auto delta = glfwGetTime() - stateChangedAt_;
	switch (status_) {
	case State::Moving: {
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

			stateChangedAt_ = glfwGetTime();
			initPos_ = getEntity().getPosition();
			status_ = State::ChangingDirection;
		} else {
			const auto factor = static_cast<float>(std::max(0.0, (1.0 - std::cos(MOVE_DURATION_FACTOR * delta)) / 2.0));
			body_->setVelocity(SPEED * factor * direction_);
			drawer_->enableAnimation("", false, 3.0);
		}
		break;
	}
	case State::ChangingDirection:
		if (delta < CHANGE_DIR_DURATION) {
			const auto factor = static_cast<float>(delta / CHANGE_DIR_DURATION / glm::two_pi<double>());
			getEntity().setQuaternion(glm::slerp(getEntity().getQuaternion(), targetQuat_, factor));
			const auto d = std::fmod(delta, CHANGE_DIR_DURATION / 2);
			getEntity().setPosition(initPos_ - glm::vec3(0, 0, 10.f * d * (d - CHANGE_DIR_DURATION / 2)));
		} else {
			stateChangedAt_ = glfwGetTime();
			status_ = State::Moving;
		}
		break;
	}
}

BigSlime::BigSlime() :
	status_(State::Pausing),
	stateChangedAt_(-HUGE_VAL) {}

void BigSlime::start() {
	getEntity().setScale(0.8f * glm::one<glm::vec3>());
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	static constexpr auto MESH_FILENAME = "big_slime.dae";
	model_ = ResourceSystem::getInstance().createOrGet<Model>(MESH_FILENAME, MESH_FILENAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);

	const auto collider = getEntity().createComponent<SphereCollider>(model_, 1.3f);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		if (status_ != State::Dead) {
			const auto& entity = opponent->getEntity();
			if (entity.getSelfMask() & Entity::Mask::Player) {
				entity.getFirstComponent<Health>()->takeDamage(1);
			}
			if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
				getEntity().createComponent<DamageEffect>();
			}
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	health_ = getEntity().createComponent<Health>(5);

	playerEntity_ = getChunk().getEntityByName("Player");

	drawer_->enableAnimation("", true, 1.0);
}

void BigSlime::update() {
	if (status_ != State::Dead && health_->isDead()) {
		status_ = State::Dead;
		if (getEntity().hasComponent<DamageEffect>()) {
			getEntity().getFirstComponent<DamageEffect>()->destroy();
		}
		getEntity().createComponent<ScatterEffect>([this] {
			getEntity().destroy();
		});
		ResourceSystem::getInstance().get<Sound>("EnemyDieSound")->createInstance()->play();
		return;
	}

	const auto delta = glfwGetTime() - stateChangedAt_;
	switch (status_) {
	case State::Pausing:
		if (delta > 0.5) {
			drawer_->enableAnimation("", true, 1.0);

			const auto diff = playerEntity_->getPosition().xy() - getEntity().getPosition().xy();
			const auto dir = glm::vec3(glm::normalize(diff), 0);
			getEntity().setQuaternion(geometry::directionToQuaternion(dir, {0, -1.f, 0}));
			static constexpr auto SPEED = 5.f;
			body_->setVelocity(SPEED * dir + glm::vec3(0, 0, 15.f));

			status_ = State::Jumping;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	case State::Jumping:
		if (std::abs(body_->getVelocity().z) < glm::epsilon<float>()) {
			ResourceSystem::getInstance().createOrGet<Sound>("SlimeJumpSound", "slime_jump.ogg")->createInstance()->play();
			drawer_->stopAnimation();

			status_ = State::Pausing;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	}
}

Rabbit::Rabbit() :
	status_(State::Pausing),
	stateChangedAt_(-HUGE_VAL) {}

void Rabbit::start() {
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	static constexpr auto MESH_FILENAME = "rabbit.dae";
	model_ = ResourceSystem::getInstance().createOrGet<Model>(MESH_FILENAME, MESH_FILENAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		if (status_ != State::Dead) {
			const auto& entity = opponent->getEntity();
			if (entity.getSelfMask() & Entity::Mask::Player) {
				entity.getFirstComponent<Health>()->takeDamage(1);
			}
			if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
				getEntity().createComponent<DamageEffect>();
			}
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	health_ = getEntity().createComponent<Health>(3);

	playerEntity_ = getChunk().getEntityByName("Player");
}

void Rabbit::update() {
	if (status_ != State::Dead && health_->isDead()) {
		status_ = State::Dead;
		if (getEntity().hasComponent<DamageEffect>()) {
			getEntity().getFirstComponent<DamageEffect>()->destroy();
		}
		getEntity().createComponent<ScatterEffect>([this] {
			getEntity().destroy();
		});
		ResourceSystem::getInstance().get<Sound>("EnemyDieSound")->createInstance()->play();
		return;
	}

	static constexpr auto JUMP_ANIM_START_TIME = 190;

	const auto delta = glfwGetTime() - stateChangedAt_;
	switch (status_) {
	case State::Jumping: {
		if (!drawer_->isPlayingAnimation()) {
			status_ = State::Pausing;
			stateChangedAt_ = glfwGetTime();
		} else {
			static constexpr auto SPEED = 5.f;
			body_->setVelocity(SPEED * direction_);
		}
		break;
	}
	case State::Pausing: {
		static constexpr auto PAUSE_DURATION = 0.4;
		if (delta > PAUSE_DURATION) {
			static constexpr auto ATTACK_RADIUS = 5.f;
			if (glm::distance(getEntity().getPosition(), playerEntity_->getPosition()) < ATTACK_RADIUS) {
				status_ = State::PreAttack;
				drawer_->enableAnimation("", false, 6.0);
			} else {
				status_ = State::Jumping;
				drawer_->enableAnimation("", false, 3.0, JUMP_ANIM_START_TIME);
				direction_ = glm::normalize(
					glm::vec3((playerEntity_->getPosition() - getEntity().getPosition()).xy(), 0));
				getEntity().setQuaternion(geometry::directionToQuaternion(direction_, {0, -1.f, 0}));
			}
			stateChangedAt_ = glfwGetTime();
		}
		break;
	}
	case State::PreAttack:
		if (drawer_->getCurrentAnimationFrame() >= 40) {
			status_ = State::Attacking;
			attackEntity_ = getChunk().createEntity(
				NameGenerator::generate("RabbitAttack"));
			attackEntity_->setSelfMask(Entity::Mask::EnemyAttack);
			attackEntity_->setFilterMask(Entity::Mask::Player);
			const auto right = glm::cross(direction_, glm::vec3(0, 0, 1.f));
			attackEntity_->setPosition(getEntity().getPosition()
				+ 2.f * glm::normalize(direction_ + right));
			const auto collider = attackEntity_->createComponent<SphereCollider>(3.f);
			collider->setGhost(true);
			collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
				attackEntity_->destroy();
				opponent->getEntity().getFirstComponent<Health>()->takeDamage(1);
			});
			ResourceSystem::getInstance().createOrGet<Sound>("RabbitAttackSound", "rabbit_attack.ogg")->createInstance()->play();
		}
		break;
	case State::Attacking:
		if (drawer_->getCurrentAnimationFrame() >= JUMP_ANIM_START_TIME) {
			attackEntity_->destroy();
			drawer_->stopAnimation();

			status_ = State::Pausing;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	}
}

Crab::Crab() :
	status_(State::Pausing),
	stateChangedAt_(-HUGE_VAL) {}

void Crab::start() {
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	static constexpr auto MESH_FILENAME = "crab.dae";
	model_ = ResourceSystem::getInstance().createOrGet<Model>(MESH_FILENAME, MESH_FILENAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		if (status_ != State::Dead) {
			const auto& entity = opponent->getEntity();
			if (entity.getSelfMask() & Entity::Mask::Player) {
				entity.getFirstComponent<Health>()->takeDamage(1);
			}
			if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
				getEntity().createComponent<DamageEffect>();
			}
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);

	health_ = getEntity().createComponent<Health>(3);

	playerEntity_ = getChunk().getEntityByName("Player");
}

void Crab::update() {
	if (status_ != State::Dead && health_->isDead()) {
		status_ = State::Dead;
		if (getEntity().hasComponent<DamageEffect>()) {
			getEntity().getFirstComponent<DamageEffect>()->destroy();
		}
		getEntity().createComponent<ScatterEffect>([this] {
			getEntity().destroy();
		});
		ResourceSystem::getInstance().get<Sound>("EnemyDieSound")->createInstance()->play();
		return;
	}

	static const auto ATTACK_ANIM_START = 50;
	const auto delta = glfwGetTime() - stateChangedAt_;
	switch (status_) {
	case State::Pausing: {
		drawer_->stopAnimation();
		static constexpr auto PAUSE_DURATION = 0.2;
		if (delta > PAUSE_DURATION) {
			static constexpr auto ATTACK_RADIUS = 5.f;
			if (glm::distance(getEntity().getPosition(), playerEntity_->getPosition()) < ATTACK_RADIUS) {
				status_ = State::PreAttack;
				drawer_->enableAnimation("", false, 1.0, ATTACK_ANIM_START);
				getEntity().setQuaternion(geometry::directionToQuaternion(direction_, {0, -1.f, 0}));
			} else {
				status_ = State::Moving;
				drawer_->enableAnimation("", false, 3.0);
				direction_ = glm::normalize(
					glm::vec3((playerEntity_->getPosition() - getEntity().getPosition()).xy(), 0));
				getEntity().setQuaternion(geometry::directionToQuaternion(direction_, {1.f, 0, 0}));
			}
			stateChangedAt_ = glfwGetTime();
		}
		break;
	}
	case State::Moving: {
		if (drawer_->getCurrentAnimationFrame() >= ATTACK_ANIM_START) {
			status_ = State::Pausing;
			stateChangedAt_ = glfwGetTime();
		} else {
			static constexpr auto SPEED = 3.f;
			const auto factor = static_cast<float>(std::max(0.0, (1.0 - std::cos(glm::two_pi<double>() * delta * 24.0 * SPEED / ATTACK_ANIM_START)) / 2.0));
			body_->setVelocity(SPEED * factor * direction_);
		}
		break;
	}
	case State::PreAttack:
		if (drawer_->getCurrentAnimationFrame() >= 60) {
			attackEntity_ = getChunk().createEntity(
				NameGenerator::generate("CrabAttack"));
			attackEntity_->setSelfMask(Entity::Mask::EnemyAttack);
			attackEntity_->setFilterMask(Entity::Mask::Player);
			attackEntity_->setPosition(getEntity().getPosition() + 2.f * direction_);
			const auto collider = attackEntity_->createComponent<SphereCollider>(3.f);
			collider->setGhost(true);
			collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
				attackEntity_->destroy();
				opponent->getEntity().getFirstComponent<Health>()->takeDamage(1);
			});

			status_ = State::Attacking;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	case State::Attacking:
		if (drawer_->getCurrentAnimationFrame() >= 75) {
			attackEntity_->destroy();
			drawer_->stopAnimation();

			status_ = State::Pausing;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	}
}

Dragon::Dragon() :
	status_(State::Hovering),
	stateChangedAt_(glfwGetTime()) {

	std::random_device randomDevice;
	engine_ = std::mt19937(randomDevice());
}

void Dragon::start() {
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StageObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	static constexpr auto MESH_FILENAME = "dragon.dae";
	model_ = ResourceSystem::getInstance().createOrGet<Model>(MESH_FILENAME, MESH_FILENAME);
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);
	drawer_->setCullFaceEnabled(false);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		if (status_ != State::Dead) {
			const auto& entity = opponent->getEntity();
			if (entity.getSelfMask() & Entity::Mask::Player) {
				entity.getFirstComponent<Health>()->takeDamage(1);
			}
			if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
				getEntity().createComponent<DamageEffect>();
			}
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	body_->setReceiveGravity(false);

	health_ = getEntity().createComponent<Health>(15);

	playerEntity_ = getChunk().getEntityByName("Player");

	drawer_->enableAnimation("", true, 1.0);
}

void Dragon::update() {
	if (status_ != State::Dead && health_->isDead()) {
		status_ = State::Dead;
		if (getEntity().hasComponent<DamageEffect>()) {
			getEntity().getFirstComponent<DamageEffect>()->destroy();
		}
		getEntity().createComponent<ScatterEffect>([this] {
			getEntity().destroy();
			SceneManager::getInstance().changeScene<GameClearScene>(false);
		});
		ResourceSystem::getInstance().get<Sound>("EnemyDieSound")->createInstance()->play();
		return;
	}

	static const float HILL_HEIGHT = 8.f;
	static const float INIT_HEIGHT = 10.f;

	const auto animSpeed = (health_->get() > 5) ? 1.0 : 0.7;

	const auto& playerPos = playerEntity_->getPosition();
	const double playerIsOnHill = playerPos.z > HILL_HEIGHT;
	const auto& pos = getEntity().getPosition();

	switch (status_) {
	case State::Hovering:
	case State::Moving:
	case State::Ascending:
	case State::Descending:
		if (drawer_->getCurrentAnimationFrame() >= 30) {
			drawer_->stopAnimation();
			drawer_->enableAnimation("", false, animSpeed);
		}
		break;
	}

	const auto delta = glfwGetTime() - stateChangedAt_;
	switch (status_) {
	case State::Hovering:
		if (delta > 2.0) {
			std::bernoulli_distribution bernoulli;
			if (bernoulli(engine_)) {
				static const auto MARGIN = 5.f;
				static const auto OFFSET = 10.f;

				const auto& aabb = getChunk().getGlobalAABB();
				glm::vec2 targetPos;
				if (std::bernoulli_distribution()(engine_)) {
					std::uniform_real_distribution<float> dist(aabb.min.x + MARGIN, aabb.max.x - MARGIN);
					targetPos = {dist(engine_), aabb.max.y + OFFSET};
				} else {
					std::uniform_real_distribution<float> dist(aabb.min.y + MARGIN, aabb.max.y - MARGIN);
					targetPos = {aabb.max.x + OFFSET, dist(engine_)};
				}

				static constexpr auto SPEED = 10.f;

				direction_ = glm::vec3(glm::normalize(targetPos - pos.xy()), 0);
				body_->setVelocity(SPEED * direction_);
				getEntity().setQuaternion(geometry::directionToQuaternion(direction_, {0, -1.f, 0}));
				targetDelta_ = glm::distance(targetPos, pos.xy()) / SPEED;

				status_ = State::Moving;
			} else {
				body_->setVelocity(glm::vec3(0, 0, -1));
				targetDelta_ = std::abs(pos.z - HILL_HEIGHT);

				status_ = State::Descending;
			}
			stateChangedAt_ = glfwGetTime();
		} else {
			lookAtPlayer();
		}
		break;
	case State::Moving:
		if (delta >= targetDelta_) {
			body_->setVelocity(glm::zero<glm::vec3>());
			lookAtPlayer();

			status_ = State::PreFire;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	case State::PreFire:
		if (drawer_->getCurrentAnimationFrame() > 50) {
			attackEntity_ = getChunk().createEntity(
				NameGenerator::generate("DragonFire"));
			attackEntity_->setSelfMask(Entity::Mask::EnemyAttack);
			attackEntity_->setFilterMask(Entity::Mask::Player | Entity::Mask::StageObject);
			const auto origin = pos + glm::vec3(8.f * direction_.xy(), -1.f);
			attackEntity_->setPosition(origin);

			constexpr auto BALL_MODEL = "fire_ball.obj";
			attackEntity_->createComponent<ModelDrawer>(
				ResourceSystem::getInstance().createOrGet<Model>(BALL_MODEL, BALL_MODEL));

			const auto collider = attackEntity_->createComponent<SphereCollider>(1.f);
			collider->setGhost(true);
			collider->registerCallback([this] (std::shared_ptr<Collider> opponent) {
				const auto& entity = opponent->getEntity();
				if (entity.getSelfMask() & Entity::Mask::Player) {
					entity.getFirstComponent<Health>()->takeDamage(1);
				}

				attackEntity_->destroy();
			});

			const auto body = attackEntity_->createComponent<PhysicalBody>(collider);
			body->setReceiveGravity(false);
			const auto theta = std::atan2(std::abs(playerPos.z - origin.z), glm::distance(playerPos.xy(), origin.xy()));
			const auto velocity = direction_ - glm::vec3(0, 0, std::tan(theta));
			body->setVelocity(15.f * velocity);
			attackEntity_->setQuaternion(geometry::directionToQuaternion(glm::normalize(velocity), {0, -1.f, 0}));

			ResourceSystem::getInstance().createOrGet<Sound>(
				"DragonFireSound", "dragon_fire.ogg")->createInstance()->play();

			status_ = State::PostFire;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	case State::PostFire:
		if (drawer_->getCurrentAnimationFrame() > 60) {
			status_ = State::Hovering;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	case State::Descending:
		if (delta >= targetDelta_) {
			body_->setVelocity(glm::zero<glm::vec3>());
			lookAtPlayer();
			static constexpr auto GLIDE_FRAME = 35;
			drawer_->enableAnimation("", false, 0.1, GLIDE_FRAME);

			static constexpr float TACKLE_SPEED = 25.f;
			body_->setVelocity(TACKLE_SPEED * direction_);
			targetDelta_ = 2.0f * glm::distance(playerPos, pos) / TACKLE_SPEED;

			status_ = State::Tackling;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	case State::Tackling:
		drawer_->stopAnimation();
		if (delta >= targetDelta_) {
			body_->setVelocity(glm::vec3(0, 0, 1.f));
			drawer_->enableAnimation("", false, animSpeed);
			targetDelta_ = glm::abs(pos.z - INIT_HEIGHT);

			status_ = State::Ascending;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	case State::Ascending:
		if (delta >= targetDelta_) {
			body_->setVelocity(glm::zero<glm::vec3>());
			lookAtPlayer();

			status_ = State::Hovering;
			stateChangedAt_ = glfwGetTime();
		}
		break;
	}
}

void Dragon::lookAtPlayer() {
	direction_ = glm::vec3(glm::normalize(playerEntity_->getPosition().xy() - getEntity().getPosition().xy()), 0);
	getEntity().setQuaternion(geometry::directionToQuaternion(direction_, {0, -1.f, 0}));
}

TotemPoll::TotemPoll() : activated_(false) {}

void TotemPoll::start() {
	getEntity().setSelfMask(Entity::Mask::StageObject);
	getEntity().setFilterMask(
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	playerEntity_ = getChunk().getEntityByName("Player");
}

void TotemPoll::update() {
	if (!activated_) {
		static constexpr auto RADIUS = 6.f;
		if (glm::distance(getEntity().getPosition().xy(), playerEntity_->getPosition().xy()) < RADIUS) {
			activated_ = true;

			const auto health = playerEntity_->getFirstComponent<Health>();
			health->setMaxHealth(15);
			health->set(15);

			ResourceSystem::getInstance().createOrGet<Sound>(
				"CureSound", "cure.ogg")->createInstance()->play();
		}
	}
}

}