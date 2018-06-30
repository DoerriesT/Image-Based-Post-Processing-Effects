#include "Buffer.h"

std::shared_ptr<Buffer> Buffer::create(GLenum _target, GLenum _usage, size_t _size)
{
	return std::shared_ptr<Buffer>(new Buffer(_target, _usage, _size));
}

Buffer::~Buffer()
{
	glDeleteBuffers(1, &handle);
}

GLuint Buffer::getHandle() const
{
	return handle;
}

Buffer::Buffer(GLenum _target, GLenum _usage, size_t _size)
	:target(_target), usage(_usage), size(_size)
{
	glGenBuffers(1, &handle);

	glBindBuffer(target, handle);
	glBufferData(target, size, NULL, usage);
}
