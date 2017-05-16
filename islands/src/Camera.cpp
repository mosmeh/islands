#include "Camera.h"

namespace islands {

Camera::Camera() :
	PROJECTION(glm::perspective(glm::quarter_pi<float>(), 16.f / 9, 0.1f, 100.f)),
	CAMERA_OFFSET(10.f * glm::vec3(-1.f, -1.f, 1.f)) {}

Camera& Camera::getInstance() {
	static Camera instance;
	return instance;
}

void Camera::lookAt(const glm::vec3& position) {
	targetPos_ = position;
	updateProjectionViewMatrix();
}

const glm::mat4& Camera::getProjectionMatrix() const {
	return PROJECTION;
}

const glm::mat4& Camera::getViewMatrix() const {
	return view_;
}

const glm::mat4& Camera::getProjectionViewMatrix() const {
	return projView_;
}

void Camera::updateProjectionViewMatrix() {
	view_ = glm::lookAt(targetPos_ + CAMERA_OFFSET, targetPos_, glm::vec3(0, 0, 1));
	projView_ = PROJECTION * view_;
}

}