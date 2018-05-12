#pragma once
#include <glad\glad.h>
#include <string>
#include <map>
#include <memory>
#include ".\..\JobManager.h"

struct PackedJobTexture;
namespace gli
{
	class texture;
}


class Texture
{
public:
	static std::shared_ptr<Texture> createTexture(const GLuint &_id, const GLenum &_target);
	static std::shared_ptr<Texture> createTexture(const std::string &_filename, const bool &_instantLoading = false);
	static void setAnisotropicFilteringAll(const float &_anisotropicFiltering);

	Texture(const Texture &) = delete;
	Texture(const Texture &&) = delete;
	Texture &operator= (const Texture &) = delete;
	Texture &operator= (const Texture &&) = delete;
	~Texture();
	GLuint getId() const;
	GLenum getTarget() const;
	bool isValid() const;
	void setAnisotropicFiltering(const float &_anisotropicFiltering);

private:
	static std::map<std::string, std::weak_ptr<Texture>> textureMap;
	static float anisotropicFiltering;
	std::string filepath;
	bool valid;
	JobManager::SharedJob dataJob;
	GLuint id;
	GLenum target;


	explicit Texture(const GLuint &_id, const GLenum &_target);
	explicit Texture(const std::string &_filename, const bool &_instantLoading = false);
	void initOpenGL(const gli::texture &_file);
	void initOpenGLFromData(PackedJobTexture *texture);
};