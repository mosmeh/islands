#pragma once

namespace islands {

class Window {
public:
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	virtual ~Window() = default;

	static Window& getInstance();

	GLFWwindow* getHandle() const;
	bool update();
	glm::uvec2 getFramebufferSize() const;
	void registerFramebufferResizeCallback(std::function<void(int, int)>);
	float getDeltaTime() const;
	void saveScreenShot(const char* filename) const;

private:
	GLFWwindow* window_;
	int width_, height_;
	std::vector<std::function<void(int, int)>> fbResizeCallbacks_;
	double lastUpdateTime_;
	float deltaTime_;

	Window();
	
	void updateFramebufferSize(int width, int height);
	static void framebufferSizeCallback(GLFWwindow*, int, int);
};

}
