#pragma once
#include "Component.h"
#include "Common.h"
#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Matrix.h"
#include "Events.h"

class ObjectComponent : public Component {
public:
	void initialize(int sceneX, int sceneY, int objectWidth, int objectHeight, bool hud, GLuint program) {
		_hud = hud;
		_program = program;
		_objectWidth = objectWidth;
		_objectHeight = objectHeight;
		_sceneX = sceneX;
		_sceneY = sceneY;

		float objectWidthN = (float)_objectWidth / (float)resolution.first;
		float objectHeightN = (float)_objectHeight / (float)resolution.second;
		float startX = (float)(_sceneX) / (float)resolution.first;
		float startY = (float)(resolution.second - _sceneY) / (float)resolution.second;
		// Order of coordinates: X, Y
		// 0   2
		// | / |
		// 1   3
		float vertexData[] = { startX,                startY,
							   startX,                startY - objectHeightN,
							   startX + objectWidthN, startY,
							   startX + objectWidthN, startY - objectHeightN };
		assert(_buffer.bindVBO(vertexData, sizeof(vertexData), GL_STATIC_DRAW) == TW_OK);
		_aPositionLocation = glGetAttribLocation(_program, _aPositionString.c_str());
	}
	bool _hud;
	Matrix2D _transform;
	Matrix2D _camera;
	int _cameraCoefSpeed;
	//
	float _sceneX, _sceneY;
	int _objectWidth, _objectHeight;
	//
	Buffer _buffer;
	//
	GLint _aPositionLocation;
	GLuint _program;
	//
	std::string _aPositionString = "a_Position";
};

class TextureComponent : public Component {
public:
	void initialize(int textureID, GLuint program) {
		_componentID = 1;
		_texture = TextureManager::instance()->getTexture(textureID);
		_program = program;
		float posXInAtlasN = (float)_texture->getX() / (float)_texture->getAtlas()->getWidth();
		float posYInAtlasN = (float)_texture->getY() / (float)_texture->getAtlas()->getHeight();
		float textureWidthN = (float)_texture->getWidth() / (float)_texture->getAtlas()->getWidth();
		float textureHeightN = (float)_texture->getHeight() / (float)_texture->getAtlas()->getHeight();
		// Order of coordinates: S, T
		// 0   2
		// | / |
		// 1   3
		float textureData[] = { posXInAtlasN,                 posYInAtlasN,
								posXInAtlasN,                 posYInAtlasN + textureHeightN,
								posXInAtlasN + textureWidthN, posYInAtlasN,
								posXInAtlasN + textureWidthN, posYInAtlasN + textureHeightN };
		assert(_buffer.bindVBO(textureData, sizeof(textureData), GL_STATIC_DRAW) == TW_OK);
		_aTextureCoordinatesLocation = glGetAttribLocation(_program, _aTextureCoordinatesString.c_str());
		_uTextureUnitLocation = glGetUniformLocation(_program, _uTextureUnitString.c_str());
		_uAdjustXLocation = glGetUniformLocation(_program, _uAdjustX.c_str());
		_uAdjustYLocation = glGetUniformLocation(_program, _uAdjustY.c_str());

		_textureObject = _texture->getAtlas()->getTexureObjectID();

	}

	void initialize(std::shared_ptr<Texture> texture, GLuint program) {
		_componentID = 1;
		_texture = texture;
		_program = program;
		float posXInAtlasN = (float)_texture->getX() / (float)_texture->getAtlas()->getWidth();
		float posYInAtlasN = (float)_texture->getY() / (float)_texture->getAtlas()->getHeight();
		float textureWidthN = (float)_texture->getWidth() / (float)_texture->getAtlas()->getWidth();
		float textureHeightN = (float)_texture->getHeight() / (float)_texture->getAtlas()->getHeight();
		// Order of coordinates: S, T
		// 0   2
		// | / |
		// 1   3
		float textureData[] = { posXInAtlasN,                 posYInAtlasN,
								posXInAtlasN,                 posYInAtlasN + textureHeightN,
								posXInAtlasN + textureWidthN, posYInAtlasN,
								posXInAtlasN + textureWidthN, posYInAtlasN + textureHeightN };
		assert(_buffer.bindVBO(textureData, sizeof(textureData), GL_STATIC_DRAW) == TW_OK);
		_aTextureCoordinatesLocation = glGetAttribLocation(_program, _aTextureCoordinatesString.c_str());
		_uTextureUnitLocation = glGetUniformLocation(_program, _uTextureUnitString.c_str());
		_uAdjustXLocation = glGetUniformLocation(_program, _uAdjustX.c_str());
		_uAdjustYLocation = glGetUniformLocation(_program, _uAdjustY.c_str());

		_textureObject = texture->getAtlas()->getTexureObjectID();

	}

	//
	Buffer _buffer;
	std::shared_ptr<Texture> _texture;
	//
	GLuint _program;
	//atlas ID
	GLuint _textureObject;
	GLint _aTextureCoordinatesLocation;
	GLint _uTextureUnitLocation;
	GLint _uAdjustXLocation;
	GLint _uAdjustYLocation;

	//
	std::string _aTextureCoordinatesString = "a_TextureCoordinates";
	std::string _uTextureUnitString = "u_TextureUnit";
	std::string _uAdjustX = "u_AdjustX";
	std::string _uAdjustY = "u_AdjustY";

};

class AnimatedTextureComponent : public Component {
public:
	void initialize(int textureID, std::vector<int> tilesOrder, std::vector<int> tilesLatency, GLuint program) {
		_componentID = 1;
		_texture = TextureManager::instance()->getTexture(textureID);
		_program = program;
		_tilesLatency = tilesLatency;
		_tilesOrder = tilesOrder;
		float posXInAtlasN = (float)_texture->getX() / (float)_texture->getAtlas()->getWidth();
		float posYInAtlasN = (float)_texture->getY() / (float)_texture->getAtlas()->getHeight();
		float widthTile = (float)_texture->getWidth() / (float)_texture->getColumn() / (float)_texture->getAtlas()->getWidth();
		_widthTile = widthTile;
		float heightTile = (float)_texture->getHeight() / (float)_texture->getRow() / (float)_texture->getAtlas()->getHeight();
		_heightTile = heightTile;
		// Order of coordinates: S, T
		// 0   2
		// | / |
		// 1   3
		float textureData[] = { posXInAtlasN,                 posYInAtlasN,
								posXInAtlasN,                 posYInAtlasN + heightTile,
								posXInAtlasN + widthTile, posYInAtlasN,
								posXInAtlasN + widthTile, posYInAtlasN + heightTile };
		assert(_buffer.bindVBO(textureData, sizeof(textureData), GL_STATIC_DRAW) == TW_OK);
		_aTextureCoordinatesLocation = glGetAttribLocation(_program, _aTextureCoordinatesString.c_str());
		_uTextureUnitLocation = glGetUniformLocation(_program, _uTextureUnitString.c_str());
		_uAdjustXLocation = glGetUniformLocation(_program, _uAdjustX.c_str());
		_uAdjustYLocation = glGetUniformLocation(_program, _uAdjustY.c_str());
		_textureObject = _texture->getAtlas()->getTexureObjectID();

	}
	void initialize(std::shared_ptr<Texture> texture, std::vector<int> tilesOrder, std::vector<int> tilesLatency, GLuint program) {
		_componentID = 1;
		_texture = texture;
		_program = program;
		_tilesLatency = tilesLatency;
		_tilesOrder = tilesOrder;
		float posXInAtlasN = (float)_texture->getX() / (float)_texture->getAtlas()->getWidth();
		float posYInAtlasN = (float)_texture->getY() / (float)_texture->getAtlas()->getHeight();
		float widthTile = (float)_texture->getWidth() / (float)_texture->getColumn() / (float)_texture->getAtlas()->getWidth();
		_widthTile = widthTile;
		float heightTile = (float)_texture->getHeight() / (float)_texture->getRow() / (float)_texture->getAtlas()->getHeight();
		_heightTile = heightTile;
		// Order of coordinates: S, T
		// 0   2
		// | / |
		// 1   3
		float textureData[] = { posXInAtlasN,                 posYInAtlasN,
								posXInAtlasN,                 posYInAtlasN + heightTile,
								posXInAtlasN + widthTile, posYInAtlasN,
								posXInAtlasN + widthTile, posYInAtlasN + heightTile };
		assert(_buffer.bindVBO(textureData, sizeof(textureData), GL_STATIC_DRAW) == TW_OK);
		_aTextureCoordinatesLocation = glGetAttribLocation(_program, _aTextureCoordinatesString.c_str());
		_uTextureUnitLocation = glGetUniformLocation(_program, _uTextureUnitString.c_str());
		_uAdjustXLocation = glGetUniformLocation(_program, _uAdjustX.c_str());
		_uAdjustYLocation = glGetUniformLocation(_program, _uAdjustY.c_str());
		_textureObject = texture->getAtlas()->getTexureObjectID();

	}

	float _widthTile;
	float _heightTile;
	int _currentAnimateTile = 0;
	int _currentLatency = 0;
	std::vector<int> _tilesOrder;
	std::vector<int> _tilesLatency;
	//
	Buffer _buffer;
	std::shared_ptr<Texture> _texture;
	//
	GLuint _program;
	GLuint _textureObject;
	GLint _aTextureCoordinatesLocation;
	GLint _uTextureUnitLocation;
	GLint _uAdjustXLocation;
	GLint _uAdjustYLocation;

	//
	std::string _aTextureCoordinatesString = "a_TextureCoordinates";
	std::string _uTextureUnitString = "u_TextureUnit";
	std::string _uAdjustX = "u_AdjustX";
	std::string _uAdjustY = "u_AdjustY";
};


enum MoveTypes {
	PlayerControlled = 0,
	StaticallyDefined = 1
};

class MoveComponent : public Component, IMouseEvent {
public:
	void initialize(MoveTypes type, GLuint program, int speed) {
		_type = type;
		_program = program;
		_speed = speed;
		_result.identity();
		_uMatrixLocation = glGetUniformLocation(_program, _uMatrix.c_str());
		_leftClick = { 0, 0 };
		_rightClick = { 0, 0 };
		_coords = { 0, 0 };
		_move = false;
		if (_type == PlayerControlled)
			MouseEvent::instance().registerComponent(this);
	}

	void initialize(MoveTypes type, GLuint program, int speed, std::tuple<float, float> endPoint) {
		this->initialize(type, program, speed);
		_leftClick = endPoint;
	}

	void mouseClickDownLeft(int x, int y) {
		_leftClick = { x, y };
	}
	void mouseClickDownRight(int x, int y) {
		_rightClick = { x, y };
	}

	void setTransform(std::tuple<float, float> coords) {
		_coords = coords;
	}

	~MoveComponent() {
		if (_type == PlayerControlled)
			MouseEvent::instance().unregisterComponent(this);
	}

	int _speed;
	MoveTypes _type;
	bool _move = false;
	Matrix2D _result;
	std::tuple<float, float> _leftClick;
	std::tuple<float, float> _rightClick;
	std::tuple<float, float> _coords;
	GLuint _program;
	//
	GLint _uMatrixLocation;
	//
	std::string _uMatrix = "u_Matrix";
};