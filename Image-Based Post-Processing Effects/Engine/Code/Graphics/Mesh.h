#pragma once
#include <vector>
#include <string>
#include <glm\vec3.hpp>
#include <glm\vec2.hpp>
#include <map>
#include <glad\glad.h>
#include "..\JobManager.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

class Mesh
{
public:
	static std::shared_ptr<Mesh> createMesh(const std::string &_filepath, bool _instantLoading = false);

	Mesh(const Mesh &) = delete;
	Mesh(const Mesh &&) = delete;
	Mesh &operator= (const Mesh &) = delete;
	Mesh &operator= (const Mesh &&) = delete;
	~Mesh();
	void enableVertexAttribArrays() const;
	void render() const;
	bool isValid() const;

private:
	static std::map<std::string, std::weak_ptr<Mesh>> meshMap;
	std::string filepath;
	bool valid;
	JobManager::SharedJob dataJob;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	std::size_t indexCount;

	explicit Mesh(const std::string &_filepath, bool _instantLoading = false);
	void initOpenGL(const std::vector<Vertex> &_vertices, const std::vector<unsigned int> &_indices);
};