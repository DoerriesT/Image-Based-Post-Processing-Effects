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

std::vector< GLint> ShaderProgram::createPointLightUniform(const std::string &_name) const
{
	std::string pointLightColor = _name + ".color";
	std::string pointLightPosition = _name + ".position";
	std::string pointLightRadius = _name + ".radius";
	std::string pointLightRenderShadows = _name + ".renderShadows";

	std::vector<GLint> ids;
	ids.push_back(glGetUniformLocation(programId, pointLightColor.c_str()));
	ids.push_back(glGetUniformLocation(programId, pointLightPosition.c_str()));
	ids.push_back(glGetUniformLocation(programId, pointLightRadius.c_str()));
	ids.push_back(glGetUniformLocation(programId, pointLightRenderShadows.c_str()));

	return ids;
}

std::vector<GLint> ShaderProgram::createSpotLightUniform(const std::string &_name) const
{
	std::string spotLightColor = _name + ".color";
	std::string spotLightPosition = _name + ".position";
	std::string spotLightDirection = _name + ".direction";
	std::string spotLightOuterAngle = _name + ".outerAngle";
	std::string spotLightInnerAngle = _name + ".innerAngle";
	std::string spotLightRadius = _name + ".radius";
	std::string spotLightRenderShadows = _name + ".renderShadows";
	std::string spotLightViewProjectionMatrix = _name + ".viewProjectionMatrix";

	std::vector<GLint> ids;
	ids.push_back(glGetUniformLocation(programId, spotLightColor.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightPosition.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightDirection.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightOuterAngle.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightInnerAngle.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightRadius.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightRenderShadows.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightViewProjectionMatrix.c_str()));

	return ids;
}

std::vector<GLint> ShaderProgram::createDirectionalLightUniform(const std::string &_name) const
{
	std::string directionalLightColor = _name + ".color";
	std::string directionalLightDirection = _name + ".direction";
	std::string directionalLightRenderShadows = _name + ".renderShadows";
	std::string directionalLightViewProjectionMatrix = _name + ".viewProjectionMatrices";
	std::string directionalLightSplits = _name + ".splits";

	std::vector<GLint> ids;
	ids.push_back(glGetUniformLocation(programId, directionalLightColor.c_str()));
	ids.push_back(glGetUniformLocation(programId, directionalLightDirection.c_str()));
	ids.push_back(glGetUniformLocation(programId, directionalLightRenderShadows.c_str()));
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		ids.push_back(glGetUniformLocation(programId, (directionalLightViewProjectionMatrix + "["+std::to_string(i) + "]").c_str()));
	}
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		ids.push_back(glGetUniformLocation(programId, (directionalLightSplits + "[" + std::to_string(i) + "]").c_str()));
	}
	
	return ids;
}

std::vector<GLint> ShaderProgram::createMaterialUniform(const std::string &_name) const
{
	std::string albedo = _name + ".albedo";
	std::string metallic = _name + ".metallic";
	std::string roughness = _name + ".roughness";
	std::string ao = _name + ".ao";
	std::string emissive = _name + ".emissive";
	std::string mapBitField = _name + ".mapBitField";

	std::vector<GLint> locations;

	locations.push_back(createUniform(albedo));
	locations.push_back(createUniform(metallic));
	locations.push_back(createUniform(roughness));
	locations.push_back(createUniform(ao));
	locations.push_back(createUniform(emissive));
	locations.push_back(createUniform(mapBitField));

	return locations;
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

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, std::shared_ptr<PointLight> _value) const
{
	setUniform(_locations[0], _value->getColor());
	setUniform(_locations[1], _value->getViewPosition());
	setUniform(_locations[2], _value->getRadius());
	setUniform(_locations[3], _value->isRenderShadows());
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, std::shared_ptr<SpotLight> _value) const
{
	setUniform(_locations[0], _value->getColor());
	setUniform(_locations[1], _value->getViewPosition());
	setUniform(_locations[2], _value->getViewDirection());
	setUniform(_locations[3], _value->getOuterAngleCos());
	setUniform(_locations[4], _value->getInnerAngleCos());
	setUniform(_locations[5], _value->getRadius());
	setUniform(_locations[6], _value->isRenderShadows());
	setUniform(_locations[7], _value->getViewProjectionMatrix());
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, std::shared_ptr<DirectionalLight> _value) const
{
	setUniform(_locations[0], _value->getColor());
	setUniform(_locations[1], _value->getViewDirection());
	setUniform(_locations[2], _value->isRenderShadows());
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		setUniform(_locations[3 + i], _value->getViewProjectionMatrices()[i]);
	}
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		setUniform(_locations[3 + SHADOW_CASCADES + i], _value->getSplits()[i]);
	}
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, const Material *_value) const
{
	setUniform(_locations[0], glm::pow(_value->getAlbedo(), glm::vec4(glm::vec3(1.0f / 2.2f), 1.0f)));
	setUniform(_locations[1], _value->getMetallic());
	setUniform(_locations[2], _value->getRoughness());
	setUniform(_locations[3], 1.0f);
	setUniform(_locations[4], _value->getEmissive());
	setUniform(_locations[5], (int)_value->getMapBitField());
}
