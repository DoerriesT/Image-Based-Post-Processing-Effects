#include <iostream>
#include "Mesh.h"
#include "Utilities\ContainerUtility.h"
#include "Utilities\Utility.h"
#include "OpenGL\GLUtility.h"
#include <functional>
#include "Engine.h"
#include "JobManager.h"

const std::uint32_t MAGIC_NUMBER = 0xFFABCDFF;

std::map<std::string, std::weak_ptr<Mesh>> Mesh::m_meshMap;

std::shared_ptr<Mesh> Mesh::createMesh(const std::string &_filepath, std::size_t _reserveCount, bool _instantLoading)
{
	if (ContainerUtility::contains(m_meshMap, _filepath))
	{
		return std::shared_ptr<Mesh>(m_meshMap[_filepath]);
	}
	else
	{
		std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(_filepath, _reserveCount, _instantLoading));
		m_meshMap[_filepath] = mesh;
		return mesh;
	}
}

Mesh::~Mesh()
{
	if (m_dataJob)
	{
		m_dataJob->kill();
	}
	ContainerUtility::remove(m_meshMap, m_filepath);
}

std::shared_ptr<SubMesh> Mesh::getSubMesh(std::size_t _index) const
{
	assert(_index < m_subMeshes.size());
	return m_subMeshes[_index];
}

const std::vector<std::shared_ptr<SubMesh>> Mesh::getSubMeshes() const
{
	return m_subMeshes;
}

std::size_t Mesh::size() const
{
	return m_subMeshes.size();
}

bool Mesh::isValid() const
{
	return m_valid;
}

AxisAlignedBoundingBox Mesh::getAABB() const
{
	return m_aabb;
}

Mesh::Mesh(const std::string &_filepath, std::size_t _reserveCount, bool _instantLoading)
	:m_filepath(_filepath), m_valid(false)
{
	for (size_t i = 0; i < _reserveCount; ++i)
	{
		m_subMeshes.push_back(SubMesh::createSubMesh());
	}

	auto dataPreparation = [=](JobManager::SharedJob job)
	{
		std::vector<char> data = Utility::readBinaryFile(_filepath);
		assert(data.size());
		void *userData = new std::vector<char>(std::move(data));
		job->setUserData(userData);
	};

	auto dataInitialization = [=](JobManager::SharedJob job)
	{
		// magic number (4) | mesh count (4) | {vertex buffer size (4) | vertex buffer | index buffer size (4) | index buffer} * mesh count
		std::vector<char> &data = *(std::vector<char> *)job->getUserData();

		char *rawData = data.data();

		assert(*(std::uint32_t *)rawData == MAGIC_NUMBER);

		// skip past magic number
		std::size_t currentOffset = sizeof(std::uint32_t);

		// read subMesh count and reserve space
		std::size_t numSubMeshes = static_cast<size_t>(*(std::uint32_t *)&rawData[currentOffset]);
		m_subMeshes.reserve(numSubMeshes);

		// skip past sub mesh count
		currentOffset += sizeof(std::uint32_t);

		// read all sub mesh data
		for (size_t i = 0; i < numSubMeshes; ++i)
		{
			// read aabb
			AxisAlignedBoundingBox meshAabb = *(AxisAlignedBoundingBox *)&rawData[currentOffset];
			// skip past aabb
			currentOffset += sizeof(AxisAlignedBoundingBox);

			// read buffer size
			std::size_t vertexBufferSize = static_cast<size_t>(*(std::uint32_t *)&rawData[currentOffset]);
			// skip past bufferSize
			currentOffset += sizeof(std::uint32_t);
			// get pointer to buffer data
			char *vertexBuffer = &rawData[currentOffset];
			// skip past buffer data
			currentOffset += vertexBufferSize;

			// read buffer size
			std::size_t indexBufferSize = static_cast<size_t>(*(std::uint32_t *)&rawData[currentOffset]);
			// skip past bufferSize
			currentOffset += sizeof(std::uint32_t);
			// get pointer to buffer data
			char *indexBuffer = &rawData[currentOffset];
			// skip past buffer data
			currentOffset += indexBufferSize;

			if (_reserveCount)
			{
				assert(_reserveCount == numSubMeshes);
				m_subMeshes[i]->setData(static_cast<uint32_t>(vertexBufferSize), vertexBuffer, static_cast<uint32_t>(indexBufferSize), indexBuffer, meshAabb);
			}
			else
			{
				m_subMeshes.push_back(SubMesh::createSubMesh(static_cast<uint32_t>(vertexBufferSize), vertexBuffer, static_cast<uint32_t>(indexBufferSize), indexBuffer, meshAabb));
			}
		}

		m_aabb = *(AxisAlignedBoundingBox *)&rawData[currentOffset];

		// set flag that mesh can be used
		m_valid = true;
		m_dataJob.reset();

		job->markDone(true);
	};


	auto dataCleanup = [=](JobManager::SharedJob job)
	{
		std::vector<char> *data = (std::vector<char> *)job->getUserData();
		delete data;
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

std::shared_ptr<SubMesh> SubMesh::createSubMesh()
{
	return std::shared_ptr<SubMesh>(new SubMesh());
}

std::shared_ptr<SubMesh> SubMesh::createSubMesh(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices, const AxisAlignedBoundingBox &_aabb)
{
	return std::shared_ptr<SubMesh>(new SubMesh(_vertexBufferSize, _vertices, _indexBufferSize, _indices, _aabb));
}

SubMesh::SubMesh()
	:m_valid(false), m_dataIsSet(false)
{
}

SubMesh::SubMesh(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices, const AxisAlignedBoundingBox &_aabb)
	: m_valid(false), m_dataIsSet(false)
{
	setData(_vertexBufferSize, _vertices, _indexBufferSize, _indices, _aabb);
}

void SubMesh::setData(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices, const AxisAlignedBoundingBox &_aabb)
{
	assert(!m_dataIsSet);
	m_dataIsSet = true;
	m_indexCount = static_cast<size_t>(_indexBufferSize) / sizeof(std::uint32_t);
	m_aabb = _aabb;

	// create vertex data for physics
	{
		for (std::size_t i = 0; i < static_cast<size_t>(_vertexBufferSize)/sizeof(Vertex); ++i)
		{
			m_vertices.push_back(((Vertex *)_vertices)[i].m_position);
		}

		for (std::size_t i = 0; i <m_indexCount; ++i)
		{
			m_indices.push_back(((std::uint32_t *)_indices)[i]);
		}
	}

	// create buffers/arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(_vertexBufferSize), _vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<size_t>(_indexBufferSize), _indices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// vertex texture coord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_texCoord));

	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));

	glBindVertexArray(0);

	// create buffers/arrays
	glGenVertexArrays(1, &m_positionVAO);
	glGenBuffers(1, &m_positionVBO);
	glBindVertexArray(m_positionVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionVBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), m_vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);

	m_valid = true;
}

SubMesh::~SubMesh()
{
	glBindVertexArray(m_VAO);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VBO);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_VAO);

	glBindVertexArray(m_positionVAO);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_positionVBO);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_positionVAO);
}

bool SubMesh::isValid() const
{
	return m_valid;
}

void SubMesh::enableVertexAttribArrays() const
{
	assert(m_VAO);
	glBindVertexArray(m_VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void SubMesh::enableVertexAttribArraysPositionOnly() const
{
	assert(m_positionVAO);
	glBindVertexArray(m_positionVAO);
	glEnableVertexAttribArray(0);
}

void SubMesh::render() const
{
#ifdef _DEBUG
	GLUtility::glErrorCheck("BEFORE");
#endif // DEBUG	
	assert(m_indexCount);
	glDrawElements(GL_TRIANGLES, (GLsizei)m_indexCount, GL_UNSIGNED_INT, NULL);

#ifdef _DEBUG
	GLUtility::glErrorCheck("AFTER");
#endif // DEBUG	
}

AxisAlignedBoundingBox SubMesh::getAABB() const
{
	return m_aabb;
}

const std::vector<glm::vec3>& SubMesh::getVertices() const
{
	return m_vertices;
}

const std::vector<std::uint32_t>& SubMesh::getIndices() const
{
	return m_indices;
}

std::size_t SubMesh::getIndexCount() const
{
	return m_indexCount;
}
