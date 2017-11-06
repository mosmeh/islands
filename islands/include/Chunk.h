#pragma once

#include "Resource.h"
#include "Entity.h"
#include "Collision.h"
#include "Geometry.h"

namespace islands {

static const std::string LEVEL_DIR = "level";

class Chunk : public Resource {
public:
	Chunk(const std::string& name, const std::string& filename);
	virtual ~Chunk() = default;

	bool isLoaded() const override;

	void update();
	void draw();

	std::shared_ptr<Entity> createEntity(const std::string& name);
	const std::vector<std::shared_ptr<Entity>>& getEntities() const;
	std::shared_ptr<Entity> getEntityByName(const std::string& name) const;

	const geometry::AABB& getGlobalAABB() const;

private:
	const std::string filename_;
	float cameraOffset_;
	geometry::AABB aabb_;
	std::vector<std::shared_ptr<Entity>> entities_;

	void loadImpl() override;
	void cleanEntities();
};

}