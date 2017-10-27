#pragma once

#include "Resource.h"
#include "Component.h"
#include "Mesh.h"
#include "Geometry.h"

namespace islands {

class Model : public Resource {
public:
	Model(const std::string& name, const std::string& filename);

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) = default;

	virtual ~Model() = default;

	const std::vector<std::shared_ptr<Mesh>>& getMeshes();
	const geometry::AABB& getLocalAABB();

private:
	const std::string filename_;
	std::vector<std::shared_ptr<Mesh>> meshes_;
	geometry::AABB localAABB_;

	void loadImpl() override;
};

class ModelDrawer : public Drawable {
public:
	ModelDrawer(std::shared_ptr<Model> model);
	virtual ~ModelDrawer() = default;

	void update() override;
	void draw() override;

	void setVisible(bool visible);
	void setCastShadow(bool castShadow);
	void setReceiveShadow(bool receiveShadow);
	void setLightmapTexture(std::shared_ptr<Texture2D> texture);

	void enableAnimation(const std::string& name, bool loop = true, float tps = 24.f);
	void stopAnimation();
	bool isPlayingAnimation() const;

protected:
	std::shared_ptr<Model> model_;
	bool visible_, castShadow_, receiveShadow_;
	std::shared_ptr<Texture2D> lightmap_;
	bool animPlaying_, animLoop_;
	std::string animName_;
	float animStartTime_, animDuration_;
};

}