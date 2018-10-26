#include "Material.h"
#include <glad\glad.h>
#include "Texture.h"

const std::uint32_t Material::ALBEDO = 1 << 0;
const std::uint32_t Material::NORMAL = 1 << 1;
const std::uint32_t Material::METALLIC = 1 << 2;
const std::uint32_t Material::ROUGHNESS = 1 << 3;
const std::uint32_t Material::AO = 1 << 4;
const std::uint32_t Material::EMISSIVE = 1 << 5;

Material::Material(const glm::vec4 &_albedo, float _metallic, float _roughness, const glm::vec3 &_emissive)
	: m_albedo(_albedo), m_metallic(_metallic), m_roughness(_roughness), m_emissive(_emissive)
{
}

Material::Material(const std::shared_ptr<Texture> &_albedoMap, 
	const std::shared_ptr<Texture> &_normalMap, 
	const std::shared_ptr<Texture> &_metallicMap, 
	const std::shared_ptr<Texture> &_roughnessMap, 
	const std::shared_ptr<Texture> &_aoMap, 
	const std::shared_ptr<Texture> &_emissiveMap,
	const std::shared_ptr<Texture> &_displacementMap)
	: m_albedo(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), 
	m_metallic(0.0f), 
	m_roughness(0.0f), 
	m_albedoMap(_albedoMap), 
	m_normalMap(_normalMap), 
	m_metallicMap(_metallicMap), 
	m_roughnessMap(_roughnessMap), 
	m_aoMap(_aoMap), 
	m_emissiveMap(_emissiveMap),
	m_displacementMap(_displacementMap)
{
}

Material::Material(const std::string &_basePath, std::uint32_t _flags)
	: m_albedo(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), m_metallic(0.0f), m_roughness(0.0f)
{
	std::string albedoPath = _basePath + "_a.dds";
	std::string normalPath = _basePath + "_n.dds";
	std::string metallicPath = _basePath + "_m.dds";
	std::string roughnessPath = _basePath + "_r.dds";
	std::string aoPath = _basePath + "_ao.dds";
	std::string emissivePath = _basePath + "_e.dds";
	//std::string displacementPath = _basePath + "_d.dds";

	if ((_flags & ALBEDO) == ALBEDO)
	{
		m_albedoMap = Texture::createTexture(albedoPath.c_str());
	}
	if ((_flags & NORMAL) == NORMAL)
	{
		m_normalMap = Texture::createTexture(normalPath.c_str());
	}
	if ((_flags & METALLIC) == METALLIC)
	{
		m_metallicMap = Texture::createTexture(metallicPath.c_str());
	}
	if ((_flags & ROUGHNESS) == ROUGHNESS)
	{
		m_roughnessMap = Texture::createTexture(roughnessPath.c_str());
	}
	if ((_flags & AO) == AO)
	{
		m_aoMap = Texture::createTexture(aoPath.c_str());
	}
	if ((_flags & EMISSIVE) == EMISSIVE)
	{
		m_emissiveMap = Texture::createTexture(emissivePath.c_str());
	}
}

const std::shared_ptr<Texture> Material::getAlbedoMap() const
{
	return m_albedoMap;
}

const std::shared_ptr<Texture> Material::getNormalMap() const
{
	return m_normalMap;
}

const std::shared_ptr<Texture> Material::getMetallicMap() const
{
	return m_metallicMap;
}

const std::shared_ptr<Texture> Material::getRoughnessMap() const
{
	return m_roughnessMap;
}

const std::shared_ptr<Texture> Material::getAoMap() const
{
	return m_aoMap;
}

const std::shared_ptr<Texture> Material::getEmissiveMap() const
{
	return m_emissiveMap;
}

const std::shared_ptr<Texture> Material::getDisplacementMap() const
{
	return m_displacementMap;
}

const std::uint32_t Material::getMapBitField() const
{
	std::uint32_t field = 0;
	if (m_albedoMap && m_albedoMap->isValid())
	{
		field |= ALBEDO;
	}
	if (m_normalMap && m_normalMap->isValid())
	{
		field |= NORMAL;
	}
	if (m_metallicMap && m_metallicMap->isValid())
	{
		field |= METALLIC;
	}
	if (m_roughnessMap && m_roughnessMap->isValid())
	{
		field |= ROUGHNESS;
	}
	if (m_aoMap && m_aoMap->isValid())
	{
		field |= AO;
	}
	if (m_emissiveMap && m_emissiveMap->isValid())
	{
		field |= EMISSIVE;
	}
	return field;
}

const glm::vec4 &Material::getAlbedo() const
{
	return m_albedo;
}

float Material::getMetallic() const
{
	return m_metallic;
}

float Material::getRoughness() const
{
	return m_roughness;
}

const glm::vec3 &Material::getEmissive() const
{
	return m_emissive;
}

void Material::setAlbedoMap(const std::shared_ptr<Texture> &_albedoMap)
{
	m_albedoMap = _albedoMap;
}

void Material::setNormalMap(const std::shared_ptr<Texture> &_normalMap)
{
	m_normalMap = _normalMap;
}

void Material::setMetallicMap(const std::shared_ptr<Texture> &_metallicMap)
{
	m_metallicMap = _metallicMap;
}

void Material::setRoughnessMap(const std::shared_ptr<Texture> &_roughnessMap)
{
	m_roughnessMap = _roughnessMap;
}

void Material::setAoMap(const std::shared_ptr<Texture> &_aoMap)
{
	m_aoMap = _aoMap;
}

void Material::setEmissiveMap(const std::shared_ptr<Texture> &_emissiveMap)
{
	m_emissiveMap = _emissiveMap;
}

void Material::setDisplacementMap(const std::shared_ptr<Texture> &_displacementMap)
{
	m_displacementMap = _displacementMap;
}

void Material::setAlbedo(const glm::vec4 & _albedo)
{
	m_albedo = _albedo;
}

void Material::setMetallic(float _metallic)
{
	m_metallic = _metallic;
}

void Material::setRoughness(float _roughness)
{
	m_roughness = _roughness;
}

void Material::setEmissive(const glm::vec3 &_emissive)
{
	m_emissive = _emissive;
}

void Material::bindTextures() const
{
	if (m_albedoMap && m_albedoMap->isValid())
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(m_albedoMap->getTarget(), m_albedoMap->getId());
	}
	if (m_normalMap && m_normalMap->isValid())
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(m_normalMap->getTarget(), m_normalMap->getId());
	}
	if (m_metallicMap && m_metallicMap->isValid())
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(m_metallicMap->getTarget(), m_metallicMap->getId());
	}
	if (m_roughnessMap && m_roughnessMap->isValid())
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(m_roughnessMap->getTarget(), m_roughnessMap->getId());
	}
	if (m_aoMap && m_aoMap->isValid())
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(m_aoMap->getTarget(), m_aoMap->getId());
	}
	if (m_emissiveMap && m_emissiveMap->isValid())
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(m_emissiveMap->getTarget(), m_emissiveMap->getId());
	}
	if (m_displacementMap && m_displacementMap->isValid())
	{
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(m_displacementMap->getTarget(), m_displacementMap->getId());
	}
}
