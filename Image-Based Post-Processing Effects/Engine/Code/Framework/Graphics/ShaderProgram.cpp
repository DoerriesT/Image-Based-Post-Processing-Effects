#include <glm\gtc\type_ptr.hpp>
#include <glm\ext.hpp>
#include <cassert>
#include <iostream>
#include "ShaderProgram.h"
#include ".\..\..\Utilities\Utility.h"
#include ".\..\..\Graphics\Material.h"
#include ".\..\..\Graphics\Lights.h"


ShaderProgram::ShaderProgram(const char *_vertexShaderPath, const char *_fragmentShaderPath, const char *_geometryShaderPath)
{
	GLuint vertexShader = createShader(GL_VERTEX_SHADER, _vertexShaderPath);
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, _fragmentShaderPath);
	GLuint geometryShader = 0;

	if (_geometryShaderPath)
	{
		geometryShader = createShader(GL_GEOMETRY_SHADER, _geometryShaderPath);
	}

	// shader Program
	programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	if (_geometryShaderPath)
	{
		glAttachShader(programId, geometryShader);
	}

	glLinkProgram(programId);
	statusCheck(GL_LINK_STATUS);

	// delete the shaders as they're linked into our program now and no longer necessery
	glDetachShader(programId, vertexShader);
	glDetachShader(programId, fragmentShader);
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

std::shared_ptr<ShaderProgram> ShaderProgram::createShaderProgram(const char *_vertexShaderPath, const char *_fragmentShaderPath, const char *_geometryShaderPath)
{
	return std::shared_ptr<ShaderProgram>(new ShaderProgram(_vertexShaderPath, _fragmentShaderPath, _geometryShaderPath));
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

const GLint ShaderProgram::createUniform(const std::string &_name) const
{
	const GLint id = glGetUniformLocation(programId, _name.c_str());

#ifdef _DEBUG
	// make sure we have not already created this uniform
	assert(uniforms.find(_name) == uniforms.end());
	uniforms[_name] = id;
#endif // DEBUG

	return id;
}

std::vector< GLint> ShaderProgram::createPointLightUniform(const std::string &_name) const
{
	std::string pointLightColor = _name + ".color";
	std::string pointLightPosition = _name + ".position";
	std::string pointLightRenderShadows = _name + ".renderShadows";
	std::string pointLightShadowMap = _name + ".shadowMap";
	std::string pointLightViewProjectionMatrix = _name + ".viewProjectionMatrix";

	std::vector<GLint> ids;
	ids.push_back(glGetUniformLocation(programId, pointLightColor.c_str()));
	ids.push_back(glGetUniformLocation(programId, pointLightPosition.c_str()));
	ids.push_back(glGetUniformLocation(programId, pointLightRenderShadows.c_str()));
	ids.push_back(glGetUniformLocation(programId, pointLightShadowMap.c_str()));
	ids.push_back(glGetUniformLocation(programId, pointLightViewProjectionMatrix.c_str()));

#ifdef _DEBUG
	// make sure we have not already created these uniforms
	assert(uniforms.find(pointLightColor) == uniforms.end());
	assert(uniforms.find(pointLightPosition) == uniforms.end());
	assert(uniforms.find(pointLightRenderShadows) == uniforms.end());
	assert(uniforms.find(pointLightShadowMap) == uniforms.end());
	assert(uniforms.find(pointLightViewProjectionMatrix) == uniforms.end());

	uniforms[pointLightColor] = ids[0];
	uniforms[pointLightPosition] = ids[1];
	uniforms[pointLightRenderShadows] = ids[2];
	uniforms[pointLightShadowMap] = ids[3];
	uniforms[pointLightViewProjectionMatrix] = ids[4];
#endif // DEBUG

	return ids;
}

std::vector<GLint> ShaderProgram::createSpotLightUniform(const std::string &_name) const
{
	std::string spotLightColor = _name + ".color";
	std::string spotLightPosition = _name + ".position";
	std::string spotLightDirection = _name + ".direction";
	std::string spotLightAngle = _name + ".angle";
	std::string spotLightRenderShadows = _name + ".renderShadows";
	std::string spotLightShadowMap = _name + ".shadowMap";
	std::string spotLightViewProjectionMatrix = _name + ".viewProjectionMatrix";

	std::vector<GLint> ids;
	ids.push_back(glGetUniformLocation(programId, spotLightColor.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightPosition.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightDirection.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightAngle.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightRenderShadows.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightShadowMap.c_str()));
	ids.push_back(glGetUniformLocation(programId, spotLightViewProjectionMatrix.c_str()));

#ifdef _DEBUG
	// make sure we have not already created these uniforms
	assert(uniforms.find(spotLightColor) == uniforms.end());
	assert(uniforms.find(spotLightPosition) == uniforms.end());
	assert(uniforms.find(spotLightDirection) == uniforms.end());
	assert(uniforms.find(spotLightAngle) == uniforms.end());
	assert(uniforms.find(spotLightRenderShadows) == uniforms.end());
	assert(uniforms.find(spotLightShadowMap) == uniforms.end());
	assert(uniforms.find(spotLightViewProjectionMatrix) == uniforms.end());

	uniforms[spotLightColor] = ids[0];
	uniforms[spotLightPosition] = ids[1];
	uniforms[spotLightDirection] = ids[2];
	uniforms[spotLightAngle] = ids[3];
	uniforms[spotLightRenderShadows] = ids[4];
	uniforms[spotLightShadowMap] = ids[5];
	uniforms[spotLightViewProjectionMatrix] = ids[6];
#endif // DEBUG

	return ids;
}

std::vector<GLint> ShaderProgram::createDirectionalLightUniform(const std::string &_name) const
{
	std::string directionalLightColor = _name + ".color";
	std::string directionalLightDirection = _name + ".direction";
	std::string directionalLightRenderShadows = _name + ".renderShadows";
	std::string directionalLightShadowMap = _name + ".shadowMap";
	std::string directionalLightViewProjectionMatrix = _name + ".viewProjectionMatrix";

	std::vector<GLint> ids;
	ids.push_back(glGetUniformLocation(programId, directionalLightColor.c_str()));
	ids.push_back(glGetUniformLocation(programId, directionalLightDirection.c_str()));
	ids.push_back(glGetUniformLocation(programId, directionalLightRenderShadows.c_str()));
	ids.push_back(glGetUniformLocation(programId, directionalLightShadowMap.c_str()));
	ids.push_back(glGetUniformLocation(programId, directionalLightViewProjectionMatrix.c_str()));

#ifdef _DEBUG
	// make sure we have not already created these uniforms
	assert(uniforms.find(directionalLightColor) == uniforms.end());
	assert(uniforms.find(directionalLightDirection) == uniforms.end());
	assert(uniforms.find(directionalLightRenderShadows) == uniforms.end());
	assert(uniforms.find(directionalLightShadowMap) == uniforms.end());
	assert(uniforms.find(directionalLightViewProjectionMatrix) == uniforms.end());

	uniforms[directionalLightColor] = ids[0];
	uniforms[directionalLightDirection] = ids[1];
	uniforms[directionalLightRenderShadows] = ids[2];
	uniforms[directionalLightShadowMap] = ids[3];
	uniforms[directionalLightViewProjectionMatrix] = ids[4];
#endif // DEBUG

	return ids;
}

std::vector<GLint> ShaderProgram::createPointLightUniformArray(const std::string &_name, unsigned int _size) const
{
	std::vector<GLint> locations;
	locations.reserve(_size * 5);
	for (unsigned int i = 0; i < _size; ++i)
	{
		std::vector<GLint> pointLightLocations = createPointLightUniform(_name + "[" + std::to_string(i) + "]");
		locations.insert(locations.end(), pointLightLocations.begin(), pointLightLocations.end());
	}
	return locations;
}

std::vector<GLint> ShaderProgram::createSpotLightUniformArray(const std::string &_name, unsigned int _size) const
{
	std::vector<GLint> locations;
	locations.reserve(_size * 7);
	for (unsigned int i = 0; i < _size; ++i)
	{
		std::vector<GLint> spotLightLocations = createSpotLightUniform(_name + "[" + std::to_string(i) + "]");
		locations.insert(locations.end(), spotLightLocations.begin(), spotLightLocations.end());
	}
	return locations;
}

std::vector<GLint> ShaderProgram::createDirectionalLightUniformArray(const std::string &_name, unsigned int _size) const
{
	std::vector<GLint> locations;
	locations.reserve(_size * 5);
	for (unsigned int i = 0; i < _size; ++i)
	{
		std::vector<GLint> directionalLightLocations = createDirectionalLightUniform(_name + "[" + std::to_string(i) + "]");
		locations.insert(locations.end(), directionalLightLocations.begin(), directionalLightLocations.end());
	}
	return locations;
}

std::vector<GLint> ShaderProgram::createMaterialUniform(const std::string &_name) const
{
	std::string albedo = _name + ".albedo";
	std::string metallic = _name + ".metallic";
	std::string roughness = _name + ".roughness";
	std::string ao = _name + ".ao";
	std::string emissive = _name + ".emissive";
	std::string mapBitField = _name + ".mapBitField";
	std::string albedoMap = _name + ".albedoMap";
	std::string normalMap = _name + ".normalMap";
	std::string metallicMap = _name + ".metallicMap";
	std::string roughnessMap = _name + ".roughnessMap";
	std::string aoMap = _name + ".aoMap";
	std::string emissiveMap = _name + ".emissiveMap";

#ifdef _DEBUG
	// make sure we have not already created these uniforms
	assert(uniforms.find(albedo) == uniforms.end());
	assert(uniforms.find(metallic) == uniforms.end());
	assert(uniforms.find(roughness) == uniforms.end());
	assert(uniforms.find(ao) == uniforms.end());
	assert(uniforms.find(emissive) == uniforms.end());
	assert(uniforms.find(mapBitField) == uniforms.end());
	assert(uniforms.find(albedoMap) == uniforms.end());
	assert(uniforms.find(normalMap) == uniforms.end());
	assert(uniforms.find(metallicMap) == uniforms.end());
	assert(uniforms.find(roughnessMap) == uniforms.end());
	assert(uniforms.find(aoMap) == uniforms.end());
	assert(uniforms.find(emissiveMap) == uniforms.end());
#endif // DEBUG

	std::vector<GLint> locations;

	locations.push_back(createUniform(albedo));
	locations.push_back(createUniform(metallic));
	locations.push_back(createUniform(roughness));
	locations.push_back(createUniform(ao));
	locations.push_back(createUniform(emissive));
	locations.push_back(createUniform(mapBitField));
	locations.push_back(createUniform(albedoMap));
	locations.push_back(createUniform(normalMap));
	locations.push_back(createUniform(metallicMap));
	locations.push_back(createUniform(roughnessMap));
	locations.push_back(createUniform(aoMap));
	locations.push_back(createUniform(emissiveMap));

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

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, std::shared_ptr<PointLight> _value, int _shadowMapTextureUnit) const
{
	setUniform(_locations[0], _value->getColor());
	setUniform(_locations[1], _value->getViewPosition());
	setUniform(_locations[2], _value->isRenderShadows());
	setUniform(_locations[3], _shadowMapTextureUnit);
	setUniform(_locations[4], _value->getViewProjectionMatrix());
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, std::shared_ptr<SpotLight> _value, int _shadowMapTextureUnit) const
{
	setUniform(_locations[0], _value->getColor());
	setUniform(_locations[1], _value->getViewPosition());
	setUniform(_locations[2], _value->getViewDirection());
	setUniform(_locations[3], _value->getAngle());
	setUniform(_locations[4], _value->isRenderShadows());
	setUniform(_locations[5], _shadowMapTextureUnit);
	setUniform(_locations[6], _value->getViewProjectionMatrix());
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, std::shared_ptr<DirectionalLight> _value, int _shadowMapTextureUnit) const
{
	setUniform(_locations[0], _value->getColor());
	setUniform(_locations[1], _value->getViewDirection());
	setUniform(_locations[2], _value->isRenderShadows());
	setUniform(_locations[3], _shadowMapTextureUnit);
	setUniform(_locations[4], _value->getViewProjectionMatrix());
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, const Material *_value) const
{
	setUniform(_locations[0], glm::pow(_value->getAlbedo(), glm::vec4(glm::vec3(1.0 / 2.2), 1.0)));
	setUniform(_locations[1], _value->getMetallic());
	setUniform(_locations[2], _value->getRoughness());
	setUniform(_locations[3], 1.0f);
	setUniform(_locations[4], _value->getEmissive());
	setUniform(_locations[5], (int)_value->getMapBitField());
	setUniform(_locations[6], 0);
	setUniform(_locations[7], 1);
	setUniform(_locations[8], 2);
	setUniform(_locations[9], 3);
	setUniform(_locations[10], 4);
	setUniform(_locations[11], 5);
}
