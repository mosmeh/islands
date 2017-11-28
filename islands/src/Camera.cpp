#include "Camera.h"

namespace islands {

const glm::mat4 Camera::PROJECTION = glm::perspective(glm::radians(30.f), 16.f / 9, 0.1f, 100.f);

Camera::Camera() : offset_(15.f) {}

Camera& Camera::getInstance() {
	static Camera instance;
	return instance;
}

void Camera::lookAt(const glm::vec3& position) {
	targetPos_ = position;
	updateProjectionViewMatrix();
}

void Camera::setOffset(float offset) {
	assert(offset > 0.f);
	offset_ = offset;
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
	const auto eye = targetPos_ + offset_ * glm::vec3(-1.f, -1.f, 1.f);
	view_ = glm::lookAt(eye, targetPos_, glm::vec3(0, 0, 1));
	projView_ = PROJECTION * view_;
}

}
