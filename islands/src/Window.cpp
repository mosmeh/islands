#include "Window.h"
#include "Log.h"
#include "Version.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace islands {

Window::Window() :
	lastUpdateTime_(0.0),
	deltaTime_(0.0) {

	SLOG << "GLFW: Creating window" << std::endl;
	std::stringstream ss;
	ss << APP_NAME << " v" << VERSION_MAJOR << "." << VERSION_MINOR;
	window_ = glfwCreateWindow(1280, 720, ss.str().c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window_);

	SLOG << "glad: Loading" << std::endl;
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		SLOG << "glad: Failed to load" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	glfwGetFramebufferSize(window_, &width_, &height_);
	glViewport(0, 0, width_, height_);

	glfwSetWindowAspectRatio(window_, 16, 9);
	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow*, int width, int height) {
		getInstance().updateFramebufferSize(width, height);
	});

	sys::disableIME(window_);
}

void Window::updateFramebufferSize(int width, int height) {
	width_ = width;
	height_ = height;
	glViewport(0, 0, width, height);

	for (const auto callback : fbResizeCallbacks_) {
		callback(width, height);
	}
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

	constexpr auto TARGET_DELTA_TIME = 1.0 / 60;
	constexpr auto MAX_DELTA_TIME = 1.0 / 15;

	const auto sleepDuration = 1000 * (TARGET_DELTA_TIME - glfwGetTime() + lastUpdateTime_);
	if (sleepDuration > 0) {
		sys::sleep(std::chrono::milliseconds(static_cast<unsigned long>(sleepDuration)));
	}

	const auto now = glfwGetTime();
	deltaTime_ = static_cast<float>(std::min(MAX_DELTA_TIME, now - lastUpdateTime_));
	lastUpdateTime_ = now;

	return true;
}

glm::uvec2 Window::getFramebufferSize() const {
	return {width_, height_};
}

void Window::registerFramebufferResizeCallback(std::function<void(int, int)> callback) {
	fbResizeCallbacks_.emplace_back(callback);
}

float Window::getDeltaTime() const {
	return deltaTime_;
}

void Window::saveScreenShot(const char* filename) const {
	static constexpr int numComponents = 3;

	SLOG << "Screen shot: Saving to " << filename << std::endl;

	const auto size = numComponents * width_ * height_;
	const auto stride = numComponents * width_;

	std::vector<char> pixels(4 * width_ * height_);
	glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	for (int y = 0; y < height_ / 2; ++y) {
		for (int x = 0; x < stride; ++x) {
			std::swap(pixels.at(x + y * stride), pixels.at(x + (height_ - y - 1) * stride));
		}
	}

	stbi_write_png(filename, width_, height_, numComponents, pixels.data(), 0);
}

}
