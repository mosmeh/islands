#include "Input.h"
#include "Window.h"

namespace islands {

Input::Input() {
	glfwSetKeyCallback(Window::getInstance().getHandle(), [](GLFWwindow*, int key, int, int action, int) {
		for (const auto callback : getInstance().keyboardCallbacks_) {
			callback(key, action);
		}

		switch (action) {
		case GLFW_PRESS:
			getInstance().pressedKeys_.emplace(key);
			break;
		case GLFW_RELEASE:
			getInstance().pressedKeys_.erase(key);
			break;
		}
	});
}

Input::~Input() {
	glfwSetKeyCallback(Window::getInstance().getHandle(), nullptr);
}

Input& Input::getInstance() {
	static Input instance;
	return instance;
}

void Input::update() {
	auto dir = glm::zero<glm::vec2>();

	keyboard_.update();
	if (keyboard_.isPresent()) {
		dir += keyboard_.getDirection();
	}

	gamepad_.update();
	if (gamepad_.isPresent()) {
		dir += gamepad_.getDirection();
	}

	if (glm::length2(dir) > glm::epsilon<float>()) {
		direction_ = glm::normalize(dir);
	} else {
		direction_ = glm::zero<glm::vec2>();
	}
}

void Input::registerKeyboardCallback(const KeyboardCallback& callback) {
	keyboardCallbacks_.push_back(callback);
}

const glm::vec2 Input::getDirection() const {
	return direction_;
}

bool Input::isCommandActive(Command command) const {
	if (keyboard_.isPresent() && keyboard_.isCommandActive(command)) {
		return true;
	} else if (gamepad_.isPresent() && gamepad_.isCommandActive(command)) {
		return true;
	}
	return false;
}

bool Input::anyButtonPressed() const {
	if (keyboard_.isPresent() && keyboard_.anyButtonPressed()) {
		return true;
	} else if (gamepad_.isPresent() && gamepad_.anyButtonPressed()) {
		return true;
	}
	return false;
}

bool Input::anyButtonExceptArrowPressed() const {
	if (keyboard_.isPresent() && keyboard_.anyButtonExceptArrowPressed()) {
		return true;
	} else if (gamepad_.isPresent() && gamepad_.anyButtonExceptArrowPressed()) {
		return true;
	}
	return false;
}

const Input::Keyboard& Input::getKeyboard() const {
	return keyboard_;
}

const Input::Gamepad& Input::getGamepad() const {
	return gamepad_;
}

bool Input::Keyboard::isPresent() const {
	return true;
}

void Input::Keyboard::update() {
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

	if (direction_.x != 0 && direction_.y != 0) {
		direction_ = glm::normalize(direction_);
	}
}

glm::vec2 Input::Keyboard::getDirection() const {
	return direction_;
}

bool Input::Keyboard::isCommandActive(Command command) const {
	switch (command) {
	case Command::Jump:
		return isKeyPressed(GLFW_KEY_X);
	case Command::Attack:
		return isKeyPressed(GLFW_KEY_Z);
	}
	throw;
}

bool Input::Keyboard::anyButtonPressed() const {
	return !Input::getInstance().pressedKeys_.empty();
}

bool Input::Keyboard::anyButtonExceptArrowPressed() const {
	const auto& pressedKeys = Input::getInstance().pressedKeys_;
	if (pressedKeys.empty()) {
		return false;
	} else {
		return std::all_of(pressedKeys.begin(), pressedKeys.end(), [](int key) {
			switch (key) {
			case GLFW_KEY_UP:
			case GLFW_KEY_DOWN:
			case GLFW_KEY_LEFT:
			case GLFW_KEY_RIGHT:
				return false;
			default:
				return true;
			}
		});
	}
}

bool Input::Keyboard::isKeyPressed(int key) const {
	assert(GLFW_KEY_SPACE <= key && key <= GLFW_KEY_LAST);
	return glfwGetKey(Window::getInstance().getHandle(), key) == GLFW_PRESS;
}

Input::Gamepad::Gamepad() : present_(false) {}

bool Input::Gamepad::isPresent() const {
	return present_;
}

void Input::Gamepad::update() {
	present_ = false;
	for (id_ = GLFW_JOYSTICK_1; id_ <= GLFW_JOYSTICK_LAST; ++id_) {
		if (glfwJoystickPresent(id_) && glfwJoystickIsGamepad(id_)) {
			present_ = true;
			break;
		}
	}

	if (present_) {
		static const float DEADZONE_RADIUS_SQUARED = 0.1f;

		if (glfwGetGamepadState(id_, &state_)) {
			if (!isButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP) &&
				!isButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT) &&
				!isButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN) &&
				!isButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT)) {

				const auto dir = glm::vec2(
					state_.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
					(isDualShock4() ? 1 : -1) * state_.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
				if (glm::length2(dir) > DEADZONE_RADIUS_SQUARED) {
					direction_ = glm::normalize(dir);
				} else {
					direction_ = glm::zero<glm::vec2>();
				}
			} else {
				if (isButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP)) {
					direction_ = {0, -1};
				} else if (isButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)) {
					direction_ = {0, 1};
				}
				if (isButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT)) {
					direction_ = {1, 0};
				} else if (isButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT)) {
					direction_ = {-1, 0};
				}
			}
		}
	}
}

glm::vec2 Input::Gamepad::getDirection() const {
	return direction_;
}

bool Input::Gamepad::isCommandActive(Command command) const {
	switch (command) {
	case Command::Jump:
		return isButtonPressed(GLFW_GAMEPAD_BUTTON_A);
	case Command::Attack:
		return isButtonPressed(GLFW_GAMEPAD_BUTTON_B);
	}
	throw;
}

bool Input::Gamepad::anyButtonPressed() const {
	for (int i = GLFW_GAMEPAD_BUTTON_A; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i) {
		if (state_.buttons[i]) {
			return true;
		}
	}
	return false;
}

bool Input::Gamepad::anyButtonExceptArrowPressed() const {
	for (int i = GLFW_GAMEPAD_BUTTON_A; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i) {
		switch (i) {
		case GLFW_GAMEPAD_BUTTON_DPAD_UP:
		case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
		case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
		case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
			continue;
		default:
			if (state_.buttons[i]) {
				return true;
			}
		}
	}
	return false;
}

std::string Input::Gamepad::getName() const {
	return glfwGetGamepadName(id_);
}

bool Input::Gamepad::isDualShock4() const {
	return getName() == "Sony DualShock 4";
}

bool Input::Gamepad::isButtonPressed(int button) const {
	assert(GLFW_GAMEPAD_BUTTON_A <= button && button <= GLFW_GAMEPAD_BUTTON_LAST);
	return state_.buttons[static_cast<size_t>(button)] == GLFW_PRESS;
}

}