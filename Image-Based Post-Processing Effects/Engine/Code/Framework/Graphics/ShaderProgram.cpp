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
	const char *vertexShaderCode = readTextResourceFile(_vertexShaderPath);
	const char *fragmentShaderCode = readTextResourceFile(_fragmentShaderPath);
	const char *geometryShaderCode = nullptr;
	if (_geometryShaderPath)
	{
		geometryShaderCode = readTextResourceFile(_geometryShaderPath);
	}

	unsigned int vertex, fragment, geometry;
	int success;
	char infoLog[512];

	// vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexShaderCode, NULL);
	glCompileShader(vertex);
	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << _vertexShaderPath << "\n" <<infoLog << std::endl;
	};

	// fragement Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << _fragmentShaderPath << "\n" << infoLog << std::endl;
	};

	if (_geometryShaderPath)
	{
		// geometry Shader
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &geometryShaderCode, NULL);
		glCompileShader(geometry);
		// print compile errors if any
		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometry, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << _geometryShaderPath << "\n" << infoLog << std::endl;
			assert(false);
		};
	}

	// shader Program
	programId = glCreateProgram();
	glAttachShader(programId, vertex);
	glAttachShader(programId, fragment);
	if (_geometryShaderPath)
	{
		glAttachShader(programId, geometry);
	}
	glLinkProgram(programId);
	// print linking errors if any
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// delete the shaders as they're linked into our program now and no longer necessery
	glDetachShader(programId, vertex);
	glDetachShader(programId, fragment);
	if (_geometryShaderPath)
	{
		glDetachShader(programId, geometry);
		glDeleteShader(geometry);
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// validate program
	glValidateProgram(programId);
	glGetProgramiv(programId, GL_VALIDATE_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, 512, NULL, infoLog);
		std::cout << "WARNING::SHADER::PROGRAM::VALIDATION_FAILED\n" << infoLog << std::endl;
	}

	delete[] vertexShaderCode;
	delete[] fragmentShaderCode;
	if (_geometryShaderPath)
	{
		delete[] geometryShaderCode;
	}
}

std::shared_ptr<ShaderProgram> ShaderProgram::createShaderProgram(const char *_vertexShaderPath, const char *_fragmentShaderPath, const char *_geometryShaderPath)
{
	return std::shared_ptr<ShaderProgram>(new ShaderProgram(_vertexShaderPath, _fragmentShaderPath, _geometryShaderPath));
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

std::vector<GLint> ShaderProgram::createPointLightUniformArray(const std::string &_name, const unsigned int &_size) const
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

std::vector<GLint> ShaderProgram::createSpotLightUniformArray(const std::string &_name, const unsigned int &_size) const
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

std::vector<GLint> ShaderProgram::createDirectionalLightUniformArray(const std::string &_name, const unsigned int &_size) const
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

void ShaderProgram::setUniform(const GLint &_location, const GLuint &_value) const
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
	setUniform(_locations[1], _value->getPosition());
	setUniform(_locations[2], _value->isRenderShadows());
	setUniform(_locations[3], _shadowMapTextureUnit);
	setUniform(_locations[4], _value->getViewProjectionMatrix());
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, std::shared_ptr<SpotLight> _value, int _shadowMapTextureUnit) const
{
	setUniform(_locations[0], _value->getColor());
	setUniform(_locations[1], _value->getPosition());
	setUniform(_locations[2], _value->getDirection());
	setUniform(_locations[3], _value->getAngle());
	setUniform(_locations[4], _value->isRenderShadows());
	setUniform(_locations[5], _shadowMapTextureUnit);
	setUniform(_locations[6], _value->getViewProjectionMatrix());
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, std::shared_ptr<DirectionalLight> _value, int _shadowMapTextureUnit) const
{
	setUniform(_locations[0], _value->getColor());
	setUniform(_locations[1], _value->getDirection());
	setUniform(_locations[2], _value->isRenderShadows());
	setUniform(_locations[3], _shadowMapTextureUnit);
	setUniform(_locations[4], _value->getViewProjectionMatrix());
}

void ShaderProgram::setUniform(const std::vector<GLint> &_locations, const Material *_value) const
{
	setUniform(_locations[0], _value->getAlbedo());
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
