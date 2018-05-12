#include "Material.h"
#include <glad\glad.h>
#include "Texture.h"

const std::uint32_t Material::ALBEDO = 1 << 0;
const std::uint32_t Material::NORMAL = 1 << 1;
const std::uint32_t Material::METALLIC = 1 << 2;
const std::uint32_t Material::ROUGHNESS = 1 << 3;
const std::uint32_t Material::AO = 1 << 4;
const std::uint32_t Material::EMISSIVE = 1 << 5;

Material::Material(const glm::vec4 &_albedo, const float &_metallic, const float &_roughness, const glm::vec3 &_emissive)
	: albedo(_albedo), metallic(_metallic), roughness(_roughness), emissive(_emissive)
{
}

Material::Material(const std::shared_ptr<Texture> &_albedoMap, const std::shared_ptr<Texture> &_normalMap, const std::shared_ptr<Texture> &_metallicMap, const std::shared_ptr<Texture> &_roughnessMap, const std::shared_ptr<Texture> &_aoMap, const std::shared_ptr<Texture> &_emissiveMap)
	: albedo(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), metallic(0.0f), roughness(0.0f), albedoMap(_albedoMap), normalMap(_normalMap), metallicMap(_metallicMap), roughnessMap(_roughnessMap), aoMap(_aoMap), emissiveMap(_emissiveMap)
{
	if (albedoMap)
	{
		mapBitField |= ALBEDO;
	}
	if (normalMap)
	{
		mapBitField |= NORMAL;
	}
	if (metallicMap)
	{
		mapBitField |= METALLIC;
	}
	if (roughnessMap)
	{
		mapBitField |= ROUGHNESS;
	}
	if (aoMap)
	{
		mapBitField |= AO;
	}
	if (emissiveMap)
	{
		mapBitField |= EMISSIVE;
	}
}

Material::Material(const std::string &_basePath, const std::uint32_t &_flags)
	: albedo(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), metallic(0.0f), roughness(0.0f)
{
	std::string albedoPath = _basePath + "_a.dds";
	std::string normalPath = _basePath + "_n.dds";
	std::string metallicPath = _basePath + "_m.dds";
	std::string roughnessPath = _basePath + "_r.dds";
	std::string aoPath = _basePath + "_ao.dds";
	std::string emissivePath = _basePath + "_e.dds";

	if ((_flags & ALBEDO) == ALBEDO)
	{
		albedoMap = Texture::createTexture(albedoPath.c_str());
		mapBitField |= ALBEDO;
	}
	if ((_flags & NORMAL) == NORMAL)
	{
		normalMap = Texture::createTexture(normalPath.c_str());
		mapBitField |= NORMAL;
	}
	if ((_flags & METALLIC) == METALLIC)
	{
		metallicMap = Texture::createTexture(metallicPath.c_str());
		mapBitField |= METALLIC;
	}
	if ((_flags & ROUGHNESS) == ROUGHNESS)
	{
		roughnessMap = Texture::createTexture(roughnessPath.c_str());
		mapBitField |= ROUGHNESS;
	}
	if ((_flags & AO) == AO)
	{
		aoMap = Texture::createTexture(aoPath.c_str());
		mapBitField |= AO;
	}
	if ((_flags & EMISSIVE) == EMISSIVE)
	{
		emissiveMap = Texture::createTexture(emissivePath.c_str());
		mapBitField |= EMISSIVE;
	}
}

const std::shared_ptr<Texture> Material::getAlbedoMap() const
{
	return albedoMap;
}

const std::shared_ptr<Texture> Material::getNormalMap() const
{
	return normalMap;
}

const std::shared_ptr<Texture> Material::getMetallicMap() const
{
	return metallicMap;
}

const std::shared_ptr<Texture> Material::getRoughnessMap() const
{
	return roughnessMap;
}

const std::shared_ptr<Texture> Material::getAoMap() const
{
	return aoMap;
}

const std::shared_ptr<Texture> Material::getEmissiveMap() const
{
	return emissiveMap;
}

const std::uint32_t Material::getMapBitField() const
{
	std::uint32_t field = 0;
	if (albedoMap && albedoMap->isValid())
	{
		field |= ALBEDO;
	}
	if (normalMap && normalMap->isValid())
	{
		field |= NORMAL;
	}
	if (metallicMap && metallicMap->isValid())
	{
		field |= METALLIC;
	}
	if (roughnessMap && roughnessMap->isValid())
	{
		field |= ROUGHNESS;
	}
	if (aoMap && aoMap->isValid())
	{
		field |= AO;
	}
	if (emissiveMap && emissiveMap->isValid())
	{
		field |= EMISSIVE;
	}
	return field;
}

const glm::vec4 &Material::getAlbedo() const
{
	return albedo;
}

const float &Material::getMetallic() const
{
	return metallic;
}

const float &Material::getRoughness() const
{
	return roughness;
}

const glm::vec3 &Material::getEmissive() const
{
	return emissive;
}

void Material::setAlbedoMap(const std::shared_ptr<Texture> &_albedoMap)
{
	albedoMap = _albedoMap;
	if (albedoMap)
	{
		mapBitField |= ALBEDO;
	}
	else
	{
		mapBitField ^= ALBEDO;
	}
}

void Material::setNormalMap(const std::shared_ptr<Texture> &_normalMap)
{
	normalMap = _normalMap;
	if (normalMap)
	{
		mapBitField |= NORMAL;
	}
	else
	{
		mapBitField ^= NORMAL;
	}
}

void Material::setMetallicMap(const std::shared_ptr<Texture> &_metallicMap)
{
	metallicMap = _metallicMap;
	if (metallicMap)
	{
		mapBitField |= METALLIC;
	}
	else
	{
		mapBitField ^= METALLIC;
	}
}

void Material::setRoughnessMap(const std::shared_ptr<Texture> &_roughnessMap)
{
	roughnessMap = _roughnessMap;
	if (roughnessMap)
	{
		mapBitField |= ROUGHNESS;
	}
	else
	{
		mapBitField ^= ROUGHNESS;
	}
}

void Material::setAoMap(const std::shared_ptr<Texture> &_aoMap)
{
	aoMap = _aoMap;
	if (aoMap)
	{
		mapBitField |= AO;
	}
	else
	{
		mapBitField ^= AO;
	}
}

void Material::setEmissiveMap(const std::shared_ptr<Texture> &_emissiveMap)
{
	emissiveMap = _emissiveMap;
	if (emissiveMap)
	{
		mapBitField |= EMISSIVE;
	}
	else
	{
		mapBitField ^= EMISSIVE;
	}
}

void Material::setAlbedo(const glm::vec4 & _albedo)
{
	albedo = _albedo;
}

void Material::setMetallic(const float & _metallic)
{
	metallic = _metallic;
}

void Material::setRoughness(const float & _roughness)
{
	roughness = _roughness;
}

void Material::setEmissive(const glm::vec3 &_emissive)
{
	emissive = _emissive;
}

void Material::bindTextures() const
{
	if (albedoMap && albedoMap->isValid())
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(albedoMap->getTarget(), albedoMap->getId());
	}
	if (normalMap && normalMap->isValid())
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(normalMap->getTarget(), normalMap->getId());
	}
	if (metallicMap && metallicMap->isValid())
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(metallicMap->getTarget(), metallicMap->getId());
	}
	if (roughnessMap && roughnessMap->isValid())
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(roughnessMap->getTarget(), roughnessMap->getId());
	}
	if (aoMap && aoMap->isValid())
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(aoMap->getTarget(), aoMap->getId());
	}
	if (emissiveMap && emissiveMap->isValid())
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(emissiveMap->getTarget(), emissiveMap->getId());
	}
}
