#include "GLTimerQuery.h"
#include <cstdint>

GLTimerQuery::GLTimerQuery(double &_result)
	:result(_result)
{
	glGenQueries(1, &id);
	glBeginQuery(GL_TIME_ELAPSED, id);
}

GLTimerQuery::~GLTimerQuery()
{
	uint64_t nanoseconds = 0;
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectui64v(id, GL_QUERY_RESULT, &nanoseconds);
	glDeleteQueries(1, &id);
	result = nanoseconds / 1000000.0;
}
