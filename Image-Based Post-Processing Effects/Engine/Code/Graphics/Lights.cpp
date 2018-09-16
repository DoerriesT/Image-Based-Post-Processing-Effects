#include "Lights.h"
#include <glad\glad.h>
#include <glm\trigonometric.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include ".\..\Utilities\Utility.h"

const unsigned int DirectionalLight::DEFAULT_SHADOW_MAP_RESOLUTION = 2048;

DirectionalLight::DirectionalLight(Mobility _mobility, const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows, unsigned int _shadowMapResolution)
	:mobility(_mobility), color(_color), direction(glm::normalize(_direction)), renderShadows(false), shadowMapResolution(_shadowMapResolution)
{
	setRenderShadows(_renderShadows);
}

void DirectionalLight::createShadowMap()
{
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, shadowMapResolution, shadowMapResolution, SHADOW_CASCADES);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
}

std::shared_ptr<DirectionalLight> DirectionalLight::createDirectionalLight(Mobility _mobility, const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows, unsigned int _shadowMapResolution)
{
	return std::shared_ptr<DirectionalLight>(new DirectionalLight(_mobility, _color, _direction, _renderShadows, _shadowMapResolution));
}

DirectionalLight::~DirectionalLight()
{
	if (renderShadows)
	{
		glDeleteTextures(1, &shadowMap);
	}
}

void DirectionalLight::setRenderShadows(bool _renderShadows)
{
	if (renderShadows != _renderShadows)
	{
		renderShadows = _renderShadows;

		if (renderShadows)
		{
			createShadowMap();
		}
		else
		{
			glDeleteTextures(1, &shadowMap);
		}
	}
}

void DirectionalLight::setColor(const glm::vec3 &_color)
{
	color = _color;
}

void DirectionalLight::setDirection(const glm::vec3 &_direction)
{
	direction = glm::normalize(_direction);
}

void DirectionalLight::setViewProjectionMatrices(glm::mat4 *_viewProjectionMatrices)
{
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		viewProjectionMatrices[i] = _viewProjectionMatrices[i];
	}
}

void DirectionalLight::setSplits(float *_splits)
{
	for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
	{
		splits[i] = _splits[i];
	}
}

void DirectionalLight::setShadowMapResolution(unsigned int _resolution)
{
	shadowMapResolution = _resolution;
	if (renderShadows)
	{
		glDeleteTextures(1, &shadowMap);
		createShadowMap();
	}
}

void DirectionalLight::updateViewValues(const glm::mat4 &_viewMatrix)
{
	viewDirection = glm::vec3(_viewMatrix * glm::vec4(direction, 0.0));
}

bool DirectionalLight::isRenderShadows() const
{
	return renderShadows;
}

Mobility DirectionalLight::getMobility() const
{
	return mobility;
}

glm::vec3 DirectionalLight::getColor() const
{
	return color;
}

glm::vec3 DirectionalLight::getDirection() const
{
	return direction;
}

glm::vec3 DirectionalLight::getViewDirection() const
{
	return viewDirection;
}

const glm::mat4 *DirectionalLight::getViewProjectionMatrices() const
{
	return viewProjectionMatrices;
}

const float *DirectionalLight::getSplits() const
{
	return splits;
}

unsigned int DirectionalLight::getShadowMap() const
{
	return shadowMap;
}

unsigned int DirectionalLight::getShadowMapResolution() const
{
	return shadowMapResolution;
}


const unsigned int PointLight::DEFAULT_SHADOW_MAP_RESOLUTION = 1024;

PointLight::PointLight(Mobility _mobility, 
	float _luminousPower,
	const glm::vec3 &_color, 
	const glm::vec3 &_position, 
	float _radius, 
	bool _renderShadows, 
	unsigned int _shadowMapResolution)
	:mobility(_mobility),
	luminousPower(_luminousPower),
	color(_color), 
	position(_position), 
	radius(_radius), 
	renderShadows(false), 
	shadowMapResolution(_shadowMapResolution)
{
	assert(_radius > 0.0);

	setRenderShadows(_renderShadows);
	updateViewProjectionMatrices();
}

void PointLight::updateViewProjectionMatrices()
{
	const float aspectRatio = 1.0f;
	const float nearPlane = 0.1f;
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), aspectRatio, nearPlane, radius);

	viewProjectionMatrices[0] = projection * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	viewProjectionMatrices[1] = projection * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	viewProjectionMatrices[2] = projection * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	viewProjectionMatrices[3] = projection * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	viewProjectionMatrices[4] = projection * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
	viewProjectionMatrices[5] = projection * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
}

void PointLight::createShadowMap()
{
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
	if (renderShadows)
	{
		glDeleteTextures(1, &shadowMap);
	}
}

void PointLight::setRenderShadows(bool _renderShadows)
{
	if (renderShadows != _renderShadows)
	{
		renderShadows = _renderShadows;

		if (renderShadows)
		{
			createShadowMap();
		}
		else
		{
			glDeleteTextures(1, &shadowMap);
		}
	}
}

void PointLight::setColor(const glm::vec3 &_color)
{
	color = _color;
}

void PointLight::setPosition(const glm::vec3 &_position)
{
	position = _position;
	updateViewProjectionMatrices();
}

void PointLight::setLuminousPower(float _luminousPower)
{
	luminousPower = _luminousPower;
}

void PointLight::setRadius(float _radius)
{
	assert(_radius > 0.0);

	radius = _radius;
	updateViewProjectionMatrices();
}

void PointLight::setShadowMapResolution(unsigned int _resolution)
{
	shadowMapResolution = _resolution;
	if (renderShadows)
	{
		glDeleteTextures(1, &shadowMap);
		createShadowMap();
	}
}

void PointLight::updateViewValues(const glm::mat4 &_viewMatrix)
{
	glm::vec4 tmp = _viewMatrix * glm::vec4(position, 1.0);
	viewPosition = glm::vec3(tmp / tmp.w);
}

bool PointLight::isRenderShadows() const
{
	return renderShadows;
}

Mobility PointLight::getMobility() const
{
	return mobility;
}

glm::vec3 PointLight::getColor() const
{
	return color;
}

glm::vec3 PointLight::getPosition() const
{
	return position;
}

glm::vec3 PointLight::getViewPosition() const
{
	return viewPosition;
}

glm::vec4 PointLight::getBoundingSphere() const
{
	return glm::vec4(position, radius);
}

float PointLight::getLuminousPower() const
{
	return luminousPower;
}

float PointLight::getLuminousIntensity() const
{
	return luminousPower * (1.0f / (4.0f * glm::pi<float>()));
}

float PointLight::getRadius() const
{
	return radius;
}

float PointLight::getInvSqrRadius() const
{
	return 1.0f / (radius * radius);
}

const glm::mat4 *PointLight::getViewProjectionMatrices() const
{
	return viewProjectionMatrices;
}

unsigned int PointLight::getShadowMap() const
{
	return shadowMap;
}

unsigned int PointLight::getShadowMapResolution() const
{
	return shadowMapResolution;
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
	:mobility(_mobility),
	luminousPower(_luminousPower),
	color(_color), 
	position(_position), 
	direction(glm::normalize(_direction)), 
	outerAngle(glm::radians(_outerAngle)), 
	innerAngle(glm::radians(_innerAngle)),
	angleScale(1.0f / glm::max(0.001f, glm::cos(innerAngle * 0.5f) - glm::cos(outerAngle * 0.5f))),
	angleOffset(-glm::cos(outerAngle * 0.5f) * angleScale), // Careful; make sure initialization order is kept
	radius(_radius),
	renderShadows(false),
	projector(_projector),
	shadowMapResolution(_shadowMapResolution),
	projectionTexture(_projectionTexture)
{
	assert(_radius > 0.0);
	assert(innerAngle <= outerAngle);
	assert(innerAngle >= 0.0f);
	assert(outerAngle > 0.0f);
	assert(outerAngle <= glm::radians(90.0f));

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
	if (abs(direction.x) < 0.001f && abs(direction.z) < 0.001f)
	{
		upDir = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	viewProjectionMatrix  = glm::perspective(outerAngle, ASPECT_RATIO, NEAR_PLANE, radius) * glm::lookAt(position, position + direction, upDir);
}

void SpotLight::updateBoundingSphere()
{
	if (outerAngle > glm::pi<float>() * 0.25f)
	{
		boundingSphere = glm::vec4(position + glm::cos(outerAngle) * radius * direction, glm::sin(outerAngle) * radius);
	}
	else
	{
		boundingSphere = glm::vec4(position + radius / (2.0f * glm::cos(outerAngle)) * direction, radius / (2.0f * glm::cos(outerAngle)));
	}
}

void SpotLight::createShadowMap()
{
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
	if (renderShadows)
	{
		glDeleteTextures(1, &shadowMap);
	}
}

void SpotLight::setRenderShadows(bool _renderShadows)
{
	if (renderShadows != _renderShadows)
	{
		renderShadows = _renderShadows;

		if (renderShadows)
		{
			createShadowMap();
		}
		else
		{
			glDeleteTextures(1, &shadowMap);
		}
	}
}

void SpotLight::setProjector(bool _projector)
{
	projector = _projector;
}

void SpotLight::setColor(const glm::vec3 &_color)
{
	color = _color;
}

void SpotLight::setPosition(const glm::vec3 &_position)
{
	position = _position;
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::setDirection(const glm::vec3 &_direction)
{
	direction = glm::normalize(_direction);
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::setLuminousPower(float _luminousPower)
{
	luminousPower = _luminousPower;
}

void SpotLight::setInnerAngle(float _angle)
{
	assert(innerAngle <= outerAngle);
	assert(innerAngle >= 0.0f);

	innerAngle = glm::radians(_angle);
	angleScale = 1.0f / glm::max(0.001f, glm::cos(innerAngle * 0.5f) - glm::cos(outerAngle * 0.5f));
	angleOffset = -glm::cos(outerAngle * 0.5f) * angleScale;
}

void SpotLight::setOuterAngle(float _angle)
{
	assert(innerAngle <= outerAngle);
	assert(outerAngle > 0.0f);
	assert(outerAngle <= glm::radians(90.0f));

	outerAngle = glm::radians(_angle);
	angleScale = 1.0f / glm::max(0.001f, glm::cos(innerAngle * 0.5f) - glm::cos(outerAngle * 0.5f));
	angleOffset = -glm::cos(outerAngle * 0.5f) * angleScale;
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::setRadius(float _radius)
{
	assert(_radius > 0.0);

	radius = _radius;
	updateViewProjectionMatrix();
	updateBoundingSphere();
}

void SpotLight::setShadowMapResolution(unsigned int _resolution)
{
	shadowMapResolution = _resolution;
	if (renderShadows)
	{
		glDeleteTextures(1, &shadowMap);
		createShadowMap();
	}
}

void SpotLight::setProjectionTexture(const std::shared_ptr<Texture> &_projectionTexture)
{
	projectionTexture = _projectionTexture;
}

void SpotLight::updateViewValues(const glm::mat4 &_viewMatrix)
{
	glm::vec4 tmp = _viewMatrix * glm::vec4(position, 1.0);
	viewPosition = glm::vec3(tmp / tmp.w);
	viewDirection = glm::vec3(_viewMatrix * glm::vec4(direction, 0.0));
}

bool SpotLight::isRenderShadows() const
{
	return renderShadows;
}

bool SpotLight::isProjector() const
{
	return projector;
}

Mobility SpotLight::getMobility() const
{
	return mobility;
}

glm::vec3 SpotLight::getColor() const
{
	return color;
}

glm::vec3 SpotLight::getPosition() const
{
	return position;
}

glm::vec3 SpotLight::getDirection() const
{
	return direction;
}

glm::vec3 SpotLight::getViewPosition() const
{
	return viewPosition;
}

glm::vec3 SpotLight::getViewDirection() const
{
	return viewDirection;
}

glm::vec4 SpotLight::getBoundingSphere() const
{
	return boundingSphere;
}

float SpotLight::getLuminousPower() const
{
	return luminousPower;
}

float SpotLight::getLuminousIntensity() const
{
	return luminousPower * (1.0f / glm::pi<float>());
}

float SpotLight::getInnerAngle() const
{
	return innerAngle;
}

float SpotLight::getOuterAngle() const
{
	return outerAngle;
}

float SpotLight::getAngleScale() const
{
	return angleScale;
}

float SpotLight::getAngleOffset() const
{
	return angleOffset;
}

float SpotLight::getRadius() const
{
	return radius;
}

float SpotLight::getInvSqrRadius() const
{
	return 1.0f / (radius * radius);
}

glm::mat4 SpotLight::getViewProjectionMatrix() const
{
	return viewProjectionMatrix;
}

unsigned int SpotLight::getShadowMap() const
{
	return shadowMap;
}

unsigned int SpotLight::getShadowMapResolution() const
{
	return shadowMapResolution;
}

std::shared_ptr<Texture> SpotLight::getProjectionTexture() const
{
	return projectionTexture;
}
