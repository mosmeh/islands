#include "Enemy.h"
#include "Chunk.h"
#include "NameGenerator.h"
#include "Scene.h"
#include "Sound.h"

namespace islands {
namespace enemy {

void Slime::start() {
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	model_ = Model::createOrGet("slime.dae");
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);
	drawer_->setCullFaceEnabled(false);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		const auto& entity = opponent->getEntity();
		if (entity.getSelfMask() & Entity::Mask::Player) {
			entity.getFirstComponent<Health>()->takeDamage(1);
		}
		if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
			getEntity().createComponent<DamageEffect>();
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	health_ = getEntity().createComponent<Health>(3);

	machine_.changeState<Moving>();
}

void Slime::update() {
	machine_.update(*this);
}

void Slime::Moving::update(Slime& parent) {
	static constexpr float SPEED = 2.f;
	static constexpr double MOVE_DURATION_FACTOR = 5.0;

	if (parent.health_->isDead()) {
		changeState<Dead<Slime>>();
	} else if (getElapsed() > glm::two_pi<double>() / MOVE_DURATION_FACTOR) {
		changeState<Turning>();
	} else {
		const auto factor = static_cast<float>(std::max(0.0, (1.0 - std::cos(MOVE_DURATION_FACTOR * getElapsed())) / 2.0));
		parent.body_->setVelocity(SPEED * factor * parent.direction_);
		parent.drawer_->enableAnimation("", false, 3.0);
	}
}

void Slime::Turning::start(Slime& parent) {
	const auto& playerPos = parent.getChunk().getEntityByName("Player")->getPosition();
	parent.direction_ = glm::normalize(playerPos - parent.getEntity().getPosition());
	parent.direction_.z = 0.f;

	if (glm::dot(parent.direction_, glm::vec3(0, 1.f, 0)) < 1.f - glm::epsilon<float>()) {
		targetQuat_ = glm::rotation(glm::vec3(0, -1.f, 0), parent.direction_);
	} else {
		targetQuat_ = glm::angleAxis(glm::pi<float>(), glm::vec3(0, 0, 1.f));
	}
	initPos_ = parent.getEntity().getPosition();

	parent.drawer_->stopAnimation();
}

void Slime::Turning::update(Slime& parent) {
	static constexpr double CHANGE_DIR_DURATION = 1.0;

	if (parent.health_->isDead()) {
		changeState<Dead<Slime>>();
	} else if (getElapsed() < CHANGE_DIR_DURATION) {
		const auto factor = static_cast<float>(getElapsed() / CHANGE_DIR_DURATION / glm::two_pi<double>());
		parent.getEntity().setQuaternion(glm::slerp(parent.getEntity().getQuaternion(), targetQuat_, factor));
		const auto d = std::fmod(getElapsed(), CHANGE_DIR_DURATION / 2);
		parent.getEntity().setPosition(initPos_ - glm::vec3(0, 0, 10.f * d * (d - CHANGE_DIR_DURATION / 2)));
	} else {
		changeState<Moving>();
	}
}

void BigSlime::start() {
	getEntity().setScale(0.8f * glm::one<glm::vec3>());
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	model_ = Model::createOrGet("big_slime.dae");
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);
	drawer_->setCullFaceEnabled(false);

	const auto collider = getEntity().createComponent<SphereCollider>(model_, 1.3f);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		const auto& entity = opponent->getEntity();
		if (entity.getSelfMask() & Entity::Mask::Player) {
			entity.getFirstComponent<Health>()->takeDamage(1);
		}
		if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
			getEntity().createComponent<DamageEffect>();
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	health_ = getEntity().createComponent<Health>(5);

	machine_.changeState<Pausing>();
	drawer_->enableAnimation("", true, 1.0);
}

void BigSlime::update() {
	machine_.update(*this);
}

void BigSlime::Pausing::update(BigSlime& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead<BigSlime>>();
	} else if (getElapsed() > 0.5) {
		changeState<Jumping>();
	}
}

void BigSlime::Jumping::start(BigSlime& parent) {
	static constexpr auto SPEED = 5.f;

	parent.drawer_->enableAnimation("", true, 1.0);

	auto& entity = parent.getEntity();
	const auto& playerPos = parent.getChunk().getEntityByName("Player")->getPosition().xy();
	const auto diff = playerPos - entity.getPosition().xy();
	const auto dir = glm::vec3(glm::normalize(diff), 0);
	entity.setQuaternion(geometry::directionToQuaternion(dir, {0, -1.f, 0}));
	parent.body_->setVelocity(SPEED * dir + glm::vec3(0, 0, 15.f));
}

void BigSlime::Jumping::update(BigSlime& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead<BigSlime>>();
	} else if (std::abs(parent.body_->getVelocity().z) < glm::epsilon<float>()) {
		Sound::createOrGet("slime_jump.ogg")->createInstance()->play();
		parent.drawer_->stopAnimation();
		changeState<Pausing>();
	}
}

void Rabbit::start() {
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	model_ = Model::createOrGet("rabbit.dae");
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		const auto& entity = opponent->getEntity();
		if (entity.getSelfMask() & Entity::Mask::Player) {
			entity.getFirstComponent<Health>()->takeDamage(1);
		}
		if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
			getEntity().createComponent<DamageEffect>();
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	health_ = getEntity().createComponent<Health>(3);

	machine_.changeState<Pausing>();
}

void Rabbit::update() {
	machine_.update(*this);
}

void Rabbit::Jumping::start(Rabbit& parent) {
	const auto& pos = parent.getEntity().getPosition();
	const auto& playerPos = parent.getChunk().getEntityByName("Player")->getPosition();

	parent.drawer_->enableAnimation("", false, 3.0, JUMP_ANIM_START_TIME);
	parent.direction_ = glm::normalize(glm::vec3((playerPos - pos).xy(), 0));
	parent.getEntity().setQuaternion(geometry::directionToQuaternion(parent.direction_, {0, -1.f, 0}));
}

void Rabbit::Jumping::update(Rabbit& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead<Rabbit>>();
	} else if (parent.drawer_->isPlayingAnimation()) {
		static constexpr auto SPEED = 5.f;
		parent.body_->setVelocity(SPEED * parent.direction_);
	} else {
		changeState<Pausing>();
	}
}

void Rabbit::Pausing::start(Rabbit& parent) {
	parent.drawer_->stopAnimation();
}

void Rabbit::Pausing::update(Rabbit& parent) {
	static constexpr auto PAUSE_DURATION = 0.4;

	if (parent.health_->isDead()) {
		changeState<Dead<Rabbit>>();
	} else if (getElapsed() > PAUSE_DURATION) {
		static constexpr auto ATTACK_RADIUS = 5.f;

		const auto& pos = parent.getEntity().getPosition();
		const auto& playerPos = parent.getChunk().getEntityByName("Player")->getPosition();

		if (glm::distance(pos, playerPos) < ATTACK_RADIUS) {
			changeState<PreAttack>();
		} else {
			changeState<Jumping>();
		}
	}
}

void Rabbit::PreAttack::start(Rabbit& parent) {
	parent.drawer_->enableAnimation("", false, 6.0);
}

void Rabbit::PreAttack::update(Rabbit& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead<Rabbit>>();
	} else if (parent.drawer_->getCurrentAnimationFrame() >= 40) {
		changeState<Attacking>();
	}
}

void Rabbit::Attacking::start(Rabbit& parent) {
	attackEntity_ = parent.getChunk().createEntity(
		NameGenerator::generate("RabbitAttack"));
	attackEntity_->setSelfMask(Entity::Mask::EnemyAttack);
	attackEntity_->setFilterMask(Entity::Mask::Player);
	const auto right = glm::cross(parent.direction_, glm::vec3(0, 0, 1.f));
	attackEntity_->setPosition(parent.getEntity().getPosition()
		+ 2.f * glm::normalize(parent.direction_ + right));
	const auto collider = attackEntity_->createComponent<SphereCollider>(3.f);
	collider->setGhost(true);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		opponent->getEntity().getFirstComponent<Health>()->takeDamage(1);
		attackEntity_->destroy();
	});
	Sound::createOrGet("rabbit_attack.ogg")->createInstance()->play();
}

void Rabbit::Attacking::update(Rabbit& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead<Rabbit>>();
	} else if (parent.drawer_->getCurrentAnimationFrame() >= JUMP_ANIM_START_TIME) {
		attackEntity_->destroy();
		changeState<Pausing>();
	}
}

void Crab::start() {
	getEntity().setSelfMask(Entity::Mask::Enemy);
	getEntity().setFilterMask(
		Entity::Mask::StaticObject |
		Entity::Mask::Player |
		Entity::Mask::PlayerAttack
	);

	model_ = Model::createOrGet("crab.dae");
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		const auto& entity = opponent->getEntity();
		if (entity.getSelfMask() & Entity::Mask::Player) {
			entity.getFirstComponent<Health>()->takeDamage(1);
		}
		if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
			getEntity().createComponent<DamageEffect>();
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	health_ = getEntity().createComponent<Health>(3);

	machine_.changeState<Pausing>();
}

void Crab::update() {
	machine_.update(*this);
}

void Crab::Pausing::start(Crab& parent) {
	parent.drawer_->stopAnimation();
}

void Crab::Pausing::update(Crab& parent) {
	static constexpr auto PAUSE_DURATION = 0.2;

	if (parent.health_->isDead()) {
		changeState<Dead<Crab>>();
	} else if (getElapsed() > PAUSE_DURATION) {
		static constexpr auto ATTACK_RADIUS = 5.f;

		const auto& pos = parent.getEntity().getPosition();
		const auto& playerPos = parent.getChunk().getEntityByName("Player")->getPosition();
		if (glm::distance(pos, playerPos) < ATTACK_RADIUS) {
			changeState<PreAttack>();
		} else {
			changeState<Moving>();
		}
	}
}

void Crab::Moving::start(Crab& parent) {
	parent.drawer_->enableAnimation("", false, 3.0);

	const auto& pos = parent.getEntity().getPosition();
	const auto& playerPos = parent.getChunk().getEntityByName("Player")->getPosition();

	parent.direction_ = glm::normalize(
		glm::vec3((playerPos - pos).xy(), 0));
	parent.getEntity().setQuaternion(geometry::directionToQuaternion(parent.direction_, {1.f, 0, 0}));
}

void Crab::Moving::update(Crab& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead<Crab>>();
	} else if (parent.drawer_->getCurrentAnimationFrame() >= ATTACK_ANIM_START) {
		changeState<Pausing>();
	} else {
		static constexpr auto SPEED = 3.f;
		const auto factor = static_cast<float>(std::max(0.0, (1.0 - std::cos(glm::two_pi<double>() * getElapsed() * 24.0 * SPEED / ATTACK_ANIM_START)) / 2.0));
		parent.body_->setVelocity(SPEED * factor * parent.direction_);
	}
}

void Crab::PreAttack::start(Crab& parent) {
	parent.drawer_->enableAnimation("", false, 1.0, ATTACK_ANIM_START);
	parent.getEntity().setQuaternion(geometry::directionToQuaternion(parent.direction_, {0, -1.f, 0}));
}

void Crab::PreAttack::update(Crab& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead<Crab>>();
	} else if (parent.drawer_->getCurrentAnimationFrame() >= 60) {
		changeState<Attacking>();
	}
}

void Crab::Attacking::start(Crab& parent) {
	attackEntity_ = parent.getChunk().createEntity(
		NameGenerator::generate("CrabAttack"));
	attackEntity_->setSelfMask(Entity::Mask::EnemyAttack);
	attackEntity_->setFilterMask(Entity::Mask::Player);
	attackEntity_->setPosition(parent.getEntity().getPosition() + 2.f * parent.direction_);
	const auto collider = attackEntity_->createComponent<SphereCollider>(3.f);
	collider->setGhost(true);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		attackEntity_->destroy();
		opponent->getEntity().getFirstComponent<Health>()->takeDamage(1);
	});
}

void Crab::Attacking::update(Crab& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead<Crab>>();
	} else if (parent.drawer_->getCurrentAnimationFrame() >= 75) {
		attackEntity_->destroy();
		changeState<Pausing>();
	}
}

Dragon::Dragon() {
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

	model_ = Model::createOrGet("dragon.dae");
	drawer_ = getEntity().createComponent<ModelDrawer>(model_);
	drawer_->setCullFaceEnabled(false);

	const auto collider = getEntity().createComponent<SphereCollider>(model_);
	collider->registerCallback([this](std::shared_ptr<Collider> opponent) {
		const auto& entity = opponent->getEntity();
		if (entity.getSelfMask() & Entity::Mask::Player) {
			entity.getFirstComponent<Health>()->takeDamage(1);
		}
		if (entity.getSelfMask() & Entity::Mask::PlayerAttack) {
			getEntity().createComponent<DamageEffect>();
		}
	});

	body_ = getEntity().createComponent<PhysicalBody>(collider);
	body_->setReceiveGravity(false);

	health_ = getEntity().createComponent<Health>(15);

	drawer_->enableAnimation("", true, 1.0);
	machine_.changeState<Hovering>();
}

void Dragon::update() {
	machine_.update(*this);
}

void Dragon::Hovering::start(Dragon& parent) {
	parent.body_->setVelocity(glm::zero<glm::vec3>());
	parent.lookAtPlayer();
}

void Dragon::Hovering::update(Dragon& parent) {
	parent.loopHoveringAnimation();

	if (parent.health_->isDead()) {
		changeState<Dead>();
	} else if (getElapsed() > 2.0) {
		if (std::bernoulli_distribution()(parent.engine_)) {
			changeState<Moving>();
		} else {
			changeState<Descending>();
		}
	} else {
		parent.lookAtPlayer();
	}
}

void Dragon::Moving::start(Dragon& parent) {
	static const auto MARGIN = 5.f;
	static const auto OFFSET = 10.f;

	const auto& aabb = parent.getChunk().getGlobalAABB();
	glm::vec2 targetPos;
	if (std::bernoulli_distribution()(parent.engine_)) {
		std::uniform_real_distribution<float> dist(aabb.min.x + MARGIN, aabb.max.x - MARGIN);
		targetPos = {dist(parent.engine_), aabb.max.y + OFFSET};
	} else {
		std::uniform_real_distribution<float> dist(aabb.min.y + MARGIN, aabb.max.y - MARGIN);
		targetPos = {aabb.max.x + OFFSET, dist(parent.engine_)};
	}

	static constexpr auto SPEED = 10.f;

	const auto& pos = parent.getEntity().getPosition();

	parent.direction_ = glm::vec3(glm::normalize(targetPos - pos.xy()), 0);
	parent.body_->setVelocity(SPEED * parent.direction_);
	parent.getEntity().setQuaternion(geometry::directionToQuaternion(parent.direction_, {0, -1.f, 0}));
	targetDelta_ = glm::distance(targetPos, pos.xy()) / SPEED;
}

void Dragon::Moving::update(Dragon& parent) {
	parent.loopHoveringAnimation();

	if (parent.health_->isDead()) {
		changeState<Dead>();
	} else if (getElapsed() >= targetDelta_) {
		changeState<PreFire>();
	}
}

void Dragon::PreFire::start(Dragon& parent) {
	parent.body_->setVelocity(glm::zero<glm::vec3>());
	parent.lookAtPlayer();
}

void Dragon::PreFire::update(Dragon& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead>();
	} else if (parent.drawer_->getCurrentAnimationFrame() > 50) {
		changeState<PostFire>();
	}
}

void Dragon::PostFire::start(Dragon& parent) {
	const auto& pos = parent.getEntity().getPosition();
	const auto& playerPos = parent.getChunk().getEntityByName("Player")->getPosition();

	const auto attackEntity = parent.getChunk().createEntity(
		NameGenerator::generate("DragonFire"));
	attackEntity->setSelfMask(Entity::Mask::EnemyAttack);
	attackEntity->setFilterMask(Entity::Mask::Player | Entity::Mask::StageObject);
	const auto origin = pos + glm::vec3(8.f * parent.direction_.xy(), -1.f);
	attackEntity->setPosition(origin);

	attackEntity->createComponent<ModelDrawer>(Model::createOrGet("fire_ball.obj"));

	const auto collider = attackEntity->createComponent<SphereCollider>(1.f);
	collider->setGhost(true);
	collider->registerCallback([this, attackEntity](std::shared_ptr<Collider> opponent) {
		const auto& entity = opponent->getEntity();
		if (entity.getSelfMask() & Entity::Mask::Player) {
			entity.getFirstComponent<Health>()->takeDamage(1);
		}

		attackEntity->destroy();
	});

	const auto body = attackEntity->createComponent<PhysicalBody>(collider);
	body->setReceiveGravity(false);
	const auto theta = std::atan2(std::abs(playerPos.z - origin.z), glm::distance(playerPos.xy(), origin.xy()));
	const auto velocity = parent.direction_ - glm::vec3(0, 0, std::tan(theta));
	body->setVelocity(15.f * velocity);
	attackEntity->setQuaternion(geometry::directionToQuaternion(glm::normalize(velocity), {0, -1.f, 0}));

	Sound::createOrGet("dragon_fire.ogg")->createInstance()->play();
}

void Dragon::PostFire::update(Dragon& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead>();
	} else if (parent.drawer_->getCurrentAnimationFrame() > 60) {
		changeState<Hovering>();
	}
}

void Dragon::Descending::start(Dragon& parent) {
	static const float HILL_HEIGHT = 8.f;

	parent.body_->setVelocity(glm::vec3(0, 0, -1));
	targetDelta_ = std::abs(parent.getEntity().getPosition().z - HILL_HEIGHT);
}

void Dragon::Descending::update(Dragon& parent) {
	parent.loopHoveringAnimation();

	if (parent.health_->isDead()) {
		changeState<Dead>();
	} else if (getElapsed() >= targetDelta_) {
		changeState<Tackling>();
	}
}

void Dragon::Tackling::start(Dragon& parent) {
	parent.body_->setVelocity(glm::zero<glm::vec3>());
	parent.lookAtPlayer();
	static constexpr auto GLIDE_FRAME = 35;
	parent.drawer_->enableAnimation("", false, 0.1, GLIDE_FRAME);

	static constexpr float TACKLE_SPEED = 25.f;
	parent.body_->setVelocity(TACKLE_SPEED * parent.direction_);
	parent.body_->setGhost(true);

	const auto& pos = parent.getEntity().getPosition();
	const auto& playerPos = parent.getChunk().getEntityByName("Player")->getPosition();
	targetDelta_ = 2.0f * glm::distance(playerPos, pos) / TACKLE_SPEED;
}

void Dragon::Tackling::update(Dragon& parent) {
	parent.drawer_->stopAnimation();

	if (parent.health_->isDead()) {
		changeState<Dead>();
	} else if (getElapsed() >= targetDelta_) {
		changeState<Ascending>();
	}
}

void Dragon::Ascending::start(Dragon& parent) {
	parent.body_->setGhost(false);
	parent.body_->setVelocity(glm::vec3(0, 0, 1.f));

	const auto animSpeed = (parent.health_->get() > 5) ? 1.0 : 0.7;
	parent.drawer_->enableAnimation("", false, animSpeed);

	static const float INIT_HEIGHT = 10.f;
	targetDelta_ = glm::abs(parent.getEntity().getPosition().z - INIT_HEIGHT);
}

void Dragon::Ascending::update(Dragon& parent) {
	if (parent.health_->isDead()) {
		changeState<Dead>();
	} else if (getElapsed() >= targetDelta_) {
		changeState<Hovering>();
	}
}

void Dragon::Dead::start(Dragon& parent) {
	auto& entity = parent.getEntity();
	if (entity.hasComponent<Collider>()) {
		entity.getFirstComponent<Collider>()->clearCallbacks();
	}
	if (entity.hasComponent<DamageEffect>()) {
		entity.getFirstComponent<DamageEffect>()->destroy();
	}
	entity.createComponent<ScatterEffect>([&entity] {
		entity.destroy();
		SceneManager::getInstance().changeScene<GameClearScene>(false);
	});
	Sound::get("enemy_die.ogg")->createInstance()->play();
}

void Dragon::lookAtPlayer() {
	const auto& pos = getEntity().getPosition();
	const auto& playerPos = getChunk().getEntityByName("Player")->getPosition();
	direction_ = glm::vec3(glm::normalize(playerPos.xy() - pos.xy()), 0);
	getEntity().setQuaternion(geometry::directionToQuaternion(direction_, {0, -1.f, 0}));
}

void Dragon::loopHoveringAnimation() {
	if (!drawer_->isPlayingAnimation() || drawer_->getCurrentAnimationFrame() >= 30) {
		drawer_->stopAnimation();

		const auto animSpeed = (health_->get() > 5) ? 1.0 : 0.7;
		drawer_->enableAnimation("", false, animSpeed);
	}
}

}
}
