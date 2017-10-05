#include "Collision.h"
#include "Entity.h"
#include "Camera.h"
#include "Chunk.h"

namespace islands {

Collider::Collider(std::shared_ptr<Model> model) : model_(model) {}

void Collider::registerCallback(std::function<void(std::shared_ptr<Collider>)> callback) {
	callbacks_.emplace_back(callback);
}

void Collider::notifyCollision(std::shared_ptr<Collider> opponent) const {
	for (const auto callback : callbacks_) {
		callback(opponent);
	}
}

void Collider::update() {
	const auto& localAABB = getModel()->getLocalAABB();
	const auto& modelMat = getEntity().getModelMatrix();

	globalAABB_.min = glm::vec3(INFINITY);
	globalAABB_.max = glm::vec3(-INFINITY);
	for (const auto& a : {localAABB.min, localAABB.max}) {
		for (const auto& b : {localAABB.min, localAABB.max}) {
			for (const auto& c : {localAABB.min, localAABB.max}) {
				const auto worldPos = (modelMat * glm::vec4(a.x, b.y, c.z, 1)).xyz();
				globalAABB_.min = glm::min(globalAABB_.min, worldPos);
				globalAABB_.max = glm::max(globalAABB_.max, worldPos);
			}
		}
	}
}

glm::vec3 Collider::getSinkingCorrectionVector(std::shared_ptr<Collider> collider) const {
	const auto normal = glm::normalize(getNormal(collider->getEntity().getPosition()));
	if (const auto a = std::dynamic_pointer_cast<AABBCollider>(collider)) {
		return normal * getSinking(a);
	} else if (const auto s = std::dynamic_pointer_cast<SphereCollider>(collider)) {
		return normal * getSinking(s);
	} else if (const auto p = std::dynamic_pointer_cast<PlaneCollider>(collider)) {
		return normal * getSinking(p);
	}
	throw std::exception("unreachable");
}

bool Collider::intersects(std::shared_ptr<Collider> collider) const {
	if (const auto a = std::dynamic_pointer_cast<AABBCollider>(collider)) {
		return intersects(a);
	} else if (const auto s = std::dynamic_pointer_cast<SphereCollider>(collider)) {
		return intersects(s);
	} else if (const auto p = std::dynamic_pointer_cast<PlaneCollider>(collider)) {
		return intersects(p);
	}
	throw std::exception("unreachable");
}

std::shared_ptr<Model> Collider::getModel() const {
	return model_;
}

const geometry::AABB& Collider::getGlobalAABB() const {
	return globalAABB_;
}

bool AABBCollider::intersects(std::shared_ptr<AABBCollider> collider) const {
	return geometry::intersect(getGlobalAABB(), collider->getGlobalAABB());
}

void SphereCollider::update() {
	Collider::update();

	const auto& aabb = getGlobalAABB();
	const auto d = aabb.max - aabb.min;
	sphere_.radius = std::min({d.x, d.y, d.z}) / 2.f;
	sphere_.center = (aabb.max + aabb.min) / 2.f;
}

glm::vec3 SphereCollider::getNormal(const glm::vec3& refPos) const {
	return glm::normalize(refPos - sphere_.center);
}

bool SphereCollider::intersects(std::shared_ptr<SphereCollider> collider) const {
	return geometry::intersect(sphere_, collider->getGlobalSphere());
}

float SphereCollider::getSinking(std::shared_ptr<SphereCollider> collider) const {
	return geometry::getSinking(sphere_, collider->getGlobalSphere());
}

bool SphereCollider::intersects(std::shared_ptr<PlaneCollider> collider) const {
	return geometry::intersect(sphere_, collider->getGlobalPlane());
}

float SphereCollider::getSinking(std::shared_ptr<PlaneCollider> collider) const {
	return geometry::getSinking(sphere_, collider->getGlobalPlane());
}

const geometry::Sphere& SphereCollider::getGlobalSphere() const {
	return sphere_;
}

PlaneCollider::PlaneCollider(std::shared_ptr<Model> model, const glm::vec3& normal) :
	Collider(model),
	globalPlane_({normal, 0}),
	offset_(0.f) {}

void PlaneCollider::update() {
	Collider::update();

	const auto& aabb = getGlobalAABB();
	const auto center = (aabb.max - aabb.min) / 2.f;
	globalPlane_.d = glm::dot(globalPlane_.normal, center) - offset_;
}

glm::vec3 PlaneCollider::getNormal(const glm::vec3&) const {
	return globalPlane_.normal;
}

bool PlaneCollider::intersects(std::shared_ptr<SphereCollider> collider) const {
	return geometry::intersect(collider->getGlobalSphere(), globalPlane_);
}

const geometry::Plane& PlaneCollider::getGlobalPlane() const {
	return globalPlane_;
}

void PlaneCollider::setOffset(float offset) {
	offset_ = offset;
}

float PlaneCollider::getSinking(std::shared_ptr<SphereCollider> collider) const {
	return geometry::getSinking(collider->getGlobalSphere(), globalPlane_);
}

}