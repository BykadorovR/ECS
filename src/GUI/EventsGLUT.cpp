#include "Events.h"

void mousePress(int button, int state, int x, int y) {
	MouseEvent::instance().mousePress(button, state, x, y);
}

void MouseEvent::registerComponent(IMouseEvent* listener) {
	_listeners.push_back(listener);
}

void MouseEvent::mousePress(int button, int state, int x, int y) {
	switch (button) {
	case GLUT_LEFT_BUTTON:
		std::cout << "Left ";
		if (state == GLUT_DOWN) {
			std::cout << "Down " << x << " " << y << std::endl;
			for (auto listener : _listeners) {
				listener->mouseClickDownLeft(x, y);
			}
		}
	break;
	case GLUT_RIGHT_BUTTON:
		std::cout << "Right ";
		if (state == GLUT_DOWN) {
			std::cout << "Down " << x << " " << y << std::endl;
			for (auto listener : _listeners) {
				listener->mouseClickDownRight(x, y);
			}
		}
	break;
	}
}
