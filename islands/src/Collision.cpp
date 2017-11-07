#include "Collision.h"
#include "Entity.h"
#include "Camera.h"
#include "Chunk.h"

namespace islands {

Collider::Collider(std::shared_ptr<Model> model) :
	model_(model),
	isGhost_(false) {}

Collider::Collider() : Collider(nullptr) {}

void Collider::registerCallback(const Callback& callback) {
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

glm::vec3 Collider::getSinkingCorrector(std::shared_ptr<Collider> collider) const {
	const auto normal = glm::normalize(getNormal(collider->getEntity().getPosition()));
	if (glm::any(glm::isnan(normal))) {
		return glm::zero<glm::vec3>();
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
	if (std::isnan(sinking)) {
		return glm::zero<glm::vec3>();
	}

	return sinking * normal;
}

bool Collider::intersects(std::shared_ptr<Collider> collider) const {
	if (!(getEntity().getFilterMask() & collider->getEntity().getSelfMask())) {
		return false;
	}

	if (const auto a = std::dynamic_pointer_cast<AABBCollider>(collider)) {
		return intersectsImpl(a);
	} else if (const auto s = std::dynamic_pointer_cast<SphereCollider>(collider)) {
		return intersectsImpl(s);
	} else if (const auto p = std::dynamic_pointer_cast<PlaneCollider>(collider)) {
		return intersectsImpl(p);
	} else if (const auto m = std::dynamic_pointer_cast<MeshCollider>(collider)) {
		return intersectsImpl(m);
	}
	throw std::exception("unreachable");
}

std::shared_ptr<Model> Collider::getModel() const {
	return model_;
}

const geometry::AABB& Collider::getGlobalAABB() const {
	return globalAABB_;
}

void Collider::setGhost(bool isGhost) {
	isGhost_ = isGhost;
}

bool Collider::isGhost() const {
	return isGhost_;
}

bool AABBCollider::intersectsImpl(std::shared_ptr<AABBCollider> collider) const {
	return geometry::intersect(getGlobalAABB(), collider->getGlobalAABB());
}

SphereCollider::SphereCollider(std::shared_ptr<Model> model) :
	Collider(model),
	radiusFixed_(false),
	sphere_{glm::zero<glm::vec3>(), 0.f}  {}

SphereCollider::SphereCollider(std::shared_ptr<Model> model, float radius) :
	Collider(model),
	radiusFixed_(true),
	sphere_{glm::zero<glm::vec3>(), radius}  {}

void SphereCollider::update() {
	Collider::update();

	const auto& aabb = getGlobalAABB();
	sphere_.center = (aabb.max + aabb.min) / 2.f;
	if (!radiusFixed_) {
		const auto d = aabb.max - aabb.min;
		sphere_.radius = std::max(0.f, std::min({d.x, d.y, d.z}) / 2.f);
	}
}

glm::vec3 SphereCollider::getNormal(const glm::vec3& refPos) const {
	return glm::normalize(refPos - sphere_.center);
}

bool SphereCollider::intersectsImpl(std::shared_ptr<SphereCollider> collider) const {
	return geometry::intersect(sphere_, collider->getGlobalSphere());
}

float SphereCollider::getSinking(std::shared_ptr<SphereCollider> collider) const {
	return geometry::getSinking(sphere_, collider->getGlobalSphere());
}

bool SphereCollider::intersectsImpl(std::shared_ptr<PlaneCollider> collider) const {
	return geometry::intersect(sphere_, collider->getGlobalPlane());
}

float SphereCollider::getSinking(std::shared_ptr<PlaneCollider> collider) const {
	return geometry::getSinking(sphere_, collider->getGlobalPlane());
}

bool SphereCollider::intersectsImpl(std::shared_ptr<MeshCollider> collider) const {
	return geometry::intersect(collider->getCollisionMesh(), sphere_);
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

bool PlaneCollider::intersectsImpl(std::shared_ptr<SphereCollider> collider) const {
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

FloorCollider::FloorCollider(std::shared_ptr<Model> model) :
	PlaneCollider(model, glm::vec3(0, 0, 1)) {}

void FloorCollider::update() {
	Collider::update();
	globalPlane_.d = -offset_;
}

MeshCollider::MeshCollider(std::shared_ptr<Model> model) : Collider(model) {
	size_t numTriangles = 0;
	for (auto mesh : model->getMeshes()) {
		assert(mesh->numIndices_ % 3 == 0);
		numTriangles += mesh->numIndices_ / 3;

		for (size_t i = 0; i < mesh->numIndices_ / 3; ++i) {
			const geometry::Triangle triangle{
				mesh->vertices_[mesh->indices_[3 * i + 0]],
				mesh->vertices_[mesh->indices_[3 * i + 1]],
				mesh->vertices_[mesh->indices_[3 * i + 2]]
			};
			assert(!triangle.isDegenerate());
		}
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
	auto sum = glm::zero<glm::vec3>();
	for (const auto& triangle : collisionMesh_.collisionTriangles) {
		sum += triangle.getNormal();
	}
	return sum / static_cast<float>(collisionMesh_.collisionTriangles.size());
}

bool MeshCollider::intersectsImpl(std::shared_ptr<SphereCollider> collider) const {
	return geometry::intersect(collisionMesh_, collider->getGlobalSphere());
}

geometry::CollisionMesh& MeshCollider::getCollisionMesh() const {
	return collisionMesh_;
}

float MeshCollider::getSinking(std::shared_ptr<SphereCollider> collider) const {
	float sum = 0.f;
	for (const auto& triangle : collisionMesh_.collisionTriangles) {
		sum += geometry::getSinking(triangle, collider->getGlobalSphere());
	}
	return sum / static_cast<float>(collisionMesh_.collisionTriangles.size());
}

}