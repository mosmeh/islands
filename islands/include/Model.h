#pragma once

#include "Resource.h"
#include "Component.h"
#include "Texture.h"
#include "Mesh.h"
#include "Geometry.h"

namespace islands {

class Model : public SharedResource<Model> {
public:
	Model(const std::string& filename);

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) = default;

	virtual ~Model() = default;

	const std::vector<std::shared_ptr<Mesh>>& getMeshes();
	bool isOpaque();
	bool hasSkinnedMesh();
	const geometry::AABB& getLocalAABB();

private:
	std::vector<std::shared_ptr<Mesh>> meshes_;
	bool opaque_, hasSkinned_;
	geometry::AABB localAABB_;

	void loadImpl() override;
};

class ModelDrawer : public Drawable {
public:
	ModelDrawer(std::shared_ptr<Model> model);
	virtual ~ModelDrawer() = default;

	void update() override;
	void draw() override;
	bool isOpaque() const override;

	std::shared_ptr<Model> getModel() const;

	void setVisible(bool visible);
	void setCullFaceEnabled(bool enabled);
	void pushMaterial(std::shared_ptr<Material> material);
	std::shared_ptr<Material> popMaterial();

	void enableAnimation(const std::string& name, bool loop = true, double tps = 24.0, size_t startFrame = 0);
	void stopAnimation();
	bool isPlayingAnimation() const;
	size_t getCurrentAnimationFrame() const;

protected:
	std::shared_ptr<Model> model_;
	bool visible_, cullFaceEnabled_;
	Material::UpdateUniformCallback defaultUpdateCallback_;
	std::stack<std::shared_ptr<Material>> materialStack_;

	struct Animation {
		std::string name;
		bool playing = false, loop;
		double duration, tps;
		double startTime;
		size_t startFrame;
	} anim_;
};

}
