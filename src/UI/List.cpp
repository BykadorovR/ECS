#include "List.h"
#include "GraphicComponents.h"
#include "CustomComponents.h"
#include "InteractionOperations.h"
#include "InteractionActions.h"
#include "InteractionComponents.h"
#include "UserInputComponents.h"

List::List(std::string name) {
	_viewName = name;
}

bool List::setPosition(std::tuple<float, float> position) {
	std::tuple<float, float> listItemSize = _views[0]->getEntity()->getComponent<ObjectComponent>()->getSize();
	_views[0]->getEntity()->getComponent<ObjectComponent>()->setMember("positionX", std::get<0>(position));
	_views[0]->getEntity()->getComponent<ObjectComponent>()->setMember("positionY", std::get<1>(position));
	for (int i = 1; i < _views.size(); i++) {
		auto prevPosition = _views[i - 1]->getEntity()->getComponent<ObjectComponent>()->getPosition();
		std::tuple<float, float> currentPosition = { std::get<0>(prevPosition), std::get<1>(prevPosition) + std::get<1>(listItemSize) };

		_views[i]->getEntity()->getComponent<ObjectComponent>()->setMember("positionX", std::get<0>(currentPosition));
		_views[i]->getEntity()->getComponent<ObjectComponent>()->setMember("positionY", std::get<1>(currentPosition));
	}
	return false;
}

bool List::setSize(std::tuple<float, float> size) {
	std::tuple<float, float> listItemSize = { std::get<0>(size), std::get<1>(size) / _views.size() };
	for (int i = 0; i < _views.size(); i++) {
		_views[i]->getEntity()->getComponent<ObjectComponent>()->setMember("sizeX", std::get<0>(listItemSize));
		_views[i]->getEntity()->getComponent<ObjectComponent>()->setMember("sizeY", std::get<1>(listItemSize));
	}
	return false;
}

bool List::addItem(std::string text) {
	_views[0]->getEntity()->getComponent<CustomStringArrayComponent>()->addCustomValue(text, "list");
	return false;
}

//views and items are different thigs. Views - labels, items - string (text)
bool List::initialize() {
	_views[0]->getEntity()->getComponent<CustomFloatComponent>()->addCustomValue(0, "page");
	for (int i = 0; i < _views.size(); i++) {
		_views[i]->initialize();
		_views[i]->getEntity()->getComponent<TextComponent>()->setAllignment({TextAllignment::CENTER, TextAllignment::CENTER});
		auto mapText = std::make_shared<ExpressionOperation>();
		//NOTE: we send list without index only because we use SIZE
		mapText->addArgument(_views[0]->getEntity()->getComponent<CustomStringArrayComponent>(), "list");
		mapText->addArgument(nullptr, std::to_string(i));
		mapText->addArgument(_views[0]->getEntity()->getComponent<CustomFloatComponent>(), "page");
		mapText->initializeOperation("${1} + ${2} < SIZE ${0}");
		_views[0]->getEntity()->createComponent<InteractionComponent>()->attachOperation(mapText, InteractionType::COMMON_END);
		auto setLine = std::make_shared<AssignAction>();
		setLine->addArgument(_views[i]->getEntity()->getComponent<TextComponent>(), "text");
		setLine->addArgument(_views[0]->getEntity()->getComponent<CustomStringArrayComponent>(), "list");
		setLine->addArgument(_views[0]->getEntity()->getComponent<CustomFloatComponent>(), "page");
		setLine->addArgument(nullptr, std::to_string(i));
		setLine->initializeAction("${0} SET ${1} AT ( ${2} + ${3} )");
		mapText->registerAction(setLine);
	}
	
	return false;
}

ListFactory::ListFactory(std::shared_ptr<Scene> activeScene, std::shared_ptr<ViewFactory> itemFactory) {
	_activeScene = activeScene;
	_itemFactory = itemFactory;
}

std::shared_ptr<View> ListFactory::createView(std::string name, std::shared_ptr<View> parent) {
	std::shared_ptr<List> list = std::make_shared<List>(name);
	list->setParent(parent);
	for (int i = 0; i < 3; i++) {
		list->addView(_itemFactory->createView("Label", list));
	}
	list->getViews()[0]->getEntity()->createComponent<CustomStringArrayComponent>();
	list->getViews()[0]->getEntity()->createComponent<CustomFloatComponent>();
	return list;
}