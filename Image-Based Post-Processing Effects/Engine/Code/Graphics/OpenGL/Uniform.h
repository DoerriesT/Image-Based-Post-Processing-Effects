#pragma once
#include <string>
#include <glad\glad.h>
#include "ShaderProgram.h"
#include "Graphics\Material.h"
#include "Graphics\Lights.h"

template<typename Type>
class Uniform
{
public:
	typedef Type ValueType;
	
	Uniform(const std::string &_name = "");
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const Type &_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	GLint location;
	Type value;
	std::string name;
	bool firstTime;
};

template<typename Type>
inline Uniform<Type>::Uniform(const std::string &_name)
	:location(-1), name(_name),  firstTime(true)
{
}

template<typename Type>
inline void Uniform<Type>::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	location = shaderProgram->createUniform(name);
	//assert(isValid());
}

template<typename Type>
inline void Uniform<Type>::set(const Type &_value)
{
	//assert(isValid());
	if (firstTime || value != _value)
	{
		firstTime = false;
		value = _value;
		shaderProgram->setUniform(location, value);
	}
}

template<typename Type>
inline bool Uniform<Type>::isValid()
{
	return location != -1;
}

class UniformPointLight
{
public:
	UniformPointLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<PointLight> &_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	Uniform<glm::vec3> color;
	Uniform<glm::vec3> viewPosition;
	Uniform<GLfloat> radius;
	Uniform<GLboolean> renderShadows;
	std::string name;
	bool firstTime;
};

class UniformSpotLight
{
public:
	UniformSpotLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<SpotLight> &_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	Uniform<glm::vec3> color;
	Uniform<glm::vec3> viewPosition;
	Uniform<glm::vec3> viewDirection;
	Uniform<GLfloat> outerAngle;
	Uniform<GLfloat> innerAngle;
	Uniform<GLfloat> radius;
	Uniform<GLboolean> renderShadows;
	Uniform<GLboolean> projector;
	Uniform<glm::mat4> viewProjection;
	std::string name;
	bool firstTime;
};

class UniformDirectionalLight
{
public:
	UniformDirectionalLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<DirectionalLight> &_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	Uniform<glm::vec3> color;
	Uniform<glm::vec3> viewDirection;
	Uniform<GLboolean> renderShadows;
	Uniform<glm::mat4> viewProjection[SHADOW_CASCADES];
	Uniform<GLfloat> splits[SHADOW_CASCADES];
	std::string name;
	bool firstTime;
};

class UniformMaterial
{
public:
	UniformMaterial(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const Material *_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	Uniform<glm::vec4> albedo;
	Uniform<GLfloat> metallic;
	Uniform<GLfloat> roughness;
	Uniform<glm::vec3> emissive;
	Uniform<GLint> mapBitField;
	Uniform<GLboolean> displacement;
	std::string name;
	bool firstTime;
};