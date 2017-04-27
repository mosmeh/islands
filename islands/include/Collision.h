#pragma once

#include "Component.h"
#include "Model.h"

//#define SHOW_BOX

namespace islands {

struct BoundingBox {
	glm::vec3 min, max;

	BoundingBox() :
		min(INFINITY),
		max(-INFINITY) {}

	void expand(const glm::vec3& vertex);
	BoundingBox calculateAABB(const glm::mat4& model) const;
	bool intersects(const BoundingBox& box) const;
};

struct Sphere {
	glm::vec3 center;
	float radius;

	bool triangleIntersects(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) const;
};

struct Triangle {
	glm::vec3 v0, v1, v2;
};

struct Capsule {
	float r;
	glm::vec3 a, b;

	Capsule applyTransform(const glm::mat4& model, const glm::vec3& scale) const;
};

class CollisionListener {
public:
	virtual ~CollisionListener() = default;
	virtual void onCollision() = 0;
};

class Collider :
	public std::enable_shared_from_this<Collider>, 
	public Component {
public:
	Collider(std::shared_ptr<Model> model);
	virtual ~Collider() = default;

	void update() override;

	void registerListener(std::shared_ptr<CollisionListener> listener);
	void notifyCollision() const;

	bool intersects(const std::shared_ptr<Collider> collider) const;

	std::shared_ptr<Model> getModel() const;

protected:
	std::shared_ptr<Model> model_;
	bool boundingBoxConstructed_;
	BoundingBox boundingBox_, aabb_;
	std::vector<std::shared_ptr<CollisionListener>> listeners_;

#ifdef SHOW_BOX
	std::shared_ptr<Program> program_;
	GLuint vertexArray_, vertexBuffer_;
#endif

	virtual bool intersectsImpl(std::shared_ptr<MeshCollider> collider) const = 0;
	virtual bool intersectsImpl(std::shared_ptr<SphereCollider> collider) const = 0;
};

class SphereCollider : public Collider {
public:
	SphereCollider(std::shared_ptr<Model> model, const float radius);
	virtual ~SphereCollider() = default;

	void update() override;

	bool intersectsImpl(std::shared_ptr<MeshCollider> collider) const override;
	bool intersectsImpl(std::shared_ptr<SphereCollider> collider) const override;

	std::vector<Triangle> getCollidingTriangles(std::shared_ptr<MeshCollider> collider) const;

	const Sphere& getSphere() const;

private:
	Sphere sphere_;
};

class MeshCollider : public Collider {
public:
	using Collider::Collider;
	virtual ~MeshCollider() = default;

	bool intersectsImpl(std::shared_ptr<MeshCollider> collider) const override;
	bool intersectsImpl(std::shared_ptr<SphereCollider> collider) const override;
};

class CapsuleCollider : public Collider {
public:
	CapsuleCollider(std::shared_ptr<Model> model, Capsule capsule);
	virtual ~CapsuleCollider() = default;

	void update();
	bool intersectsImpl(std::shared_ptr<CapsuleCollider> collider) const;
	bool intersectsImpl(std::shared_ptr<MeshCollider> collider) const;

	const Capsule& getCapsule() const;

private:
	Capsule capsule_, transformedCapsule_;
};

}