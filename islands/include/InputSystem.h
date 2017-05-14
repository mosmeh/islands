#pragma once

namespace islands {

class InputSystem {
public:
	InputSystem(const InputSystem&) = delete;
	InputSystem& operator=(const InputSystem&) = delete;
	virtual ~InputSystem();

	static InputSystem& getInstance();

	void setWindow(GLFWwindow* window);
	void registerKeyboardCallback(GLFWkeyfun callback);
	bool isKeyPressed(int key) const;

private:
	GLFWwindow* window_;
	std::vector<GLFWkeyfun> keyboardCallbacks_;

	InputSystem();

	static void keyboardCallback(GLFWwindow*, int, int, int, int);
};

}