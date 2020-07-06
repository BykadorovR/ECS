﻿#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <chrono>
#include <thread>

#include "Interface.h"
#include "Grid.h"
#include "State.h"
#include "UserInputEvents.h"
#include "TextureAtlas.h"
#include "Texture.h"
#include "Entity.h"
#include "SceneManager.h"
#include "GraphicComponents.h"
#include "GraphicSystems.h"
#include "TextureManager.h"
#include "GlyphsLoader.h"
#include "Common.h"
#include "UIActions.h"

#include "Back.h"
#include "Label.h"
#include "Button.h"
#include "InteractionOperations.h"
#include "UserInputComponents.h"
#include "InteractionComponents.h"
#include "InteractionSystems.h"
#include "InteractionActions.h"
#include "UserInputSystems.h"
#include "Decorator.h"
#include "List.h"
#include "CustomComponents.h"

std::shared_ptr<Scene> activeScene;
std::shared_ptr<StateSystem> stateSystem;
std::shared_ptr<DrawSystem> drawSystem;
std::shared_ptr<InteractionSystem> interactionSystem;
std::shared_ptr<MouseSystem> mouseSystem;
std::shared_ptr<KeyboardSystem> keyboardSystem;

void attachShowOperations(std::shared_ptr<Entity> entity, std::shared_ptr<List> list, std::shared_ptr<ScrollerVerticalDecorator> decorator) {
	auto printOperation = std::make_shared<ExpressionOperation>();
	printOperation->addArgument(entity, "", "");
	printOperation->initializeOperation("CLICK ${0}");
	auto printAction = std::make_shared<PrintOperationsAction>();
	printAction->setList(list);
	printAction->setEntity(entity);
	printOperation->registerAction(printAction);
	entity->createComponent<InteractionComponent>()->attachOperation(printOperation, InteractionType::MOUSE_START);
}

void attachShowComponents(std::shared_ptr<Entity> entity, std::shared_ptr<List> list, std::shared_ptr<ScrollerVerticalDecorator> decorator) {
	list->getViews()[0]->getEntity()->createComponent<CustomFloatComponent>()->addCustomValue("currentEntity", -1);
	list->getViews()[0]->getEntity()->createComponent<CustomFloatArrayComponent>()->initializeEmpty("registeredEntities");

	auto printComponentsOperation = std::make_shared<ExpressionOperation>();
	printComponentsOperation->addArgument(entity, "", "");
	printComponentsOperation->initializeOperation("CLICK ${0}");
	auto printComponentsAction = std::make_shared<PrintComponentsAction>();
	printComponentsAction->setList(list);
	printComponentsAction->setEntity(entity);
	printComponentsOperation->registerAction(printComponentsAction);
	entity->createComponent<InteractionComponent>()->attachOperation(printComponentsOperation, InteractionType::MOUSE_START);

	auto clearComponentsOperation = std::make_shared<ExpressionOperation>();
	clearComponentsOperation->addArgument(entity, "", "");
	std::string listIndexes = clearComponentsOperation->addArgument(list->getEntities());
	std::string decoratorIndexes = clearComponentsOperation->addArgument(decorator->getEntities());
	clearComponentsOperation->initializeOperation("! ( DOUBLE_CLICK ${0} ) AND ! ( CLICK ${"+listIndexes+"} ) AND ! ( CLICK ${"+decoratorIndexes+"} )");
	auto clearComponentsAction = std::make_shared<ClearComponentsAction>();
	clearComponentsAction->setList(list);
	clearComponentsAction->setEntity(entity);
	clearComponentsOperation->registerAction(clearComponentsAction);
	entity->createComponent<InteractionComponent>()->attachOperation(clearComponentsOperation, InteractionType::MOUSE_START);
}

void surfaceCreated() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	std::shared_ptr<SceneManager> sceneManager = std::make_shared<SceneManager>();
	activeScene = sceneManager->createScene("basic");

	std::shared_ptr<TextureAtlas> atlas = TextureManager::instance()->createAtlas(GL_RGBA, { 4096, 4096 });
	std::shared_ptr<TextureRaw> textureRaw = TextureManager::instance()->createTexture("../data/textures/air_hockey_surface.png", atlas->getAtlasID(), { 0, 0 }, { 1, 1 });
	atlas->initialize();

	//1040 - 1103 for cyrillic: static_cast<int>(*(L"")))
	GlyphsLoader::instance().initialize("../data/fonts/arial.ttf", { 1040, 1103 });
	GlyphsLoader::instance().bufferSymbols(15, 15);

	std::shared_ptr<Shader> shader = std::make_shared<Shader>("../data/shaders/shader.vsh", "../data/shaders/shader.fsh");
	ShaderStore::instance()->addShader("texture", shader);
	std::shared_ptr<MainInterface> mainInterface = std::make_shared<MainInterface>();
	mainInterface->initialize(activeScene);
	
	std::shared_ptr<BackFactory> backFactory = std::make_shared<BackFactory>(activeScene);
	std::shared_ptr<Back> back = std::dynamic_pointer_cast<Back>(backFactory->createView());
	back->initialize();
	back->setPosition({ 500, 50 });
	back->setSize({ 100, 100 });
	back->setColorMask({ 0, 0, 0, 0 });
	back->setColorAddition({ 0.5, 0, 0.5, 0.5 });

	std::shared_ptr<LabelFactory> labelFactory = std::make_shared<LabelFactory>(activeScene);
	std::shared_ptr<Label> label = std::dynamic_pointer_cast<Label>(labelFactory->createView());
	label->initialize();
	label->setPosition({ 500, 50 });
	label->setSize({ 100, 100 });
	label->setText("Hello");
	label->setEditable(true);
	std::shared_ptr<ScrollerVerticalDecoratorFactory> scrollerVerticalDecoratorFactory = std::make_shared<ScrollerVerticalDecoratorFactory>(activeScene);
	std::shared_ptr<ScrollerVerticalDecorator> scrollerDecoratorVerticalLabel = std::dynamic_pointer_cast<ScrollerVerticalDecorator>(scrollerVerticalDecoratorFactory->createView("ScrollerDecoratorVertical", label));
	scrollerDecoratorVerticalLabel->initialize();

	stateSystem = std::make_shared<StateSystem>();
	stateSystem->setEntityManager(activeScene->getEntityManager());
	interactionSystem = std::make_shared<InteractionSystem>();
	interactionSystem->setEntityManager(activeScene->getEntityManager());
	mouseSystem = std::make_shared<MouseSystem>();
	mouseSystem->setEntityManager(activeScene->getEntityManager());
	keyboardSystem = std::make_shared<KeyboardSystem>();
	keyboardSystem->setEntityManager(activeScene->getEntityManager());
	drawSystem = std::make_shared<DrawSystem>();
	drawSystem->setEntityManager(activeScene->getEntityManager());

}


void drawFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Interaction should be called before graphic so we avoid use cases when sprite rendered in some coord but should be moved out due to some trigger
	//so we observe "false" moves to coord under triger and instant move out
	interactionSystem->update(InteractionType::COMMON_START);
	stateSystem->update(InteractionType::COMMON_START);
	mouseSystem->update(InteractionType::MOUSE_START);
	keyboardSystem->update(InteractionType::KEYBOARD_START);

	drawSystem->update();

	keyboardSystem->update(InteractionType::KEYBOARD_END);
	mouseSystem->update(InteractionType::MOUSE_END);
	stateSystem->update(InteractionType::COMMON_END);
	interactionSystem->update(InteractionType::COMMON_END);
}

//need to separate to cpp and h due to a lot of dependencies between classes
int main(int argc, char **argv) {
	FreeConsole();
	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Core profile deprecate all fixed function API calls

	GLFWwindow* mainWindow;
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	//mainWindow = glfwCreateWindow(std::get<0>(currentResolution), std::get<1>(currentResolution), "Main window", monitor, NULL);
	mainWindow = glfwCreateWindow(std::get<0>(currentResolution), std::get<1>(currentResolution), "Main window", NULL, NULL);
	if (mainWindow == NULL) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(mainWindow);

	glewExperimental = true;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}
	//disable VSync
	glfwSwapInterval(0);

	glfwSetMouseButtonCallback(mainWindow, mousePressed);
	glfwSetKeyCallback(mainWindow, keyboardPress);
	glfwSetCharCallback(mainWindow, textInput);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	surfaceCreated();
	while (!glfwWindowShouldClose(mainWindow)) {
		std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
		// OpenGL API calls go here...
		//TODO: separate display and bussness logic (display will be at 60 FPS, but business w/o restriction)
		drawFrame();
		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - startTime).count();
		OUT_STREAM("Elapsed: " + std::to_string(elapsed) + " ms\n");
	}

	glfwDestroyWindow(mainWindow);
	glfwTerminate();	
	return 0;
}