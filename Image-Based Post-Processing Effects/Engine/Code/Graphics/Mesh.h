#pragma once
#include <vector>
#include <string>
#include <glm\vec3.hpp>
#include <glm\vec2.hpp>
#include <map>
#include <glad\glad.h>
#include "..\JobManager.h"
#include "AxisAlignedBoundingBox.h"

class SubMesh;

struct Vertex
{
	glm::vec3 m_position;
	glm::vec2 m_texCoord;
	glm::vec3 m_normal;
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
	AxisAlignedBoundingBox getAABB() const;

private:
	static std::map<std::string, std::weak_ptr<Mesh>> m_meshMap;
	std::string m_filepath;
	bool m_valid;
	JobManager::SharedJob m_dataJob;
	std::vector<std::shared_ptr<SubMesh>> m_subMeshes;
	AxisAlignedBoundingBox m_aabb;

	explicit Mesh(const std::string &_filepath, std::size_t _reserveCount = 0, bool _instantLoading = false);
};

class SubMesh
{
	friend Mesh;
public:
	static std::shared_ptr<SubMesh> createSubMesh();
	static std::shared_ptr<SubMesh> createSubMesh(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize,  char *_indices, const AxisAlignedBoundingBox &_aabb);

	SubMesh(const SubMesh &) = delete;
	SubMesh(const SubMesh &&) = delete;
	SubMesh &operator= (const SubMesh &) = delete;
	SubMesh &operator= (const SubMesh &&) = delete;
	~SubMesh();
	bool isValid() const;
	void enableVertexAttribArrays() const;
	void enableVertexAttribArraysPositionOnly() const;
	void render() const;
	AxisAlignedBoundingBox getAABB() const;
	const std::vector<glm::vec3> &getVertices() const;
	const std::vector<std::uint32_t> &getIndices() const;
	std::size_t getIndexCount() const;

private:
	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_EBO;
	GLuint m_positionVAO;
	GLuint m_positionVBO;
	std::size_t m_indexCount;
	bool m_valid;
	bool m_dataIsSet;
	AxisAlignedBoundingBox m_aabb;
	std::vector<glm::vec3> m_vertices;
	std::vector<std::uint32_t> m_indices;


	explicit SubMesh();
	explicit SubMesh(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices, const AxisAlignedBoundingBox &_aabb);

	void setData(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices, const AxisAlignedBoundingBox &_aabb);
};