#include "Window.h"
#include "Log.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace islands {

Window::Window() : lastUpdateTime_(0.f) {
	SLOG << "GLFW: Creating window" << std::endl;
	window_ = glfwCreateWindow(1280, 720, u8"天空の島", nullptr, nullptr);
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

void Window::saveScreenShot(const char* filename) const {
	constexpr int numComponents = 3;

	SLOG << "Screen shot: Saving to " << filename << std::endl;

	const auto size = numComponents * width_ * height_;

	// TODO: 次を調べる
	// 画面サイズが2のべき乗でないときうまく動かない
	// size だけ確保するとアクセス違反を起こす
	const auto pixels = std::make_unique<char[]>(4 * width_ * height_);
	glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	for (int y = 0; y < height_ / 2; ++y) {
		// swap_ranges を使うと unsafe だと怒られる
		for (int x = 0; x < width_ * numComponents; ++x) {
			std::swap(pixels[x + y * width_ * numComponents],
				pixels[x + (height_ - y - 1) * width_ * numComponents]);
		}
	}

	stbi_write_png(filename, width_, height_, numComponents, pixels.get(), 0);
}

}