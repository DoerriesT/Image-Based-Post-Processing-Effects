#include "GLTimerQuery.h"
#include <cstdint>

GLTimerQuery::GLTimerQuery(double &_result)
	:m_result(_result)
{
	glGenQueries(1, &m_id);
	glBeginQuery(GL_TIME_ELAPSED, m_id);
}

GLTimerQuery::~GLTimerQuery()
{
	uint64_t nanoseconds = 0;
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectui64v(m_id, GL_QUERY_RESULT, &nanoseconds);
	glDeleteQueries(1, &m_id);
	m_result = nanoseconds / 1000000.0;
}
