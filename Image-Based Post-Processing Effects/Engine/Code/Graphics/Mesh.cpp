#include <iostream>
#include "Mesh.h"
#include "Utilities\ContainerUtility.h"
#include "Utilities\Utility.h"
#include "OpenGL\GLUtility.h"
#include <functional>
#include "Engine.h"
#include "JobManager.h"

const std::uint32_t MAGIC_NUMBER = 0xFFABCDFF;

std::map<std::string, std::weak_ptr<Mesh>> Mesh::meshMap;

std::shared_ptr<Mesh> Mesh::createMesh(const std::string &_filepath, std::size_t _reserveCount, bool _instantLoading)
{
	if (ContainerUtility::contains(meshMap, _filepath))
	{
		return std::shared_ptr<Mesh>(meshMap[_filepath]);
	}
	else
	{
		std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(_filepath, _reserveCount, _instantLoading));
		meshMap[_filepath] = mesh;
		return mesh;
	}
}

Mesh::~Mesh()
{
	if (dataJob)
	{
		dataJob->kill();
	}
	ContainerUtility::remove(meshMap, filepath);
}

std::shared_ptr<SubMesh> Mesh::getSubMesh(std::size_t _index) const
{
	assert(_index < subMeshes.size());
	return subMeshes[_index];
}

const std::vector<std::shared_ptr<SubMesh>> Mesh::getSubMeshes() const
{
	return subMeshes;
}

std::size_t Mesh::size() const
{
	return subMeshes.size();
}

bool Mesh::isValid() const
{
	return valid;
}

AxisAlignedBoundingBox Mesh::getAABB() const
{
	return aabb;
}

Mesh::Mesh(const std::string &_filepath, std::size_t _reserveCount, bool _instantLoading)
	:filepath(_filepath), valid(false)
{
	for (size_t i = 0; i < _reserveCount; ++i)
	{
		subMeshes.push_back(SubMesh::createSubMesh());
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
		subMeshes.reserve(numSubMeshes);

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
				subMeshes[i]->setData(static_cast<uint32_t>(vertexBufferSize), vertexBuffer, static_cast<uint32_t>(indexBufferSize), indexBuffer, meshAabb);
			}
			else
			{
				subMeshes.push_back(SubMesh::createSubMesh(static_cast<uint32_t>(vertexBufferSize), vertexBuffer, static_cast<uint32_t>(indexBufferSize), indexBuffer, meshAabb));
			}
		}

		aabb = *(AxisAlignedBoundingBox *)&rawData[currentOffset];

		// set flag that mesh can be used
		valid = true;
		dataJob.reset();

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
		dataJob = JobManager::getInstance().queue(dataPreparation, dataInitialization, dataCleanup);
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
	:valid(false), dataIsSet(false)
{
}

SubMesh::SubMesh(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices, const AxisAlignedBoundingBox &_aabb)
	: valid(false), dataIsSet(false)
{
	setData(_vertexBufferSize, _vertices, _indexBufferSize, _indices, _aabb);
}

void SubMesh::setData(std::uint32_t _vertexBufferSize, char *_vertices, std::uint32_t _indexBufferSize, char *_indices, const AxisAlignedBoundingBox &_aabb)
{
	assert(!dataIsSet);
	dataIsSet = true;
	indexCount = static_cast<size_t>(_indexBufferSize) / sizeof(std::uint32_t);
	aabb = _aabb;

	// create vertex data for physics
	{
		for (std::size_t i = 0; i < static_cast<size_t>(_vertexBufferSize)/sizeof(Vertex); ++i)
		{
			vertices.push_back(((Vertex *)_vertices)[i].position);
		}

		for (std::size_t i = 0; i <indexCount; ++i)
		{
			indices.push_back(((std::uint32_t *)_indices)[i]);
		}
	}

	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<size_t>(_vertexBufferSize), _vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<size_t>(_indexBufferSize), _indices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// vertex texture coord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glBindVertexArray(0);

	// create buffers/arrays
	glGenVertexArrays(1, &positionVAO);
	glGenBuffers(1, &positionVBO);
	glBindVertexArray(positionVAO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);

	valid = true;
}

SubMesh::~SubMesh()
{
	glBindVertexArray(VAO);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAO);

	glBindVertexArray(positionVAO);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &positionVBO);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &positionVAO);
}

bool SubMesh::isValid() const
{
	return valid;
}

void SubMesh::enableVertexAttribArrays() const
{
	assert(VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void SubMesh::enableVertexAttribArraysPositionOnly() const
{
	assert(positionVAO);
	glBindVertexArray(positionVAO);
	glEnableVertexAttribArray(0);
}

void SubMesh::render() const
{
#ifdef _DEBUG
	GLUtility::glErrorCheck("BEFORE");
#endif // DEBUG	
	assert(indexCount);
	glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, NULL);

#ifdef _DEBUG
	GLUtility::glErrorCheck("AFTER");
#endif // DEBUG	
}

AxisAlignedBoundingBox SubMesh::getAABB() const
{
	return aabb;
}

const std::vector<glm::vec3>& SubMesh::getVertices() const
{
	return vertices;
}

const std::vector<std::uint32_t>& SubMesh::getIndices() const
{
	return indices;
}

std::size_t SubMesh::getIndexCount() const
{
	return indexCount;
}
