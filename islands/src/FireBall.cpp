#include "FireBall.h"
#include "ResourceSystem.h"
#include "Chunk.h"
#include "Health.h"

namespace islands {

FireBall::FireBall() : flying_(false) {}

void FireBall::start() {
	entity_ = getChunk().createEntity("Ball");
	entity_->setScale(glm::vec3(0.9f));

	constexpr auto BALL_MODEL = "sphere.obj";
	const auto model = ResourceSystem::getInstance().createOrGet<Model>(BALL_MODEL, BALL_MODEL);
	drawer_ = entity_->createComponent<ModelDrawer>(model);
	drawer_->setVisible(false);

	const auto collider = getChunk().getPhysics().createCollider<SphereCollider>(model);
	collider->setSelfMask(Collider::Mask::PlayerAttack);
	collider->setFilterMask(
		Collider::Mask::StaticObject |
		Collider::Mask::Enemy
	);
	collider->registerCallback([this] (Collider::MaskType mask, std::shared_ptr<Collider> collider) {
		if (flying_) {
			flying_ = false;
			drawer_->setVisible(false);

			if (mask & Collider::Mask::Enemy) {
				collider->getEntity().getFirstComponent<Health>()->takeDamage(10);
			}
		}
	});
	entity_->attachComponent(collider);

	body_ = getChunk().getPhysics().createBody(collider);
	body_->setReceiveGravity(false);
	entity_->attachComponent(body_);
}

void FireBall::update() {}

void FireBall::fire() {
	flying_ = true;
	const auto orientationVec = getEntity().getQuaternion() * glm::vec3(1.f, 0, 0);
	const auto ballDir = glm::vec3(orientationVec.xy, 0);
	entity_->setPosition(getEntity().getPosition() + 1.3f * ballDir + glm::vec3(0, 0, 2.f));
	drawer_->setVisible(true);
	body_->setVelocity(12.f * ballDir);
}

}