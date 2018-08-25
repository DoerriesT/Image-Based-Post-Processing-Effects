#include "EnvironmentProbe.h"
#include <gli\gli.hpp>
#include <gli\convert.hpp>
#include "Texture.h"

const unsigned int EnvironmentProbe::REFLECTANCE_RESOLUTION = 1024;
const unsigned int EnvironmentProbe::IRRADIANCE_RESOLUTION = 64;

std::shared_ptr<EnvironmentProbe> EnvironmentProbe::createEnvironmentProbe(const glm::vec3 &_position)
{
	return std::shared_ptr<EnvironmentProbe>(new EnvironmentProbe(_position));
}

std::shared_ptr<EnvironmentProbe> EnvironmentProbe::createEnvironmentProbe(const glm::vec3 &_position, const std::shared_ptr<Texture> &_reflectanceMap, const std::shared_ptr<Texture> &_irradianceMap)
{
	return std::shared_ptr<EnvironmentProbe>(new EnvironmentProbe(_position, _reflectanceMap, _irradianceMap));
}

std::shared_ptr<Texture> EnvironmentProbe::getReflectanceMap() const
{
	return reflectanceMap;
}

std::shared_ptr<Texture> EnvironmentProbe::getIrradianceMap() const
{
	return irradianceMap;
}

glm::vec3 EnvironmentProbe::getPosition() const
{
	return position;
}

bool EnvironmentProbe::isValid() const
{
	return valid;
}

void EnvironmentProbe::setValid(bool _valid)
{
	valid = _valid;
}

void EnvironmentProbe::saveToFile(const std::string & _reflectanceMapFilepath, const std::string & _irradianceMapFilepath)
{
	std::unique_ptr<uint32_t[]> buffer = std::make_unique<uint32_t[]>(REFLECTANCE_RESOLUTION * REFLECTANCE_RESOLUTION);

	GLuint textureIds[2] = { reflectanceMap->getId(), irradianceMap->getId() };
	unsigned int resolutions[2] = { REFLECTANCE_RESOLUTION, IRRADIANCE_RESOLUTION };
	unsigned int levels[2] = { 5, 1 };
	std::string filepaths[2] = { _reflectanceMapFilepath, _irradianceMapFilepath };

	for (unsigned int i = 0; i < 2; ++i)
	{
		gli::texture2d texture = gli::texture2d(gli::format::FORMAT_RG11B10_UFLOAT_PACK32, gli::extent2d(resolutions[i], resolutions[i]), levels[i]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureIds[i]);

		for (unsigned int level = 0; level < levels[i]; ++level)
		{
			glGetTexImage(GL_TEXTURE_2D, level, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, buffer.get());

			unsigned int mipRes = unsigned int(resolutions[i] * std::pow(0.5f, level));

			for (unsigned int y = 0; y < mipRes; ++y)
			{
				for (unsigned int x = 0; x < mipRes; ++x)
				{
					texture.store<glm::uint32>(gli::extent2d(x, y), level, buffer[(x + y * mipRes)]);
				}
			}
		}

		gli::save_dds(texture, filepaths[i]);
	}
}

EnvironmentProbe::EnvironmentProbe(const glm::vec3 &_position)
	:position(_position), valid(false)
{
	GLuint reflectanceId;
	glGenTextures(1, &reflectanceId);
	glBindTexture(GL_TEXTURE_2D, reflectanceId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, REFLECTANCE_RESOLUTION, REFLECTANCE_RESOLUTION, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	reflectanceMap = Texture::createTexture(reflectanceId, GL_TEXTURE_2D);

	GLuint irradianceId;
	glGenTextures(1, &irradianceId);
	glBindTexture(GL_TEXTURE_2D, irradianceId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, IRRADIANCE_RESOLUTION, IRRADIANCE_RESOLUTION, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	irradianceMap = Texture::createTexture(irradianceId, GL_TEXTURE_2D);
}

EnvironmentProbe::EnvironmentProbe(const glm::vec3 &_position, const std::shared_ptr<Texture> &_reflectanceMap, const std::shared_ptr<Texture> &_irradianceMap)
	:position(_position), valid(true), reflectanceMap(_reflectanceMap), irradianceMap(_irradianceMap)
{
	glBindTexture(GL_TEXTURE_2D, reflectanceMap->getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, irradianceMap->getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
