#include "Uniform.h"

UniformPointLight::UniformPointLight(const std::string &_name)
	:color(_name + ".color"),
	viewPosition(_name + ".position"),
	radius(_name + ".radius"),
	renderShadows(_name + ".renderShadows"),
	name(_name),
	firstTime(true)
{
}

void UniformPointLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	color.create(shaderProgram);
	viewPosition.create(shaderProgram);
	radius.create(shaderProgram);
	renderShadows.create(shaderProgram);
}

void UniformPointLight::set(const std::shared_ptr<PointLight> &_value)
{
	color.set(_value->getColor());
	viewPosition.set(_value->getViewPosition());
	radius.set(_value->getRadius());
	renderShadows.set(_value->isRenderShadows());
}

bool UniformPointLight::isValid()
{
	return color.isValid() &&
		viewPosition.isValid() &&
		radius.isValid() &&
		renderShadows.isValid();
}

UniformSpotLight::UniformSpotLight(const std::string &_name)
	:color(_name + ".color"),
	viewPosition(_name + ".position"),
	viewDirection(_name + ".direction"),
	outerAngle(_name + ".outerAngle"),
	innerAngle(_name + ".innerAngle"),
	radius(_name + ".radius"),
	renderShadows(_name + ".renderShadows"),
	viewProjection(_name + ".viewProjectionMatrix"),
	name(_name),
	firstTime(true)
{
}

void UniformSpotLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	color.create(shaderProgram);
	viewPosition.create(shaderProgram);
	viewDirection.create(shaderProgram);
	outerAngle.create(shaderProgram);
	innerAngle.create(shaderProgram);
	radius.create(shaderProgram);
	renderShadows.create(shaderProgram);
	viewProjection.create(shaderProgram);
}

void UniformSpotLight::set(const std::shared_ptr<SpotLight> &_value)
{
	color.set(_value->getColor());
	viewPosition.set(_value->getViewPosition());
	viewDirection.set(_value->getViewDirection());
	outerAngle.set(_value->getOuterAngleCos());
	innerAngle.set(_value->getInnerAngleCos());
	radius.set(_value->getRadius());
	renderShadows.set(_value->isRenderShadows());
	viewProjection.set(_value->getViewProjectionMatrix());
}

bool UniformSpotLight::isValid()
{
	return viewPosition.isValid() &&
		viewDirection.isValid() &&
		outerAngle.isValid() &&
		innerAngle.isValid() &&
		radius.isValid() &&
		renderShadows.isValid() &&
		viewProjection.isValid();
}

UniformDirectionalLight::UniformDirectionalLight(const std::string &_name)
	:color(_name + ".color"),
	viewDirection(_name + ".direction"),
	renderShadows(_name + ".renderShadows"),
	name(_name),
	firstTime(true)
{
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		viewProjection[i] = Uniform<glm::mat4>(_name + ".viewProjectionMatrices[" + std::to_string(i) + "]");
		splits[i] = Uniform<GLfloat>(_name + ".splits[" + std::to_string(i) + "]");
	}
}

void UniformDirectionalLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	color.create(shaderProgram);
	viewDirection.create(shaderProgram);
	renderShadows.create(shaderProgram);
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		viewProjection[i].create(shaderProgram);
		splits[i].create(shaderProgram);
	}
}

void UniformDirectionalLight::set(const std::shared_ptr<DirectionalLight> &_value)
{
	color.set(_value->getColor());
	viewDirection.set(_value->getViewDirection());
	renderShadows.set(_value->isRenderShadows());

	const glm::mat4 *viewProjections = _value->getViewProjectionMatrices();
	const float *cascadeSplits = _value->getSplits();

	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		viewProjection[i].set(viewProjections[i]);
		splits[i].set(cascadeSplits[i]);
	}
}

bool UniformDirectionalLight::isValid()
{
	bool validArrays = true;

	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		validArrays &= viewProjection[i].isValid();
		validArrays &= splits[i].isValid();
	}

	return validArrays && color.isValid() && viewDirection.isValid() && renderShadows.isValid();
}

UniformMaterial::UniformMaterial(const std::string &_name)
	:albedo(_name + ".albedo"),
	metallic(_name + ".metallic"),
	roughness(_name + ".roughness"),
	emissive(_name + ".emissive"),
	mapBitField(_name + ".mapBitField"),
	name(_name),
	firstTime(true)
{
}

void UniformMaterial::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	albedo.create(shaderProgram);
	metallic.create(shaderProgram);
	roughness.create(shaderProgram);
	emissive.create(shaderProgram);
	mapBitField.create(shaderProgram);
}

void UniformMaterial::set(const Material *_value)
{
	albedo.set(_value->getAlbedo());
	metallic.set(_value->getMetallic());
	roughness.set(_value->getRoughness());
	emissive.set(_value->getEmissive());
	mapBitField.set(_value->getMapBitField());
}

bool UniformMaterial::isValid()
{
	return albedo.isValid() &&
		metallic.isValid() &&
		roughness.isValid() &&
		emissive.isValid() &&
		mapBitField.isValid();
}
