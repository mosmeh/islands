#include "Window.h"
#include "Log.h"

namespace islands {

Window::Window() : lastUpdateTime_(0.f) {
	SLOG << "GLFW: Creating window" << std::endl;
	window_ = glfwCreateWindow(1280, 720, u8"“V‹ó‚Ì“‡", nullptr, nullptr);
	glfwMakeContextCurrent(window_);

	SLOG << "glad: Loading" << std::endl;
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		SLOG << "glad: Failed to load" << std::endl;
		throw;
	}

	glfwGetFramebufferSize(window_, &width_, &height_);
	glViewport(0, 0, width_, height_);

	glfwSetWindowAspectRatio(window_, 16, 9);
	glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
	glfwSetInputMode(window_, GLFW_STICKY_KEYS, GLFW_TRUE);

	sys::disableIME(window_);
}

void Window::framebufferSizeCallback(GLFWwindow*, int width, int height) {
	Window::getInstance().width_ = width;
	Window::getInstance().height_ = height;
	glViewport(0, 0, width, height);
}

Window& Window::getInstance() {
	static Window instance;
	return instance;
}

GLFWwindow* Window::getHandle() const {
	return window_;
}

bool Window::update() {
	if (glfwWindowShouldClose(window_)) {
		return false;
	}

	glfwSwapBuffers(window_);
	glfwPollEvents();

	static constexpr auto TARGET_FPS = 60;
	const auto sleepDuration = 1000 * (1.0 / TARGET_FPS - glfwGetTime() + lastUpdateTime_);
	if (sleepDuration > 0) {
		sys::sleep(std::chrono::milliseconds(static_cast<unsigned long>(sleepDuration)));
	}
	lastUpdateTime_ = glfwGetTime();

	return true;
}

glm::uvec2 Window::getFramebufferSize() const {
	return {width_, height_};
}

}