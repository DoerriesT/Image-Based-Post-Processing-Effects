#pragma once
#include <glad/glad.h>

#ifndef PROFILING_ENABLED
#define PROFILING_ENABLED 1
#endif // !PROFILING_ENABLED

#if PROFILING_ENABLED
#define SCOPED_TIMER_QUERY(x) GLTimerQuery timer(x);
#else
#define SCOPED_TIMER_QUERY(x)
#endif // PROFILING_ENABLED

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
	double & m_result;
	GLuint m_id;
};