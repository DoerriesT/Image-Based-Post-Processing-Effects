#pragma once
#include <glad\glad.h>
#include <string>
#ifdef _DEBUG
#include <map>
#endif // DEBUG
#include <vector>
#include <glm\mat4x4.hpp>
#include <memory>

class Material;
class PointLight;
class SpotLight;
class DirectionalLight;
struct Lights;

const int SHADOW_MAP_TEXTURE_UNIT_BEGIN = 16;

class ShaderProgram
{
public:
	static std::shared_ptr<ShaderProgram> createShaderProgram(const char *_vertexShaderPath,
		const char *_fragmentShaderPath,
		const char *_tesselationControlShaderPath = nullptr,
		const char *_tesselationEvaluationShaderPath = nullptr,
		const char *_geometryShaderPath = nullptr);
	static std::shared_ptr<ShaderProgram> createShaderProgram(const char *_computeShaderPath);

	ShaderProgram(const ShaderProgram &) = delete;
	ShaderProgram(const ShaderProgram &&) = delete;
	ShaderProgram &operator= (const ShaderProgram &) = delete;
	ShaderProgram &operator= (const ShaderProgram &&) = delete;
	~ShaderProgram();
	void bind();
	GLuint getId() const;
	const GLint createUniform(const std::string &_name) const;
	std::vector<GLint> createPointLightUniform(const std::string &_name) const;
	std::vector<GLint> createSpotLightUniform(const std::string &_name) const;
	std::vector<GLint> createDirectionalLightUniform(const std::string &_name) const;
	std::vector<GLint> createMaterialUniform(const std::string &_name) const;
	void setUniform(const GLint &_location, const GLboolean &_value) const;
	void setUniform(const GLint &_location, const GLint &_value) const;
	void setUniform(const GLint &_location, GLuint _value) const;
	void setUniform(const GLint &_location, const GLfloat &value) const;
	void setUniform(const GLint &_location, const glm::mat3 &_value) const;
	void setUniform(const GLint &_location, const glm::mat4 &_value) const;
	void setUniform(const GLint &_location, const glm::vec2 &_value) const;
	void setUniform(const GLint &_location, const glm::vec3 &_value) const;
	void setUniform(const GLint &_location, const glm::vec4 &_value) const;
	void setUniform(const std::vector<GLint> &_locations, std::shared_ptr<PointLight> _value, int _shadowMapTextureUnit) const;
	void setUniform(const std::vector<GLint> &_locations, std::shared_ptr<SpotLight> _value, int _shadowMapTextureUnit) const;
	void setUniform(const std::vector<GLint> &_locations, std::shared_ptr<DirectionalLight> _value, int _shadowMapTextureUnit) const;
	void setUniform(const std::vector<GLint> &_locations, const Material *_value) const;

private:
	GLuint programId;

	explicit ShaderProgram(const char *_vertexShaderPath,
		const char *_fragmentShaderPath,
		const char *_tesselationControlShaderPath = nullptr,
		const char *_tesselationEvaluationShaderPath = nullptr,
		const char *_geometryShaderPath = nullptr);
	explicit ShaderProgram(const char *_computeShaderPath);
	GLuint createShader(GLenum _type, const char *_shaderPath);
	void statusCheck(GLenum _type);
};