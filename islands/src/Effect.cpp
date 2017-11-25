#include "Effect.h"
#include "ResourceSystem.h"
#include "Camera.h"

namespace islands {

DamageEffect::DamageEffect(double duration) :
	duration_(duration),
	program_(ResourceSystem::getInstance().createOrGet<Program>(
		"DamageProgram", "skinning.vert", "damage.frag")) {}

void DamageEffect::start() {
	drawer_ = getEntity().getFirstComponent<ModelDrawer>();
	startedAt_ = glfwGetTime();
}

void DamageEffect::update() {
	if (glfwGetTime() - startedAt_ < duration_) {
		drawer_->setVisible(false);
	} else {
		drawer_->setVisible(true);
		destroy();
	}
}

void DamageEffect::draw() {
	program_->use();
	program_->setUniform("MVP", getEntity().calculateMVPMatrix());
	program_->setUniform("time", static_cast<glm::float32>(glfwGetTime() - startedAt_));

	std::stringstream ss;
	for (const auto mesh : drawer_->getModel()->getMeshes()) {
		mesh->getMaterial().apply(program_);

		const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh);
		assert(skinned);
		skinned->applyBoneTransform(program_);
		mesh->draw();
	}
}

bool DamageEffect::isOpaque() const {
	return true;
}

ScatterEffect::ScatterEffect(const FinishCallback& callback) :
	callback_(callback),
	program_(ResourceSystem::getInstance().createOrGet<Program>(
		"ScatterProgram", "scatter.vert", "scatter.geom", "scatter.frag")) {}

void ScatterEffect::start() {
	drawer_ = getEntity().getFirstComponent<ModelDrawer>();
	startedAt_ = glfwGetTime();
}

void ScatterEffect::update() {
	if (glfwGetTime() - startedAt_ <= 1.0) {
		drawer_->setVisible(false);
	} else {
		drawer_->setVisible(true);
		callback_();
		destroy();
	}
}

void ScatterEffect::draw() {
	program_->use();
	program_->setUniform("M", getEntity().getModelMatrix());
	program_->setUniform("MV", Camera::getInstance().getViewMatrix() * getEntity().getModelMatrix());
	program_->setUniform("VP", Camera::getInstance().getProjectionViewMatrix());
	program_->setUniform("time", static_cast<glm::float32>(2.0 * (glfwGetTime() - startedAt_)));

	glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

	std::stringstream ss;
	for (const auto mesh : drawer_->getModel()->getMeshes()) {
		const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh);
		assert(skinned);
		skinned->applyBoneTransform(program_);
		mesh->draw();
	}

	glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

bool ScatterEffect::isOpaque() const {
	return false;
}

}
