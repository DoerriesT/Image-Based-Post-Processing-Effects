#pragma once
#include <glad\glad.h>
#include <memory>
#include "Uniform.h"
#include "MediumDescription.h"

class ShaderProgram;
struct Level;
struct RenderData;

class VolumetricLighting
{
public:
	explicit VolumetricLighting(unsigned int _width, unsigned int _height);
	VolumetricLighting(const VolumetricLighting &) = delete;
	VolumetricLighting(const VolumetricLighting &&) = delete;
	VolumetricLighting &operator= (const VolumetricLighting &) = delete;
	VolumetricLighting &operator= (const VolumetricLighting &&) = delete;
	~VolumetricLighting();
	void init();
	void render(GLuint _depthTexture, const RenderData &_renderData, const std::shared_ptr<Level> &_level);
	void resize(unsigned int _width, unsigned int _height);
	GLuint getLightVolumeTexture() const;

private:
	bool meshCreated;
	unsigned int width;
	unsigned int height;

	MediumDescription currentMediumDescription;

	std::shared_ptr<ShaderProgram> lightVolumeShader;
	std::shared_ptr<ShaderProgram> phaseLUTShader;

	GLuint phaseLUT;
	GLuint lightVolumeFbo;
	GLuint lightVolumeTexture;
	GLuint depthStencilTexture;

	// light volume mesh
	GLuint lightVolumeVAO;
	GLuint lightVolumeVBO;
	GLuint lightVolumeEBO;

	// light volume
	Uniform<GLint> uDisplacementTextureLV = Uniform<GLint>("uDisplacementTexture");
	Uniform<glm::mat4> uInvLightViewProjectionLV = Uniform<glm::mat4>("uInvLightViewProjection");
	Uniform<glm::mat4> uViewProjectionLV = Uniform<glm::mat4>("uViewProjection");
	Uniform<GLint> uPhaseLUTLV = Uniform<GLint>("uPhaseLUT");
	Uniform<glm::vec3> uCamPosLV = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec3> uLightIntensitysLV = Uniform<glm::vec3>("uLightIntensity");
	Uniform<glm::vec3> uSigmaExtinctionLV = Uniform<glm::vec3>("uSigmaExtinction");
	Uniform<glm::vec3> uScatterPowerLV = Uniform<glm::vec3>("uScatterPower");
	Uniform<glm::vec3> uLightDirLV = Uniform<glm::vec3>("uLightDir");

	// phase lookup
	Uniform<GLint> uNumPhaseTermsPL = Uniform<GLint>("uNumPhaseTerms");
	std::vector<GLint> uPhaseParamsPL;
	std::vector<GLint> uPhaseFuncPL;

	void computePhaseLUT();
	void createLightVolumeMesh();
	void createAttachments(unsigned int _width, unsigned int _height);
};