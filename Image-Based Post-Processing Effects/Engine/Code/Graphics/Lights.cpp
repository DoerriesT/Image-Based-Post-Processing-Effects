#include "Lights.h"
#include <glad\glad.h>
#include <glm\trigonometric.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include ".\..\Utilities\Utility.h"

DirectionalLight::DirectionalLight(const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows)
	:color(_color), direction(glm::normalize(_direction)), renderShadows(false)
{
	setRenderShadows(_renderShadows);
}

std::shared_ptr<DirectionalLight> DirectionalLight::createDirectionalLight(const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows)
{
	return std::shared_ptr<DirectionalLight>(new DirectionalLight(_color, _direction, _renderShadows));
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
			glGenTextures(1, &shadowMap);
			glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, SHADOW_CASCADES);
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
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
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

void DirectionalLight::updateViewValues(const glm::mat4 &_viewMatrix)
{
	viewDirection = glm::vec3(_viewMatrix * glm::vec4(direction, 0.0));
}

bool DirectionalLight::isRenderShadows() const
{
	return renderShadows;
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

const float * DirectionalLight::getSplits() const
{
	return splits;
}

unsigned int DirectionalLight::getShadowMap() const
{
	return shadowMap;
}

PointLight::PointLight(const glm::vec3 &_color, const glm::vec3 &_position, bool _renderShadows)
	:color(_color), position(_position), renderShadows(false)
{
	setRenderShadows(_renderShadows);
	updateViewProjectionMatrix();
}

void PointLight::updateViewProjectionMatrix()
{
	static const float NEAR_PLANE = 1.0f;
	static const float FAR_PLANE = 25.0f;
	static const float ASPECT_RATIO = 1.0f;
	static const glm::vec3 direction(0.0f, -1.0f, 0.01f);
	static const glm::vec3 upDir(0.0f, 1.0f, 0.0f);
	viewProjectionMatrix = glm::perspective(glm::acos(glm::radians(45.0f)) * 2.0f, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE) * glm::lookAt(position, position + direction, upDir);
}

std::shared_ptr<PointLight> PointLight::createPointLight(const glm::vec3 &_color, const glm::vec3 &_position, bool _renderShadows)
{
	return std::shared_ptr<PointLight>(new PointLight(_color, _position, _renderShadows));
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
			glGenTextures(1, &shadowMap);
			glBindTexture(GL_TEXTURE_2D, shadowMap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 1024, 1024, 0, GL_RG, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
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
	updateViewProjectionMatrix();
}

void PointLight::setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix)
{
	viewProjectionMatrix = _viewProjectionMatrix;
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

glm::mat4 PointLight::getViewProjectionMatrix() const
{
	return viewProjectionMatrix;
}

unsigned int PointLight::getShadowMap() const
{
	return shadowMap;
}

SpotLight::SpotLight(const glm::vec3 &_color, const glm::vec3 &_position, const glm::vec3 &_direction, float _angle, bool _renderShadows)
	:color(_color), position(_position), direction(glm::normalize(_direction)), angle(glm::cos(glm::radians(_angle))), renderShadows(false)
{
	setRenderShadows(_renderShadows);
	updateViewProjectionMatrix();
}

void SpotLight::updateViewProjectionMatrix()
{
	static const float NEAR_PLANE = 1.0f;
	static const float FAR_PLANE = 25.0f;
	static const float ASPECT_RATIO = 1.0f;
	static const glm::vec3 upDir(0.0f, 1.0f, 0.0f);
	viewProjectionMatrix  = glm::perspective(glm::acos(angle) * 2.0f, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE) * glm::lookAt(position, position + direction, upDir);
}

std::shared_ptr<SpotLight> SpotLight::createSpotLight(const glm::vec3 &_color, const glm::vec3 &_position, const glm::vec3 &_direction, float _angle, bool _renderShadows)
{
	return std::shared_ptr<SpotLight>(new SpotLight(_color, _position, _direction, _angle, _renderShadows));
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
			glGenTextures(1, &shadowMap);
			glBindTexture(GL_TEXTURE_2D, shadowMap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 1024, 1024, 0, GL_RG, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
		else
		{
			glDeleteTextures(1, &shadowMap);
		}
	}
}

void SpotLight::setColor(const glm::vec3 &_color)
{
	color = _color;
}

void SpotLight::setPosition(const glm::vec3 &_position)
{
	position = _position;
	updateViewProjectionMatrix();
}

void SpotLight::setDirection(const glm::vec3 &_direction)
{
	direction = glm::normalize(_direction);
	updateViewProjectionMatrix();
}

void SpotLight::setAngle(float _angle)
{
	angle = glm::cos(glm::radians(_angle));
	updateViewProjectionMatrix();
}

void SpotLight::setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix)
{
	viewProjectionMatrix = _viewProjectionMatrix;
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

float SpotLight::getAngle() const
{
	return angle;
}

glm::mat4 SpotLight::getViewProjectionMatrix() const
{
	return viewProjectionMatrix;
}

unsigned int SpotLight::getShadowMap() const
{
	return shadowMap;
}
