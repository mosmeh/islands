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

void InputSystem::update() {
	auto dir = glm::zero<glm::vec2>();
	if (keyboard_.isPresent()) {
		keyboard_.update();
		dir += keyboard_.getDirection();
	}
	if (joystick_.isPresent()) {
		joystick_.update();
		dir += joystick_.getDirection();
	}
	if (glm::length2(dir) > glm::epsilon<float>()) {
		direction_ = glm::normalize(dir);
	} else {
		direction_ = glm::zero<glm::vec2>();
	}
}

void InputSystem::setWindow(GLFWwindow* window) {
	assert(window);
	window_ = window;
	glfwSetKeyCallback(window, keyboardCallback);
	keyboard_.setWindow(window);
}

void InputSystem::registerKeyboardCallback(GLFWkeyfun callback) {
	assert(callback);
	keyboardCallbacks_.push_back(callback);
}

const glm::vec2 InputSystem::getDirection() const {
	return direction_;
}

bool InputSystem::isCommandActive(Command command) const {
	if (keyboard_.isPresent() && keyboard_.isCommandActive(command)) {
		return true;
	} else if (joystick_.isPresent() && joystick_.isCommandActive(command)) {
		return true;
	}
	return false;
}

void InputSystem::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	for (const auto callback : getInstance().keyboardCallbacks_) {
		callback(window, key, scancode, action, mods);
	}
}

InputSystem::Keyboard::Keyboard() : window_(nullptr) {}

bool InputSystem::Keyboard::isPresent() const {
	return window_;
}

void InputSystem::Keyboard::update() {
	direction_ = glm::zero<glm::vec2>();
	if (isKeyPressed(GLFW_KEY_UP)) {
		direction_ += glm::vec2(0, -1);
	}
	if (isKeyPressed(GLFW_KEY_RIGHT)) {
		direction_ += glm::vec2(1, 0);
	}
	if (isKeyPressed(GLFW_KEY_DOWN)) {
		direction_ += glm::vec2(0, 1);
	}
	if (isKeyPressed(GLFW_KEY_LEFT)) {
		direction_ += glm::vec2(-1, 0);
	}

	const bool x = isKeyPressed(GLFW_KEY_LEFT) ^ isKeyPressed(GLFW_KEY_RIGHT);
	const bool y = isKeyPressed(GLFW_KEY_UP) ^ isKeyPressed(GLFW_KEY_DOWN);
	if (x && y) {
		direction_ = glm::normalize(direction_);
	}
}

void InputSystem::Keyboard::setWindow(GLFWwindow* window) {
	assert(window);
	window_ = window;
}

glm::vec2 InputSystem::Keyboard::getDirection() const {
	return direction_;
}

bool InputSystem::Keyboard::isCommandActive(Command command) const {
	switch (command) {
	case Command::Jump:
		return isKeyPressed(GLFW_KEY_X);
	case Command::Attack:
		return isKeyPressed(GLFW_KEY_Z);
	}
	throw;
}

bool InputSystem::Keyboard::isKeyPressed(int key) const {
	assert(window_);
	assert(GLFW_KEY_SPACE <= key && key <= GLFW_KEY_LAST);
	return glfwGetKey(window_, key) == GLFW_PRESS;
}

InputSystem::Joystick::Joystick() :
	present_(false),
	buttons_(nullptr) {

	for (id_ = GLFW_JOYSTICK_1; id_ <= GLFW_JOYSTICK_LAST; ++id_) {
		if (glfwJoystickPresent(id_)) {
			present_ = true;
			break;
		}
	}
}

bool InputSystem::Joystick::isPresent() const {
	return present_;
}

void InputSystem::Joystick::update() {
	static const float DEADZONE_RADIUS_SQUARED = 0.1f;

	int numAxes, numButtons;
	const auto axes = glfwGetJoystickAxes(id_, &numAxes);
	assert(numAxes >= NUM_AXES);
	buttons_ = glfwGetJoystickButtons(id_, &numButtons);
	assert(numButtons >= Button::Count);

	if (isButtonPressed(Button::Up) == GLFW_RELEASE &&
		isButtonPressed(Button::Right) == GLFW_RELEASE &&
		isButtonPressed(Button::Down) == GLFW_RELEASE &&
		isButtonPressed(Button::Left) == GLFW_RELEASE) {

		const auto dir = glm::vec2(
			axes[static_cast<size_t>(Axis::LeftHorizontal)],
			axes[static_cast<size_t>(Axis::LeftVertical)]);
		if (glm::length2(dir) > DEADZONE_RADIUS_SQUARED) {
			direction_ = glm::normalize(dir);
		} else {
			direction_ = glm::zero<glm::vec2>();
		}
	} else {
		if (isButtonPressed(Button::Up) == GLFW_PRESS) {
			direction_ = {0, -1};
		} else if (isButtonPressed(Button::Down) == GLFW_PRESS) {
			direction_ = {0, 1};
		}
		if (isButtonPressed(Button::Right) == GLFW_PRESS) {
			direction_ = {1, 0};
		} else if (isButtonPressed(Button::Left) == GLFW_PRESS) {
			direction_ = {-1, 0};
		}
	}
}

glm::vec2 InputSystem::Joystick::getDirection() const {
	return direction_;
}

bool InputSystem::Joystick::isCommandActive(Command command) const {
	switch (command) {
	case Command::Jump:
		return isButtonPressed(Button::X);
	case Command::Attack:
		return isButtonPressed(Button::O);
	}
	throw;
}

bool InputSystem::Joystick::isButtonPressed(Button button) const {
	if (!buttons_) {
		return false;
	}
	return buttons_[static_cast<size_t>(button)];
}

}