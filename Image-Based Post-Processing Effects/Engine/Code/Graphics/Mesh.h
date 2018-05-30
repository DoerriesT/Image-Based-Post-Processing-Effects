#pragma once
#include <vector>
#include <string>
#include <glm\vec3.hpp>
#include <glm\vec2.hpp>
#include <map>
#include <glad\glad.h>
#include "..\JobManager.h"

class SubMesh;

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
};

class Mesh
{
public:
	static std::shared_ptr<Mesh> createMesh(const std::string &_filepath, std::size_t _reserveCount = 0, bool _instantLoading = false);

	Mesh(const Mesh &) = delete;
	Mesh(const Mesh &&) = delete;
	Mesh &operator= (const Mesh &) = delete;
	Mesh &operator= (const Mesh &&) = delete;
	~Mesh();
	std::shared_ptr<SubMesh> getSubMesh(std::size_t _index = 0) const;
	const std::vector<std::shared_ptr<SubMesh>> getSubMeshes() const;
	std::size_t size() const;
	bool isValid() const;

private:
	static std::map<std::string, std::weak_ptr<Mesh>> meshMap;
	std::string filepath;
	bool valid;
	JobManager::SharedJob dataJob;
	std::vector<std::shared_ptr<SubMesh>> subMeshes;

	explicit Mesh(const std::string &_filepath, std::size_t _reserveCount = 0, bool _instantLoading = false);
};

class SubMesh
{
	friend Mesh;
public:
	static std::shared_ptr<SubMesh> createSubMesh();
	static std::shared_ptr<SubMesh> createSubMesh(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize,  char *_indices);

	SubMesh(const SubMesh &) = delete;
	SubMesh(const SubMesh &&) = delete;
	SubMesh &operator= (const SubMesh &) = delete;
	SubMesh &operator= (const SubMesh &&) = delete;
	~SubMesh();
	bool isValid() const;
	void enableVertexAttribArrays() const;
	void render() const;

private:
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	std::size_t indexCount;
	bool valid;
	bool dataIsSet;


	explicit SubMesh();
	explicit SubMesh(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices);

	void setData(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices);
};