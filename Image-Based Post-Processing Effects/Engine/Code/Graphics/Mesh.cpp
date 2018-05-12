#include <iostream>
#include "Mesh.h"
#include ".\..\Utilities\Utility.h"
#include <functional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include ".\..\Engine.h"
#include ".\..\JobManager.h"

std::map<std::string, std::weak_ptr<Mesh>> Mesh::meshMap;

std::shared_ptr<Mesh> Mesh::createMesh(const std::string &_filepath, bool _instantLoading)
{
	if (contains(meshMap, _filepath))
	{
		return std::shared_ptr<Mesh>(meshMap[_filepath]);
	}
	else
	{
		std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(_filepath, _instantLoading));
		meshMap[_filepath] = mesh;
		return mesh;
	}
}

Mesh::Mesh(const std::string &_filepath, bool _instantLoading)
	:filepath(_filepath), valid(false), dataJob(nullptr)
{
	auto dataPreparation = [=](JobManager::SharedJob job)
	{
		// load scene
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(_filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			job->markDone(false);
			return;
		}

		// assert scene has at least one mesh
		assert(scene->mNumMeshes);

		// get first mesh
		aiMesh *mesh = scene->mMeshes[0];
		
		// prepare vertex and index lists
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = glm::normalize(vector);

			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.texCoords = vec;
			}
			else
			{
				vertex.texCoords = glm::vec2(0.0f, 0.0f);
			}

			if (mesh->mTangents && mesh->mBitangents)
			{
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.tangent = glm::normalize(vector);

				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.bitangent = glm::normalize(vector);
			}
			else
			{
				vertex.tangent = glm::vec3();
				vertex.bitangent = glm::vec3();
			}

			vertices.push_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];

			for (uint32_t j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);
			}
		}
		
		auto *result = new std::pair<std::vector<Vertex>, std::vector<unsigned int>>(std::move(vertices), std::move(indices));
		assert(result);
		job->setUserData(result);
	};

	auto dataInitialization = [=](JobManager::SharedJob job)
	{
		auto *p = (std::pair<std::vector<Vertex>, std::vector<unsigned int>> *)job->getUserData();
		indexCount = p->second.size();

		// initialize with opengl
		initOpenGL(p->first, p->second);

		// set flag that mesh can be used
		valid = true;
		dataJob.reset();

		job->markDone(true);
	};


	auto dataCleanup = [=](JobManager::SharedJob job)
	{
		auto *p = (std::pair<std::vector<Vertex>, std::vector<unsigned int>> *)job->getUserData();
		delete p;
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

Mesh::~Mesh()
{
	if (dataJob)
	{
		dataJob->kill();
	}
	remove(meshMap, filepath);
	if (valid)
	{
		glBindVertexArray(VAO);
		glDisableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &EBO);
		glDeleteBuffers(1, &VBO);

		glBindVertexArray(0);
		glDeleteVertexArrays(1, &VAO);
	}
}

void Mesh::enableVertexAttribArrays() const
{
	assert(VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
}

void Mesh::initOpenGL(const std::vector<Vertex> &_vertices, const std::vector<unsigned int> &_indices)
{
	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(std::uint32_t), &_indices[0], GL_STATIC_DRAW);

	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

	glBindVertexArray(0);
}

void Mesh::render()  const
{
#ifdef _DEBUG
	glErrorCheck("BEFORE");
#endif // DEBUG	
	assert(indexCount);
	glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, NULL);

#ifdef _DEBUG
	glErrorCheck("AFTER");
#endif // DEBUG	
}

bool Mesh::isValid() const
{
	return valid;
}
