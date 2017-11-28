#pragma once

namespace islands {

class Camera {
public:
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;
	virtual ~Camera() = default;

	static Camera& getInstance();

	void lookAt(const glm::vec3& position);
	void setOffset(float offset);
	const glm::mat4& getProjectionMatrix() const;
	const glm::mat4& getViewMatrix() const;
	const glm::mat4& getProjectionViewMatrix() const;

private:
	static const glm::mat4 PROJECTION;
	glm::mat4 view_, projView_;
	glm::vec3 targetPos_;
	float offset_;

	Camera();

	void updateProjectionViewMatrix();
};

}
