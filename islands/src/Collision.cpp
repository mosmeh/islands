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
	globalAABB_ = getModel()->getLocalAABB().transform(getEntity().getModelMatrix());
}

glm::vec3 Collider::getSinkingCorrectionVector(std::shared_ptr<Collider> collider) const {
	auto normal = glm::normalize(getNormal(collider->getEntity().getPosition()));
	if (glm::any(glm::isnan(normal))) {
		normal = glm::zero<glm::vec3>();
	}
	float sinking;
	if (const auto a = std::dynamic_pointer_cast<AABBCollider>(collider)) {
		sinking = getSinking(a);
	} else if (const auto s = std::dynamic_pointer_cast<SphereCollider>(collider)) {
		sinking = getSinking(s);
	} else if (const auto p = std::dynamic_pointer_cast<PlaneCollider>(collider)) {
		sinking = getSinking(p);
	} else if (const auto m = std::dynamic_pointer_cast<MeshCollider>(collider)) {
		sinking = getSinking(m);
	} else {
		throw std::exception("unreachable");
	}
	return sinking * normal;
}

bool Collider::intersects(std::shared_ptr<Collider> collider) const {
	if (const auto a = std::dynamic_pointer_cast<AABBCollider>(collider)) {
		return intersects(a);
	} else if (const auto s = std::dynamic_pointer_cast<SphereCollider>(collider)) {
		return intersects(s);
	} else if (const auto p = std::dynamic_pointer_cast<PlaneCollider>(collider)) {
		return intersects(p);
	} else if (const auto m = std::dynamic_pointer_cast<MeshCollider>(collider)) {
		return intersects(m);
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

SphereCollider::SphereCollider(std::shared_ptr<Model> model) :
	Collider(model),
	sphere_{glm::zero<glm::vec3>(), 0.f}  {}

void SphereCollider::update() {
	Collider::update();

	const auto& aabb = getGlobalAABB();
	const auto d = aabb.max - aabb.min;
	sphere_.radius = std::max(0.f, std::min({d.x, d.y, d.z}) / 2.f);
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

bool SphereCollider::intersects(std::shared_ptr<MeshCollider> collider) const {
	return geometry::intersect(collider->getCollisionMesh(), sphere_);
}

float SphereCollider::getSinking(std::shared_ptr<MeshCollider> collider) const {
	return geometry::getSinking(collider->getCollisionMesh().nearestTriangle, sphere_);
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

MeshCollider::MeshCollider(std::shared_ptr<Model> model) : Collider(model) {
	size_t numTriangles = 0;
	for (auto mesh : model->getMeshes()) {
		assert(mesh->numIndices_ % 3 == 0);
		numTriangles += mesh->numIndices_ / 3;
	}
	collisionMesh_.triangles.reserve(numTriangles);
}

void MeshCollider::update() {
	Collider::update();

	const auto& modelMat = getEntity().getModelMatrix();
	collisionMesh_.triangles.clear();
	for (auto mesh : getModel()->getMeshes()) {
		for (size_t i = 0; i < mesh->numIndices_ / 3; ++i) {
			const geometry::Triangle triangle{
				mesh->vertices_[mesh->indices_[3 * i + 0]],
				mesh->vertices_[mesh->indices_[3 * i + 1]],
				mesh->vertices_[mesh->indices_[3 * i + 2]]
			};
			collisionMesh_.triangles.emplace_back(triangle.transform(modelMat));
		}
	}
}

glm::vec3 MeshCollider::getNormal(const glm::vec3&) const {
	return collisionMesh_.nearestTriangle.getNormal();
}

bool MeshCollider::intersects(std::shared_ptr<SphereCollider> collider) const {
	return geometry::intersect(collisionMesh_, collider->getGlobalSphere());
}

geometry::CollisionMesh& MeshCollider::getCollisionMesh() const {
	return collisionMesh_;
}

float MeshCollider::getSinking(std::shared_ptr<SphereCollider> collider) const {
	return geometry::getSinking(collisionMesh_.nearestTriangle, collider->getGlobalSphere());
}

}