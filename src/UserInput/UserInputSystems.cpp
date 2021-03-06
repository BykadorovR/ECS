#include "GraphicComponents.h"
#include "UserInputSystems.h"
#include "Operation.h"
#include "InteractionComponents.h"
#include "UserInputComponents.h"
#include <locale>
#include <codecvt>

MouseSystem::MouseSystem() {
	MouseEvent::instance().registerComponent(this);
}

void MouseSystem::mouseDoubleClickDownLeft(int x, int y) {
	//first update coords in mouse components
	for (auto entity : _entityManager->getEntities()) {
		if (std::get<1>(entity) == EntityState::ENTITY_UNREGISTERED)
			continue;

		auto mouseComponent = std::get<0>(entity)->getComponent<MouseComponent>();
		if (mouseComponent) {
			mouseComponent->setMember("leftClickX", x);
			mouseComponent->setMember("leftClickY", y);
			mouseComponent->setMember("doubleClick", true);
		}
	}

	_needUpdate = { true, true };
}

void MouseSystem::mouseClickDownLeft(int x, int y) {
	//first update coords in mouse components
	for (auto entity : _entityManager->getEntities()) {
		if (std::get<1>(entity) == EntityState::ENTITY_UNREGISTERED)
			continue;

		auto mouseComponent = std::get<0>(entity)->getComponent<MouseComponent>();
		if (mouseComponent) {
			mouseComponent->setMember("leftClickX", x);
			mouseComponent->setMember("leftClickY", y);
			mouseComponent->setMember("doubleClick", false);
		}
	}

	_needUpdate = { true, true };
}

void MouseSystem::update(InteractionType type) {
	if (std::get<0>(_needUpdate) || std::get<1>(_needUpdate)) {
		for (auto entity : _entityManager->getEntities()) {
			if (std::get<1>(entity) == EntityState::ENTITY_UNREGISTERED)
				continue;

			auto interactionComponent = std::get<0>(entity)->getComponent<InteractionComponent>();
			auto mouseComponent = std::get<0>(entity)->getComponent<MouseComponent>();
			if (interactionComponent) {
				std::vector<std::tuple<std::shared_ptr<Operation>, InteractionType> > operations = interactionComponent->getOperations();
				for (auto operation : operations) {
					if (std::get<1>(operation) == type && mouseComponent && std::get<0>(operation)->checkOperation()) {
						for (auto action : std::get<0>(operation)->getActions()) {
							action->doAction();
						}
					}
				}
			}
		}
		if (type == InteractionType::MOUSE_START)
			std::get<0>(_needUpdate) = false;
		else if (type == InteractionType::MOUSE_END)
			std::get<1>(_needUpdate) = false;
	}
}

void MouseSystem::mouseClickDownRight(int x, int y) {
}

MouseSystem::~MouseSystem() {
	MouseEvent::instance().unregisterComponent(this);
}

KeyboardSystem::KeyboardSystem() {
	KeyboardEvent::instance().registerComponent(this);
}

void KeyboardSystem::keyboardPressed(int key, int action, int mode) {
	//Here should be handled HARDWARE keys, so "W" key is "W" on ALL keyboard layouts
	//first update coords in keyboard components
	if (action == GLFW_PRESS) {
		for (auto entity : _entityManager->getEntities()) {
			if (std::get<1>(entity) == EntityState::ENTITY_UNREGISTERED)
				continue;

			auto keyboardComponent = std::get<0>(entity)->getComponent<KeyboardComponent>();
			if (keyboardComponent) {
				//replace symbol with code
				keyboardComponent->setMember("symbol", "");
				keyboardComponent->setMember("code", key);
			}
		}
		_needUpdate = { true, true };
	}
}

void KeyboardSystem::textInput(unsigned int character) {
	//Here we handle UTF8 characters pressed. IMPORTANT: it's keyboard layout dependent function! So "W" on US and French keyboards are on different places
	//We don't need to convert character from multibyte to UTF8 because it's already UTF8
	auto k = (wchar_t)character;
	std::wstring wideText;
	wideText.push_back(k);

	std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::string text = converter.to_bytes(wideText);
	//first update coords in keyboard components
	for (auto entity : _entityManager->getEntities()) {
		auto keyboardComponent = std::get<0>(entity)->getComponent<KeyboardComponent>();
		if (keyboardComponent) {
			keyboardComponent->setMember("symbol", text);
		}
	}

	//check keyboard related operations
	_needUpdate = { true, true };
}

void KeyboardSystem::update(InteractionType type) {
	if (std::get<0>(_needUpdate) || std::get<1>(_needUpdate)) {
		for (auto entity : _entityManager->getEntities()) {
			auto interactionComponent = std::get<0>(entity)->getComponent<InteractionComponent>();
			auto keyboardComponent = std::get<0>(entity)->getComponent<KeyboardComponent>();
			if (interactionComponent) {
				std::vector<std::tuple<std::shared_ptr<Operation>, InteractionType> > operations = interactionComponent->getOperations();
				for (auto operation : operations) {
					if (std::get<1>(operation) == type && keyboardComponent && std::get<0>(operation)->checkOperation()) {
						for (auto action : std::get<0>(operation)->getActions()) {
							action->doAction();
						}
					}
				}
			}
		}
		if (type == InteractionType::KEYBOARD_START)
			std::get<0>(_needUpdate) = false;
		else if (type == InteractionType::KEYBOARD_END)
			std::get<1>(_needUpdate) = false;
	}
}

KeyboardSystem::~KeyboardSystem() {
	KeyboardEvent::instance().unregisterComponent(this);
}