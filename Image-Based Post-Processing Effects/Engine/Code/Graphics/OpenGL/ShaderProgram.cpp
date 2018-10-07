#include <glm\gtc\type_ptr.hpp>
#include <glm\ext.hpp>
#include <cassert>
#include <iostream>
#include "ShaderProgram.h"
#include "GLUtility.h"
#include "Utilities\Utility.h"
#include "Graphics\Material.h"
#include "Graphics\Lights.h"


ShaderProgram::ShaderProgram(
	const std::vector<std::tuple<ShaderType, std::string, int>> &_defines,
	const char *_vertexShaderPath,
	const char *_fragmentShaderPath,
	const char *_tesselationControlShaderPath,
	const char *_tesselationEvaluationShaderPath,
	const char *_geometryShaderPath)
	:defines(_defines),
	vertexShaderPath(_vertexShaderPath),
	fragmentShaderPath(_fragmentShaderPath)
{
	if (_tesselationControlShaderPath)
	{
		tesselationControlShaderPath = _tesselationControlShaderPath;
	}
	if (_tesselationEvaluationShaderPath)
	{
		tesselationEvaluationShaderPath = _tesselationEvaluationShaderPath;
	}
	if (_geometryShaderPath)
	{
		geometryShaderPath = _geometryShaderPath;
	}
	create();
}

ShaderProgram::ShaderProgram(const std::vector<std::tuple<ShaderType, std::string, int>> &_defines, const char *_computeShaderPath)
	:defines(_defines),
	computeShaderPath(_computeShaderPath)
{
	create();
}

GLuint ShaderProgram::createShader(GLenum _type, const char *_shaderPath)
{
	GLuint shader = glCreateShader(_type);

	std::string shaderCodeStr = Utility::readTextFile(_shaderPath).data();
	if (!defines.empty())
	{
		// determine this shaders type
		ShaderType type = ShaderType::FRAGMENT;
		switch (_type)
		{
		case GL_VERTEX_SHADER:
			type = ShaderType::VERTEX;
			break;
		case GL_FRAGMENT_SHADER:
			type = ShaderType::FRAGMENT;
			break;
		case GL_TESS_CONTROL_SHADER:
			type = ShaderType::TESSELATION_CONTROL;
			break;
		case GL_TESS_EVALUATION_SHADER:
			type = ShaderType::TESSELATION_EVALUATION;
			break;
		case GL_GEOMETRY_SHADER:
			type = ShaderType::GEOMETRY;
			break;
		case GL_COMPUTE_SHADER:
			type = ShaderType::COMPUTE;
			break;
		default:
			assert(false);
		}

		// create a list of all defines for this type
		std::vector<std::pair<std::string, int>> localDefines;

		for (const auto &define : defines)
		{
			if (std::get<0>(define) == type)
			{
				localDefines.push_back({ std::get<1>(define), std::get<2>(define) });
			}
		}

		if (!localDefines.empty())
		{
			shaderCodeStr = GLUtility::shaderDefineInjection(shaderCodeStr, localDefines);
		}
	}
	shaderCodeStr = GLUtility::shaderIncludeResolve(shaderCodeStr);
	const char *shaderCode = shaderCodeStr.c_str();
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

void ShaderProgram::create()
{
	if (!computeShaderPath.empty())
	{
		GLuint computeShader = createShader(GL_COMPUTE_SHADER, computeShaderPath.c_str());

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
	else
	{
		GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderPath.c_str());
		GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderPath.c_str());
		GLuint tessControlShader = 0;
		GLuint tessEvalShader = 0;
		GLuint geometryShader = 0;

		// make sure we dont try to create a program with only control shader
		assert(!tesselationEvaluationShaderPath.empty() ? true : tesselationControlShaderPath.empty());

		if (!tesselationControlShaderPath.empty())
		{
			tessControlShader = createShader(GL_TESS_CONTROL_SHADER, tesselationControlShaderPath.c_str());
		}
		if (!tesselationEvaluationShaderPath.empty())
		{
			tessEvalShader = createShader(GL_TESS_EVALUATION_SHADER, tesselationEvaluationShaderPath.c_str());
		}
		if (!geometryShaderPath.empty())
		{
			geometryShader = createShader(GL_GEOMETRY_SHADER, geometryShaderPath.c_str());
		}

		// shader Program
		programId = glCreateProgram();
		glAttachShader(programId, vertexShader);
		glAttachShader(programId, fragmentShader);
		if (!tesselationControlShaderPath.empty())
		{
			glAttachShader(programId, tessControlShader);
		}
		if (!tesselationEvaluationShaderPath.empty())
		{
			glAttachShader(programId, tessEvalShader);
		}
		if (!geometryShaderPath.empty())
		{
			glAttachShader(programId, geometryShader);
		}

		glLinkProgram(programId);
		statusCheck(GL_LINK_STATUS);

		// delete the shaders as they're linked into our program now and no longer necessery
		glDetachShader(programId, vertexShader);
		glDetachShader(programId, fragmentShader);
		if (!tesselationControlShaderPath.empty())
		{
			glDetachShader(programId, tessControlShader);
			glDeleteShader(tessControlShader);
		}
		if (!tesselationEvaluationShaderPath.empty())
		{
			glDetachShader(programId, tessEvalShader);
			glDeleteShader(tessEvalShader);
		}
		if (!geometryShaderPath.empty())
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
}

std::shared_ptr<ShaderProgram> ShaderProgram::createShaderProgram(
	const char *_vertexShaderPath,
	const char *_fragmentShaderPath,
	const char *_tesselationControlShaderPath,
	const char *_tesselationEvaluationShaderPath,
	const char *_geometryShaderPath)
{
	return std::shared_ptr<ShaderProgram>(new ShaderProgram({}, _vertexShaderPath, _fragmentShaderPath, _tesselationControlShaderPath, _tesselationEvaluationShaderPath, _geometryShaderPath));
}

std::shared_ptr<ShaderProgram> ShaderProgram::createShaderProgram(
	const std::vector<std::tuple<ShaderType, std::string, int>> &_defines,
	const char * _vertexShaderPath,
	const char * _fragmentShaderPath,
	const char * _tesselationControlShaderPath,
	const char * _tesselationEvaluationShaderPath,
	const char * _geometryShaderPath)
{
	return std::shared_ptr<ShaderProgram>(new ShaderProgram(_defines, _vertexShaderPath, _fragmentShaderPath, _tesselationControlShaderPath, _tesselationEvaluationShaderPath, _geometryShaderPath));
}

std::shared_ptr<ShaderProgram> ShaderProgram::createShaderProgram(const char *_computeShaderPath)
{
	return std::shared_ptr<ShaderProgram>(new ShaderProgram({}, _computeShaderPath));
}

std::shared_ptr<ShaderProgram> ShaderProgram::createShaderProgram(const std::vector<std::tuple<ShaderType, std::string, int>> &_defines, const char * _computeShaderPath)
{
	return std::shared_ptr<ShaderProgram>(new ShaderProgram(_defines, _computeShaderPath));
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

std::vector<std::tuple<ShaderProgram::ShaderType, std::string, int>> ShaderProgram::getDefines() const
{
	return defines;
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

void ShaderProgram::setUniform(const GLint & _location, const glm::ivec3 & _value) const
{
	glUniform3i(_location, _value.x, _value.y, _value.z);
}

void ShaderProgram::setUniform(const GLint &_location, const glm::vec4 &_value) const
{
	glUniform4f(_location, _value.x, _value.y, _value.z, _value.w);
}

void ShaderProgram::setDefines(const std::vector<std::tuple<ShaderType, std::string, int>> &_defines)
{
	defines = _defines;
	glUseProgram(0);
	glDeleteProgram(programId);
	create();
}
