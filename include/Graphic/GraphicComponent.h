#pragma once
#include "Component.h"
#include "Common.h"
#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Matrix.h"

class ObjectComponent : public Component {
public:
	void initialize(int sceneX, int sceneY, int objectWidth, int objectHeight, GLuint program) {
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
	//
	int _sceneX, _sceneY;
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
	void initialize(Texture texture, GLuint program) {
		_texture = texture;
		_program = program;
		float posXInAtlasN = (float)_texture.getX() / (float)_texture.getAtlas()->getWidth();
		float posYInAtlasN = (float)_texture.getY() / (float)_texture.getAtlas()->getHeight();
		float textureWidthN = (float)_texture.getWidth() / (float)_texture.getAtlas()->getWidth();
		float textureHeightN = (float)_texture.getHeight() / (float)_texture.getAtlas()->getHeight();
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
		_textureID = texture.getAtlas()->getAtlasID();

	}
	//
	Buffer _buffer;
	Texture _texture;
	//
	GLuint _program;
	GLuint _textureID;
	GLint _aTextureCoordinatesLocation;
	GLint _uTextureUnitLocation;
	//
	std::string _aTextureCoordinatesString = "a_TextureCoordinates";
	std::string _uTextureUnitString = "u_TextureUnit";
};

class AnimatedTextureComponent : public Component {
public:
	float _widthTile;
	float _heightTile;
	int _currentAnimateTile = 0;
	int _currentLatency = 0;
	std::vector<int> _tilesOrder;
	std::vector<int> _tilesLatency;
	//
	GLint _uAdjustXLocation;
	GLint _uAdjustYLocation;
	//
	std::string _uAdjustX = "u_AdjustX";
	std::string _uAdjustY = "u_AdjustY";
};

class TransformComponent : public Component {
	Matrix2D _transform;
	//
	GLint _uMatrixLocation;
	//
	std::string _uMatrix = "u_Matrix";
};