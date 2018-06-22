#pragma once
#include <glad\glad.h>
#include <memory>
#include "Uniform.h"
#include "MediumDescription.h"

class ShaderProgram;
class Mesh;
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

	std::shared_ptr<Mesh> fullscreenTriangle;

	std::shared_ptr<ShaderProgram> lightVolumeShader;
	std::shared_ptr<ShaderProgram> lightVolumeBaseShader;
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
	Uniform<glm::mat4> uInvViewProjectionLV = Uniform<glm::mat4>("uInvViewProjection");
	Uniform<GLint> uPhaseLUTLV = Uniform<GLint>("uPhaseLUT");
	Uniform<GLint> uDepthTextureLV = Uniform<GLint>("uDepthTexture");
	Uniform<GLint> uPassModeLV = Uniform<GLint>("uPassMode");
	Uniform<GLfloat> uZFarLV = Uniform<GLfloat>("uZFar");
	Uniform<glm::vec3> uCamPosLV = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec3> uLightIntensitysLV = Uniform<glm::vec3>("uLightIntensity");
	Uniform<glm::vec3> uSigmaExtinctionLV = Uniform<glm::vec3>("uSigmaExtinction");
	Uniform<glm::vec3> uScatterPowerLV = Uniform<glm::vec3>("uScatterPower");
	Uniform<glm::vec3> uLightDirLV = Uniform<glm::vec3>("uLightDir");

	// light volume base
	Uniform<GLint> uDisplacementTextureLVB = Uniform<GLint>("uDisplacementTexture");
	Uniform<glm::mat4> uInvLightViewProjectionLVB = Uniform<glm::mat4>("uInvLightViewProjection");
	Uniform<glm::mat4> uViewProjectionLVB = Uniform<glm::mat4>("uViewProjection");
	Uniform<glm::mat4> uInvViewProjectionLVB = Uniform<glm::mat4>("uInvViewProjection");
	Uniform<GLint> uPhaseLUTLVB = Uniform<GLint>("uPhaseLUT");
	Uniform<GLint> uDepthTextureLVB = Uniform<GLint>("uDepthTexture");
	Uniform<GLint> uPassModeLVB = Uniform<GLint>("uPassMode");
	Uniform<GLfloat> uZFarLVB = Uniform<GLfloat>("uZFar");
	Uniform<glm::vec3> uCamPosLVB = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec3> uLightIntensitysLVB = Uniform<glm::vec3>("uLightIntensity");
	Uniform<glm::vec3> uSigmaExtinctionLVB = Uniform<glm::vec3>("uSigmaExtinction");
	Uniform<glm::vec3> uScatterPowerLVB = Uniform<glm::vec3>("uScatterPower");
	Uniform<glm::vec3> uLightDirLVB = Uniform<glm::vec3>("uLightDir");

	// phase lookup
	Uniform<GLint> uNumPhaseTermsPL = Uniform<GLint>("uNumPhaseTerms");
	std::vector<GLint> uPhaseParamsPL;
	std::vector<GLint> uPhaseFuncPL;

	void computePhaseLUT();
	void createLightVolumeMesh();
	void createAttachments(unsigned int _width, unsigned int _height);
};