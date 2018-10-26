#include "EnvironmentProbe.h"
#include <gli\gli.hpp>
#include <gli\convert.hpp>
#include "Texture.h"
#include "OpenGL\GLUtility.h"

const size_t EnvironmentProbe::REFLECTION_TEXTURE_RESOLUTION = 1024;
const size_t EnvironmentProbe::IRRADIANCE_TEXTURE_RESOLUTION = 64;

std::shared_ptr<EnvironmentProbe> EnvironmentProbe::createEnvironmentProbe(const glm::vec3 &_position, const AxisAlignedBoundingBox &_aabb, const std::string &_filePath, bool _loadFromFile)
{
	return std::shared_ptr<EnvironmentProbe>(new EnvironmentProbe(_position, _aabb, _filePath, _loadFromFile));
}

std::shared_ptr<Texture> EnvironmentProbe::getReflectionTexture() const
{
	return m_reflectionTexture;
}

glm::vec3 EnvironmentProbe::getPosition() const
{
	return m_position;
}

AxisAlignedBoundingBox EnvironmentProbe::getAxisAlignedBoundingBox() const
{
	return m_aabb;
}

std::string EnvironmentProbe::getFilePath() const
{
	return m_filePath;
}

bool EnvironmentProbe::isValid() const
{
	return m_valid;
}

void EnvironmentProbe::setValid(bool _valid)
{
	m_valid = _valid;
}

void EnvironmentProbe::saveToFile()
{
	std::unique_ptr<uint32_t[]> buffer = std::make_unique<uint32_t[]>(REFLECTION_TEXTURE_RESOLUTION * REFLECTION_TEXTURE_RESOLUTION);

	gli::texture2d texture = gli::texture2d(gli::format::FORMAT_RG11B10_UFLOAT_PACK32, gli::extent2d(REFLECTION_TEXTURE_RESOLUTION, REFLECTION_TEXTURE_RESOLUTION), 5);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_reflectionTexture->getId());

	for (size_t level = 0; level < 5; ++level)
	{
		glGetTexImage(GL_TEXTURE_2D, static_cast<GLint>(level), GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, buffer.get());

		size_t mipRes = static_cast<size_t>(static_cast<double>(REFLECTION_TEXTURE_RESOLUTION) * std::pow(0.5f, level));

		for (size_t y = 0; y < mipRes; ++y)
		{
			for (size_t x = 0; x < mipRes; ++x)
			{
				texture.store<glm::uint32>(gli::extent2d(x, y), level, buffer[(x + y * mipRes)]);
			}
		}
	}

	gli::save_dds(texture, m_filePath);

}

EnvironmentProbe::EnvironmentProbe(const glm::vec3 &_position, const AxisAlignedBoundingBox &_aabb, const std::string &_filePath, bool _loadFromFile)
	:m_position(_position),
	m_aabb(_aabb),
	m_filePath(_filePath),
	m_valid(true)
{
	if (_loadFromFile)
	{
		m_reflectionTexture = Texture::createTexture(_filePath, true);

		glBindTexture(GL_TEXTURE_2D, m_reflectionTexture->getId());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//glBindTexture(GL_TEXTURE_2D, irradianceTexture->getId());
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		GLuint reflectionId;
		glGenTextures(1, &reflectionId);
		glBindTexture(GL_TEXTURE_2D, reflectionId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, static_cast<GLsizei>(REFLECTION_TEXTURE_RESOLUTION), static_cast<GLsizei>(REFLECTION_TEXTURE_RESOLUTION), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_2D);
		m_reflectionTexture = Texture::createTexture(reflectionId, GL_TEXTURE_2D);

		//GLuint irradianceId;
		//glGenTextures(1, &irradianceId);
		//glBindTexture(GL_TEXTURE_2D, irradianceId);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, IRRADIANCE_TEXTURE_RESOLUTION, IRRADIANCE_TEXTURE_RESOLUTION, 0, GL_RGB, GL_FLOAT, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//irradianceTexture = Texture::createTexture(irradianceId, GL_TEXTURE_2D);
	}

	
}

std::shared_ptr<IrradianceVolume> IrradianceVolume::createIrradianceVolume(const glm::vec3 &_origin, const glm::ivec3 &_dimensions, float _spacing)
{
	return std::shared_ptr<IrradianceVolume>(new IrradianceVolume(_origin, _dimensions, _spacing));
}

std::shared_ptr<IrradianceVolume> IrradianceVolume::createIrradianceVolume(const glm::vec3 & _origin, const glm::ivec3 & _dimensions, float _spacing, const std::shared_ptr<Texture> &_probeTexture)
{
	return std::shared_ptr<IrradianceVolume>(new IrradianceVolume(_origin, _dimensions, _spacing, _probeTexture));
}

IrradianceVolume::IrradianceVolume(const glm::vec3 &_origin, const glm::ivec3 &_dimensions, float _spacing)
	:m_origin(_origin),
	m_dimensions(_dimensions),
	m_spacing(_spacing),
	m_data(m_dimensions.x * m_dimensions.y * m_dimensions.z)
{
	GLuint texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 9, m_dimensions.x * m_dimensions.y * m_dimensions.z, 0, GL_RGB, GL_FLOAT, m_data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_probeTexture = Texture::createTexture(texId, GL_TEXTURE_2D);
}

IrradianceVolume::IrradianceVolume(const glm::vec3 & _origin, const glm::ivec3 & _dimensions, float _spacing, const std::shared_ptr<Texture>& _probeTexture)
	:m_origin(_origin),
	m_dimensions(_dimensions),
	m_spacing(_spacing),
	m_data(m_dimensions.x * m_dimensions.y * m_dimensions.z),
	m_probeTexture(_probeTexture)
{
	glBindTexture(GL_TEXTURE_2D, m_probeTexture->getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

glm::vec3 IrradianceVolume::getOrigin() const
{
	return m_origin;
}

glm::ivec3 IrradianceVolume::getDimensions() const
{
	return m_dimensions;
}

float IrradianceVolume::getSpacing() const
{
	return m_spacing;
}

std::shared_ptr<Texture> IrradianceVolume::getProbeTexture() const
{
	return m_probeTexture;
}

IrradianceVolume::ProbeData IrradianceVolume::getProbeData(const glm::ivec3 &_index)
{
	size_t probeOffset = static_cast<size_t>(_index.z * (m_dimensions.x * m_dimensions.y) + _index.y * m_dimensions.x + _index.x);
	return m_data[probeOffset];
}

void IrradianceVolume::updateProbeData(const glm::ivec3 &_index, const ProbeData &_probeData)
{
	size_t probeOffset = static_cast<size_t>(_index.z * (m_dimensions.x * m_dimensions.y) + _index.y * m_dimensions.x + _index.x);
	m_data[probeOffset] = _probeData;
}

void IrradianceVolume::flushToGpu()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_probeTexture->getId());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 9, m_dimensions.x * m_dimensions.y * m_dimensions.z, 0, GL_RGB, GL_FLOAT, m_data.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void IrradianceVolume::saveToFile(const std::string &_filepath)
{
	const size_t width = 9;
	const size_t height = static_cast<size_t>(m_dimensions.x * m_dimensions.y * m_dimensions.z);
	std::unique_ptr<glm::vec3[]> buffer = std::make_unique<glm::vec3[]>(width * height);

	gli::texture2d texture = gli::texture2d(gli::format::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(width, height), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_probeTexture->getId());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, buffer.get());

	for (size_t y = 0; y < height; ++y)
	{
		for (size_t x = 0; x < width; ++x)
		{
			texture.store<glm::vec3>(gli::extent2d(x, y), 0, buffer[(x + y * width)]);
		}
	}
	gli::save_dds(texture, _filepath);
}
