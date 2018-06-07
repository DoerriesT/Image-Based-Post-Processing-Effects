#include "Model.h"
#include <cassert>
#include <fstream>
#include "Mesh.h"
#include ".\..\Utilities\Utility.h"
#include <glm\vec4.hpp>
#include <glm\vec3.hpp>
#include "Texture.h"

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

Model::Model(const std::string &_filepath, bool _instantLoading)
{
	std::fstream matFile(_filepath, std::ios::in);

	std::string str;

	// material count
	std::getline(matFile, str);
	assert(!str.empty());
	unsigned int materialCount = std::stoi(str.substr(MATERIAL_COUNT_STRING.size()));

	// reserve space
	submeshMaterialPairs.reserve(materialCount);

	// mesh file
	std::getline(matFile, str);
	assert(!str.empty());
	std::string meshFilepath = str.substr(MESH_FILE_STRING.size());

	mesh = Mesh::createMesh("Resources/Models/" + meshFilepath, materialCount, _instantLoading);

	// load materials
	for (unsigned int i = 0; i < materialCount; ++i)
	{
		Material material;

		// submesh/material name
		std::getline(matFile, str);
		std::getline(matFile, str);
		assert(!str.empty());
		std::string name = str.substr(NAME_STRING.size());

		nameToIndexMap[name] = i;

		// albedo
		std::getline(matFile, str);
		assert(!str.empty());
		std::vector<std::string> albedoValues = Util::split(str.substr(ALBEDO_STRING.size()), "/");
		assert(albedoValues.size() == 4);
		glm::vec4 albedo = glm::vec4(std::stof(albedoValues[0]), std::stof(albedoValues[1]), std::stof(albedoValues[2]), std::stof(albedoValues[3]));
		material.setAlbedo(albedo);

		// metallic
		std::getline(matFile, str);
		assert(!str.empty());
		float metallic = std::stof(str.substr(METALLIC_STRING.size()));
		material.setMetallic(metallic);

		// roughness
		std::getline(matFile, str);
		assert(!str.empty());
		float roughness = std::stof(str.substr(ROUGHNESS_STRING.size()));
		material.setRoughness(roughness);

		// emissive
		std::getline(matFile, str);
		assert(!str.empty());
		std::vector<std::string> emissiveValues = Util::split(str.substr(EMISSIVE_STRING.size()), "/");
		assert(emissiveValues.size() == 3);
		glm::vec3 emissive = glm::vec3(std::stof(emissiveValues[0]), std::stof(emissiveValues[1]), std::stof(emissiveValues[2]));
		material.setEmissive(emissive);

		// albedo texture
		std::getline(matFile, str);
		assert(!str.empty());
		std::string albedoPath = str.substr(ALBEDO_PATH_STRING.size());
		Util::trim(albedoPath);
		if (!albedoPath.empty())
		{
			material.setAlbedoMap(Texture::createTexture(albedoPath, _instantLoading));
		}

		// normal texture
		std::getline(matFile, str);
		assert(!str.empty());
		std::string normalPath = str.substr(NORMAL_PATH_STRING.size());
		Util::trim(normalPath);
		if (!normalPath.empty())
		{
			material.setNormalMap(Texture::createTexture(normalPath, _instantLoading));
		}

		// metallic texture
		std::getline(matFile, str);
		assert(!str.empty());
		std::string metallicPath = str.substr(METALLIC_PATH_STRING.size());
		Util::trim(metallicPath);
		if (!metallicPath.empty())
		{
			material.setMetallicMap(Texture::createTexture(metallicPath, _instantLoading));
		}

		// roughness texture
		std::getline(matFile, str);
		assert(!str.empty());
		std::string roughnessPath = str.substr(ROUGHNESS_PATH_STRING.size());
		Util::trim(roughnessPath);
		if (!roughnessPath.empty())
		{
			material.setRoughnessMap(Texture::createTexture(roughnessPath, _instantLoading));
		}

		// ao texture
		std::getline(matFile, str);
		assert(!str.empty());
		std::string aoPath = str.substr(AO_PATH_STRING.size());
		Util::trim(aoPath);
		if (!aoPath.empty())
		{
			material.setAoMap(Texture::createTexture(aoPath, _instantLoading));
		}

		// emissive texture
		std::getline(matFile, str);
		assert(!str.empty());
		std::string emissivePath = str.substr(EMISSIVE_PATH_STRING.size());
		Util::trim(emissivePath);
		if (!emissivePath.empty())
		{
			material.setEmissiveMap(Texture::createTexture(emissivePath, _instantLoading));
		}

		submeshMaterialPairs.push_back({ mesh->getSubMesh(i), material });
		if (material.getAlbedo().a != 1.0 && !material.getAlbedoMap())
		{
			transparentSubmeshes.push_back(mesh->getSubMesh(i));
		}
	}
}

std::pair<std::shared_ptr<SubMesh>, Material> &Model::operator[](std::size_t _index)
{
	assert(_index < submeshMaterialPairs.size());
	return submeshMaterialPairs[_index];
}

std::pair<std::shared_ptr<SubMesh>, Material> &Model::operator[](const std::string &_name)
{
	assert(contains(nameToIndexMap, _name));
	assert(nameToIndexMap[_name] < submeshMaterialPairs.size());
	return submeshMaterialPairs[nameToIndexMap[_name]];
}

std::size_t Model::size() const
{
	return submeshMaterialPairs.size();
}

bool Model::isValid() const
{
	return mesh->isValid();
}

const std::vector<std::shared_ptr<SubMesh>>& Model::getTransparentSubmeshes()
{
	return transparentSubmeshes;
}
