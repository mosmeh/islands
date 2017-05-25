#pragma once

namespace islands {

class InputSystem {
public:
	enum class Command {
		Jump,
		Attack
	};

	InputSystem(const InputSystem&) = delete;
	InputSystem& operator=(const InputSystem&) = delete;
	virtual ~InputSystem();

	static InputSystem& getInstance();

	void update();
	void registerKeyboardCallback(GLFWkeyfun callback);
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

	class Joystick : public InputDevice {
	public:
		Joystick();
		virtual ~Joystick() = default;

		bool isPresent() const override;
		void update() override;
		glm::vec2 getDirection() const override;
		bool isCommandActive(Command command) const override;

	private:
		// DualShock4 用コンフィグ
		enum class Axis : size_t {
			LeftHorizontal = 0,
			LeftVertical = 1,
			Count
		};

		enum class Button : size_t {
			X = 1,
			O = 2,
			Up = 14,
			Right = 15,
			Down = 16,
			Left = 17,
			Count
		};

		int id_;
		bool present_;
		const unsigned char* buttons_;
		glm::vec2 direction_;

		bool isButtonPressed(Button button) const;
	} joystick_;

	GLFWwindow* window_;
	std::vector<GLFWkeyfun> keyboardCallbacks_;
	glm::vec2 direction_;

	InputSystem();

	static void keyboardCallback(GLFWwindow*, int, int, int, int);
};

}