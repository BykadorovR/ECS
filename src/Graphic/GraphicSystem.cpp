#include "GraphicSystem.h"
#include "Camera.h"
//INFO: for now every object use OWN program, all of them use one shader, though. It's convinient enough in sense of 
//programming (no needs to think about uniforms state) but can affect performance.
//If any perf gaps, need to add vertexReset, transformReset, etc functions which will zero uniforms
//so further objects that don't contain such component wont be affected

void vertexUpdate(std::shared_ptr<ObjectComponent> object) {
	glUseProgram(object->_program);

	//bind buffer and handle vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, object->_buffer.getVBOObject());
	//index, size, type, normalized, stride, offset in GL_ARRAY_BUFFER target
	glVertexAttribPointer(object->_aPositionLocation, POSITION_COMPONENT_COUNT, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(object->_aPositionLocation);
}

void textureUpdate(std::shared_ptr<TextureComponent> object) {
	//bind buffer and handle texture shader
	glBindBuffer(GL_ARRAY_BUFFER, object->_buffer.getVBOObject());
	//index, size, type, normalized, stride, offset in GL_ARRAY_BUFFER target
	glVertexAttribPointer(object->_aTextureCoordinatesLocation, TEXTURE_COORDINATES_COMPONENT_COUNT, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(object->_aTextureCoordinatesLocation);
	glUniform1f(object->_uAdjustXLocation, 0);
	//bind texture and handle fragment shader
	//we can use multiple textures, but we use only one, do it active and send textureUnitLocation = 0 to shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object->_textureObject);
	glUniform1i(object->_uTextureUnitLocation, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void animatedTextureUpdate(std::shared_ptr<AnimatedTextureComponent> object) {
	//bind buffer and handle texture shader
	glBindBuffer(GL_ARRAY_BUFFER, object->_buffer.getVBOObject());
	//index, size, type, normalized, stride, offset in GL_ARRAY_BUFFER target
	glVertexAttribPointer(object->_aTextureCoordinatesLocation, TEXTURE_COORDINATES_COMPONENT_COUNT, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(object->_aTextureCoordinatesLocation);
	glUniform1f(object->_uAdjustXLocation, object->_widthTile * object->_tilesOrder[(object->_currentAnimateTile)]);
	//bind texture and handle fragment shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object->_textureObject);
	glUniform1i(object->_uTextureUnitLocation, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (object->_currentAnimateTile < object->_tilesOrder.size()) {
		if (object->_currentLatency < object->_tilesLatency[object->_currentAnimateTile])
			object->_currentLatency++;
		else {
			object->_currentLatency = 0;
			object->_currentAnimateTile++;
			if (object->_currentAnimateTile == object->_tilesOrder.size())
				object->_currentAnimateTile = 0;
		}
	}
}

void transformUpdate(std::shared_ptr<ObjectComponent> object, std::shared_ptr<MoveComponent> move) {
	float adjustX = std::get<0>(move->_coords);
	float adjustY = std::get<1>(move->_coords);
	Matrix2D matrix;
	matrix.translate(adjustX, adjustY);
	object->_transform = object->_transform * matrix;
	glUniformMatrix4fv(move->_uMatrixLocation, 1, false, move->_result.getData());
	object->_sceneX += adjustX;
	object->_sceneY += adjustY;
	//TODO: ISSUE!! ORDER DEPENDENCIES BETWEEN CAMERA AND TRANSFORM. CAMERA USES _coords VAR TOO
	move->_coords = { 0, 0 };
}

void cameraUpdate(std::shared_ptr<ObjectComponent> object, std::shared_ptr<CameraComponent> camera, std::tuple<float, float> coords) {
	Matrix2D move;
	float x = -std::get<0>(coords);
	float y = -std::get<1>(coords);
	move.translate(x, y);
	object->_camera = object->_camera * move;
	glUniformMatrix4fv(camera->_uViewMatrixLocation, 1, false, object->_camera.getData());
}

std::tuple<float, float> cameraFindCoords(shared_ptr<EntityManager> entityManager, std::shared_ptr<CameraComponent>& cameraComponent, std::shared_ptr<ObjectComponent>& cameraObjectComponent) {
	int targetEntityID = -1;
	std::tuple<float, float> coords = { 0, 0 };
	for (auto entity : entityManager->getEntities()) {
		cameraComponent = entity->getComponent<CameraComponent>();
		if (cameraComponent) {
			coords = cameraComponent->_coords;
			cameraObjectComponent = entity->getComponent<ObjectComponent>();
		}
	}

	return coords;
}

void DrawSystem::update(shared_ptr<EntityManager> entityManager) {
	std::shared_ptr<CameraComponent> cameraComponent;
	std::shared_ptr<ObjectComponent> cameraObjectComponent;
	auto coords = cameraFindCoords(entityManager, cameraComponent, cameraObjectComponent);

	for (auto entity : entityManager->getEntities()) {
		auto vertexObject = entity->getComponent<ObjectComponent>();
		assert(vertexObject);
		vertexUpdate(vertexObject);

		if (cameraComponent) {
			cameraObjectComponent->_sceneX += std::get<0>(coords);
			cameraObjectComponent->_sceneY += std::get<1>(coords);

		}

		if (vertexObject->_hud == false) {
			cameraUpdate(vertexObject, cameraComponent, coords);
		}

		auto transformTextureObject = entity->getComponent<MoveComponent>();
		if (transformTextureObject)
			transformUpdate(vertexObject, transformTextureObject);

		auto textureObject = entity->getComponent<TextureComponent>();
		if (textureObject)
			textureUpdate(textureObject);

		auto animatedTextureObject = entity->getComponent<AnimatedTextureComponent>();
		if (animatedTextureObject)
			animatedTextureUpdate(animatedTextureObject);
	}
}
