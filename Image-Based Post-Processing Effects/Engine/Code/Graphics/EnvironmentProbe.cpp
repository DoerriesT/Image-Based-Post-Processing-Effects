#include "EnvironmentProbe.h"
#include <gli\gli.hpp>
#include <gli\convert.hpp>
#include "Texture.h"

const unsigned int EnvironmentProbe::REFLECTANCE_RESOLUTION = 512;
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

void EnvironmentProbe::setValid(const bool &_valid)
{
	valid = _valid;
}

void EnvironmentProbe::saveToFile(const std::string & _reflectanceMapFilepath, const std::string & _irradianceMapFilepath)
{
	auto size = REFLECTANCE_RESOLUTION * REFLECTANCE_RESOLUTION * 3;
	auto bufferPositiveX = new float[size];
	auto bufferNegativeX = new float[size];
	auto bufferPositiveY = new float[size];
	auto bufferNegativeY = new float[size];
	auto bufferPositiveZ = new float[size];
	auto bufferNegativeZ = new float[size];

	GLuint textureIds[2] = { reflectanceMap->getId(), irradianceMap->getId() };
	unsigned int resolutions[2] = { REFLECTANCE_RESOLUTION, IRRADIANCE_RESOLUTION };
	unsigned int levels[2] = { 5, 1 };
	std::string filepaths[2] = { _reflectanceMapFilepath, _irradianceMapFilepath };
	float *buffers[6] = { bufferPositiveX, bufferNegativeX, bufferPositiveY, bufferNegativeY, bufferPositiveZ, bufferNegativeZ };

	for (unsigned int i = 0; i < 2; ++i)
	{
		gli::texture_cube texture = gli::texture_cube(gli::format::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(resolutions[i], resolutions[i]), levels[i]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureIds[i]);

		for (unsigned int level = 0; level < levels[i]; ++level)
		{
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, GL_RGB, GL_FLOAT, bufferPositiveX);
			glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, GL_RGB, GL_FLOAT, bufferNegativeX);
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, GL_RGB, GL_FLOAT, bufferPositiveY);
			glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, GL_RGB, GL_FLOAT, bufferNegativeY);
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, GL_RGB, GL_FLOAT, bufferPositiveZ);
			glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, GL_RGB, GL_FLOAT, bufferNegativeZ);

			unsigned int mipRes = unsigned int(resolutions[i] * std::pow(0.5f, level));

			for (unsigned int face = 0; face < 6; ++face)
			{
				for (unsigned int y = 0; y < mipRes; ++y)
				{
					for (unsigned int x = 0; x < mipRes; ++x)
					{
						glm::highp_f32vec3 texel;
						texel[0] = buffers[face][(x + y * mipRes) * 3];
						texel[1] = buffers[face][(x + y * mipRes) * 3 + 1];
						texel[2] = buffers[face][(x + y * mipRes) * 3 + 2];
						texture.store(gli::extent2d(x, y), face, level, texel);
					}
				}
			}

			gli::texture_cube tex16 = gli::convert(texture, gli::FORMAT_RGB16_SFLOAT_PACK16);
			gli::save_dds(tex16, filepaths[i]);
		}
	}

	delete[] bufferPositiveX;
	delete[] bufferNegativeX;
	delete[] bufferPositiveY;
	delete[] bufferNegativeY;
	delete[] bufferPositiveZ;
	delete[] bufferNegativeZ;
}

EnvironmentProbe::EnvironmentProbe(const glm::vec3 &_position)
	:position(_position), valid(false)
{
	GLuint reflectanceId;
	glGenTextures(1, &reflectanceId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, reflectanceId);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, REFLECTANCE_RESOLUTION, REFLECTANCE_RESOLUTION, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	reflectanceMap = Texture::createTexture(reflectanceId, GL_TEXTURE_CUBE_MAP);

	GLuint irradianceId;
	glGenTextures(1, &irradianceId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceId);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, IRRADIANCE_RESOLUTION, IRRADIANCE_RESOLUTION, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	irradianceMap = Texture::createTexture(irradianceId, GL_TEXTURE_CUBE_MAP);
}

EnvironmentProbe::EnvironmentProbe(const glm::vec3 &_position, const std::shared_ptr<Texture> &_reflectanceMap, const std::shared_ptr<Texture> &_irradianceMap)
	:position(_position), valid(true), reflectanceMap(_reflectanceMap), irradianceMap(_irradianceMap)
{
}
