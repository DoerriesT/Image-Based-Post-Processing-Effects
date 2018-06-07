#pragma once
#include <memory>
#include <string>
#include <map>
#include <vector>
#include "Material.h"

class Mesh;
class SubMesh;

class Model
{
public:
	explicit Model(const std::string &_filepath, bool instantLoading = false);
	std::pair<std::shared_ptr<SubMesh>, Material> &operator[](std::size_t _index);
	std::pair<std::shared_ptr<SubMesh>, Material> &operator[](const std::string &_name);
	std::size_t size() const;
	bool isValid() const;
	const std::vector<std::shared_ptr<SubMesh>> &getTransparentSubmeshes();

private:
	std::shared_ptr<Mesh> mesh;
	std::vector<std::pair<std::shared_ptr<SubMesh>, Material>> submeshMaterialPairs;
	std::vector<std::shared_ptr<SubMesh>> transparentSubmeshes;
	std::map<std::string, std::size_t> nameToIndexMap;
};