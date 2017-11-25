#pragma once

#include "Resource.h"
#include "Component.h"
#include "Texture.h"
#include "Mesh.h"
#include "Geometry.h"

namespace islands {

class Model : public SharedResource<Model> {
public:
	Model(const std::string& name, const std::string& filename);

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) = default;

	virtual ~Model() = default;

	const std::vector<std::shared_ptr<Mesh>>& getMeshes();
	bool isOpaque();
	bool hasSkinnedMesh();
	const geometry::AABB& getLocalAABB();

private:
	const std::string filename_;
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
	void setTexture(std::shared_ptr<Texture2D> texture);
	void setCullFaceEnabled(bool enabled);

	void setVertexShader(std::shared_ptr<Shader> shader);
	void setGeometryShader(std::shared_ptr<Shader> shader);
	void setFragmentShader(std::shared_ptr<Shader> shader);

	void enableAnimation(const std::string& name, bool loop = true, double tps = 24.0, size_t startFrame = 0);
	void stopAnimation();
	bool isPlayingAnimation() const;
	size_t getCurrentAnimationFrame() const;

protected:
	std::shared_ptr<Model> model_;
	bool visible_, cullFaceEnabled_;
	std::shared_ptr<Texture2D> texture_;
	std::shared_ptr<Shader> vertex_, geometry_, fragment_;
	std::shared_ptr<Program> program_, skinningProgram_;

	struct Animation {
		std::string name;
		bool playing = false, loop;
		double duration, tps;
		double startTime;
		size_t startFrame;
	} anim_;

	void updateProgram();
};

}
