#pragma once
#include <glad\glad.h>
#include <memory>

class Buffer
{
public:
	static std::shared_ptr<Buffer> create(GLenum _target, GLenum _usage, size_t _size);
	Buffer(const Buffer &) = delete;
	Buffer(const Buffer &&) = delete;
	Buffer &operator= (const Buffer &) = delete;
	Buffer &operator= (const Buffer &&) = delete;
	~Buffer();
	GLuint getHandle() const;

private:
	GLuint handle;
	GLenum target;
	GLenum usage;
	size_t size;

	explicit Buffer(GLenum _target, GLenum _usage, size_t _size);
};
