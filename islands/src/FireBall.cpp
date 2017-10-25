#include "FireBall.h"
#include "ResourceSystem.h"
#include "Chunk.h"

namespace islands {

void FireBall::start() {
	entity_ = getChunk().createEntity("Ball");
	entity_->setScale(glm::vec3(0.9f));

	constexpr auto BALL_MODEL = "sphere.obj";
	const auto model = ResourceSystem::getInstance().createOrGet<Model>(BALL_MODEL, BALL_MODEL);
	drawer_ = entity_->createComponent<ModelDrawer>(model);
	drawer_->setVisible(false);

	const auto collider = std::make_shared<SphereCollider>(model);
	collider->setSelfMask(Collider::Mask::PlayerAttack);
	collider->setFilterMask(
		Collider::Mask::StaticObject |
		Collider::Mask::Enemy
	);
	collider->registerCallback([this] (Collider::MaskType, std::shared_ptr<Collider>) {
		entity_->getFirstComponent<ModelDrawer>()->setVisible(false);
	});
	entity_->attachComponent(collider);

	body_ = getChunk().getPhysics().createBody(collider);
	body_->setReceiveGravity(false);
	entity_->attachComponent(body_);
}

void FireBall::update() {}

void FireBall::fire() const {
	const auto u = glm::normalize(glm::vec3(body_->getVelocity().xy, 0));

	entity_->setPosition(entity_->getPosition() + 1.3f * u + glm::vec3(0, 0, 2.f));
	drawer_->setVisible(true);
	body_->setVelocity(12.f * u);
}

}