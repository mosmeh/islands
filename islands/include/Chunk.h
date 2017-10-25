#pragma once

#include "Resource.h"
#include "Entity.h"
#include "Collision.h"
#include "Physics.h"
#include "Geometry.h"

namespace islands {

static const std::string LEVEL_DIR = "level";

class Chunk : public Resource {
public:
	Chunk(const std::string& name, const std::string& filename);
	virtual ~Chunk() = default;

	bool isLoaded() const override;

	std::shared_ptr<Entity> createEntity(const std::string& name);
	std::shared_ptr<Entity> getEntityByName(const std::string& name) const;
	void update();
	void draw();

	Physics& getPhysics();

	const geometry::AABB& getGlobalAABB() const;

private:
	const std::string filename_;
	geometry::AABB aabb_;
	std::vector<std::shared_ptr<Entity>> entities_;
	std::vector<std::shared_ptr<Entity>> entitiesToBeAdded_;
	Physics physics_;

	void loadImpl() override;
};

}