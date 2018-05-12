#pragma once
#include <string>
#include <glad\glad.h>
#include "ShaderProgram.h"
#include ".\..\..\Graphics\Material.h"


class UniformPointLight
{
public:
	UniformPointLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<PointLight> &_value, const GLint &_shadowMapTextureUnit);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	std::vector<GLint> locations;
	std::pair<std::shared_ptr<PointLight>, GLint> value;
	std::string name;
	bool firstTime;
};

class UniformSpotLight
{
public:
	UniformSpotLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<SpotLight> &_value, const GLint &_shadowMapTextureUnit);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	std::vector<GLint> locations;
	std::pair<std::shared_ptr<SpotLight>, GLint> value;
	std::string name;
	bool firstTime;
};

class UniformDirectionalLight
{
public:
	UniformDirectionalLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<DirectionalLight> &_value, const GLint &_shadowMapTextureUnit);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	std::vector<GLint> locations;
	std::pair<std::shared_ptr<DirectionalLight>, GLint> value;
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
	std::vector<GLint> locations;
	Material value;
	std::uint32_t mapBitField;
	std::string name;
	bool firstTime;
};

template<typename Type>
class Uniform
{
public:
	typedef Type ValueType;
	
	Uniform(const std::string &_name);
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