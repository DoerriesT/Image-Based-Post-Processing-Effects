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
	std::shared_ptr<ShaderProgram> m_shaderProgram;
	GLint m_location;
	Type m_value;
	std::string m_name;
	bool m_firstTime;
};

template<typename Type>
inline Uniform<Type>::Uniform(const std::string &_name)
	:m_location(-1), m_name(_name),  m_firstTime(true)
{
}

template<typename Type>
inline void Uniform<Type>::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	m_shaderProgram = _shaderProgram;
	m_location = m_shaderProgram->createUniform(m_name);
	m_firstTime = true;
	//assert(isValid());
}

template<typename Type>
inline void Uniform<Type>::set(const Type &_value)
{
	//assert(isValid());
	if (m_firstTime || m_value != _value)
	{
		m_firstTime = false;
		m_value = _value;
		m_shaderProgram->setUniform(m_location, m_value);
	}
}

template<typename Type>
inline bool Uniform<Type>::isValid()
{
	return m_location != -1;
}

class UniformPointLight
{
public:
	UniformPointLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<PointLight> &_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> m_shaderProgram;
	Uniform<glm::vec3> m_color;
	Uniform<glm::vec3> m_viewPosition;
	Uniform<GLfloat> m_radius;
	Uniform<GLfloat> m_invSqrAttRadius;
	Uniform<GLboolean> m_renderShadows;
	std::string m_name;
	bool m_firstTime;
};

class UniformSpotLight
{
public:
	UniformSpotLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<SpotLight> &_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> m_shaderProgram;
	Uniform<glm::vec3> m_color;
	Uniform<glm::vec3> m_viewPosition;
	Uniform<glm::vec3> m_viewDirection;
	Uniform<GLfloat> m_angleScale;
	Uniform<GLfloat> m_angleOffset;
	Uniform<GLfloat> m_invSqrAttRadius;
	Uniform<GLboolean> m_renderShadows;
	Uniform<GLboolean> m_projector;
	Uniform<glm::mat4> m_viewProjection;
	std::string m_name;
	bool m_firstTime;
};

class UniformDirectionalLight
{
public:
	UniformDirectionalLight(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const std::shared_ptr<DirectionalLight> &_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> m_shaderProgram;
	Uniform<glm::vec3> m_color;
	Uniform<glm::vec3> m_viewDirection;
	Uniform<GLboolean> m_renderShadows;
	Uniform<glm::mat4> m_viewProjection[SHADOW_CASCADES];
	Uniform<GLfloat> m_splits[SHADOW_CASCADES];
	std::string m_name;
	bool m_firstTime;
};

class UniformMaterial
{
public:
	UniformMaterial(const std::string &_name);
	void create(const std::shared_ptr<ShaderProgram> &_shaderProgram);
	void set(const Material *_value);
	bool isValid();

private:
	std::shared_ptr<ShaderProgram> m_shaderProgram;
	Uniform<glm::vec4> m_albedo;
	Uniform<GLfloat> m_metallic;
	Uniform<GLfloat> m_roughness;
	Uniform<glm::vec3> m_emissive;
	Uniform<GLint> m_mapBitField;
	Uniform<GLboolean> m_displacement;
	std::string m_name;
	bool m_firstTime;
};