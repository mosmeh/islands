#pragma once

#include "Resource.h"
#include "Entity.h"
#include "Geometry.h"
#include "Sound.h"

namespace islands {

static const std::string LEVEL_DIR = "level";

class Chunk : public Resource {
public:
	Chunk(const std::string& filename);
	virtual ~Chunk() = default;

	void update();
	void draw();

	std::shared_ptr<Entity> createEntity(const std::string& name);
	const std::list<std::shared_ptr<Entity>>& getEntities() const;
	std::shared_ptr<Entity> getEntityByName(const std::string& name) const;

	const geometry::AABB& getGlobalAABB() const;

	std::shared_ptr<Sound> getBGM() const;

private:
	float cameraOffset_;
	std::shared_ptr<Sound> bgm_;
	geometry::AABB aabb_;
	std::list<std::shared_ptr<Entity>> entities_;

	void loadImpl() override;
	void cleanEntities();
};

}
