#pragma once

#include "Resource.h"
#include "Entity.h"
#include "Collision.h"
#include "PhysicsSystem.h"

namespace islands {

class Chunk : public Resource {
public:
	Chunk(const std::string& name, const std::string& filename);
	virtual ~Chunk() = default;

	bool isLoaded() const override;

	void addEntity(std::shared_ptr<Entity> entity);
	std::shared_ptr<Entity> getEntity(const std::string& name) const;
	void update();
	void draw();

	PhysicsSystem& getPhysicsSystem();

private:
	const std::string filename_;
	std::vector<std::shared_ptr<Entity>> entities_;
	std::vector<std::shared_ptr<Entity>> entitiesToBeAdded_;
	PhysicsSystem physicsSystem_;

	void loadImpl() override;
};

}