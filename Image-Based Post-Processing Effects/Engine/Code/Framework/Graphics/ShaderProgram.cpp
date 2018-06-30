#include <glm\gtc\type_ptr.hpp>
#include <glm\ext.hpp>
#include <cassert>
#include <iostream>
#include "ShaderProgram.h"
#include ".\..\..\Utilities\Utility.h"
#include ".\..\..\Graphics\Material.h"
#include ".\..\..\Graphics\Lights.h"


ShaderProgram::ShaderProgram(const char *_vertexShaderPath,
	const char *_fragmentShaderPath,
	const char *_tesselationControlShaderPath,
	const char *_tesselationEvaluationShaderPath,
	const char *_geometryShaderPath)
{
	GLuint vertexShader = createShader(GL_VERTEX_SHADER, _vertexShaderPath);
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, _fragmentShaderPath);
	GLuint tessControlShader = 0;
	GLuint tessEvalShader = 0;
	GLuint geometryShader = 0;

	// make sure we dont try to create a program with only control shader
	assert(_tesselationEvaluationShaderPath ? true : !bool(_tesselationControlShaderPath));

	if (_tesselationControlShaderPath)
	{
		tessControlShader = createShader(GL_TESS_CONTROL_SHADER, _tesselationControlShaderPath);
	}
	if (_tesselationEvaluationShaderPath)
	{
		tessEvalShader = createShader(GL_TESS_EVALUATION_SHADER, _tesselationEvaluationShaderPath);
	}
	if (_geometryShaderPath)
	{
		geometryShader = createShader(GL_GEOMETRY_SHADER, _geometryShaderPath);
	}

	// shader Program
	programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	if (_tesselationControlShaderPath)
	{
		glAttachShader(programId, tessControlShader);
	}
	if (_tesselationEvaluationShaderPath)
	{
		glAttachShader(programId, tessEvalShader);
	}
	if (_geometryShaderPath)
	{
		glAttachShader(programId, geometryShader);
	}

	glLinkProgram(programId);
	statusCheck(GL_LINK_STATUS);

	// delete the shaders as they're linked into our program now and no longer necessery
	glDetachShader(programId, vertexShader);
	glDetachShader(programId, fragmentShader);
	if (_tesselationControlShaderPath)
	{
		glDetachShader(programId, tessControlShader);
		glDeleteShader(tessControlShader);
	}
	if (_tesselationEvaluationShaderPath)
	{
		glDetachShader(programId, tessEvalShader);
		glDeleteShader(tessEvalShader);
	}
	if (_geometryShaderPath)
	{
		glDetachShader(programId, geometryShader);
		glDeleteShader(geometryShader);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// validate program
	glValidateProgram(programId);
	statusCheck(GL_VALIDATE_STATUS);
}

ShaderProgram::ShaderProgram(const char *_computeShaderPath)
{
	GLuint computeShader = createShader(GL_COMPUTE_SHADER, _computeShaderPath);

	// shader Program
	programId = glCreateProgram();
	glAttachShader(programId, computeShader);

	glLinkProgram(programId);
	statusCheck(GL_LINK_STATUS);

	// delete the shaders as they're linked into our program now and no longer necessery
	glDetachShader(programId, computeShader);
	glDeleteShader(computeShader);

	// validate program
	glValidateProgram(programId);
	statusCheck(GL_VALIDATE_STATUS);
}

GLuint ShaderProgram::createShader(GLenum _type, const char *_shaderPath)
{
	GLuint shader = glCreateShader(_type);

	std::vector<char> data = readTextFile(_shaderPath);
	const char *shaderCode = data.data();
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);

	// print compile errors if any
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << _shaderPath << "\n" << infoLog << std::endl;
		assert(false);
	};

	return shader;
}

void ShaderProgram::statusCheck(GLenum _type)
{
	GLint success;
	glGetProgramiv(programId, _type, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		if (_type == GL_LINK_STATUS)
		{
			std::cout << "WARNING::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		else if (_type == GL_VALIDATE_STATUS)
		{
			std::cout << "WARNING::SHADER::PROGRAM::VALIDATION_FAILED\n" << infoLog << std::endl;
		}
		else
		{
			std::cout << infoLog << std::endl;
		}
	}
}

std::shared_ptr<ShaderProgram> ShaderProgram::createShaderProgram(const char *_vertexShaderPath,
	const char *_fragmentShaderPath,
	const char *_tesselationControlShaderPath,
	const char *_tesselationEvaluationShaderPath,
	const char *_geometryShaderPath)
{
	return std::shared_ptr<ShaderProgram>(new ShaderProgram(_vertexShaderPath, _fragmentShaderPath, _tesselationControlShaderPath, _tesselationEvaluationShaderPath, _geometryShaderPath));
}

std::shared_ptr<ShaderProgram> ShaderProgram::createShaderProgram(const char *_computeShaderPath)
{
	return std::shared_ptr<ShaderProgram>(new ShaderProgram(_computeShaderPath));
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(programId);
}

void ShaderProgram::bind()
{
	glUseProgram(programId);
}

GLuint ShaderProgram::getId() const
{
	return programId;
}

const GLint ShaderProgram::createUniform(const std::string &_name) const
{
	return glGetUniformLocation(programId, _name.c_str());
}

void ShaderProgram::setUniform(const GLint &_location, const GLboolean &_value) const
{
	glUniform1i(_location, _value);
}

void ShaderProgram::setUniform(const GLint &_location, const GLint &_value) const
{
	glUniform1i(_location, _value);
}

void ShaderProgram::setUniform(const GLint &_location, GLuint _value) const
{
	glUniform1ui(_location, _value);
}

void ShaderProgram::setUniform(const GLint &_location, const GLfloat &_value) const
{
	glUniform1f(_location, _value);
}

void ShaderProgram::setUniform(const GLint &_location, const glm::mat3 &_value) const
{
	glUniformMatrix3fv(_location, 1, GL_FALSE, glm::value_ptr(_value));
}

void ShaderProgram::setUniform(const GLint &_location, const glm::mat4 &_value) const
{
	glUniformMatrix4fv(_location, 1, GL_FALSE, glm::value_ptr(_value));
}

void ShaderProgram::setUniform(const GLint &_location, const glm::vec2 &_value) const
{
	glUniform2f(_location, _value.x, _value.y);
}

void ShaderProgram::setUniform(const GLint &_location, const glm::vec3 &_value) const
{
	glUniform3f(_location, _value.x, _value.y, _value.z);
}

void ShaderProgram::setUniform(const GLint &_location, const glm::vec4 &_value) const
{
	glUniform4f(_location, _value.x, _value.y, _value.z, _value.w);
}