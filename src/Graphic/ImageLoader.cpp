#include "ImageLoader.h"
#include "Common.h"
#include <cassert>

void PNGCallback(png_structp PNGPtr, png_byte* rawData, png_size_t readLength) {
	DataHandle* handle = (DataHandle*)png_get_io_ptr(PNGPtr);
	const png_byte* PNGSrc = handle->data + handle->offset;

	memcpy(rawData, PNGSrc, readLength);
	handle->offset += readLength;
}

int ImageLoader::getImageFromPNG(std::string PNGData) {
	unsigned char* _PNGData;
	assert(png_check_sig((png_const_bytep)PNGData.c_str(), 8));

	png_structp PNGPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	assert(PNGPtr != NULL);
	png_infop infoPtr = png_create_info_struct(PNGPtr);
	assert(infoPtr != NULL);

	DataHandle PNGDataHandle((png_byte*)PNGData.c_str(), (png_size_t)PNGData.size(), 0);
	png_set_read_fn(PNGPtr, &PNGDataHandle, PNGCallback);

	if (setjmp(png_jmpbuf(PNGPtr))) {
		assert("Error reading PNG file!");
	}

	int sts = readAndUpdateInfo(PNGPtr, infoPtr);
	sts = readEntirePNGImage(PNGPtr, infoPtr);

	png_read_end(PNGPtr, infoPtr);
	png_destroy_read_struct(&PNGPtr, &infoPtr, NULL);
	return sts;
}

int ImageLoader::readAndUpdateInfo(png_structp PNGPtr, png_infop infoPTR) {
	png_read_info(PNGPtr, infoPTR);
	png_get_IHDR(PNGPtr, infoPTR, &_width, &_height, &_bitDepth, &_colorFormat, NULL, NULL, NULL);

	// Convert transparency to full alpha
	if (png_get_valid(PNGPtr, infoPTR, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(PNGPtr);

	// Convert grayscale, if needed.
	if (_colorFormat == PNG_COLOR_TYPE_GRAY && _bitDepth < 8)
		png_set_expand_gray_1_2_4_to_8(PNGPtr);

	// Convert paletted images, if needed.
	if (_colorFormat == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(PNGPtr);

	// Add alpha channel, if there is none (rationale: GL_RGBA is faster than GL_RGB on many GPUs)
	if (_colorFormat == PNG_COLOR_TYPE_PALETTE || _colorFormat == PNG_COLOR_TYPE_RGB)
		png_set_add_alpha(PNGPtr, 0xFF, PNG_FILLER_AFTER);

	// Ensure 8-bit packing
	if (_bitDepth < 8)
		png_set_packing(PNGPtr);
	else if (_bitDepth == 16)
		png_set_scale_16(PNGPtr);

	png_read_update_info(PNGPtr, infoPTR);

	// Read the new color type after updates have been made.
	_colorFormat = png_get_color_type(PNGPtr, infoPTR);
	return 0;
}

int ImageLoader::readEntirePNGImage(const png_structp PNGPtr, const png_infop infoPTR) {
	const png_size_t rowSize = png_get_rowbytes(PNGPtr, infoPTR);
	const int dataLength = rowSize * _height;
	assert(rowSize > 0);

	png_byte* rawImage = new png_byte[dataLength];
	assert(rawImage != NULL);

	png_byte** rowPtrs = new png_byte*[_height];

	png_uint_32 i;
	for (i = 0; i < _height; i++) {
		rowPtrs[i] = &rawImage[i * rowSize];
	}

	png_read_image(PNGPtr, rowPtrs);
	_data = std::vector<unsigned char>(rawImage, rawImage + dataLength);
	return 0;
}

GLenum ImageLoader::getGLColorFormat() {
	assert(_colorFormat == PNG_COLOR_TYPE_GRAY
		|| _colorFormat == PNG_COLOR_TYPE_RGB_ALPHA
		|| _colorFormat == PNG_COLOR_TYPE_GRAY_ALPHA);

	switch (_colorFormat) {
	case PNG_COLOR_TYPE_GRAY:
		return GL_LUMINANCE;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		return GL_RGBA;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		return GL_LUMINANCE_ALPHA;
	}

	return 0;
}
//Load PNG file to visualise as Texture
int ImageLoader::loadPNG(const char* relativePath) {
	assert(relativePath != NULL);
	std::string pngRawData = rawFileContent(relativePath);
	int sts = getImageFromPNG(pngRawData);

	return sts;
}

std::tuple<float, float> ImageLoader::getSize() {
	return { _width, _height };
}

int ImageLoader::getBitdepth() {
	return _bitDepth;
}
std::vector<unsigned char> ImageLoader::getData() {
	return _data;
}

