#include "Effect.h"
#include "ResourceSystem.h"
#include "Camera.h"

namespace islands {

DamageEffect::DamageEffect(std::shared_ptr<Model> model) :
	ModelDrawer(model),
	program_(ResourceSystem::getInstance().createOrGet<Program>(
		"DamageProgram", "skinning.vert", "damage.frag")),
	startedAt_(-INFINITY) {}

void DamageEffect::draw() {
	if (isActive()) {
		program_->use();
		program_->setUniform("MVP", getEntity().calculateMVPMatrix());
		program_->setUniform("time", static_cast<glm::float32>(glfwGetTime() - startedAt_));

		std::stringstream ss;
		for (const auto mesh : model_->getMeshes()) {
			program_->setUniform("diffuse", mesh->getMaterial()->getDiffuseColor());

			const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh);
			assert(skinned);

			for (size_t i = 0; i < skinned->getNumBones(); ++i) {
				ss.str("");
				ss << "bones[" << i << "]";
				program_->setUniform(ss.str().c_str(), skinned->getBoneTransform(i));
			}

			mesh->draw();
		}
	}
}

void DamageEffect::activate() {
	startedAt_ = glfwGetTime();
}

bool DamageEffect::isActive() const {
	return glfwGetTime() - startedAt_ < 0.3;
}

ScatterEffect::ScatterEffect(std::shared_ptr<Model> model) :
	ModelDrawer(model),
	program_(ResourceSystem::getInstance().createOrGet<Program>(
		"ScatterProgram", "scatter.vert", "scatter.geom", "scatter.frag")),
	startedAt_(INFINITY) {}

void ScatterEffect::start() {
	startedAt_ = glfwGetTime();
}

void ScatterEffect::update() {
	ModelDrawer::update();
}

void ScatterEffect::draw() {
	program_->use();
	program_->setUniform("M", getEntity().getModelMatrix());
	program_->setUniform("MV", Camera::getInstance().getViewMatrix() * getEntity().getModelMatrix());
	program_->setUniform("VP", Camera::getInstance().getProjectionViewMatrix());
	program_->setUniform("time", static_cast<glm::float32>(2.0 * (glfwGetTime() - startedAt_)));

	glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::stringstream ss;
	for (const auto mesh : model_->getMeshes()) {
		const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh);
		assert(skinned);

		for (size_t i = 0; i < skinned->getNumBones(); ++i) {
			ss.str("");
			ss << "bones[" << i << "]";
			program_->setUniform(ss.str().c_str(), skinned->getBoneTransform(i));
		}

		mesh->draw();
	}

	glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

bool ScatterEffect::isFinished() const {
	return glfwGetTime() - startedAt_ > 2.0;
}

}