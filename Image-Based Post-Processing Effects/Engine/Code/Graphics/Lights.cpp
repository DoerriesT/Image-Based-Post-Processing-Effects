#include "Lights.h"
#include <glad\glad.h>
#include <glm\trigonometric.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include ".\..\Utilities\Utility.h"

const unsigned int DirectionalLight::DEFAULT_SHADOW_MAP_RESOLUTION = 2048;

DirectionalLight::DirectionalLight(Mobility _mobility, const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows, unsigned int _shadowMapResolution)
	:m_mobility(_mobility), m_color(_color), m_direction(glm::normalize(_direction)), m_renderShadows(false), m_shadowMapResolution(_shadowMapResolution)
{
	setRenderShadows(_renderShadows);
}

void DirectionalLight::createShadowMap()
{
	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadowMap);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, m_shadowMapResolution, m_shadowMapResolution, SHADOW_CASCADES);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
}

std::shared_ptr<DirectionalLight> DirectionalLight::createDirectionalLight(Mobility _mobility, const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows, unsigned int _shadowMapResolution)
{
	return std::shared_ptr<DirectionalLight>(new DirectionalLight(_mobility, _color, _direction, _renderShadows, _shadowMapResolution));
}

DirectionalLight::~DirectionalLight()
{
	if (m_renderShadows)
	{
		glDeleteTextures(1, &m_shadowMap);
	}
}

void DirectionalLight::setRenderShadows(bool _renderShadows)
{
	if (m_renderShadows != _renderShadows)
	{
		m_renderShadows = _renderShadows;

		if (m_renderShadows)
		{
			createShadowMap();
		}
		else
		{
			glDeleteTextures(1, &m_shadowMap);
		}
	}
}

void DirectionalLight::setColor(const glm::vec3 &_color)
{
	m_color = _color;
}

void DirectionalLight::setDirection(const glm::vec3 &_direction)
{
	m_direction = glm::normalize(_direction);
}

void DirectionalLight::setViewProjectionMatrices(glm::mat4 *_viewProjectionMatrices)
{
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		m_viewProjectionMatrices[i] = _viewProjectionMatrices[i];
	}
}

void DirectionalLight::setSplits(float *_splits)
{
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		m_splits[i] = _splits[i];
	}
}

void DirectionalLight::setShadowMapResolution(unsigned int _resolution)
{
	m_shadowMapResolution = _resolution;
	if (m_renderShadows)
	{
		glDeleteTextures(1, &m_shadowMap);
		createShadowMap();
	}
}

void DirectionalLight::updateViewValues(const glm::mat4 &_viewMatrix)
{
	m_viewDirection = glm::vec3(_viewMatrix * glm::vec4(m_direction, 0.0));
}

bool DirectionalLight::isRenderShadows() const
{
	return m_renderShadows;
}

Mobility DirectionalLight::getMobility() const
{
	return m_mobility;
}

glm::vec3 DirectionalLight::getColor() const
{
	return m_color;
}

glm::vec3 DirectionalLight::getDirection() const
{
	return m_direction;
}

glm::vec3 DirectionalLight::getViewDirection() const
{
	return m_viewDirection;
}

const glm::mat4 *DirectionalLight::getViewProjectionMatrices() const
{
	return m_viewProjectionMatrices;
}

const float *DirectionalLight::getSplits() const
{
	return m_splits;
}

unsigned int DirectionalLight::getShadowMap() const
{
	return m_shadowMap;
}

unsigned int DirectionalLight::getShadowMapResolution() const
{
	return m_shadowMapResolution;
}


const unsigned int PointLight::DEFAULT_SHADOW_MAP_RESOLUTION = 1024;

PointLight::PointLight(Mobility _mobility, 
	float _luminousPower,
	const glm::vec3 &_color, 
	const glm::vec3 &_position, 
	float _radius, 
	bool _renderShadows, 
	unsigned int _shadowMapResolution)
	:m_mobility(_mobility),
	m_luminousPower(_luminousPower),
	m_color(_color), 
	m_position(_position), 
	m_radius(_radius), 
	m_renderShadows(false), 
	m_shadowMapResolution(_shadowMapResolution)
{
	assert(_radius > 0.0);

	setRenderShadows(_renderShadows);
	updateViewProjectionMatrices();
}

void PointLight::updateViewProjectionMatrices()
{
	const float aspectRatio = 1.0f;
	const float nearPlane = 0.1f;
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), aspectRatio, nearPlane, m_radius);

	m_viewProjectionMatrices[0] = projection * glm::lookAt(m_position, m_position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	m_viewProjectionMatrices[1] = projection * glm::lookAt(m_position, m_position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	m_viewProjectionMatrices[2] = projection * glm::lookAt(m_position, m_position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	m_viewProjectionMatrices[3] = projection * glm::lookAt(m_position, m_position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	m_viewProjectionMatrices[4] = projection * glm::lookAt(m_position, m_position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
	m_viewProjectionMatrices[5] = projection * glm::lookAt(m_position, m_position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
}

void PointLight::createShadowMap()
{
	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, m_shadowMapResolution, m_shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);
}

std::shared_ptr<PointLight> PointLight::createPointLight(
	Mobility _mobility,
	float _luminousPower, 
	const glm::vec3 &_color, 
	const glm::vec3 &_position, 
	float _radius, 
	bool _renderShadows, 
	unsigned int _shadowMapResolution)
{
	return std::shared_ptr<PointLight>(new PointLight(_mobility, _luminousPower, _color, _position, _radius, _renderShadows, _shadowMapResolution));
}

PointLight::~PointLight()
{
	if (m_renderShadows)
	{
		glDeleteTextures(1, &m_shadowMap);
	}
}

void PointLight::setRenderShadows(bool _renderShadows)
{
	if (m_renderShadows != _renderShadows)
	{
		m_renderShadows = _renderShadows;

		if (m_renderShadows)
		{
			createShadowMap();
		}
		else
		{
			glDeleteTextures(1, &m_shadowMap);
		}
	}
}

void PointLight::setColor(const glm::vec3 &_color)
{
	m_color = _color;
}

void PointLight::setPosition(const glm::vec3 &_position)
{
	m_position = _position;
	updateViewProjectionMatrices();
}

void PointLight::setLuminousPower(float _luminousPower)
{
	m_luminousPower = _luminousPower;
}

void PointLight::setRadius(float _radius)
{
	assert(_radius > 0.0);

	m_radius = _radius;
	updateViewProjectionMatrices();
}

void PointLight::setShadowMapResolution(unsigned int _resolution)
{
	m_shadowMapResolution = _resolution;
	if (m_renderShadows)
	{
		glDeleteTextures(1, &m_shadowMap);
		createShadowMap();
	}
}

void PointLight::updateViewValues(const glm::mat4 &_viewMatrix)
{
	glm::vec4 tmp = _viewMatrix * glm::vec4(m_position, 1.0);
	m_viewPosition = glm::vec3(tmp / tmp.w);
}

bool PointLight::isRenderShadows() const
{
	return m_renderShadows;
}

Mobility PointLight::getMobility() const
{
	return m_mobility;
}

glm::vec3 PointLight::getColor() const
{
	return m_color;
}

glm::vec3 PointLight::getPosition() const
{
	return m_position;
}

glm::vec3 PointLight::getViewPosition() const
{
	return m_viewPosition;
}

glm::vec4 PointLight::getBoundingSphere() const
{
	return glm::vec4(m_position, m_radius);
}

float PointLight::getLuminousPower() const
{
	return m_luminousPower;
}

float PointLight::getLuminousIntensity() const
{
	return m_luminousPower * (1.0f / (4.0f * glm::pi<float>()));
}

float PointLight::getRadius() const
{
	return m_radius;
}

float PointLight::getInvSqrRadius() const
{
	return 1.0f / (m_radius * m_radius);
}

const glm::mat4 *PointLight::getViewProjectionMatrices() const
{
	return m_viewProjectionMatrices;
}

unsigned int PointLight::getShadowMap() const
{
	return m_shadowMap;
}

unsigned int PointLight::getShadowMapResolution() const
{
	return m_shadowMapResolution;
}


const unsigned int SpotLight::DEFAULT_SHADOW_MAP_RESOLUTION = 1024;

SpotLight::SpotLight(Mobility _mobility, 
	float _luminousPower,
	const glm::vec3 &_color, 
	const glm::vec3 &_position, 
	const glm::vec3 &_direction, 
	float _outerAngle, 
	float _innerAngle, 
	float _radius, 
	bool _renderShadows, 
	unsigned int _shadowMapResolution, 
	bool _projector, 
	const std::shared_ptr<Texture> &_projectionTexture)
	:m_mobility(_mobility),
	m_luminousPower(_luminousPower),
	m_color(_color), 
	m_position(_position), 
	m_direction(glm::normalize(_direction)), 
	m_outerAngle(glm::radians(_outerAngle)), 
	m_innerAngle(glm::radians(_innerAngle)),
	m_angleScale(1.0f / glm::max(0.001f, glm::cos(m_innerAngle * 0.5f) - glm::cos(m_outerAngle * 0.5f))),
	m_angleOffset(-glm::cos(m_outerAngle * 0.5f) * m_angleScale), // Careful; make sure initialization order is kept
	m_radius(_radius),
	m_renderShadows(false),
	m_projector(_projector),
	m_shadowMapResolution(_shadowMapResolution),
	m_projectionTexture(_projectionTexture)
{
	assert(_radius > 0.0);
	assert(m_innerAngle <= m_outerAngle);
	assert(m_innerAngle >= 0.0f);
	assert(m_outerAngle > 0.0f);
	assert(m_outerAngle <= glm::radians(90.0f));

	setRenderShadows(_renderShadows);
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::updateViewProjectionMatrix()
{
	static const float NEAR_PLANE = 0.1f;
	static const float ASPECT_RATIO = 1.0f;

	glm::vec3 upDir(0.0f, 1.0f, 0.0f);
	// choose different up vector if light direction would be linearly dependent otherwise
	if (abs(m_direction.x) < 0.001f && abs(m_direction.z) < 0.001f)
	{
		upDir = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	m_viewProjectionMatrix  = glm::perspective(m_outerAngle, ASPECT_RATIO, NEAR_PLANE, m_radius) * glm::lookAt(m_position, m_position + m_direction, upDir);
}

void SpotLight::updateBoundingSphere()
{
	if (m_outerAngle > glm::pi<float>() * 0.25f)
	{
		m_boundingSphere = glm::vec4(m_position + glm::cos(m_outerAngle) * m_radius * m_direction, glm::sin(m_outerAngle) * m_radius);
	}
	else
	{
		m_boundingSphere = glm::vec4(m_position + m_radius / (2.0f * glm::cos(m_outerAngle)) * m_direction, m_radius / (2.0f * glm::cos(m_outerAngle)));
	}
}

void SpotLight::createShadowMap()
{
	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_shadowMapResolution, m_shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

std::shared_ptr<SpotLight> SpotLight::createSpotLight(
	Mobility _mobility,
	float _luminousPower, 
	const glm::vec3 &_color, 
	const glm::vec3 &_position, 
	const glm::vec3 &_direction, 
	float _outerAngle, 
	float _innerAngle, 
	float _radius, 
	bool _renderShadows, 
	unsigned int _shadowMapResolution, 
	bool _projector, 
	const std::shared_ptr<Texture> &_projectionTexture)
{
	return std::shared_ptr<SpotLight>(new SpotLight(_mobility, _luminousPower, _color, _position, _direction, _outerAngle, _innerAngle, _radius, _renderShadows, _shadowMapResolution, _projector, _projectionTexture));
}

SpotLight::~SpotLight()
{
	if (m_renderShadows)
	{
		glDeleteTextures(1, &m_shadowMap);
	}
}

void SpotLight::setRenderShadows(bool _renderShadows)
{
	if (m_renderShadows != _renderShadows)
	{
		m_renderShadows = _renderShadows;

		if (m_renderShadows)
		{
			createShadowMap();
		}
		else
		{
			glDeleteTextures(1, &m_shadowMap);
		}
	}
}

void SpotLight::setProjector(bool _projector)
{
	m_projector = _projector;
}

void SpotLight::setColor(const glm::vec3 &_color)
{
	m_color = _color;
}

void SpotLight::setPosition(const glm::vec3 &_position)
{
	m_position = _position;
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::setDirection(const glm::vec3 &_direction)
{
	m_direction = glm::normalize(_direction);
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::setLuminousPower(float _luminousPower)
{
	m_luminousPower = _luminousPower;
}

void SpotLight::setInnerAngle(float _angle)
{
	assert(m_innerAngle <= m_outerAngle);
	assert(m_innerAngle >= 0.0f);

	m_innerAngle = glm::radians(_angle);
	m_angleScale = 1.0f / glm::max(0.001f, glm::cos(m_innerAngle * 0.5f) - glm::cos(m_outerAngle * 0.5f));
	m_angleOffset = -glm::cos(m_outerAngle * 0.5f) * m_angleScale;
}

void SpotLight::setOuterAngle(float _angle)
{
	assert(m_innerAngle <= m_outerAngle);
	assert(m_outerAngle > 0.0f);
	assert(m_outerAngle <= glm::radians(90.0f));

	m_outerAngle = glm::radians(_angle);
	m_angleScale = 1.0f / glm::max(0.001f, glm::cos(m_innerAngle * 0.5f) - glm::cos(m_outerAngle * 0.5f));
	m_angleOffset = -glm::cos(m_outerAngle * 0.5f) * m_angleScale;
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::setRadius(float _radius)
{
	assert(_radius > 0.0);

	m_radius = _radius;
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::setShadowMapResolution(unsigned int _resolution)
{
	m_shadowMapResolution = _resolution;
	if (m_renderShadows)
	{
		glDeleteTextures(1, &m_shadowMap);
		createShadowMap();
	}
}

void SpotLight::setProjectionTexture(const std::shared_ptr<Texture> &_projectionTexture)
{
	m_projectionTexture = _projectionTexture;
}

void SpotLight::updateViewValues(const glm::mat4 &_viewMatrix)
{
	glm::vec4 tmp = _viewMatrix * glm::vec4(m_position, 1.0);
	m_viewPosition = glm::vec3(tmp / tmp.w);
	m_viewDirection = glm::vec3(_viewMatrix * glm::vec4(m_direction, 0.0));
}

bool SpotLight::isRenderShadows() const
{
	return m_renderShadows;
}

bool SpotLight::isProjector() const
{
	return m_projector;
}

Mobility SpotLight::getMobility() const
{
	return m_mobility;
}

glm::vec3 SpotLight::getColor() const
{
	return m_color;
}

glm::vec3 SpotLight::getPosition() const
{
	return m_position;
}

glm::vec3 SpotLight::getDirection() const
{
	return m_direction;
}

glm::vec3 SpotLight::getViewPosition() const
{
	return m_viewPosition;
}

glm::vec3 SpotLight::getViewDirection() const
{
	return m_viewDirection;
}

glm::vec4 SpotLight::getBoundingSphere() const
{
	return m_boundingSphere;
}

float SpotLight::getLuminousPower() const
{
	return m_luminousPower;
}

float SpotLight::getLuminousIntensity() const
{
	return m_luminousPower * (1.0f / glm::pi<float>());
}

float SpotLight::getInnerAngle() const
{
	return m_innerAngle;
}

float SpotLight::getOuterAngle() const
{
	return m_outerAngle;
}

float SpotLight::getAngleScale() const
{
	return m_angleScale;
}

float SpotLight::getAngleOffset() const
{
	return m_angleOffset;
}

float SpotLight::getRadius() const
{
	return m_radius;
}

float SpotLight::getInvSqrRadius() const
{
	return 1.0f / (m_radius * m_radius);
}

glm::mat4 SpotLight::getViewProjectionMatrix() const
{
	return m_viewProjectionMatrix;
}

unsigned int SpotLight::getShadowMap() const
{
	return m_shadowMap;
}

unsigned int SpotLight::getShadowMapResolution() const
{
	return m_shadowMapResolution;
}

std::shared_ptr<Texture> SpotLight::getProjectionTexture() const
{
	return m_projectionTexture;
}
