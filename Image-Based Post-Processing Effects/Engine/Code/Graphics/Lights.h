#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <vector>
#include <memory>

const unsigned int SHADOW_CASCADES = 4;

class DirectionalLight
{
public:
	static const unsigned int DEFAULT_SHADOW_MAP_RESOLUTION;

	static std::shared_ptr<DirectionalLight> createDirectionalLight(const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows = false, unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);

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
	glm::vec3 getColor() const;
	glm::vec3 getDirection() const;
	glm::vec3 getViewDirection() const;
	const glm::mat4 *getViewProjectionMatrices() const;
	const float *getSplits() const;
	unsigned int getShadowMap() const;
	unsigned int getShadowMapResolution() const;

private:
	glm::vec3 color;
	glm::vec3 direction;
	glm::vec3 viewDirection;
	bool renderShadows;
	unsigned int shadowMap;
	unsigned int shadowMapResolution;
	glm::mat4 viewProjectionMatrices[SHADOW_CASCADES];
	float splits[SHADOW_CASCADES];

	explicit DirectionalLight(const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows = false, unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);
	void createShadowMap();
};

class PointLight
{
public:
	static const unsigned int DEFAULT_SHADOW_MAP_RESOLUTION;

	static std::shared_ptr<PointLight> createPointLight(const glm::vec3 &_color, const glm::vec3 &_position, float _radius, bool _renderShadows = false, unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);

	PointLight(const PointLight &) = delete;
	PointLight(const PointLight &&) = delete;
	PointLight &operator= (const PointLight &) = delete;
	PointLight &operator= (const PointLight &&) = delete;
	~PointLight();
	void setRenderShadows(bool _renderShadows);
	void setColor(const glm::vec3 &_color);
	void setPosition(const glm::vec3 &_position);
	void setRadius(float _radius);
	void setShadowMapResolution(unsigned int _resolution);
	void updateViewValues(const glm::mat4 &_viewMatrix);
	bool isRenderShadows() const;
	glm::vec3 getColor() const;
	glm::vec3 getPosition() const;
	glm::vec3 getViewPosition() const;
	float getRadius() const;
	const glm::mat4 *getViewProjectionMatrices() const;
	unsigned int getShadowMap() const;
	unsigned int getShadowMapResolution() const;

private:
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 viewPosition;
	float radius;
	bool renderShadows;
	unsigned int shadowMap;
	unsigned int shadowMapResolution;
	glm::mat4 viewProjectionMatrices[6];

	explicit PointLight(const glm::vec3 &_color, const glm::vec3 &_position, float _radius, bool _renderShadows = false, unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);
	void updateViewProjectionMatrices();
	void createShadowMap();
};

class SpotLight
{
public:
	static const unsigned int DEFAULT_SHADOW_MAP_RESOLUTION;

	static std::shared_ptr<SpotLight> createSpotLight(const glm::vec3 &_color, const glm::vec3 &_position, const glm::vec3 &_direction, float _outerAngle, float _innerAngle, float _radius, bool _renderShadows = false, unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);

	SpotLight(const SpotLight &) = delete;
	SpotLight(const SpotLight &&) = delete;
	SpotLight &operator= (const SpotLight &) = delete;
	SpotLight &operator= (const SpotLight &&) = delete;
	~SpotLight();
	void setRenderShadows(bool _renderShadows);
	void setColor(const glm::vec3 &_color);
	void setPosition(const glm::vec3 &_position);
	void setDirection(const glm::vec3 &_direction);
	void setInnerAngle(float _angle);
	void setOuterAngle(float _angle);
	void setRadius(float _radius);
	void setShadowMapResolution(unsigned int _resolution);
	void updateViewValues(const glm::mat4 &_viewMatrix);
	bool isRenderShadows() const;
	glm::vec3 getColor() const;
	glm::vec3 getPosition() const;
	glm::vec3 getDirection() const;
	glm::vec3 getViewPosition() const;
	glm::vec3 getViewDirection() const;
	float getInnerAngle() const;
	float getInnerAngleCos() const;
	float getOuterAngle() const;
	float getOuterAngleCos() const;
	float getRadius() const;
	glm::mat4 getViewProjectionMatrix() const;
	unsigned int getShadowMap() const;
	unsigned int getShadowMapResolution() const;

private:
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 viewPosition;
	glm::vec3 viewDirection;
	float outerAngle;
	float outerAngleCos;
	float innerAngle;
	float innerAngleCos;
	float radius;
	bool renderShadows;
	unsigned int shadowMap;
	unsigned int shadowMapResolution;
	glm::mat4 viewProjectionMatrix;

	explicit SpotLight(const glm::vec3 &_color, const glm::vec3 &_position, const glm::vec3 &_direction, float _outerAngle, float _innerAngle, float _radius, bool _renderShadows = false, unsigned int _shadowMapResolution = DEFAULT_SHADOW_MAP_RESOLUTION);
	void updateViewProjectionMatrix();
	void createShadowMap();
};

struct Lights
{
	std::vector<std::shared_ptr<DirectionalLight>> directionalLights;
	std::vector<std::shared_ptr<PointLight>> pointLights;
	std::vector<std::shared_ptr<SpotLight>> spotLights;
};