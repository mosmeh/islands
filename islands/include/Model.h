#pragma once

#include "Resource.h"
#include "Component.h"
#include "Mesh.h"

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

class Model : public Resource {
public:
	Model(const std::string& name, const std::string& filename);

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) = default;

	virtual ~Model() = default;

	const std::vector<std::shared_ptr<Mesh>>& getMeshes();
	const BoundingBox& getBoundingBox() const;

private:
	const std::string filename_;
	std::vector<std::shared_ptr<Mesh>> meshes_;
	BoundingBox boundingBox_;

	void loadImpl() override;
};

class ModelDrawer :
	public Drawable,
	public Resource {
public:
	ModelDrawer(std::shared_ptr<Model> model);
	virtual ~ModelDrawer() = default;

	void draw() override;

	bool isLoaded() const override;

	void setVisible(bool visible);
	void setCastShadow(bool castShadow);
	void setReceiveShadow(bool receiveShadow);
	void setLightmapTexture(std::shared_ptr<Texture2D> texture);

	void enableAnimation(const std::string& name, bool loop = true, float tps = 24.f);
	void stopAnimation();
	bool isPlayingAnimation() const;

private:
	std::shared_ptr<Model> model_;
	bool visible_, castShadow_, receiveShadow_;
	std::shared_ptr<Texture2D> lightmap_;
	bool animPlaying_, animLoop_;
	std::string animName_;
	float animStartTime_, animDuration_;

	void loadImpl() override;
};

}