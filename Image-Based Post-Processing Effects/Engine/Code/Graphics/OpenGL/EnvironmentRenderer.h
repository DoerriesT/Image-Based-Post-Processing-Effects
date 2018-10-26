#pragma once
#include <glad\glad.h>
#include "Uniform.h"
#include "Graphics\EnvironmentProbe.h"

struct AtmosphereParams;
class ShaderProgram;
class EnvironmentProbe;
class IrradianceVolume;
class Mesh;
class Texture;
struct Level;

const unsigned int ATMOSPHERE_MAP_SIZE = 1024;
const unsigned int ENVIRONMENT_MAP_SIZE = 1024;
const unsigned int BRDF_LUT_SIZE = 512;

class EnvironmentRenderer
{
public:
	explicit EnvironmentRenderer();
	EnvironmentRenderer(const EnvironmentRenderer &) = delete;
	EnvironmentRenderer(const EnvironmentRenderer &&) = delete;
	EnvironmentRenderer &operator= (const EnvironmentRenderer &) = delete;
	EnvironmentRenderer &operator= (const EnvironmentRenderer &&) = delete;
	~EnvironmentRenderer();
	void init();
	void updateCubeSide(unsigned int _side, GLuint _source);
	void generateMipmaps();
	void calculateReflectance(const std::shared_ptr<EnvironmentProbe> &_environmentProbe);
	void calculateIrradiance(const std::shared_ptr<EnvironmentProbe> &_environmentProbe);
	void calculateIrradiance(const std::shared_ptr<IrradianceVolume> &_irradianceVolume, const glm::ivec3 &_index);
	std::shared_ptr<Texture> calculateAtmosphere(const AtmosphereParams &_atmosphereParams);
	GLuint getEnvironmentMap() const;

private:
	// shaders
	std::shared_ptr<ShaderProgram> m_atmosphereShader;
	std::shared_ptr<ShaderProgram> m_blitShader;
	std::shared_ptr<ShaderProgram> m_reflectanceOctShader;
	std::shared_ptr<ShaderProgram> m_irradianceOctShader;

	std::unique_ptr<float[]> m_cubeFaceBuffer;

	// fullscreenTriangle
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	GLuint m_environmentFbo;
	GLuint m_convolutionFbo;

	GLuint m_environmentMap;

	// blit
	GLint m_uScreenTextureBlit;

	// irradiance
	Uniform<glm::vec2> m_uImageSizeIO = Uniform<glm::vec2>("uImageSize");

	// reflectance
	Uniform<GLint> m_uEnvironmentResolution = Uniform<GLint>("uEnvironmentResolution");
	Uniform<GLfloat> m_uRoughnessRO = Uniform<GLfloat>("uRoughness");
	Uniform<glm::vec2> m_uImageSizeRO = Uniform<glm::vec2>("uImageSize");

	// atmosphere
	Uniform<glm::mat3> m_uRotationA = Uniform<glm::mat3>("uRotation");
	Uniform<glm::mat4> m_uInverseProjectionA = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec3> m_uLightDirA = Uniform<glm::vec3>("uLightDir");
	Uniform<GLfloat> m_uRayleighBrightnessA = Uniform<GLfloat>("uRayleighBrightness");
	Uniform<GLfloat> m_uMieBrightnessA = Uniform<GLfloat>("uMieBrightness");
	Uniform<GLfloat> m_uMieDistributionA = Uniform<GLfloat>("uMieDistribution");
	Uniform<GLfloat> m_uSpotBrightnessA = Uniform<GLfloat>("uSpotBrightness");
	Uniform<GLfloat> m_uSurfaceHeightA = Uniform<GLfloat>("uSurfaceHeight");
	Uniform<GLint> m_uStepCountA = Uniform<GLint>("uStepCount");
	Uniform<glm::vec3> m_uIntensityA = Uniform<glm::vec3>("uIntensity");
	Uniform<GLfloat> m_uScatterStrengthA = Uniform<GLfloat>("uScatterStrength");
	Uniform<GLfloat> m_uRayleighStrengthA = Uniform<GLfloat>("uRayleighStrength");
	Uniform<GLfloat> m_uMieStrengthA = Uniform<GLfloat>("uMieStrength");
	Uniform<GLfloat> m_uRayleighCollectionPowerA = Uniform<GLfloat>("uRayleighCollectionPower");
	Uniform<GLfloat> m_uMieCollectionPowerA = Uniform<GLfloat>("uMieCollectionPower");

	// orientations
	glm::mat3 m_rotations[6];

};

