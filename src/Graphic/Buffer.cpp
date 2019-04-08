#include "Buffer.h"
#include "Common.h"
#include <assert.h>
#include <stdlib.h>
#include "iostream"

int Buffer::bindVBO(float* data, GLsizeiptr size, GLenum usage) {
	_buffer = data;
	glGenBuffers(1, &vboObject);
	CHECK_STATUS(vboObject == 0);
	CHECK_STATUS(data == NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vboObject);
	glBufferData(GL_ARRAY_BUFFER, size, (GLvoid*) data, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return TW_OK;
}

GLuint Buffer::getVBOObject() {
	return vboObject;
}

float* Buffer::getBuffer() {
	return _buffer;
}