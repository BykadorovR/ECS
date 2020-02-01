﻿#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sstream>

#include "Windows.h"
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

#include "Back.h"
#include "Label.h"
#include "Button.h"
#include "UserInputOperations.h"
#include "UserInputComponents.h"
#include "InteractionComponents.h"

std::shared_ptr<Scene> activeScene;
std::shared_ptr<DrawSystem> drawSystem;
void surfaceCreated() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	std::shared_ptr<SceneManager> sceneManager = std::make_shared<SceneManager>();
	activeScene = sceneManager->createScene("basic");

	std::shared_ptr<TextureAtlas> atlas = TextureManager::instance()->createAtlas(GL_RGBA, { 4096, 4096 });
	std::shared_ptr<TextureRaw> textureRaw = TextureManager::instance()->createTexture("../data/textures/air_hockey_surface.png", atlas->getAtlasID(), { 0, 0 }, { 1, 1 });
	atlas->initialize();

	std::shared_ptr<GlyphsLoader> glyphsLoader = std::make_shared<GlyphsLoader>("../data/fonts/arial.ttf",
		std::make_tuple<int, int>(static_cast<int>(*(L"А")),
			static_cast<int>(*(L"я"))));
	glyphsLoader->bufferSymbols(24);

	//TODO: shaders should be global
	std::shared_ptr<Shader> shader = std::make_shared<Shader>("../data/shaders/shader.vsh", "../data/shaders/shader.fsh");
	{
		std::shared_ptr<ButtonFactory> buttonFactory = std::make_shared<ButtonFactory>(activeScene);
		std::shared_ptr<Button> button = std::dynamic_pointer_cast<Button>(buttonFactory->createView());
		//TODO: rewrite to Back options and LabelOptions
		button->initialize({ 300, 200 }, { 100, 100 }, textureRaw->getTextureID(), L"I me name Me Button Bugton", {1, 0, 1, 1}, 1, glyphsLoader, shader);
		button->getBack()->getEntity()->createComponent<MouseComponent>();
		auto clickInside = std::make_shared<SimpleOperation>();

		auto clickExpression = std::make_shared<Expression>();
		clickExpression->addArgument(button->getBack()->getEntity()->getComponent<MouseComponent>(), "leftClickX");
		clickExpression->addArgument(button->getBack()->getEntity()->getComponent<MouseComponent>(), "leftClickY");
		clickExpression->addArgument(button->getBack()->getEntity()->getComponent<ObjectComponent>(), "positionX");
		clickExpression->addArgument(button->getBack()->getEntity()->getComponent<ObjectComponent>(), "positionY");
		clickExpression->addArgument(button->getBack()->getEntity()->getComponent<ObjectComponent>(), "sizeX");
		clickExpression->addArgument(button->getBack()->getEntity()->getComponent<ObjectComponent>(), "sizeY");
		clickExpression->setCondition("${0} > ${2} AND ${0} < ${2} + ${4} AND ${1} > ${3} AND ${1} < {$3} + ${5}");
		//clickExpression->setCondition("3 + 2 * ( 1 - 5 ) ^ 2 ^ 2 / 2 - 2");
		float result = 0;
		clickExpression->calculateExpression(&result);

		clickInside->setExpression(clickExpression);

		button->getBack()->getEntity()->createComponent<InteractionComponent>()->attachOperation(clickInside);
		button->getLabel()->setPageNumber(1);
		button->getLabel()->setLineSpacingCoeff(0.8);
		button->getLabel()->setTextAllignment({ TextAllignment::RIGHT, TextAllignment::CENTER });
	}

	drawSystem = std::make_shared<DrawSystem>();
}


void drawFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawSystem->update(activeScene->getEntityManager());
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
	mainWindow = glfwCreateWindow(std::get<0>(resolution), std::get<1>(resolution), "Main window", NULL /*monitor*/, NULL);
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

	glfwSetMouseButtonCallback(mainWindow, mousePressed);
	glfwSetKeyCallback(mainWindow, keyboardPress);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	surfaceCreated();
	while (!glfwWindowShouldClose(mainWindow)) {
		// OpenGL API calls go here...

		drawFrame();
		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
	}

	glfwDestroyWindow(mainWindow);
	glfwTerminate();	
	return 0;
}