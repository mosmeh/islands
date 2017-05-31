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
	void saveScreenShot(const char* filename) const;

private:
	GLFWwindow* window_;
	int width_, height_;
	double lastUpdateTime_;

	Window();
	
	static void framebufferSizeCallback(GLFWwindow*, int, int);
};

}