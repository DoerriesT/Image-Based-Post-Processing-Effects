#pragma once
#include <glm\vec3.hpp>
#include <glad\glad.h>
#include <string>
#include <memory>
#include <vector>
#include "AxisAlignedBoundingBox.h"

class Texture;

class EnvironmentProbe
{
public:
	static const unsigned int REFLECTION_TEXTURE_RESOLUTION;
	static const unsigned int IRRADIANCE_TEXTURE_RESOLUTION;

	static std::shared_ptr<EnvironmentProbe> createEnvironmentProbe(const glm::vec3 &_position, const AxisAlignedBoundingBox &_aabb, const std::string &_filePath, bool _loadFromFile = true);
	
	EnvironmentProbe(const EnvironmentProbe &) = delete;
	EnvironmentProbe(const EnvironmentProbe &&) = delete;
	EnvironmentProbe &operator= (const EnvironmentProbe &) = delete;
	EnvironmentProbe &operator= (const EnvironmentProbe &&) = delete;
	std::shared_ptr<Texture> getReflectionTexture() const;
	glm::vec3 getPosition() const;
	AxisAlignedBoundingBox getAxisAlignedBoundingBox() const;
	std::string getFilePath() const;
	bool isValid() const;
	void setValid(bool _valid);
	void saveToFile();

private:
	std::shared_ptr<Texture> reflectionTexture;
	glm::vec3 position;
	AxisAlignedBoundingBox aabb;
	std::string filePath;
	bool valid;

	explicit EnvironmentProbe(const glm::vec3 &_position, const AxisAlignedBoundingBox &_aabb, const std::string &_filePath, bool _loadFromFile = true);
};

class IrradianceVolume
{
public:
	struct ProbeData
	{
		glm::vec3 c[9];
	};

	static std::shared_ptr<IrradianceVolume> createIrradianceVolume(const glm::vec3 &_origin, const glm::ivec3 &_dimensions, float _spacing);
	static std::shared_ptr<IrradianceVolume> createIrradianceVolume(const glm::vec3 &_origin, const glm::ivec3 &_dimensions, float _spacing, const std::shared_ptr<Texture> &_probeTexture);
	IrradianceVolume(const IrradianceVolume &) = delete;
	IrradianceVolume(const IrradianceVolume &&) = delete;
	IrradianceVolume &operator= (const IrradianceVolume &) = delete;
	IrradianceVolume &operator= (const IrradianceVolume &&) = delete;
	glm::vec3 getOrigin() const;
	glm::ivec3 getDimensions() const;
	float getSpacing() const;
	std::shared_ptr<Texture> getProbeTexture() const;
	ProbeData getProbeData(const glm::ivec3 &_index);
	void updateProbeData(const glm::ivec3 &_index, const ProbeData &_probeData);
	void flushToGpu();
	void saveToFile(const std::string &_filepath);

private:
	glm::vec3 origin;
	glm::ivec3 dimensions;
	float spacing;
	std::shared_ptr<Texture> probeTexture;
	std::vector<ProbeData> data;

	explicit IrradianceVolume(const glm::vec3 &_origin, const glm::ivec3 &_dimensions, float _spacing);
	explicit IrradianceVolume(const glm::vec3 &_origin, const glm::ivec3 &_dimensions, float _spacing, const std::shared_ptr<Texture> &_probeTexture);
};