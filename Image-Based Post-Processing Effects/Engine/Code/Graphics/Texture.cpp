#include "Texture.h"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <functional>
#include "Utilities\ContainerUtility.h"
#include "Utilities\Utility.h"
#include "Engine.h"
#include "JobManager.h"
#include <stb_image.h>
#include <gli\texture.hpp>
#include <gli\load.hpp>

struct PackedJobTexture
{
	bool m_isDDS;
	gli::texture m_gliTexture;
	unsigned char *m_stbiData;
	int m_width;
	int m_height;
	int m_channels;
};


std::map<std::string, std::weak_ptr<Texture>> Texture::m_textureMap;
float Texture::m_anisotropicFiltering = 1.0f;

std::shared_ptr<Texture> Texture::createTexture(GLuint _id, GLenum _target)
{
	std::string idStr = std::to_string(_id);
	if (ContainerUtility::contains(m_textureMap, idStr))
	{
		return std::shared_ptr<Texture>(m_textureMap[idStr]);
	}
	else
	{
		std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture(_id, _target));
		m_textureMap[idStr] = texture;
		return texture;
	}
}

std::shared_ptr<Texture> Texture::createTexture(const std::string &_filename, bool _instantLoading)
{
	if (ContainerUtility::contains(m_textureMap, _filename))
	{
		return std::shared_ptr<Texture>(m_textureMap[_filename]);
	}
	else
	{
		std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture(_filename, _instantLoading));
		m_textureMap[_filename] = texture;
		return texture;
	}
}

Texture::Texture(GLuint _id, GLenum _target)
	:m_filepath(std::to_string(_id)), m_valid(true), m_dataJob(nullptr), m_id(_id), m_target(_target)
{
}

Texture::Texture(const std::string &_filename, bool _instantLoading)
	: m_filepath(_filename), m_valid(false), m_dataJob(nullptr)
{
	JobManager::Work dataPreparation = [=](JobManager::SharedJob job)
	{
		bool isPNG = Utility::getPathFileExtension(_filename) == "png";

		PackedJobTexture *pack = new PackedJobTexture();
		if (isPNG)
		{
			pack->m_stbiData = stbi_load(_filename.c_str(), &pack->m_width, &pack->m_height, &pack->m_channels, 0);
			assert(pack->m_stbiData);
		}
		else
		{
			pack->m_gliTexture = gli::load(_filename);
			assert(!pack->m_gliTexture.empty());
		}

		pack->m_isDDS = !isPNG;

		job->setUserData(pack);
	};

	JobManager::Work dataInitialization = [=](JobManager::SharedJob job)
	{
		PackedJobTexture *textureData = (PackedJobTexture *)job->getUserData();

		if (textureData->m_isDDS)
		{
			initOpenGL(textureData->m_gliTexture);
		}
		else
		{
			initOpenGLFromData(textureData);
		}

		// set flag that texture can be used
		m_valid = true;
		m_dataJob.reset();

		job->markDone(true);
	};

	JobManager::Work dataCleanup = [=](JobManager::SharedJob job)
	{
		PackedJobTexture *pack = (PackedJobTexture *)job->getUserData();

		if (!pack->m_isDDS)
		{
			stbi_image_free(pack->m_stbiData);
		}

		delete pack;
	};


	if (_instantLoading)
	{
		JobManager::getInstance().run(dataPreparation, dataInitialization, dataCleanup);
	}
	else
	{
		m_dataJob = JobManager::getInstance().queue(dataPreparation, dataInitialization, dataCleanup);
	}
}

Texture::~Texture()
{
	if (m_dataJob)
	{
		m_dataJob->kill();
	}
	ContainerUtility::remove(m_textureMap, m_filepath);
	if (m_valid)
	{
		glDeleteTextures(1, &m_id);
	}
}

GLuint Texture::getId() const
{
	assert(m_valid);
	return m_id;
}

GLenum Texture::getTarget() const
{
	return m_target;
}

bool Texture::isValid() const
{
	return m_valid;
}

void Texture::setAnisotropicFilteringAll(float _anisotropicFiltering)
{
	if (m_anisotropicFiltering != _anisotropicFiltering)
	{
		for (auto &pair : m_textureMap)
		{
			pair.second.lock()->setAnisotropicFiltering(_anisotropicFiltering);
		}
		m_anisotropicFiltering = _anisotropicFiltering;
	}
}

void Texture::setAnisotropicFiltering(float _anisotropicFiltering)
{
	if (GLAD_GL_EXT_texture_filter_anisotropic)
	{
		glBindTexture(m_target, m_id);
		glTexParameterf(m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, static_cast<GLfloat>(_anisotropicFiltering));
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
	m_target = textureType;
	m_id = 0;

	glGenTextures(1, &m_id);
	glBindTexture(textureType, m_id);
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

	setAnisotropicFiltering(m_anisotropicFiltering);

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
	m_target = GL_TEXTURE_2D;
	m_id = 0;

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);
	GLenum format;

	setAnisotropicFiltering(m_anisotropicFiltering);

	switch (texture->m_channels)
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
	glTexImage2D(m_target, 0, format, texture->m_width, texture->m_height, 0, format, GL_UNSIGNED_BYTE, texture->m_stbiData);

	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(m_target);
}
