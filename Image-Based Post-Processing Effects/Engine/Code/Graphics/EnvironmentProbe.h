#pragma once
#include <glm\vec3.hpp>
#include <glad\glad.h>
#include <string>
#include <memory>

class Texture;

class EnvironmentProbe
{
public:
	static const unsigned int REFLECTANCE_RESOLUTION;
	static const unsigned int IRRADIANCE_RESOLUTION;

	static std::shared_ptr<EnvironmentProbe> createEnvironmentProbe(const glm::vec3 &_position);
	static std::shared_ptr<EnvironmentProbe> createEnvironmentProbe(const glm::vec3 &_position, const std::shared_ptr<Texture> &_reflectanceMap, const std::shared_ptr<Texture> &_irradianceMap);
	
	EnvironmentProbe(const EnvironmentProbe &) = delete;
	EnvironmentProbe(const EnvironmentProbe &&) = delete;
	EnvironmentProbe &operator= (const EnvironmentProbe &) = delete;
	EnvironmentProbe &operator= (const EnvironmentProbe &&) = delete;
	std::shared_ptr<Texture> getReflectanceMap() const;
	std::shared_ptr<Texture> getIrradianceMap() const;
	glm::vec3 getPosition() const;
	bool isValid() const;
	void setValid(bool _valid);
	void saveToFile(const std::string &_reflectanceMapFilepath, const std::string &_irradianceMapFilepath);

private:
	std::shared_ptr<Texture> reflectanceMap;
	std::shared_ptr<Texture> irradianceMap;
	glm::vec3 position;
	bool valid;

	explicit EnvironmentProbe(const glm::vec3 &_position);
	explicit EnvironmentProbe(const glm::vec3 &_position, const std::shared_ptr<Texture> &_reflectanceMap, const std::shared_ptr<Texture> &_irradianceMap);
};