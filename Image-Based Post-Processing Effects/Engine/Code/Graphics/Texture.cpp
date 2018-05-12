#include "Texture.h"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <functional>
#include ".\..\Utilities\Utility.h"
#include ".\..\Engine.h"
#include ".\..\JobManager.h"
#include <stb_image.h>
#include <gli\texture.hpp>
#include <gli\load.hpp>

struct PackedJobTexture
{
	bool isDDS;
	gli::texture gliTexture;
	unsigned char *stbiData;
	int width;
	int height;
	int channels;
};


std::map<std::string, std::weak_ptr<Texture>> Texture::textureMap;
float Texture::anisotropicFiltering = 1.0f;

std::shared_ptr<Texture> Texture::createTexture(const GLuint &_id, const GLenum &_target)
{
	std::string idStr = std::to_string(_id);
	if (contains(textureMap, idStr))
	{
		return std::shared_ptr<Texture>(textureMap[idStr]);
	}
	else
	{
		std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture(_id, _target));
		textureMap[idStr] = texture;
		return texture;
	}
}

std::shared_ptr<Texture> Texture::createTexture(const std::string &_filename, const bool &_instantLoading)
{
	if (contains(textureMap, _filename))
	{
		return std::shared_ptr<Texture>(textureMap[_filename]);
	}
	else
	{
		std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture(_filename, _instantLoading));
		textureMap[_filename] = texture;
		return texture;
	}
}

Texture::Texture(const GLuint &_id, const GLenum &_target)
	:filepath(std::to_string(_id)), valid(true), dataJob(nullptr), id(_id), target(_target)
{
}

Texture::Texture(const std::string &_filename, const bool &_instantLoading)
	: filepath(_filename), valid(false), dataJob(nullptr)
{
	JobManager::Work dataPreparation = [=](JobManager::SharedJob job)
	{
		bool isPNG = Util::getPathFileExtension(_filename) == "png";

		PackedJobTexture *pack = new PackedJobTexture();
		if (isPNG)
		{
			pack->stbiData = stbi_load(_filename.c_str(), &pack->width, &pack->height, &pack->channels, 0);
			assert(pack->stbiData);
		}
		else
		{
			pack->gliTexture = gli::load(_filename);
			assert(!pack->gliTexture.empty());
		}

		pack->isDDS = !isPNG;

		job->setUserData(pack);
	};

	JobManager::Work dataInitialization = [=](JobManager::SharedJob job)
	{
		PackedJobTexture *textureData = (PackedJobTexture *)job->getUserData();

		if (textureData->isDDS)
		{
			initOpenGL(textureData->gliTexture);
		}
		else
		{
			initOpenGLFromData(textureData);
		}

		// set flag that texture can be used
		valid = true;
		dataJob.reset();

		job->markDone(true);
	};

	JobManager::Work dataCleanup = [=](JobManager::SharedJob job)
	{
		PackedJobTexture *pack = (PackedJobTexture *)job->getUserData();

		if (!pack->isDDS)
		{
			stbi_image_free(pack->stbiData);
		}

		delete pack;
	};


	if (_instantLoading)
	{
		JobManager::getInstance().run(dataPreparation, dataInitialization, dataCleanup);
	}
	else
	{
		dataJob = JobManager::getInstance().queue(dataPreparation, dataInitialization, dataCleanup);
	}
}

Texture::~Texture()
{
	if (dataJob)
	{
		dataJob->kill();
	}
	remove(textureMap, filepath);
	if (valid)
	{
		glDeleteTextures(1, &id);
	}
}

GLuint Texture::getId() const
{
	assert(valid);
	return id;
}

GLenum Texture::getTarget() const
{
	return target;
}

bool Texture::isValid() const
{
	return valid;
}

void Texture::setAnisotropicFilteringAll(const float &_anisotropicFiltering)
{
	if (anisotropicFiltering != _anisotropicFiltering)
	{
		for (auto &pair : textureMap)
		{
			pair.second.lock()->setAnisotropicFiltering(_anisotropicFiltering);
		}
		anisotropicFiltering = _anisotropicFiltering;
	}
}

void Texture::setAnisotropicFiltering(const float &_anisotropicFiltering)
{
	if (GLAD_GL_EXT_texture_filter_anisotropic)
	{
		glBindTexture(target, id);
		glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, static_cast<GLfloat>(_anisotropicFiltering));
	}
	else
	{
		std::cout << "OpenGL extension GL_EXT_texture_filter_anisotropic is not supported!" << std::endl;
	}
}

void Texture::initOpenGL(const gli::texture &_file)
{
	const gli::texture &texture = _file;

	gli::gl glTranslator(gli::gl::PROFILE_GL33);
	gli::gl::format const format = glTranslator.translate(texture.format(), texture.swizzles());

	GLenum textureType = glTranslator.translate(texture.target());
	target = textureType;
	id = 0;

	glGenTextures(1, &id);
	glBindTexture(textureType, id);
	// Base and max level are not supported by OpenGL ES 2.0
	glTexParameteri(textureType, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(textureType, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));
	//Texture swizzle is not supported by OpenGL ES 2.0 and OpenGL 3.2
	glTexParameteri(textureType, GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
	glTexParameteri(textureType, GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
	glTexParameteri(textureType, GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
	glTexParameteri(textureType, GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);
	glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setAnisotropicFiltering(anisotropicFiltering);

	glm::tvec3<GLsizei> const extent(texture.extent());
	GLsizei const totalFaces = static_cast<GLsizei>(texture.layers() * texture.faces());

	for (std::size_t layer = 0; layer < texture.layers(); ++layer)
	{
		for (std::size_t level = 0; level < texture.levels(); ++level)
		{
			for (std::size_t face = 0; face < texture.faces(); ++face)
			{
				GLsizei const LayerGL = static_cast<GLsizei>(layer);
				glm::tvec3<GLsizei> extent(texture.extent(level));
				textureType = gli::is_target_cube(texture.target()) ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face) : textureType;

				switch (texture.target())
				{
					case gli::TARGET_1D:
					{
						if (gli::is_compressed(texture.format()))
						{
							glCompressedTexImage1D(textureType, format.Internal, static_cast<GLint>(level), 0, extent.x, static_cast<GLsizei>(texture.size(level)), texture.data(layer, face, level));
						}
						else
						{
							glTexImage1D(textureType, static_cast<GLint>(level), format.Internal, extent.x, 0, format.External, format.Type, texture.data(layer, face, level));
						}
						break;
					}
					case gli::TARGET_1D_ARRAY:
					case gli::TARGET_2D:
					case gli::TARGET_CUBE:
					{
						if (gli::is_compressed(texture.format()))
						{
							glCompressedTexImage2D(textureType, static_cast<GLint>(level), format.Internal, extent.x, texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : extent.y, 0, static_cast<GLsizei>(texture.size(level)), texture.data(layer, face, level));
						}
						else
						{
							glTexImage2D(textureType, static_cast<GLint>(level), format.Internal, extent.x, texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : extent.y, 0, format.External, format.Type, texture.data(layer, face, level));
						}
						break;
					}
					case gli::TARGET_2D_ARRAY:
					case gli::TARGET_3D:
					case gli::TARGET_CUBE_ARRAY:
					{
						if (gli::is_compressed(texture.format()))
						{
							glCompressedTexImage3D(textureType, static_cast<GLint>(level), format.Internal, extent.x, extent.y, texture.target() == gli::TARGET_3D ? extent.z : LayerGL, 0, static_cast<GLsizei>(texture.size(level)), texture.data(layer, face, level));
						}
						else
						{
							glTexImage3D(textureType, static_cast<GLint>(level), format.Internal, extent.x, extent.y, texture.target() == gli::TARGET_3D ? extent.z : LayerGL, 0, format.External, format.Type, texture.data(layer, face, level));
						}
						break;
					}
					default:
					{
						assert(false);
						break;
					}
				}
			}
		}
	}
}

void Texture::initOpenGLFromData(PackedJobTexture *texture)
{
	target = GL_TEXTURE_2D;
	id = 0;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	GLenum format;

	setAnisotropicFiltering(anisotropicFiltering);

	switch (texture->channels)
	{
		case 4:
			format = GL_RGBA;
			break;
		case 3:
			format = GL_RGB;
			break;
		default:
			format = GL_RED;
			break;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(target, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, texture->stbiData);

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(target);
}
