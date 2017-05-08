#pragma once

namespace islands {

class SceneManager {
public:
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	virtual ~SceneManager() = default;

	static SceneManager& getInstance();

	void lookAt(const glm::vec3& position);
	const glm::mat4& getProjectionMatrix() const;
	const glm::mat4& getViewMatrix() const;
	const glm::mat4& getProjectionViewMatrix() const;

private:
	const glm::mat4 PROJECTION;
	const glm::vec3 CAMERA_OFFSET;
	glm::mat4 view_, projView_;
	glm::vec3 targetPos_;

	SceneManager();

	void updateProjectionViewMatrix();
};

}