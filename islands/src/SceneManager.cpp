#include "SceneManager.h"

namespace islands {

SceneManager::SceneManager() :
	proj_(glm::perspective(glm::quarter_pi<float>(), 16.f / 9, 0.1f, 100.f)) {}

SceneManager& SceneManager::getInstance() {
	static SceneManager instance;
	return instance;
}

void SceneManager::setCameraPosition(const glm::vec3& position) {
	cameraPos_ = position;
	updateProjectionViewMatrix();
}

void SceneManager::lookAt(const glm::vec3& position) {
	targetPos_ = position;
	updateProjectionViewMatrix();
}

const glm::mat4& SceneManager::getProjectionMatrix() const {
	return proj_;
}

const glm::mat4& SceneManager::getViewMatrix() const {
	return view_;
}

const glm::mat4& SceneManager::getProjectionViewMatrix() const {
	return projView_;
}

void SceneManager::updateProjectionViewMatrix() {
	view_ = glm::lookAt(cameraPos_, targetPos_, glm::vec3(0, 0, 1));
	projView_ = proj_ * view_;
}

}