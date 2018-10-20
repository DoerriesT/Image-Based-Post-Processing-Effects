#pragma once
#include <glad/glad.h>

class GLTimerQuery
{
public:
	explicit GLTimerQuery(double &_result);
	GLTimerQuery(const GLTimerQuery &) = delete;
	GLTimerQuery(const GLTimerQuery &&) = delete;
	GLTimerQuery &operator= (const GLTimerQuery &) = delete;
	GLTimerQuery &operator= (const GLTimerQuery &&) = delete;
	~GLTimerQuery();

private:
	double & result;
	GLuint id;
};