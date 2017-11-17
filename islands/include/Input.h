#pragma once

namespace islands {

class Input {
public:
	enum class Command {
		Jump,
		Attack
	};

	using KeyboardCallback = std::function<void(int, int)>;

	class Device {
	public:
		Device() = default;
		virtual ~Device() = default;

		virtual bool isPresent() const = 0;
		virtual void update() = 0;
		virtual glm::vec2 getDirection() const = 0;
		virtual bool isCommandActive(Command command) const = 0;
		virtual bool anyButtonPressed() const = 0;
		virtual bool anyButtonExceptArrowPressed() const = 0;
	};

	class Keyboard : public Device {
	public:
		Keyboard() = default;
		virtual ~Keyboard() = default;

		bool isPresent() const override;
		void update() override;
		glm::vec2 getDirection() const override;
		bool isCommandActive(Command command) const override;
		bool anyButtonPressed() const override;
		bool anyButtonExceptArrowPressed() const override;

	private:
		glm::vec2 direction_;

		bool isKeyPressed(int key) const;
	};

	class Gamepad : public Device {
	public:
		Gamepad();
		virtual ~Gamepad() = default;

		bool isPresent() const override;
		void update() override;
		glm::vec2 getDirection() const override;
		bool isCommandActive(Command command) const override;
		bool anyButtonPressed() const override;
		bool anyButtonExceptArrowPressed() const override;
		std::string getName() const;
		bool isDualShock4() const;

	private:
		int id_;
		bool present_;
		GLFWgamepadstate state_;
		glm::vec2 direction_;

		bool isButtonPressed(int button) const;
	};

	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
	virtual ~Input();

	static Input& getInstance();

	void update();
	void registerKeyboardCallback(const KeyboardCallback& callback);
	const glm::vec2 getDirection() const;
	bool isCommandActive(Command command) const;
	bool anyButtonPressed() const;
	bool anyButtonExceptArrowPressed() const;
	const Keyboard& getKeyboard() const;
	const Gamepad& getGamepad() const;

private:
	GLFWwindow* window_;
	std::vector<KeyboardCallback> keyboardCallbacks_;
	Keyboard keyboard_;
	Gamepad gamepad_;
	glm::vec2 direction_;
	std::unordered_set<int> pressedKeys_;

	Input();
};

}
