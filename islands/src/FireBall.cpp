#include "FireBall.h"
#include "Chunk.h"
#include "Health.h"

namespace islands {

FireBall::FireBall(const glm::vec3& pos, const glm::quat& quat) :
	pos_(pos),
	quat_(quat) {}

void FireBall::start() {
	getEntity().setScale(glm::vec3(0.9f));
	getEntity().setSelfMask(Entity::Mask::PlayerAttack);
	getEntity().setFilterMask(
		Entity::Mask::StageObject |
		Entity::Mask::Enemy
	);

	const auto model = Model::createOrGet("fire_ball.obj");
	getEntity().createComponent<ModelDrawer>(model);

	const auto collider = getEntity().createComponent<SphereCollider>(model, 2.f);
	collider->registerCallback([this] (std::shared_ptr<Collider> collider) {
		const auto& entity = collider->getEntity();
		if (entity.getSelfMask() & Entity::Mask::Enemy) {
			entity.getFirstComponent<Health>()->takeDamage(1);
		}
		getEntity().destroy();
	});

	const auto body = getEntity().createComponent<PhysicalBody>(collider);
	body->setReceiveGravity(false);

	const auto orientationVec = quat_ * glm::vec3(1.f, 0, 0);
	const auto ballDir = glm::vec3(orientationVec.xy, 0);
	getEntity().setPosition(pos_ + 1.3f * ballDir + glm::vec3(0, 0, 2.f));
	body->setVelocity(12.f * ballDir);
	getEntity().setQuaternion(geometry::directionToQuaternion(ballDir, {0, -1.f, 0}));
}

void FireBall::update() {}

}
