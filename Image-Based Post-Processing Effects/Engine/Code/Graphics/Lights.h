#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <vector>
#include <memory>
#include "EntityComponentSystem\Component.h"

const unsigned int SHADOW_CASCADES = 3;

class Texture;

class DirectionalLight
{
public:
	static const unsigned int DEFAULT_SHADOW_MAP_RESOLUTION;

	static std::shared_ptr<DirectionalLight> createDirectionalLight(Mobility _mobility, const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows = false, unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);

	DirectionalLight(const DirectionalLight &) = delete;
	DirectionalLight(const DirectionalLight &&) = delete;
	DirectionalLight &operator= (const DirectionalLight &) = delete;
	DirectionalLight &operator= (const DirectionalLight &&) = delete;
	~DirectionalLight();
	void setRenderShadows(bool _renderShadows);
	void setColor(const glm::vec3 &_color);
	void setDirection(const glm::vec3 &_direction);
	void setViewProjectionMatrices(glm::mat4 *_viewProjectionMatrix);
	void setSplits(float *_splits);
	void setShadowMapResolution(unsigned int _resolution);
	void updateViewValues(const glm::mat4 &_viewMatrix);
	bool isRenderShadows() const;
	Mobility getMobility() const;
	glm::vec3 getColor() const;
	glm::vec3 getDirection() const;
	glm::vec3 getViewDirection() const;
	const glm::mat4 *getViewProjectionMatrices() const;
	const float *getSplits() const;
	unsigned int getShadowMap() const;
	unsigned int getShadowMapResolution() const;

private:
	Mobility m_mobility;
	glm::vec3 m_color;
	glm::vec3 m_direction;
	glm::vec3 m_viewDirection;
	bool m_renderShadows;
	unsigned int m_shadowMap;
	unsigned int m_shadowMapResolution;
	glm::mat4 m_viewProjectionMatrices[SHADOW_CASCADES];
	float m_splits[SHADOW_CASCADES];

	explicit DirectionalLight(Mobility _mobility, const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows = false, unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);
	void createShadowMap();
};

class PointLight
{
public:
	static const unsigned int DEFAULT_SHADOW_MAP_RESOLUTION;

	static std::shared_ptr<PointLight> createPointLight(Mobility _mobility, 
		float _luminousPower,
		const glm::vec3 &_color, 
		const glm::vec3 &_position, 
		float _radius, 
		bool _renderShadows = false, 
		unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);

	PointLight(const PointLight &) = delete;
	PointLight(const PointLight &&) = delete;
	PointLight &operator= (const PointLight &) = delete;
	PointLight &operator= (const PointLight &&) = delete;
	~PointLight();
	void setRenderShadows(bool _renderShadows);
	void setColor(const glm::vec3 &_color);
	void setPosition(const glm::vec3 &_position);
	void setLuminousPower(float _luminousPower);
	void setRadius(float _radius);
	void setShadowMapResolution(unsigned int _resolution);
	void updateViewValues(const glm::mat4 &_viewMatrix);
	bool isRenderShadows() const;
	Mobility getMobility() const;
	glm::vec3 getColor() const;
	glm::vec3 getPosition() const;
	glm::vec3 getViewPosition() const;
	glm::vec4 getBoundingSphere() const;
	float getLuminousPower() const;
	float getLuminousIntensity() const;
	float getRadius() const;
	float getInvSqrRadius() const;
	const glm::mat4 *getViewProjectionMatrices() const;
	unsigned int getShadowMap() const;
	unsigned int getShadowMapResolution() const;

private:
	Mobility m_mobility;
	glm::vec3 m_color;
	glm::vec3 m_position;
	glm::vec3 m_viewPosition;
	float m_luminousPower;
	float m_radius;
	bool m_renderShadows;
	unsigned int m_shadowMap;
	unsigned int m_shadowMapResolution;
	glm::mat4 m_viewProjectionMatrices[6];

	explicit PointLight(Mobility _mobility, 
		float _luminousPower,
		const glm::vec3 &_color, 
		const glm::vec3 &_position, 
		float _radius, 
		bool _renderShadows = false, 
		unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);
	void updateViewProjectionMatrices();
	void createShadowMap();
};

class SpotLight
{
public:
	static const unsigned int DEFAULT_SHADOW_MAP_RESOLUTION;

	static std::shared_ptr<SpotLight> createSpotLight(Mobility _mobility, 
		float _luminousPower,
		const glm::vec3 &_color, 
		const glm::vec3 &_position, 
		const glm::vec3 &_direction, 
		float _outerAngle, 
		float _innerAngle, 
		float _radius, 
		bool _renderShadows = false, 
		unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION, 
		bool _projector = false, 
		const std::shared_ptr<Texture> &_projectionTexture = nullptr);

	SpotLight(const SpotLight &) = delete;
	SpotLight(const SpotLight &&) = delete;
	SpotLight &operator= (const SpotLight &) = delete;
	SpotLight &operator= (const SpotLight &&) = delete;
	~SpotLight();
	void setRenderShadows(bool _renderShadows);
	void setProjector(bool _projector);
	void setColor(const glm::vec3 &_color);
	void setPosition(const glm::vec3 &_position);
	void setDirection(const glm::vec3 &_direction);
	void setLuminousPower(float _luminousPower);
	void setInnerAngle(float _angle);
	void setOuterAngle(float _angle);
	void setRadius(float _radius);
	void setShadowMapResolution(unsigned int _resolution);
	void setProjectionTexture(const std::shared_ptr<Texture> &_projectionTexture);
	void updateViewValues(const glm::mat4 &_viewMatrix);
	bool isRenderShadows() const;
	bool isProjector() const;
	Mobility getMobility() const;
	glm::vec3 getColor() const;
	glm::vec3 getPosition() const;
	glm::vec3 getDirection() const;
	glm::vec3 getViewPosition() const;
	glm::vec3 getViewDirection() const;
	glm::vec4 getBoundingSphere() const;
	float getLuminousPower() const;
	float getLuminousIntensity() const;
	float getInnerAngle() const;
	float getOuterAngle() const;
	float getAngleScale() const;
	float getAngleOffset() const;
	float getRadius() const;
	float getInvSqrRadius() const;
	glm::mat4 getViewProjectionMatrix() const;
	unsigned int getShadowMap() const;
	unsigned int getShadowMapResolution() const;
	std::shared_ptr<Texture> getProjectionTexture() const;

private:
	Mobility m_mobility;
	glm::vec3 m_color;
	glm::vec3 m_position;
	glm::vec3 m_direction;
	glm::vec3 m_viewPosition;
	glm::vec3 m_viewDirection;
	glm::vec4 m_boundingSphere;
	float m_luminousPower;
	float m_outerAngle;
	float m_innerAngle;
	float m_angleScale;
	float m_angleOffset;
	float m_radius;
	bool m_renderShadows;
	bool m_projector;
	unsigned int m_shadowMap;
	unsigned int m_shadowMapResolution;
	glm::mat4 m_viewProjectionMatrix;
	std::shared_ptr<Texture> m_projectionTexture;

	explicit SpotLight(Mobility _mobility, 
		float _luminousPower,
		const glm::vec3 &_color, 
		const glm::vec3 &_position, 
		const glm::vec3 &_direction, 
		float _outerAngle, 
		float _innerAngle, 
		float _radius, 
		bool _renderShadows = false, 
		unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION, 
		bool _projector = false, 
		const std::shared_ptr<Texture> &_projectionTexture = nullptr);
	void updateViewProjectionMatrix();
	void updateBoundingSphere();
	void createShadowMap();
};

struct Lights
{
	std::vector<std::shared_ptr<DirectionalLight>> m_directionalLights;
	std::vector<std::shared_ptr<PointLight>> m_pointLights;
	std::vector<std::shared_ptr<SpotLight>> m_spotLights;
};