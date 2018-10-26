#include "Uniform.h"
#include "Graphics\Texture.h"

UniformPointLight::UniformPointLight(const std::string &_name)
	:m_color(_name + ".color"),
	m_viewPosition(_name + ".position"),
	m_radius(_name + ".radius"),
	m_invSqrAttRadius(_name + ".invSqrAttRadius"),
	m_renderShadows(_name + ".renderShadows"),
	m_name(_name),
	m_firstTime(true)
{
}

void UniformPointLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	m_shaderProgram = _shaderProgram;
	m_color.create(m_shaderProgram);
	m_viewPosition.create(m_shaderProgram);
	m_radius.create(m_shaderProgram);
	m_invSqrAttRadius.create(m_shaderProgram);
	m_renderShadows.create(m_shaderProgram);
}

void UniformPointLight::set(const std::shared_ptr<PointLight> &_value)
{
	m_color.set(_value->getColor() * _value->getLuminousIntensity());
	m_viewPosition.set(_value->getViewPosition());
	m_radius.set(_value->getRadius());
	m_invSqrAttRadius.set(_value->getInvSqrRadius());
	m_renderShadows.set(_value->isRenderShadows());
}

bool UniformPointLight::isValid()
{
	return m_color.isValid() &&
		m_viewPosition.isValid() &&
		m_radius.isValid() &&
		m_invSqrAttRadius.isValid() &&
		m_renderShadows.isValid();
}

UniformSpotLight::UniformSpotLight(const std::string &_name)
	:m_color(_name + ".color"),
	m_viewPosition(_name + ".position"),
	m_viewDirection(_name + ".direction"),
	m_angleScale(_name + ".angleScale"),
	m_angleOffset(_name + ".angleOffset"),
	m_invSqrAttRadius(_name + ".invSqrAttRadius"),
	m_renderShadows(_name + ".renderShadows"),
	m_projector(_name + ".projector"),
	m_viewProjection(_name + ".viewProjectionMatrix"),
	m_name(_name),
	m_firstTime(true)
{
}

void UniformSpotLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	m_shaderProgram = _shaderProgram;
	m_color.create(m_shaderProgram);
	m_viewPosition.create(m_shaderProgram);
	m_viewDirection.create(m_shaderProgram);
	m_angleScale.create(m_shaderProgram);
	m_angleOffset.create(m_shaderProgram);
	m_invSqrAttRadius.create(m_shaderProgram);
	m_renderShadows.create(m_shaderProgram);
	m_projector.create(m_shaderProgram);
	m_viewProjection.create(m_shaderProgram);
}

void UniformSpotLight::set(const std::shared_ptr<SpotLight> &_value)
{
	m_color.set(_value->getColor() * _value->getLuminousIntensity());
	m_viewPosition.set(_value->getViewPosition());
	m_viewDirection.set(_value->getViewDirection());
	m_angleScale.set(_value->getAngleScale());
	m_angleOffset.set(_value->getAngleOffset());
	m_invSqrAttRadius.set(_value->getInvSqrRadius());
	m_renderShadows.set(_value->isRenderShadows());
	m_projector.set(_value->isProjector());
	m_viewProjection.set(_value->getViewProjectionMatrix());
}

bool UniformSpotLight::isValid()
{
	return m_viewPosition.isValid() &&
		m_viewDirection.isValid() &&
		m_angleScale.isValid() &&
		m_angleOffset.isValid() &&
		m_invSqrAttRadius.isValid() &&
		m_renderShadows.isValid() &&
		m_viewProjection.isValid();
}

UniformDirectionalLight::UniformDirectionalLight(const std::string &_name)
	:m_color(_name + ".color"),
	m_viewDirection(_name + ".direction"),
	m_renderShadows(_name + ".renderShadows"),
	m_name(_name),
	m_firstTime(true)
{
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		m_viewProjection[i] = Uniform<glm::mat4>(_name + ".viewProjectionMatrices[" + std::to_string(i) + "]");
		m_splits[i] = Uniform<GLfloat>(_name + ".splits[" + std::to_string(i) + "]");
	}
}

void UniformDirectionalLight::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	m_shaderProgram = _shaderProgram;
	m_color.create(m_shaderProgram);
	m_viewDirection.create(m_shaderProgram);
	m_renderShadows.create(m_shaderProgram);
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		m_viewProjection[i].create(m_shaderProgram);
		m_splits[i].create(m_shaderProgram);
	}
}

void UniformDirectionalLight::set(const std::shared_ptr<DirectionalLight> &_value)
{
	m_color.set(_value->getColor());
	m_viewDirection.set(_value->getViewDirection());
	m_renderShadows.set(_value->isRenderShadows());

	const glm::mat4 *viewProjections = _value->getViewProjectionMatrices();
	const float *cascadeSplits = _value->getSplits();

	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		m_viewProjection[i].set(viewProjections[i]);
		m_splits[i].set(cascadeSplits[i]);
	}
}

bool UniformDirectionalLight::isValid()
{
	bool validArrays = true;

	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		validArrays &= m_viewProjection[i].isValid();
		validArrays &= m_splits[i].isValid();
	}

	return validArrays && m_color.isValid() && m_viewDirection.isValid() && m_renderShadows.isValid();
}

UniformMaterial::UniformMaterial(const std::string &_name)
	:m_albedo(_name + ".albedo"),
	m_metallic(_name + ".metallic"),
	m_roughness(_name + ".roughness"),
	m_emissive(_name + ".emissive"),
	m_mapBitField(_name + ".mapBitField"),
	m_displacement(_name + ".displacement"),
	m_name(_name),
	m_firstTime(true)
{
}

void UniformMaterial::create(const std::shared_ptr<ShaderProgram> &_shaderProgram)
{
	m_shaderProgram = _shaderProgram;
	m_albedo.create(m_shaderProgram);
	m_metallic.create(m_shaderProgram);
	m_roughness.create(m_shaderProgram);
	m_emissive.create(m_shaderProgram);
	m_mapBitField.create(m_shaderProgram);
	m_displacement.create(m_shaderProgram);
}

bool disp = true;

void UniformMaterial::set(const Material *_value)
{
	m_albedo.set(_value->getAlbedo());
	m_metallic.set(_value->getMetallic());
	m_roughness.set(_value->getRoughness());
	m_emissive.set(_value->getEmissive());
	m_mapBitField.set(_value->getMapBitField());
	std::shared_ptr<Texture> displacementMap = _value->getDisplacementMap();
	m_displacement.set(displacementMap && displacementMap->isValid() && disp);
}

bool UniformMaterial::isValid()
{
	return m_albedo.isValid() &&
		m_metallic.isValid() &&
		m_roughness.isValid() &&
		m_emissive.isValid() &&
		m_mapBitField.isValid() &&
		m_displacement.isValid();
}
