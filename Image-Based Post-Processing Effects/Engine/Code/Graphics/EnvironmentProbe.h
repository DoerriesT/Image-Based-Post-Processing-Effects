#pragma once
#include <glm\vec3.hpp>
#include <glad\glad.h>
#include <string>
#include <memory>
#include <vector>

class Texture;

struct SphericalHarmonics
{
	glm::vec3 c[9];
};

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

	SphericalHarmonics sh;

private:
	std::shared_ptr<Texture> reflectanceMap;
	std::shared_ptr<Texture> irradianceMap;
	glm::vec3 position;
	bool valid;

	explicit EnvironmentProbe(const glm::vec3 &_position);
	explicit EnvironmentProbe(const glm::vec3 &_position, const std::shared_ptr<Texture> &_reflectanceMap, const std::shared_ptr<Texture> &_irradianceMap);
};

class IrradianceVolume
{
public:
	struct GLSLDesc
	{
		glm::vec3 origin;
		glm::ivec3 dimensions;
		float spacing;
	};

	struct GLSLData
	{
		glm::vec3 c[9];
	};

	static std::shared_ptr<IrradianceVolume> createIrradianceVolume(const glm::vec3 &_origin, const glm::ivec3 &_dimensions, float _spacing);
	IrradianceVolume(const IrradianceVolume &) = delete;
	IrradianceVolume(const IrradianceVolume &&) = delete;
	IrradianceVolume &operator= (const IrradianceVolume &) = delete;
	IrradianceVolume &operator= (const IrradianceVolume &&) = delete;
	glm::vec3 getOrigin() const;
	glm::ivec3 getDimensions() const;
	float getSpacing() const;
	std::shared_ptr<Texture> getProbeTexture() const;
	GLSLData getProbeData(glm::ivec3 _index);
	void updateProbeData(const glm::ivec3 &_index, const GLSLData &_probeData);
	void flushToGpu();

private:
	glm::vec3 origin;
	glm::ivec3 dimensions;
	float spacing;
	std::shared_ptr<Texture> probeTexture;
	std::vector<GLSLData> data;

	explicit IrradianceVolume(const glm::vec3 &_origin, const glm::ivec3 &_dimensions, float _spacing);
};