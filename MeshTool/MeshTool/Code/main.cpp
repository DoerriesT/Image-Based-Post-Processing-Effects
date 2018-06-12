#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <exception>
#include <iostream>
#include <cassert>
#include <vector>
#include <glm\vec3.hpp>
#include <glm\vec2.hpp>
#include <glm\geometric.hpp>
#include <fstream>
#include <limits>

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
};

const std::uint32_t MAGIC_NUMBER = 0xFFABCDFF;
const std::string MATERIAL_COUNT_STRING = "material count: ";
const std::string MESH_FILE_STRING = "mesh file: ";
const std::string NAME_STRING = "name: ";
const std::string ALBEDO_STRING = "albedo: ";
const std::string METALLIC_STRING = "metallic: ";
const std::string ROUGHNESS_STRING = "roughness: ";
const std::string EMISSIVE_STRING = "emissive: ";
const std::string ALBEDO_PATH_STRING = "albedoPath: ";
const std::string NORMAL_PATH_STRING = "normalPath: ";
const std::string METALLIC_PATH_STRING = "metallicPath: ";
const std::string ROUGHNESS_PATH_STRING = "roughnessPath: ";
const std::string AO_PATH_STRING = "aoPath: ";
const std::string EMISSIVE_PATH_STRING = "emissivePath: ";

int main()
{
	std::cout << "Source File:" << std::endl;

	std::string srcFileName;

	std::cin >> srcFileName;

	std::cout << "Destination File:" << std::endl;

	std::string dstFileName;

	std::cin >> dstFileName;

	// load scene
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(srcFileName, aiProcess_Triangulate 
		| aiProcess_FlipUVs 
		| aiProcess_CalcTangentSpace 
		| aiProcess_JoinIdenticalVertices 
		| aiProcess_GenSmoothNormals 
		| aiProcess_ImproveCacheLocality 
		| aiProcess_GenUVCoords 
		| aiProcess_FindInvalidData 
		| aiProcess_ValidateDataStructure 
		| aiProcess_PreTransformVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
		return 1;
	}

	// assert scene has at least one mesh
	if (!scene->mNumMeshes)
	{
		std::cout << "Model has no meshes!" << std::endl;
		return 1;
	}

	std::fstream dstMeshFile = std::fstream(dstFileName, std::ios::out | std::ios::binary | std::ios::trunc);
	std::fstream dstMatFile = std::fstream(dstFileName + "mat", std::ios::out | std::ios::trunc);

	// write magic number
	dstMeshFile.write((char *)&MAGIC_NUMBER, sizeof(std::uint32_t));

	// write mesh count
	dstMeshFile.write((char *)&scene->mNumMeshes, sizeof(std::uint32_t));

	// write material count
	dstMatFile << MATERIAL_COUNT_STRING + std::to_string(scene->mNumMeshes) + "\n";
	// write referenced mesh file
	dstMatFile << MESH_FILE_STRING + dstFileName + "\n\n";

	glm::vec3 minModelCorner = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxModelCorner = glm::vec3(std::numeric_limits<float>::lowest());

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		std::cout << "Processing mesh #" << i << std::endl;

		aiMesh *mesh = scene->mMeshes[i];

		if (!mesh->mTextureCoords[0])
		{
			std::cout << "Mesh has no TexCoords!" << std::endl;
			return 1;
		}

		// prepare vertex and index lists
		std::vector<Vertex> vertices;
		std::vector<std::uint32_t> indices;
		glm::vec3 minMeshCorner = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maxMeshCorner = glm::vec3(std::numeric_limits<float>::lowest());

		for (std::uint32_t j = 0; j < mesh->mNumVertices; ++j)
		{
			Vertex vertex;

			// position
			{
				glm::vec3 vec;
				vec.x = mesh->mVertices[j].x;
				vec.y = mesh->mVertices[j].y;
				vec.z = mesh->mVertices[j].z;
				vertex.position = vec;
				minMeshCorner = glm::min(minMeshCorner, vec);
				maxMeshCorner = glm::max(maxMeshCorner, vec);
				minModelCorner = glm::min(minModelCorner, vec);
				maxModelCorner = glm::max(maxModelCorner, vec);
			}

			// texCoord
			{
				if (mesh->mTextureCoords[0])
				{
					glm::vec2 vec;
					vec.x = mesh->mTextureCoords[0][j].x;
					vec.y = mesh->mTextureCoords[0][j].y;
					vertex.texCoord = vec;
				}
				else
				{
					vertex.texCoord = glm::vec2(0.0f, 0.0f);
				}
			}

			// normal
			{
				glm::vec3 vec;
				vec.x = mesh->mNormals[j].x;
				vec.y = mesh->mNormals[j].y;
				vec.z = mesh->mNormals[j].z;
				vertex.normal = glm::normalize(vec);
			}

			// tangent
			{
				glm::vec3 vec;
				vec.x = mesh->mTangents[j].x;
				vec.y = mesh->mTangents[j].y;
				vec.z = mesh->mTangents[j].z;
				vertex.tangent = glm::normalize(vec);
			}

			vertices.push_back(vertex);
		}

		// write AABB
		{
			dstMeshFile.write((char *)&minMeshCorner, sizeof(glm::vec3));
			dstMeshFile.write((char *)&maxMeshCorner, sizeof(glm::vec3));
		}

		// write vertex buffer
		{
			// write size
			std::uint32_t vertexBufferSize = sizeof(Vertex) * vertices.size();
			dstMeshFile.write((char *)&vertexBufferSize, sizeof(std::uint32_t));

			// write buffer
			dstMeshFile.write((char *)vertices.data(), vertexBufferSize);

			std::cout << "Vertices: " << vertices.size() << std::endl;
			std::cout << "Vertex Buffer Size: " << vertexBufferSize << std::endl;
		}

		for (std::uint32_t j = 0; j < mesh->mNumFaces; ++j)
		{
			aiFace &face = mesh->mFaces[j];

			for (std::uint32_t k = 0; k < face.mNumIndices; ++k)
			{
				indices.push_back(face.mIndices[k]);
			}
		}

		// write index buffer
		{
			// write size
			std::uint32_t indexBufferSize = sizeof(std::uint32_t) * indices.size();
			dstMeshFile.write((char *)&indexBufferSize, sizeof(std::uint32_t));

			// write buffer
			dstMeshFile.write((char *)indices.data(), indexBufferSize);

			std::cout << "Indices: " << indices.size() << std::endl;
			std::cout << "Index Buffer Size: " << indexBufferSize << std::endl;
		}

		// write material
		{
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

			std::string albedoPath = "";
			std::string normalPath = "";
			std::string emissivePath = "";

			if (material->GetTextureCount(aiTextureType_DIFFUSE))
			{
				aiString path;
				material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
				albedoPath = path.C_Str();
			}

			if (material->GetTextureCount(aiTextureType_NORMALS))
			{
				aiString path;
				material->GetTexture(aiTextureType_NORMALS, 0, &path);
				normalPath = path.C_Str();
			}

			if (material->GetTextureCount(aiTextureType_EMISSIVE))
			{
				aiString path;
				material->GetTexture(aiTextureType_EMISSIVE, 0, &path);
				emissivePath = path.C_Str();
			}

			dstMatFile << NAME_STRING + std::string(mesh->mName.C_Str()) + "\n";
			dstMatFile << ALBEDO_STRING + "1.0/1.0/1.0/1.0\n";
			dstMatFile << METALLIC_STRING + "0.0\n";
			dstMatFile << ROUGHNESS_STRING + "0.0\n";
			dstMatFile << EMISSIVE_STRING + "0.0/0.0/0.0\n";
			dstMatFile << ALBEDO_PATH_STRING + albedoPath + "\n";
			dstMatFile << NORMAL_PATH_STRING + normalPath + "\n";
			dstMatFile << METALLIC_PATH_STRING + "\n";
			dstMatFile << ROUGHNESS_PATH_STRING + "\n";
			dstMatFile << AO_PATH_STRING + "\n";
			dstMatFile << EMISSIVE_PATH_STRING + emissivePath + "\n\n";
		}
	}

	// write model AABB
	dstMeshFile.write((char *)&minModelCorner, sizeof(glm::vec3));
	dstMeshFile.write((char *)&maxModelCorner, sizeof(glm::vec3));

	dstMeshFile.close();
	dstMatFile.close();

	int a;
	std::cin >> a;

	return 0;
}