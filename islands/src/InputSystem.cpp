#include "InputSystem.h"

namespace islands {

InputSystem::InputSystem() : window_(nullptr) {}

InputSystem::~InputSystem() {
	glfwSetKeyCallback(window_, nullptr);
}

InputSystem& InputSystem::getInstance() {
	static InputSystem instance;
	return instance;
}

void InputSystem::setWindow(GLFWwindow* window) {
	assert(window);
	window_ = window;
	glfwSetKeyCallback(window, keyboardCallback);
}

void InputSystem::registerKeyboardCallback(GLFWkeyfun callback) {
	assert(callback);
	keyboardCallbacks_.push_back(callback);
}

bool InputSystem::isKeyPressed(int key) const {
	assert(GLFW_KEY_SPACE <= key && key <= GLFW_KEY_LAST);
	return glfwGetKey(window_, key) == GLFW_PRESS;
}

void InputSystem::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	for (const auto callback : getInstance().keyboardCallbacks_) {
		callback(window, key, scancode, action, mods);
	}
}

}