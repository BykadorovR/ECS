#pragma once
#include <GLFW/glfw3.h>
#include "vector"

/*
GLFW callback
*/
void keyboardPress(GLFWwindow* window, int key, int scancode, int action, int mods);

class IKeyboardEvent {
public:
	virtual void keyboardPressed(int character, int action, int mode) = 0;
};

/*
Singleton
*/
class KeyboardEvent {
private:
	std::vector<IKeyboardEvent* > _listeners;
public:
	static KeyboardEvent& instance() {
		static KeyboardEvent object;
		return object;
	}

	void registerComponent(IKeyboardEvent* listener);
	void unregisterComponent(IKeyboardEvent* listener);

	void keyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mods);
};

/*
GLFW callback
*/
void mousePress(GLFWwindow* window, int button, int action, int mods);

class IMouseEvent {
public:
	virtual void mouseClickDownLeft(int x, int y) = 0;
	virtual void mouseClickDownRight(int x, int y) = 0;
};

/*
Singleton
*/
class MouseEvent {
private:

	std::vector<IMouseEvent* > _listeners;
public:
	static MouseEvent& instance() {
		static MouseEvent object;
		return object;
	}

	void registerComponent(IMouseEvent* listener);
	void unregisterComponent(IMouseEvent* listener);

	void mousePress(GLFWwindow* window, int button, int action, int mods);
};
