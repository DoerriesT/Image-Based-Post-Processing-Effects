#pragma once
#include <glad\glad.h>
#include <string>
#include <vector>
#include <tuple>
#include <glm\mat4x4.hpp>
#include <memory>

class ShaderProgram
{
public:
	enum class ShaderType
	{
		VERTEX, FRAGMENT, TESSELATION_CONTROL, TESSELATION_EVALUATION, GEOMETRY, COMPUTE
	};

	static std::shared_ptr<ShaderProgram> createShaderProgram(
		const char *_vertexShaderPath,
		const char *_fragmentShaderPath,
		const char *_tesselationControlShaderPath = nullptr,
		const char *_tesselationEvaluationShaderPath = nullptr,
		const char *_geometryShaderPath = nullptr);
	static std::shared_ptr<ShaderProgram> createShaderProgram(
		const std::vector<std::tuple<ShaderType, std::string, int>> &_defines,
		const char *_vertexShaderPath,
		const char *_fragmentShaderPath,
		const char *_tesselationControlShaderPath = nullptr,
		const char *_tesselationEvaluationShaderPath = nullptr,
		const char *_geometryShaderPath = nullptr);
	static std::shared_ptr<ShaderProgram> createShaderProgram(const char *_computeShaderPath);
	static std::shared_ptr<ShaderProgram> createShaderProgram(const std::vector<std::tuple<ShaderType, std::string, int>> &_defines, const char *_computeShaderPath);

	ShaderProgram(const ShaderProgram &) = delete;
	ShaderProgram(const ShaderProgram &&) = delete;
	ShaderProgram &operator= (const ShaderProgram &) = delete;
	ShaderProgram &operator= (const ShaderProgram &&) = delete;
	~ShaderProgram();
	void bind();
	GLuint getId() const;
	std::vector<std::tuple<ShaderType, std::string, int>> getDefines() const;
	const GLint createUniform(const std::string &_name) const;
	void setUniform(const GLint &_location, const GLboolean &_value) const;
	void setUniform(const GLint &_location, const GLint &_value) const;
	void setUniform(const GLint &_location, GLuint _value) const;
	void setUniform(const GLint &_location, const GLfloat &value) const;
	void setUniform(const GLint &_location, const glm::mat3 &_value) const;
	void setUniform(const GLint &_location, const glm::mat4 &_value) const;
	void setUniform(const GLint &_location, const glm::vec2 &_value) const;
	void setUniform(const GLint &_location, const glm::vec3 &_value) const;
	void setUniform(const GLint &_location, const glm::ivec3 &_value) const;
	void setUniform(const GLint &_location, const glm::vec4 &_value) const;
	void setDefines(const std::vector<std::tuple<ShaderType, std::string, int>> &_defines);

private:
	GLuint programId;
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	std::string tesselationControlShaderPath;
	std::string tesselationEvaluationShaderPath;
	std::string geometryShaderPath;
	std::string computeShaderPath;
	std::vector<std::tuple<ShaderType, std::string, int>> defines;

	explicit ShaderProgram(
		const std::vector<std::tuple<ShaderType, std::string, int>> &_defines, 
		const char *_vertexShaderPath,
		const char *_fragmentShaderPath,
		const char *_tesselationControlShaderPath = nullptr,
		const char *_tesselationEvaluationShaderPath = nullptr,
		const char *_geometryShaderPath = nullptr);
	explicit ShaderProgram(const std::vector<std::tuple<ShaderType, std::string, int>> &_defines, const char *_computeShaderPath);
	GLuint createShader(GLenum _type, const char *_shaderPath);
	void statusCheck(GLenum _type);
	void create();
};