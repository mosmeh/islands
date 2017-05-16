#include "Collision.h"
#include "Entity.h"
#include "Camera.h"
#include "Chunk.h"

namespace islands {

void BoundingBox::expand(const glm::vec3& vertex) {
	min = glm::min(min, vertex);
	max = glm::max(max, vertex);
}

BoundingBox BoundingBox::calculateAABB(const glm::mat4& model) const {
	BoundingBox aabb;

	for (const auto& a : {min, max}) {
		for (const auto& b : {min, max}) {
			for (const auto& c : {min, max}) {
				aabb.expand((model * glm::vec4(a.x, b.y, c.z, 1)).xyz());
			}
		}
	}

	return aabb;
}

bool BoundingBox::intersects(const BoundingBox& box) const {
	return glm::all(glm::greaterThan(max, box.min)) && glm::all(glm::lessThan(min, box.max));
}

bool Sphere::triangleIntersects(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) const {
	// see http://realtimecollisiondetection.net/blog/?p=103
	const auto a = v0 - center;
	const auto b = v1 - center;
	const auto c = v2 - center;
	const auto v = glm::cross(b - a, c - a);
	const auto d = glm::dot(a, v);
	if (glm::dot(v, center) < d) {
		return false;
	}
	const auto e = glm::dot(v, v);
	const auto rr = radius * radius;
	if (d * d > rr * e) {
		return false;
	}

	const auto aa = glm::dot(a, a);
	const auto ab = glm::dot(a, b);
	const auto ac = glm::dot(a, c);
	const auto bb = glm::dot(b, b);
	const auto bc = glm::dot(b, c);
	const auto cc = glm::dot(c, c);
	if (((aa > rr) & (ab > aa) & (ac > aa)) ||
		((bb > rr) & (ab > bb) & (bc > bb)) ||
		((cc > rr) & (ac > cc) & (bc > cc))) {
		return false;
	}

	const auto AB = b - a;
	const auto BC = c - b;
	const auto CA = a - c;
	const auto d1 = ab - aa;
	const auto d2 = bc - bb;
	const auto d3 = ac - cc;
	const auto e1 = glm::dot(AB, AB);
	const auto e2 = glm::dot(BC, BC);
	const auto e3 = glm::dot(CA, CA);
	const auto q1 = a * e1 - d1 * AB;
	const auto q2 = b * e2 - d2 * BC;
	const auto q3 = c * e3 - d3 * CA;
	const auto qc = c * e1 - q1;
	const auto qa = a * e2 - q2;
	const auto qb = b * e3 - q3;

	return !(((glm::dot(q1, q1) > rr * e1 * e1) & (glm::dot(q1, qc) > 0)) ||
		((glm::dot(q2, q2) > rr * e2 * e2) & (glm::dot(q2, qa) > 0)) ||
		((glm::dot(q3, q3) > rr * e3 * e3) & (glm::dot(q3, qb) > 0)));
}

Collider::Collider(std::shared_ptr<Model> model) :
	model_(model),
	boundingBoxConstructed_(false) {

#ifdef SHOW_BOX
	program_ = std::make_shared<Program>("boxProg", "box.vert", "box.frag");
	program_->loadAsync();
	glGenVertexArrays(1, &vertexArray_);
	glBindVertexArray(vertexArray_);
	glGenBuffers(1, &vertexBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(GL_ARRAY_BUFFER, 10 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
	glBindVertexArray(0);
#endif
}

void Collider::update() {
	if (!boundingBoxConstructed_) {
		for (const auto mesh : model_->getMeshes()) {
			for (size_t i = 0; i < mesh->numVertices_; ++i) {
				boundingBox_.expand(mesh->vertices_[i]);
			}
		}
		boundingBoxConstructed_ = true;
	}
	aabb_ = boundingBox_.calculateAABB(getEntity().getModelMatrix());

#ifdef SHOW_BOX
	const glm::vec3 vertices[10] = {
		aabb_.min,
		{aabb_.max.x, aabb_.min.y, aabb_.min.z},
		{aabb_.max.x, aabb_.min.y, aabb_.max.z},
		{aabb_.min.x, aabb_.min.y, aabb_.max.z},
		aabb_.min,
		{aabb_.min.x, aabb_.max.y, aabb_.min.z},
		{aabb_.max.x, aabb_.max.y, aabb_.min.z},
		{aabb_.max.x, aabb_.max.y, aabb_.max.z},
		{aabb_.min.x, aabb_.max.y, aabb_.max.z},
		{aabb_.min.x, aabb_.max.y, aabb_.min.z}
	};
	program_->use();
	program_->setUniform("VP", Camera::getInstance().getProjectionViewMatrix());
	glBindVertexArray(vertexArray_);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 10 * sizeof(glm::vec3), vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_LINE_STRIP, 0, 10);
	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
#endif
}

void Collider::registerCallback(std::function<void(void)> callback) {
	callbacks_.emplace_back(callback);
}

void Collider::notifyCollision() const {
	for (const auto callback : callbacks_) {
		callback();
	}
}

bool Collider::intersects(const std::shared_ptr<Collider> collider) const {
	if (!aabb_.intersects(collider->aabb_)) {
		return false;
	}

	if (const auto meshCollider = std::dynamic_pointer_cast<MeshCollider>(collider)) {
		return intersectsImpl(meshCollider);
	} else if (const auto sphereCollider = std::dynamic_pointer_cast<SphereCollider>(collider)) {
		return intersectsImpl(sphereCollider);
	} else if (const auto planeCollider = std::dynamic_pointer_cast<PlaneCollider>(collider)) {
		return intersectsImpl(planeCollider);
	} else {
		throw;
	}
}

std::shared_ptr<Model> Collider::getModel() const {
	return model_;
}

bool MeshCollider::intersectsImpl(std::shared_ptr<MeshCollider> collider) const {
	// TODO
	throw;

	/*for (const auto mesh : getModel()->getMeshes()) {
		for (size_t i = 0; i < mesh->numIndgices_; i += 3) {
			const auto& u0 = mesh->vertices_[mesh->indices_[i]];
			const auto& u1 = mesh->vertices_[mesh->indices_[i + 1]];
			const auto& u2 = mesh->vertices_[mesh->indices_[i + 2]];

			// Moeller, 1997b
			{
				const auto n = glm::cross(u1 - u0, u2 - u0);
				const auto d = -glm::dot(n, u0);
				const auto a = glm::dot(v0, n) + d;
				const auto b = glm::dot(v1, n) + d;
				const auto c = glm::dot(v2, n) + d;
				if ((a > 0 && b > 0 && c > 0) || (a < 0 && b < 0 && c < 0)) {
					// v lies on one side of u
					return false;
				}
			}

			{
				const auto n = glm::cross(v1 - v0, v2 - v0);
				const auto d = -glm::dot(n, v0);
				const auto a = glm::dot(u0, n) + d;
				const auto b = glm::dot(u1, n) + d;
				const auto c = glm::dot(u2, n) + d;
				if ((a > 0 && b > 0 && c > 0) || (a < 0 && b < 0 && c < 0)) {
					// u lies on one side of v
					return false;
				}
			}
		}
	}

	return false;*/
}

bool MeshCollider::intersectsImpl(std::shared_ptr<SphereCollider> collider) const {
	for (const auto mesh : getModel()->getMeshes()) {
		for (size_t i = 0; i < mesh->numIndices_; i += 3) {
			const auto& v0 = (getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i]], 1)).xyz();
			const auto& v1 = (getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i + 1]], 1)).xyz();
			const auto& v2 = (getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i + 2]], 1)).xyz();

			if (collider->getSphere().triangleIntersects(v0, v1, v2)) {
				return true;
			}
		}
	}
	return false;
}

SphereCollider::SphereCollider(std::shared_ptr<Model> model, const float radius) :
	Collider(model),
	sphere_({glm::zero<glm::vec3>(), radius}) {}

void SphereCollider::update() {
	Collider::update();
	const auto p = (boundingBox_.max - boundingBox_.min);
	sphere_.radius = std::min({p.x, p.y, p.z}) / 2;
	sphere_.center = (aabb_.max + aabb_.min) / 2.f;
	//sphere_.center = getEntity().getPosition();
}

bool SphereCollider::intersectsImpl(std::shared_ptr<MeshCollider> collider) const {
	for (const auto mesh : collider->getModel()->getMeshes()) {
		for (size_t i = 0; i < mesh->numIndices_; i += 3) {
			const auto& v0 = collider->getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i]], 1);
			const auto& v1 = collider->getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i + 1]], 1);
			const auto& v2 = collider->getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i + 2]], 1);

			if (sphere_.triangleIntersects(v0, v1, v2)) {
				return true;
			}
		}
	}
	return false;
}

bool SphereCollider::intersectsImpl(std::shared_ptr<SphereCollider> collider) const {
	const auto& sphereB = collider->getSphere();
	return glm::distance(sphere_.center, sphereB.center) <= sphere_.radius + sphereB.radius;
}

bool SphereCollider::intersectsImpl(std::shared_ptr<PlaneCollider> collider) const {
	return collider->getPlane().sphereIntersects(sphere_);
}

std::vector<Triangle> SphereCollider::getCollidingTriangles(std::shared_ptr<MeshCollider> collider) const {
	std::vector<Triangle> triangles;

	for (const auto mesh : collider->getModel()->getMeshes()) {
		for (size_t i = 0; i < mesh->numIndices_; i += 3) {
			const auto& v0 = collider->getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i]], 1);
			const auto& v1 = collider->getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i + 1]], 1);
			const auto& v2 = collider->getEntity().getModelMatrix() * glm::vec4(mesh->vertices_[mesh->indices_[i + 2]], 1);

			if (sphere_.triangleIntersects(v0, v1, v2)) {
				triangles.emplace_back(Triangle{v0, v1, v2});
			}
		}
	}

	return triangles;
}

const Sphere& SphereCollider::getSphere() const {
	return sphere_;
}

CapsuleCollider::CapsuleCollider(std::shared_ptr<Model> model, Capsule capsule) :
	Collider(model),
	capsule_(capsule) {}

void CapsuleCollider::update() {
	transformedCapsule_ = capsule_.applyTransform(
		getEntity().getModelMatrix(), getEntity().getScale());
}

bool CapsuleCollider::intersectsImpl(std::shared_ptr<CapsuleCollider> collider) const {
	// TODO
	throw;

	const auto& capsuleB = collider->getCapsule();
	const auto a = capsule_.b - capsule_.a;
	const auto b = capsuleB.b - capsuleB.a;
	const auto magA = glm::length(a);
	const auto magB = glm::length(b);
	const auto aa = a / magA;
	const auto bb = b / magB;
	const auto cross = glm::cross(aa, bb);
	const auto denom = glm::dot(cross, cross);

	return false;
}

const Capsule& CapsuleCollider::getCapsule() const {
	return capsule_;
}

Capsule Capsule::applyTransform(const glm::mat4& model, const glm::vec3& scale) const {
	return {
		r * std::max({scale.x, scale.y, scale.z}),
		model * glm::vec4(a, 1),
		model * glm::vec4(b, 1)
	};
}

bool Plane::sphereIntersects(const Sphere& sphere) const {
	return glm::dot(normal, sphere.center) + d <= glm::length(normal) * sphere.radius;
}

PlaneCollider::PlaneCollider(std::shared_ptr<Model> model, const Plane& plane) :
	Collider(model),
	plane_(plane) {}

bool PlaneCollider::intersectsImpl(std::shared_ptr<SphereCollider> collider) const {
	return plane_.sphereIntersects(collider->getSphere());
}

const Plane& PlaneCollider::getPlane() const {
	return plane_;
}

}