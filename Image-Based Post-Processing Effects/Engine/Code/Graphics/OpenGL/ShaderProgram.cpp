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
	:m_defines(_defines),
	m_vertexShaderPath(_vertexShaderPath),
	m_fragmentShaderPath(_fragmentShaderPath)
{
	if (_tesselationControlShaderPath)
	{
		m_tesselationControlShaderPath = _tesselationControlShaderPath;
	}
	if (_tesselationEvaluationShaderPath)
	{
		m_tesselationEvaluationShaderPath = _tesselationEvaluationShaderPath;
	}
	if (_geometryShaderPath)
	{
		m_geometryShaderPath = _geometryShaderPath;
	}
	create();
}

ShaderProgram::ShaderProgram(const std::vector<std::tuple<ShaderType, std::string, int>> &_defines, const char *_computeShaderPath)
	:m_defines(_defines),
	m_computeShaderPath(_computeShaderPath)
{
	create();
}

GLuint ShaderProgram::createShader(GLenum _type, const char *_shaderPath)
{
	GLuint shader = glCreateShader(_type);

	std::string shaderCodeStr = Utility::readTextFile(_shaderPath).data();
	if (!m_defines.empty())
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

		for (const auto &define : m_defines)
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
	glGetProgramiv(m_programId, _type, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(m_programId, sizeof(infoLog), NULL, infoLog);
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
	if (!m_computeShaderPath.empty())
	{
		GLuint computeShader = createShader(GL_COMPUTE_SHADER, m_computeShaderPath.c_str());

		// shader Program
		m_programId = glCreateProgram();
		glAttachShader(m_programId, computeShader);

		glLinkProgram(m_programId);
		statusCheck(GL_LINK_STATUS);

		// delete the shaders as they're linked into our program now and no longer necessery
		glDetachShader(m_programId, computeShader);
		glDeleteShader(computeShader);

		// validate program
		glValidateProgram(m_programId);
		statusCheck(GL_VALIDATE_STATUS);
	}
	else
	{
		GLuint vertexShader = createShader(GL_VERTEX_SHADER, m_vertexShaderPath.c_str());
		GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, m_fragmentShaderPath.c_str());
		GLuint tessControlShader = 0;
		GLuint tessEvalShader = 0;
		GLuint geometryShader = 0;

		// make sure we dont try to create a program with only control shader
		assert(!m_tesselationEvaluationShaderPath.empty() ? true : m_tesselationControlShaderPath.empty());

		if (!m_tesselationControlShaderPath.empty())
		{
			tessControlShader = createShader(GL_TESS_CONTROL_SHADER, m_tesselationControlShaderPath.c_str());
		}
		if (!m_tesselationEvaluationShaderPath.empty())
		{
			tessEvalShader = createShader(GL_TESS_EVALUATION_SHADER, m_tesselationEvaluationShaderPath.c_str());
		}
		if (!m_geometryShaderPath.empty())
		{
			geometryShader = createShader(GL_GEOMETRY_SHADER, m_geometryShaderPath.c_str());
		}

		// shader Program
		m_programId = glCreateProgram();
		glAttachShader(m_programId, vertexShader);
		glAttachShader(m_programId, fragmentShader);
		if (!m_tesselationControlShaderPath.empty())
		{
			glAttachShader(m_programId, tessControlShader);
		}
		if (!m_tesselationEvaluationShaderPath.empty())
		{
			glAttachShader(m_programId, tessEvalShader);
		}
		if (!m_geometryShaderPath.empty())
		{
			glAttachShader(m_programId, geometryShader);
		}

		glLinkProgram(m_programId);
		statusCheck(GL_LINK_STATUS);

		// delete the shaders as they're linked into our program now and no longer necessery
		glDetachShader(m_programId, vertexShader);
		glDetachShader(m_programId, fragmentShader);
		if (!m_tesselationControlShaderPath.empty())
		{
			glDetachShader(m_programId, tessControlShader);
			glDeleteShader(tessControlShader);
		}
		if (!m_tesselationEvaluationShaderPath.empty())
		{
			glDetachShader(m_programId, tessEvalShader);
			glDeleteShader(tessEvalShader);
		}
		if (!m_geometryShaderPath.empty())
		{
			glDetachShader(m_programId, geometryShader);
			glDeleteShader(geometryShader);
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// validate program
		glValidateProgram(m_programId);
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
	glDeleteProgram(m_programId);
}

void ShaderProgram::bind()
{
	glUseProgram(m_programId);
}

GLuint ShaderProgram::getId() const
{
	return m_programId;
}

std::vector<std::tuple<ShaderProgram::ShaderType, std::string, int>> ShaderProgram::getDefines() const
{
	return m_defines;
}

const GLint ShaderProgram::createUniform(const std::string &_name) const
{
	return glGetUniformLocation(m_programId, _name.c_str());
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
	m_defines = _defines;
	glUseProgram(0);
	glDeleteProgram(m_programId);
	create();
}
