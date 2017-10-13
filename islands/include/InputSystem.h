#pragma once

namespace islands {

class InputSystem {
public:
	enum class Command {
		Jump,
		Attack
	};

	using KeyboardCallback = std::function<void(int, int)>;

	InputSystem(const InputSystem&) = delete;
	InputSystem& operator=(const InputSystem&) = delete;
	virtual ~InputSystem();

	static InputSystem& getInstance();

	void update();
	void registerKeyboardCallback(const KeyboardCallback& callback);
	const glm::vec2 getDirection() const;
	bool isCommandActive(Command command) const;

private:
	class InputDevice {
	public:
		InputDevice() = default;
		virtual ~InputDevice() = default;

		virtual bool isPresent() const = 0;
		virtual void update() = 0;
		virtual glm::vec2 getDirection() const = 0;
		virtual bool isCommandActive(Command command) const = 0;
	};

	class Keyboard : public InputDevice {
	public:
		virtual ~Keyboard() = default;

		bool isPresent() const override;
		void update() override;
		glm::vec2 getDirection() const override;
		bool isCommandActive(Command command) const override;

	private:
		glm::vec2 direction_;

		bool isKeyPressed(int key) const;
	} keyboard_;

	class Gamepad : public InputDevice {
	public:
		Gamepad();
		virtual ~Gamepad() = default;

		bool isPresent() const override;
		void update() override;
		glm::vec2 getDirection() const override;
		bool isCommandActive(Command command) const override;

	private:
		int id_;
		bool present_;
		GLFWgamepadstate state_;
		glm::vec2 direction_;

		bool isButtonPressed(int button) const;
	} gamepad_;

	GLFWwindow* window_;
	std::vector<KeyboardCallback> keyboardCallbacks_;
	glm::vec2 direction_;

	InputSystem();
};

}