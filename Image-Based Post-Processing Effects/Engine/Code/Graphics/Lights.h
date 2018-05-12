#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <vector>
#include <memory>

const unsigned int MAX_POINT_LIGHTS = 3;

const unsigned int MAX_SPOT_LIGHTS = 3;

const unsigned int MAX_DIRECTIONAL_LIGHTS = 1;

class DirectionalLight
{
public:
	static std::shared_ptr<DirectionalLight> createDirectionalLight(const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows = false);

	DirectionalLight(const DirectionalLight &) = delete;
	DirectionalLight(const DirectionalLight &&) = delete;
	DirectionalLight &operator= (const DirectionalLight &) = delete;
	DirectionalLight &operator= (const DirectionalLight &&) = delete;
	~DirectionalLight();
	void setRenderShadows(bool _renderShadows);
	void setColor(const glm::vec3 &_color);
	void setDirection(const glm::vec3 &_direction);
	void setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix);
	bool isRenderShadows() const;
	glm::vec3 getColor() const;
	glm::vec3 getDirection() const;
	glm::mat4 getViewProjectionMatrix() const;
	unsigned int getShadowMap() const;

private:
	glm::vec3 color;
	glm::vec3 direction;
	bool renderShadows;
	unsigned int shadowMap;
	glm::mat4 viewProjectionMatrix;

	explicit DirectionalLight(const glm::vec3 &_color, const glm::vec3 &_direction, bool _renderShadows = false);
};

class PointLight
{
public:
	static std::shared_ptr<PointLight> createPointLight(const glm::vec3 &_color, const glm::vec3 &_position, bool _renderShadows = false);

	PointLight(const PointLight &) = delete;
	PointLight(const PointLight &&) = delete;
	PointLight &operator= (const PointLight &) = delete;
	PointLight &operator= (const PointLight &&) = delete;
	~PointLight();
	void setRenderShadows(bool _renderShadows);
	void setColor(const glm::vec3 &_color);
	void setPosition(const glm::vec3 &_position);
	void setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix);
	bool isRenderShadows() const;
	glm::vec3 getColor() const;
	glm::vec3 getPosition() const;
	glm::mat4 getViewProjectionMatrix() const;
	unsigned int getShadowMap() const;

private:
	glm::vec3 color;
	glm::vec3 position;
	bool renderShadows;
	unsigned int shadowMap;
	glm::mat4 viewProjectionMatrix;

	explicit PointLight(const glm::vec3 &_color, const glm::vec3 &_position, bool _renderShadows = false);
	void updateViewProjectionMatrix();
};

class SpotLight
{
public:
	static std::shared_ptr<SpotLight> createSpotLight(const glm::vec3 &_color, const glm::vec3 &_position, const glm::vec3 &_direction, float _angle, bool _renderShadows = false);

	SpotLight(const SpotLight &) = delete;
	SpotLight(const SpotLight &&) = delete;
	SpotLight &operator= (const SpotLight &) = delete;
	SpotLight &operator= (const SpotLight &&) = delete;
	~SpotLight();
	void setRenderShadows(bool _renderShadows);
	void setColor(const glm::vec3 &_color);
	void setPosition(const glm::vec3 &_position);
	void setDirection(const glm::vec3 &_direction);
	void setAngle(float _angle);
	void setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix);
	bool isRenderShadows() const;
	glm::vec3 getColor() const;
	glm::vec3 getPosition() const;
	glm::vec3 getDirection() const;
	float getAngle() const;
	glm::mat4 getViewProjectionMatrix() const;
	unsigned int getShadowMap() const;

private:
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 direction;
	float angle;
	bool renderShadows;
	unsigned int shadowMap;
	glm::mat4 viewProjectionMatrix;

	explicit SpotLight(const glm::vec3 &_color, const glm::vec3 &_position, const glm::vec3 &_direction, float _angle, bool _renderShadows = false);
	void updateViewProjectionMatrix();
};

struct Lights
{
	std::vector<std::shared_ptr<DirectionalLight>> directionalLights;
	std::vector<std::shared_ptr<PointLight>> pointLights;
	std::vector<std::shared_ptr<SpotLight>> spotLights;
};