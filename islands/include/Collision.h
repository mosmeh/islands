#pragma once

#include "Component.h"
#include "Model.h"
#include "Geometry.h"

namespace islands {

class AABBCollider;
class SphereCollider;
class PlaneCollider;
class MeshCollider;

class Collider : public Component {
public:
	using Callback = std::function<void(std::shared_ptr<Collider>)>;

	Collider(std::shared_ptr<Model> model);
	virtual ~Collider() = default;

	void registerCallback(const Callback& callback);
	void notifyCollision(std::shared_ptr<Collider> opponent) const;
	const geometry::AABB& getGlobalAABB() const;

	virtual void update() override;
	virtual glm::vec3 getNormal(const glm::vec3& refPos) const = 0;

	glm::vec3 getSinkingCorrector(std::shared_ptr<Collider> collider) const;
	bool intersects(std::shared_ptr<Collider>) const;

protected:
	std::shared_ptr<Model> getModel() const;

	virtual bool intersectsImpl(std::shared_ptr<AABBCollider>) const {
		throw std::exception("not implemented");
	}
	virtual bool intersectsImpl(std::shared_ptr<SphereCollider>) const {
		throw std::exception("not implemented");
	}
	virtual bool intersectsImpl(std::shared_ptr<PlaneCollider>) const {
		throw std::exception("not implemented");
	}
	virtual bool intersectsImpl(std::shared_ptr<MeshCollider>) const {
		throw std::exception("not implemented");
	}

	virtual float getSinking(std::shared_ptr<AABBCollider>) const {
		throw std::exception("not implemented");
	}
	virtual float getSinking(std::shared_ptr<SphereCollider>) const {
		throw std::exception("not implemented");
	}
	virtual float getSinking(std::shared_ptr<PlaneCollider>) const {
		throw std::exception("not implemented");
	}
	virtual float getSinking(std::shared_ptr<MeshCollider>) const {
		throw std::exception("not implemented");
	}

private:
	std::shared_ptr<Model> model_;
	geometry::AABB globalAABB_;
	std::vector<Callback> callbacks_;
};

class AABBCollider : public Collider {
public:
	using Collider::Collider;
	virtual ~AABBCollider() = default;

	glm::vec3 getNormal(const glm::vec3&) const override {
		throw std::exception("not implemented");
	}

private:
	bool intersectsImpl(std::shared_ptr<AABBCollider>) const override;
};

class SphereCollider : public Collider {
public:
	SphereCollider(std::shared_ptr<Model> model);
	SphereCollider(std::shared_ptr<Model> model, float radius);
	virtual ~SphereCollider() = default;

	void update() override;
	glm::vec3 getNormal(const glm::vec3& refPos) const override;
	const geometry::Sphere& getGlobalSphere() const;

private:
	bool radiusFixed_;
	geometry::Sphere sphere_;

	bool intersectsImpl(std::shared_ptr<SphereCollider>) const override;
	bool intersectsImpl(std::shared_ptr<PlaneCollider>) const override;
	bool intersectsImpl(std::shared_ptr<MeshCollider>) const override;
	float getSinking(std::shared_ptr<SphereCollider>) const override;
	float getSinking(std::shared_ptr<PlaneCollider>) const override;
};

class PlaneCollider : public Collider {
public:
	PlaneCollider(std::shared_ptr<Model> model, const glm::vec3& normal);
	virtual ~PlaneCollider() = default;

	void update() override;
	glm::vec3 getNormal(const glm::vec3& refPos) const override;
	const geometry::Plane& getGlobalPlane() const;
	void setOffset(float offset);

private:
	geometry::Plane globalPlane_;
	float offset_;

	bool intersectsImpl(std::shared_ptr<SphereCollider>) const override;
	float getSinking(std::shared_ptr<SphereCollider>) const override;
};

class MeshCollider : public Collider {
public:
	MeshCollider(std::shared_ptr<Model> model);
	virtual ~MeshCollider() = default;

	void update() override;
	glm::vec3 getNormal(const glm::vec3& refPos) const override;
	geometry::CollisionMesh& getCollisionMesh() const;

private:
	mutable geometry::CollisionMesh collisionMesh_;

	bool intersectsImpl(std::shared_ptr<SphereCollider>) const override;
	float getSinking(std::shared_ptr<SphereCollider>) const override;
};

}