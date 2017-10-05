#pragma once

#include "Component.h"
#include "Model.h"
#include "Geometry.h"

namespace islands {

class AABBCollider;
class SphereCollider;
class PlaneCollider;

class Collider :
	public std::enable_shared_from_this<Collider>,
	public Component {
public:
	Collider(std::shared_ptr<Model> model);
	virtual ~Collider() = default;

	void registerCallback(std::function<void(std::shared_ptr<Collider>)> callback);
	void notifyCollision(std::shared_ptr<Collider> opponent) const;

	const geometry::AABB& getGlobalAABB() const;

	virtual void update() override;
	virtual glm::vec3 getNormal(const glm::vec3& refPos) const = 0;

	glm::vec3 getSinkingCorrectionVector(std::shared_ptr<Collider> collider) const;
	bool intersects(std::shared_ptr<Collider>) const;

	virtual bool intersects(std::shared_ptr<AABBCollider>) const {
		throw std::exception("not implemented");
	}
	virtual bool intersects(std::shared_ptr<SphereCollider>) const {
		throw std::exception("not implemented");
	}
	virtual bool intersects(std::shared_ptr<PlaneCollider>) const {
		throw std::exception("not implemented");
	}

protected:
	std::shared_ptr<Model> getModel() const;

	virtual float getSinking(std::shared_ptr<AABBCollider>) const {
		throw std::exception("not implemented");
	}
	virtual float getSinking(std::shared_ptr<SphereCollider>) const {
		throw std::exception("not implemented");
	}
	virtual float getSinking(std::shared_ptr<PlaneCollider>) const {
		throw std::exception("not implemented");
	}

private:
	std::shared_ptr<Model> model_;
	geometry::AABB globalAABB_;
	std::vector<std::function<void(std::shared_ptr<Collider>)>> callbacks_;
};

class AABBCollider : public Collider {
public:
	using Collider::Collider;
	virtual ~AABBCollider() = default;

	glm::vec3 getNormal(const glm::vec3&) const override {
		throw std::exception("not implemented");
	}

	bool intersects(std::shared_ptr<AABBCollider>) const override;
};

class SphereCollider : public Collider {
public:
	using Collider::Collider;
	virtual ~SphereCollider() = default;

	void update() override;
	glm::vec3 getNormal(const glm::vec3& refPos) const override;
	bool intersects(std::shared_ptr<SphereCollider>) const override;
	bool intersects(std::shared_ptr<PlaneCollider>) const override;
	const geometry::Sphere& getGlobalSphere() const;

private:
	geometry::Sphere sphere_;

	float getSinking(std::shared_ptr<SphereCollider>) const override;
	float getSinking(std::shared_ptr<PlaneCollider>) const override;
};

class PlaneCollider : public Collider {
public:
	PlaneCollider(std::shared_ptr<Model> model, const glm::vec3& normal);
	virtual ~PlaneCollider() = default;

	void update() override;
	glm::vec3 getNormal(const glm::vec3& refPos) const override;
	bool intersects(std::shared_ptr<SphereCollider>) const override;
	const geometry::Plane& getGlobalPlane() const;
	void setOffset(float offset);

private:
	geometry::Plane globalPlane_;
	float offset_;

	float getSinking(std::shared_ptr<SphereCollider>) const override;
};

}