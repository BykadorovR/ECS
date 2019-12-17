#pragma once
#include <tuple>
#include <GL/glew.h>
#include <ft2build.h>
#include <iostream>
#include <map>
#include "TextureAtlas.h"
#include FT_FREETYPE_H

struct CharacterInfo {
	std::tuple<int, int> size;       // Size of glyph
	std::tuple<int, int> bearing;    // Offset from baseline to left/top of glyph
	GLuint advance;    // Offset to advance to next glyph
};

class GlyphsLoader {
public:
	GlyphsLoader(std::tuple<int, int> symbolsCodes);
	void bufferSymbols(int symbolHeight);
	std::map<FT_ULong, CharacterInfo> getCharacters();
	std::map<FT_ULong, std::tuple<float, float> > getCharactersAtlasPosition();
	std::shared_ptr<TextureAtlas> getAtlas();
private:
	int _symbolHeight;
	std::tuple<int, int> _symbolsCodes;
	std::shared_ptr<TextureAtlas> _charactersAtlas;
	std::map<FT_ULong, std::tuple<float, float> > _charactersAtlasPosition;
	std::map<FT_ULong, CharacterInfo> _characters;
};


