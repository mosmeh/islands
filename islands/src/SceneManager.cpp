#include "SceneManager.h"

namespace islands {

SceneManager::SceneManager() :
	PROJECTION(glm::perspective(glm::quarter_pi<float>(), 16.f / 9, 0.1f, 100.f)),
	CAMERA_OFFSET(15.f * glm::vec3(-1.f, -1.f, 1.f)) {}

SceneManager& SceneManager::getInstance() {
	static SceneManager instance;
	return instance;
}

void SceneManager::lookAt(const glm::vec3& position) {
	targetPos_ = position;
	updateProjectionViewMatrix();
}

const glm::mat4& SceneManager::getProjectionMatrix() const {
	return PROJECTION;
}

const glm::mat4& SceneManager::getViewMatrix() const {
	return view_;
}

const glm::mat4& SceneManager::getProjectionViewMatrix() const {
	return projView_;
}

void SceneManager::updateProjectionViewMatrix() {
	view_ = glm::lookAt(targetPos_ + CAMERA_OFFSET, targetPos_, glm::vec3(0, 0, 1));
	projView_ = PROJECTION * view_;
}

}