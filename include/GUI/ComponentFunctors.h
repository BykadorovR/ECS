#pragma once
#include "Component.h"
#include "UIComponent.h"
#include "GraphicComponent.h"
#include "Texture.h"
#include "Camera.h"
#include <sstream>
#include "TextLoader.h"
#include <string>
#include "TextHelper.h"

class ComponentFunctor : public ITextEvent {
public:
	virtual void configureFunctor(std::shared_ptr<Entity> targetEntity) = 0;
	virtual std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) = 0;
	virtual void removeFunctor(std::shared_ptr<Entity> targetEntity) = 0;
	virtual void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) = 0;
	virtual int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) = 0;
};

extern std::map<std::string, std::shared_ptr<ComponentFunctor> > componentFunctors;

class TextureComponentFunctor : public ComponentFunctor {
public:
	void configureFunctor(std::shared_ptr<Entity> targetEntity) {
		int x = TextHelper::instance()->getX();
		int y = TextHelper::instance()->getY();
		int height = TextHelper::instance()->getHeight();
		int width = TextHelper::instance()->getWidth();
		float size = TextHelper::instance()->getSize();



		int index = 1;
		std::shared_ptr<TextureChangeEvent> accept = std::make_shared<TextureChangeEvent>();

		std::shared_ptr<TextCallback> callbackTextureID = std::make_shared<TextCallback>();
		callbackTextureID->setValue(&accept->_id);
		TextHelper::instance()->getValue(callbackTextureID, "textureID", x, y + height * index++, width, height, size);
		std::shared_ptr<TextCallback> callbackTextureOrder = std::make_shared<TextCallback>();
		callbackTextureOrder->setValue(&accept->_order);
		TextHelper::instance()->getValue(callbackTextureOrder, "texture order", x, y + height * index++, width, height, size);
		std::shared_ptr<TextCallback> callbackTextureLatency = std::make_shared<TextCallback>();
		callbackTextureLatency->setValue(&accept->_latency);
		TextHelper::instance()->getValue(callbackTextureLatency, "texture latency", x, y + height * index++, width, height, size);
		auto entity = TextHelper::instance()->createText("Submit", x, y + height * index++, width, height, size, false);
		std::shared_ptr<ClickInsideComponent> clickInsideComponent = entity->getComponent<ClickInsideComponent>();
		clickInsideComponent->_event = std::make_pair(accept, targetEntity);
		TextHelper::instance()->attachText(entity);
	}
	//TODO: How to use atlas and textures dynamically
	std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<TextureComponent> textureComponent(new TextureComponent());
		std::shared_ptr<TextComponent> textComponent = targetEntity->getComponent<TextComponent>();
		if (textComponent) {
			targetEntity->removeComponent<TextComponent>();
			std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
			Shader shader;
			GLuint program;
			program = shader.buildProgramFromAsset("../data/shaders/shader.vsh", "../data/shaders/shader.fsh");
			objectComponent->initialize(objectComponent->getSceneX(), objectComponent->getSceneY(), objectComponent->getWidth(), objectComponent->getHeight(), program);
		}

		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		textureComponent->initialize(objectComponent->_program);
		return textureComponent;
	}

	void removeFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<TextureComponent> textureComponent = targetEntity->getComponent<TextureComponent>();
		if (textureComponent) {
			textureComponent->_solid = 1;
			textureComponent->_texture = nullptr;
		}
	}

	void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) {
		int entityID = targetEntity->_index;
		std::shared_ptr<TextureComponent> textureComponent = targetEntity->getComponent<TextureComponent>();
		if (textureComponent && textureComponent->_texture != nullptr) {
			save->_jsonFile["Entity"][std::to_string(entityID)]["TextureComponent"]["textureID"] = textureComponent->_texture->getTextureID();
		}
		if (textureComponent && textureComponent->_tilesOrder.size() != 0)
			save->_jsonFile["Entity"][std::to_string(entityID)]["TextureComponent"]["tilesOrder"] = textureComponent->_tilesOrder;
		
		if (textureComponent && textureComponent->_tilesLatency.size() != 0)
			save->_jsonFile["Entity"][std::to_string(entityID)]["TextureComponent"]["tilesLatency"] = textureComponent->_tilesLatency;
	}

	int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) {
		int entityID = targetEntity->_index;
		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		if (!objectComponent)
			return 1;

		auto program = objectComponent->_program;
		if (!jsonFile["TextureComponent"].empty()) {
			std::shared_ptr<TextureComponent> textureComponent = targetEntity->getComponent<TextureComponent>();
			if (!textureComponent) {
				textureComponent = std::shared_ptr<TextureComponent>(new TextureComponent());
				targetEntity->addComponent(textureComponent);
			}
			
			int textureID = jsonFile["TextureComponent"]["textureID"];
			textureComponent->initialize(textureID, program);
			
			if (!jsonFile["TextureComponent"]["tilesOrder"].empty()) {
				std::vector<float> tilesOrder = jsonFile["TextureComponent"]["tilesOrder"];
				textureComponent->_tilesOrder = tilesOrder;
			}
			if (!jsonFile["TextureComponent"]["tilesLatency"].empty()) {
				std::vector<float> tilesLatency = jsonFile["TextureComponent"]["tilesLatency"];
				textureComponent->_tilesLatency = tilesLatency;
			}
		}
		else if (jsonFile["TextComponent"].empty())
		{
			std::shared_ptr<TextureComponent> textureComponent = targetEntity->getComponent<TextureComponent>();
			if (!textureComponent) {
				textureComponent = std::shared_ptr<TextureComponent>(new TextureComponent());
				targetEntity->addComponent(textureComponent);
			}
			textureComponent->initialize(program);
		}
		return 0;
	}
};

class ObjectComponentFunctor : public ComponentFunctor {
	void configureFunctor(std::shared_ptr<Entity> targetEntity) {
		int x = TextHelper::instance()->getX();
		int y = TextHelper::instance()->getY();
		int height = TextHelper::instance()->getHeight();
		int width = TextHelper::instance()->getWidth();
		float size = TextHelper::instance()->getSize();
		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		std::shared_ptr<TextCallback> callbackX = std::make_shared<TextCallback>();
		callbackX->setValue(&objectComponent->_sceneX);
		TextHelper::instance()->getValue(callbackX, "sceneX", x, y, width, height, size);
		std::shared_ptr<TextCallback> callbackY = std::make_shared<TextCallback>();
		callbackY->setValue(&objectComponent->_sceneY);
		TextHelper::instance()->getValue(callbackY, "sceneY", x, y + height * 1, width, height, size);
		std::shared_ptr<TextCallback> callbackWidth = std::make_shared<TextCallback>();
		callbackWidth->setValue(&objectComponent->_objectWidth);
		TextHelper::instance()->getValue(callbackWidth, "Width", x, y + height * 2, width, height, size);
		std::shared_ptr<TextCallback> callbackHeight = std::make_shared<TextCallback>();
		callbackHeight->setValue(&objectComponent->_objectHeight);
		TextHelper::instance()->getValue(callbackHeight, "Height", x, y + height * 3, width, height, size);
		std::shared_ptr<TextCallback> callbackScale = std::make_shared<TextCallback>();
		callbackScale->setValue(&objectComponent->_scale);
		TextHelper::instance()->getValue(callbackScale, "Scale", x, y + height * 4, width, height, size);
		std::shared_ptr<TextCallback> callbackCameraSpeed = std::make_shared<TextCallback>();
		callbackCameraSpeed->setValue(&objectComponent->_cameraCoefSpeed);
		TextHelper::instance()->getValue(callbackCameraSpeed, "CameraCoefSpeed", x, y + height * 5, width, height, size);
		
	}

	//this component can't be added to Entity, so it's just a stub
	std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<ObjectComponent> objectComponent(new ObjectComponent());
		return objectComponent;
	}

	void removeFunctor(std::shared_ptr<Entity> targetEntity) {
		targetEntity->removeComponent<ObjectComponent>();
	}
	void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) {
		int entityID = targetEntity->_index;
		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		if (!objectComponent)
			return;

		save->_jsonFile["Entity"][std::to_string(entityID)]["ObjectComponent"]["cameraCoefSpeed"] = objectComponent->_cameraCoefSpeed;
		save->_jsonFile["Entity"][std::to_string(entityID)]["ObjectComponent"]["sceneCoord"] = {objectComponent->getSceneX(), objectComponent->getSceneY()};
		save->_jsonFile["Entity"][std::to_string(entityID)]["ObjectComponent"]["objectSize"] = { objectComponent->getWidth(), objectComponent->getHeight() };
		save->_jsonFile["Entity"][std::to_string(entityID)]["ObjectComponent"]["scale"] = objectComponent->_scale;
	}

	int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) {
		int entityID = targetEntity->_index;
		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		if (!objectComponent) {
			objectComponent = std::shared_ptr<ObjectComponent>(new ObjectComponent());
			targetEntity->addComponent(objectComponent);
		}

		float cameraSpeed = jsonFile["ObjectComponent"]["cameraCoefSpeed"];
		float sceneX = jsonFile["ObjectComponent"]["sceneCoord"][0];
		float sceneY = jsonFile["ObjectComponent"]["sceneCoord"][1];
		float objectWidth = jsonFile["ObjectComponent"]["objectSize"][0];
		float objectHeight = jsonFile["ObjectComponent"]["objectSize"][1];
		float scale = jsonFile["ObjectComponent"]["scale"];
		Shader shader;
		GLuint program;
		if (jsonFile["TextComponent"].empty()) {
			program = shader.buildProgramFromAsset("../data/shaders/shader.vsh", "../data/shaders/shader.fsh");
			objectComponent->initialize(sceneX, sceneY, objectWidth, objectHeight, program);
		}
		else {
			program = shader.buildProgramFromAsset("../data/shaders/text.vsh", "../data/shaders/text.fsh");
			objectComponent->initializeText(sceneX, sceneY, objectWidth, objectHeight, program);
		}
		objectComponent->_cameraCoefSpeed = cameraSpeed;
		objectComponent->_scale = scale;
		return 0;
	}
};

class TextComponentFunctor : public ComponentFunctor {
	void configureFunctor(std::shared_ptr<Entity> targetEntity) {
		int x = TextHelper::instance()->getX();
		int y = TextHelper::instance()->getY();
		int height = TextHelper::instance()->getHeight();
		int width = TextHelper::instance()->getWidth();
		float size = TextHelper::instance()->getSize();
		

		std::shared_ptr<TextComponent> textComponent = targetEntity->getComponent<TextComponent>();
		std::shared_ptr<TextCallback> callbackText = std::make_shared<TextCallback>();
		callbackText->setValue(&textComponent->_text);
		callbackText->setValue(&textComponent->_textBack);
		TextHelper::instance()->getValue(callbackText, "Enter text to display", x, y, width, height, size);
		
		std::shared_ptr<TextCallback> callbackScale = std::make_shared<TextCallback>();
		callbackScale->setValue(&textComponent->_scale);
		TextHelper::instance()->getValue(callbackScale, "Scale", x, y + height * 1, width, height, size);

		std::shared_ptr<TextCallback> callbackRGB = std::make_shared<TextCallback>();
		callbackRGB->setValue(&textComponent->_color);
		TextHelper::instance()->getValue(callbackRGB, "RGB", x, y + height * 2, width, height, size);
	}
	//this component can't be added to Entity, so it's just a stub
	std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<TextureComponent> textureComponent = targetEntity->getComponent<TextureComponent>();
		if (textureComponent)
			targetEntity->removeComponent<TextureComponent>();

		std::shared_ptr<TextComponent> textComponent(new TextComponent());
		std::shared_ptr<TextLoader> textLoader = std::make_shared<TextLoader>();
		textLoader->bufferSymbols(48);
		textComponent->initialize();
		textComponent->_loader = textLoader;

		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		Shader shader;
		GLuint program;
		program = shader.buildProgramFromAsset("../data/shaders/text.vsh", "../data/shaders/text.fsh");
		objectComponent->_program = program;
		return textComponent;
	}

	void removeFunctor(std::shared_ptr<Entity> targetEntity) {
		targetEntity->removeComponent<TextComponent>();
	}
	void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) {
		int entityID = targetEntity->_index;
		std::shared_ptr<TextComponent> textComponent = targetEntity->getComponent<TextComponent>();
		if (!textComponent)
			return;

		std::shared_ptr<GroupEntitiesComponent> groupComponent = targetEntity->getComponent<GroupEntitiesComponent>();
		if (!groupComponent)
			return;

		save->_jsonFile["Entity"][std::to_string(entityID)]["TextComponent"]["text"] = textComponent->_text;
		save->_jsonFile["Entity"][std::to_string(entityID)]["TextComponent"]["scale"] = textComponent->_scale;
		save->_jsonFile["Entity"][std::to_string(entityID)]["TextComponent"]["color"] = textComponent->_color;
		save->_jsonFile["Entity"][std::to_string(entityID)]["TextComponent"]["type"] = textComponent->_type;
		
	}

	int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) {
		if (jsonFile["TextComponent"].empty())
			return -1;

		int entityID = targetEntity->_index;
		std::shared_ptr<TextComponent> textComponent = targetEntity->getComponent<TextComponent>();
		if (!textComponent) {
			textComponent = std::shared_ptr<TextComponent>(new TextComponent());
			targetEntity->addComponent(textComponent);
		}

		std::shared_ptr<TextLoader> loader = std::make_shared<TextLoader>();
		loader->bufferSymbols(48);
		std::string text = jsonFile["TextComponent"]["text"];
		float scale = jsonFile["TextComponent"]["scale"];
		std::vector<float> color = jsonFile["TextComponent"]["color"];
		int type = jsonFile["TextComponent"]["type"];
		textComponent->initialize(loader, text, scale, color, (TextType) type);
		return 0;
	}
};

class ClickInsideFunctor : public ComponentFunctor {
	void configureFunctor(std::shared_ptr<Entity> targetEntity) {
		int x = TextHelper::instance()->getX();
		int y = TextHelper::instance()->getY();
		int height = TextHelper::instance()->getHeight();
		int width = TextHelper::instance()->getWidth();
		float size = TextHelper::instance()->getSize();

		std::shared_ptr<ClickInsideComponent> clickInsideComponent = targetEntity->getComponent<ClickInsideComponent>();
		std::shared_ptr<TextCallback> callbackMove = std::make_shared<TextCallback>();
		callbackMove->setValue(&clickInsideComponent->_moveToByClick);
		TextHelper::instance()->getValue(callbackMove, "Move toward this object allowed? (0, 1)", x, y, width, height, size);
	}
	//this component can't be added to Entity, so it's just a stub
	std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<ClickInsideComponent> clickComponent(new ClickInsideComponent());
		clickComponent->initialize();
		return clickComponent;
	}

	void removeFunctor(std::shared_ptr<Entity> targetEntity) {
		targetEntity->removeComponent<ClickInsideComponent>();
	}
	void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) {
		int entityID = targetEntity->_index;
		std::shared_ptr<ClickInsideComponent> clickInsideComponent = targetEntity->getComponent<ClickInsideComponent>();
		if (!clickInsideComponent)
			return;

		save->_jsonFile["Entity"][std::to_string(entityID)]["ClickInsideComponent"]["moveToByClick"] = clickInsideComponent->_moveToByClick;
		save->_jsonFile["Entity"][std::to_string(entityID)]["ClickInsideComponent"]["event"] = clickInsideComponent->_event.first ? true : false;
	}

	int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) {
		int entityID = targetEntity->_index;
		if (jsonFile["ClickInsideComponent"].empty())
			return -1;

		std::shared_ptr<ClickInsideComponent> clickInsideComponent = targetEntity->getComponent<ClickInsideComponent>();
		if (!clickInsideComponent) {
			clickInsideComponent = std::shared_ptr<ClickInsideComponent>(new ClickInsideComponent());
			targetEntity->addComponent(clickInsideComponent);
		}

		bool moveToByClick = jsonFile["ClickInsideComponent"]["moveToByClick"];
		clickInsideComponent->initialize(moveToByClick);

		bool event = jsonFile["ClickInsideComponent"]["event"];
		if (event) {
			std::shared_ptr<ComponentTextEvent> textEvent = std::make_shared<ComponentTextEvent>();
			clickInsideComponent->_event = std::make_pair(textEvent, targetEntity);
			targetEntity->addComponent(clickInsideComponent);
		}

		return 0;
	}
};

class GroupEntitiesFunctor : public ComponentFunctor {
	void configureFunctor(std::shared_ptr<Entity> targetEntity) {
		int x = TextHelper::instance()->getX();
		int y = TextHelper::instance()->getY();
		int height = TextHelper::instance()->getHeight();
		int width = TextHelper::instance()->getWidth();
		float size = TextHelper::instance()->getSize();
		int index = 0;

		std::shared_ptr<GroupEntitiesComponent> groupComponent = targetEntity->getComponent<GroupEntitiesComponent>();
		std::shared_ptr<TextCallback> callbackName = std::make_shared<TextCallback>();
		callbackName->setValue(&groupComponent->_groupName);
		TextHelper::instance()->getValue(callbackName, "Group name", x, y + height * index++, width, height, size);

		std::shared_ptr<TextCallback> callbackID = std::make_shared<TextCallback>();
		callbackID->setValue((int*) &groupComponent->_groupNumber);
		TextHelper::instance()->getValue(callbackID, "Group ID", x, y + height * index++, width, height, size);
	}
	//this component can't be added to Entity, so it's just a stub
	std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<GroupEntitiesComponent> groupEntitiesComponent(new GroupEntitiesComponent());
		groupEntitiesComponent->initialize(0, "Default");
		return groupEntitiesComponent;
	}

	void removeFunctor(std::shared_ptr<Entity> targetEntity) {
		targetEntity->removeComponent<GroupEntitiesComponent>();
	}
	void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) {
		int entityID = targetEntity->_index;
		std::shared_ptr<GroupEntitiesComponent> groupEntitiesComponent = targetEntity->getComponent<GroupEntitiesComponent>();
		if (!groupEntitiesComponent)
			return;

		save->_jsonFile["Entity"][std::to_string(entityID)]["GroupEntitiesComponent"]["groupNumber"] = groupEntitiesComponent->_groupNumber;
		save->_jsonFile["Entity"][std::to_string(entityID)]["GroupEntitiesComponent"]["groupName"] = groupEntitiesComponent->_groupName;
	}

	int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) {
		int entityID = targetEntity->_index;
		if (jsonFile["GroupEntitiesComponent"].empty())
			return -1;

		std::shared_ptr<GroupEntitiesComponent> groupEntitiesComponent = targetEntity->getComponent<GroupEntitiesComponent>();
		if (!groupEntitiesComponent) {
			groupEntitiesComponent = std::shared_ptr<GroupEntitiesComponent>(new GroupEntitiesComponent());
			targetEntity->addComponent(groupEntitiesComponent);
		}

		uint32_t groupNumber = jsonFile["GroupEntitiesComponent"]["groupNumber"];
		std::string groupName = jsonFile["GroupEntitiesComponent"]["groupName"];
		groupEntitiesComponent->initialize(groupNumber, groupName);
		return 0;
	}
};

class InteractionAddToEntityFunctor : public ComponentFunctor {
	void configureFunctor(std::shared_ptr<Entity> targetEntity) {
		int x = TextHelper::instance()->getX();
		int y = TextHelper::instance()->getY();
		int height = TextHelper::instance()->getHeight();
		int width = TextHelper::instance()->getWidth();
		float size = TextHelper::instance()->getSize();

		std::shared_ptr<InteractionAddToEntityComponent> addComponent = targetEntity->getComponent<InteractionAddToEntityComponent>();
		std::shared_ptr<TextCallback> callbackObject = std::make_shared<TextCallback>();
		callbackObject->setValue((int*) &addComponent->_interactionMember);
		TextHelper::instance()->getValue(callbackObject, "Object = 0, Subject = 1", x, y, width, height, size);
	}
	//this component can't be added to Entity, so it's just a stub
	std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<InteractionAddToEntityComponent> interactionAddToEntityComponent(new InteractionAddToEntityComponent());
		return interactionAddToEntityComponent;
	}

	void removeFunctor(std::shared_ptr<Entity> targetEntity) {
		targetEntity->removeComponent<InteractionAddToEntityComponent>();
	}
	void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) {
		int entityID = targetEntity->_index;
		std::shared_ptr<InteractionAddToEntityComponent> interactionAddToEntityComponent = targetEntity->getComponent<InteractionAddToEntityComponent>();
		if (!interactionAddToEntityComponent)
			return;

		save->_jsonFile["Entity"][std::to_string(entityID)]["InteractionAddToEntityComponent"]["interactMember"] = interactionAddToEntityComponent->_interactionMember;
	}

	int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) {
		int entityID = targetEntity->_index;
		if (jsonFile["InteractionAddToEntityComponent"].empty())
			return -1;

		std::shared_ptr<InteractionAddToEntityComponent> interactionAddToEntityComponent = targetEntity->getComponent<InteractionAddToEntityComponent>();
		if (!interactionAddToEntityComponent) {
			interactionAddToEntityComponent = std::shared_ptr<InteractionAddToEntityComponent>(new InteractionAddToEntityComponent());
			targetEntity->addComponent(interactionAddToEntityComponent);
		}

		InteractionMember interactionMember = jsonFile["InteractionAddToEntityComponent"]["interactMember"];
		interactionAddToEntityComponent->initialize(interactionMember);
		return 0;
	}
};

class MoveFunctor : public ComponentFunctor {
	void configureFunctor(std::shared_ptr<Entity> targetEntity) {
		int x = TextHelper::instance()->getX();
		int y = TextHelper::instance()->getY();
		int height = TextHelper::instance()->getHeight();
		int width = TextHelper::instance()->getWidth();
		float size = TextHelper::instance()->getSize();
		int index = 0;

		auto entity = TextHelper::instance()->createText("Player controlled", x, y + height * index++, width, height, size, false);
		std::shared_ptr<ClickInsideComponent> clickInsideComponent = entity->getComponent<ClickInsideComponent>();
		std::shared_ptr<MoveEventPlayer> functorMovePlayer = std::make_shared<MoveEventPlayer>();
		clickInsideComponent->_event = std::make_pair(functorMovePlayer, targetEntity);
		TextHelper::instance()->attachText(entity);

		entity = TextHelper::instance()->createText("Predefined", x, y + height * index++, width, height, size, false);
		clickInsideComponent = entity->getComponent<ClickInsideComponent>();
		std::shared_ptr<MoveEventHardcoded> functorMoveHardcoded = std::make_shared<MoveEventHardcoded>();
		clickInsideComponent->_event = std::make_pair(functorMoveHardcoded, targetEntity);
		TextHelper::instance()->attachText(entity);
	}
	std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<MoveComponent> moveComponent(new MoveComponent());
		return moveComponent;
	}

	void removeFunctor(std::shared_ptr<Entity> targetEntity) {
		targetEntity->removeComponent<MoveComponent>();
	}

	void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) {
		int entityID = targetEntity->_index;
		std::shared_ptr<MoveComponent> moveComponent = targetEntity->getComponent<MoveComponent>();
		if (!moveComponent)
			return;

		save->_jsonFile["Entity"][std::to_string(entityID)]["MoveComponent"]["type"] = moveComponent->_type;
		if (moveComponent->_type == StaticallyDefined)
			save->_jsonFile["Entity"][std::to_string(entityID)]["MoveComponent"]["coords"] = moveComponent->_coords;
		else {
			save->_jsonFile["Entity"][std::to_string(entityID)]["MoveComponent"]["leftClick"] = { std::get<0>(moveComponent->_leftClick), std::get<1>(moveComponent->_leftClick) };
			save->_jsonFile["Entity"][std::to_string(entityID)]["MoveComponent"]["rightClick"] = { std::get<0>(moveComponent->_rightClick), std::get<1>(moveComponent->_rightClick) };
		}
		save->_jsonFile["Entity"][std::to_string(entityID)]["MoveComponent"]["speed"] = moveComponent->_speed;
		save->_jsonFile["Entity"][std::to_string(entityID)]["MoveComponent"]["move"] = moveComponent->_move;
	}

	int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) {
		int entityID = targetEntity->_index;
		if (jsonFile["MoveComponent"].empty())
			return -1;

		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		if (!objectComponent)
			return 1;
		auto program = objectComponent->_program;

		std::shared_ptr<MoveComponent> moveComponent = targetEntity->getComponent<MoveComponent>();
		if (!moveComponent) {
			moveComponent = std::shared_ptr<MoveComponent>(new MoveComponent());
			targetEntity->addComponent(moveComponent);
		}

		MoveTypes type = jsonFile["MoveComponent"]["type"];
		int speed = jsonFile["MoveComponent"]["speed"];
		bool move = jsonFile["MoveComponent"]["move"];
		if (type == StaticallyDefined) {
			std::tuple<float, float> coords = jsonFile["MoveComponent"]["coords"];
			moveComponent->initialize(type, program, coords);
			moveComponent->_speed = speed;
		}
		else {
			std::tuple<float, float> leftClick = jsonFile["MoveComponent"]["leftClick"];
			std::tuple<float, float> rightClick = jsonFile["MoveComponent"]["rightClick"];
			moveComponent->initialize(type, program);
			moveComponent->_speed = speed;
			moveComponent->_leftClick = leftClick;
			moveComponent->_rightClick = rightClick;
			moveComponent->_move = move;
		}
		return 0;
	}
};

class CameraFunctor : public ComponentFunctor {
	void configureFunctor(std::shared_ptr<Entity> targetEntity) {
		int x = TextHelper::instance()->getX();
		int y = TextHelper::instance()->getY();
		int height = TextHelper::instance()->getHeight();
		int width = TextHelper::instance()->getWidth();
		float size = TextHelper::instance()->getSize();

		std::shared_ptr<CameraComponent> cameraComponent = targetEntity->getComponent<CameraComponent>();
		std::shared_ptr<TextCallback> callbackSpeed = std::make_shared<TextCallback>();
		callbackSpeed->setValue((int*)&cameraComponent->_cameraSpeed);
		TextHelper::instance()->getValue(callbackSpeed, "Camera speed", x, y, width, height, size);
	}
	std::shared_ptr<Component> createFunctor(std::shared_ptr<Entity> targetEntity) {
		std::shared_ptr<CameraComponent> cameraComponent(new CameraComponent());
		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		cameraComponent->initialize(objectComponent->_program);

		std::shared_ptr<MoveComponent> moveComponent = targetEntity->getComponent<MoveComponent>();
		if (moveComponent && moveComponent->_type == PlayerControlled)
			targetEntity->removeComponent<MoveComponent>();
		return cameraComponent;
	}

	void removeFunctor(std::shared_ptr<Entity> targetEntity) {
		targetEntity->removeComponent<CameraComponent>();
	}

	void serializeFunctor(std::shared_ptr<Entity> targetEntity, std::shared_ptr<GUISave> save) {
		int entityID = targetEntity->_index;
		std::shared_ptr<CameraComponent> cameraComponent = targetEntity->getComponent<CameraComponent>();
		if (!cameraComponent)
			return;

		save->_jsonFile["Entity"][std::to_string(entityID)]["CameraComponent"]["leftClick"] = { std::get<0>(cameraComponent->_leftClick), std::get<1>(cameraComponent->_leftClick) };
		save->_jsonFile["Entity"][std::to_string(entityID)]["CameraComponent"]["rightClick"] = { std::get<0>(cameraComponent->_rightClick), std::get<1>(cameraComponent->_rightClick) };
		save->_jsonFile["Entity"][std::to_string(entityID)]["CameraComponent"]["speed"] = cameraComponent->_cameraSpeed;
		save->_jsonFile["Entity"][std::to_string(entityID)]["CameraComponent"]["move"] = cameraComponent->_move;
	}

	int deserializeFunctor(std::shared_ptr<Entity> targetEntity, json jsonFile) {
		int entityID = targetEntity->_index;
		if (jsonFile["CameraComponent"].empty())
			return -1;

		std::shared_ptr<ObjectComponent> objectComponent = targetEntity->getComponent<ObjectComponent>();
		if (!objectComponent)
			return 1;
		auto program = objectComponent->_program;

		std::shared_ptr<CameraComponent> cameraComponent = targetEntity->getComponent<CameraComponent>();
		if (!cameraComponent) {
			cameraComponent = std::shared_ptr<CameraComponent>(new CameraComponent());
			targetEntity->addComponent(cameraComponent);
		}

		int speed = jsonFile["CameraComponent"]["speed"];
		bool move = jsonFile["CameraComponent"]["move"];
		std::tuple<float, float> leftClick = jsonFile["CameraComponent"]["leftClick"];
		std::tuple<float, float> rightClick = jsonFile["CameraComponent"]["rightClick"];
		cameraComponent->initialize(program);
		cameraComponent->_cameraSpeed = speed;
		cameraComponent->_leftClick = leftClick;
		cameraComponent->_rightClick = rightClick;
		cameraComponent->_cameraX = 0;
		cameraComponent->_cameraY = 0;
		cameraComponent->_move = move;
		
		return 0;
	}
};

void registerComponentFunctors();