#include "Uniform.h"
#include ".\..\..\Graphics\Lights.h"

bool operator==(const std::shared_ptr<PointLight> &_lhv, const std::shared_ptr<PointLight> &_rhv)
{
	return _lhv->getColor() == _rhv->getColor() &&
		_lhv->getPosition() == _rhv->getPosition() &&
		_lhv->isRenderShadows() == _rhv->isRenderShadows() &&
		_lhv->getShadowMap() == _rhv->getShadowMap() &&
		_lhv->getViewProjectionMatrix() == _rhv->getViewProjectionMatrix();
}

bool operator==(const std::shared_ptr<SpotLight> &_lhv, const std::shared_ptr<SpotLight> &_rhv)
{
	return _lhv->getColor() == _rhv->getColor() &&
		_lhv->getPosition() == _rhv->getPosition() &&
		_lhv->getDirection() == _rhv->getDirection() &&
		_lhv->getAngle() == _rhv->getAngle() &&
		_lhv->isRenderShadows() == _rhv->isRenderShadows() &&
		_lhv->getShadowMap() == _rhv->getShadowMap() &&
		_lhv->getViewProjectionMatrix() == _rhv->getViewProjectionMatrix();
}

bool operator==(const std::shared_ptr<DirectionalLight> &_lhv, const std::shared_ptr<DirectionalLight> &_rhv)
{
	return _lhv->getColor() == _rhv->getColor() &&
		_lhv->getDirection() == _rhv->getDirection() &&
		_lhv->isRenderShadows() == _rhv->isRenderShadows() &&
		_lhv->getShadowMap() == _rhv->getShadowMap() &&
		_lhv->getViewProjectionMatrix() == _rhv->getViewProjectionMatrix();
}

bool operator==(Material &_lhv, Material &_rhv)
{
	return _lhv.getAlbedoMap() == _rhv.getAlbedoMap() &&
		_lhv.getNormalMap() == _rhv.getNormalMap() &&
		_lhv.getMetallicMap() == _rhv.getMetallicMap() &&
		_lhv.getRoughnessMap() == _rhv.getRoughnessMap() &&
		_lhv.getAoMap() == _rhv.getAoMap() &&
		_lhv.getEmissiveMap() == _rhv.getEmissiveMap() &&
		_lhv.getMapBitField() == _rhv.getMapBitField() &&
		_lhv.getAlbedo() == _rhv.getAlbedo() &&
		_lhv.getMetallic() == _rhv.getMetallic() &&
		_lhv.getRoughness() == _rhv.getRoughness() &&
		_lhv.getEmissive() == _rhv.getEmissive();
}

UniformPointLight::UniformPointLight(const std::string &_name)
	:locations(), name(_name), firstTime(true)
{
}

void UniformPointLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	locations = shaderProgram->createPointLightUniform(name);
}

void UniformPointLight::set(const std::shared_ptr<PointLight> &_value, const GLint &_shadowMapTextureUnit)
{
	auto p = std::make_pair(_value, _shadowMapTextureUnit);
	if (firstTime || value != p)
	{
		firstTime = false;
		value = p;
		shaderProgram->setUniform(locations, value.first, value.second);
	}
}

bool UniformPointLight::isValid()
{
	return !locations.empty();
}

UniformSpotLight::UniformSpotLight(const std::string &_name)
	:locations(), name(_name), firstTime(true)
{
}

void UniformSpotLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	locations = shaderProgram->createSpotLightUniform(name);
}

void UniformSpotLight::set(const std::shared_ptr<SpotLight> &_value, const GLint &_shadowMapTextureUnit)
{
	auto p = std::make_pair(_value, _shadowMapTextureUnit);
	if (firstTime || value != p)
	{
		firstTime = false;
		value = p;
		shaderProgram->setUniform(locations, value.first, value.second);
	}
}

bool UniformSpotLight::isValid()
{
	return !locations.empty();
}

UniformDirectionalLight::UniformDirectionalLight(const std::string &_name)
	:locations(), name(_name), firstTime(true)
{
}

void UniformDirectionalLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	locations = shaderProgram->createDirectionalLightUniform(name);
}

void UniformDirectionalLight::set(const std::shared_ptr<DirectionalLight> &_value, const GLint &_shadowMapTextureUnit)
{
	auto p = std::make_pair(_value, _shadowMapTextureUnit);
	//if (firstTime || value != p)
	{
		firstTime = false;
		value = p;
		shaderProgram->setUniform(locations, value.first, value.second);
	}
}

bool UniformDirectionalLight::isValid()
{
	return !locations.empty();
}

UniformMaterial::UniformMaterial(const std::string &_name)
	:locations(), name(_name), firstTime(true)
{
}

void UniformMaterial::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	shaderProgram = _shaderProgram;
	locations = shaderProgram->createMaterialUniform(name);
}

void UniformMaterial::set(const Material *_value)
{
	std::uint32_t _mapBitField = _value->getMapBitField();
	if (firstTime || 
		value.getAlbedoMap() != _value->getAlbedoMap() ||
		value.getNormalMap() != _value->getNormalMap() ||
		value.getMetallicMap() != _value->getMetallicMap() ||
		value.getRoughnessMap() != _value->getRoughnessMap() ||
		value.getAoMap() != _value->getAoMap() ||
		value.getEmissiveMap() != _value->getEmissiveMap() ||
		mapBitField != _mapBitField ||
		value.getAlbedo() != _value->getAlbedo() ||
		value.getMetallic() != _value->getMetallic() ||
		value.getRoughness() != _value->getRoughness() ||
		value.getEmissive() != _value->getEmissive())
	{
		firstTime = false;
		value = *_value;
		mapBitField = _mapBitField;
		shaderProgram->setUniform(locations, &value);
	}
}

bool UniformMaterial::isValid()
{
	return !locations.empty();
}
