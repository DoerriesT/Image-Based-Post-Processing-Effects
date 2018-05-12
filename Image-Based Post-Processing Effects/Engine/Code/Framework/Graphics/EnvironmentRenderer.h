#pragma once
#include <glad\glad.h>
#include "Uniform.h"

struct AtmosphereParams;
class ShaderProgram;
class EnvironmentProbe;
class Mesh;
class Texture;
struct Level;

const unsigned int ATMOSPHERE_MAP_SIZE = 1024;
const unsigned int ENVIRONMENT_MAP_SIZE = 512;

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
	void updateCubeSide(const unsigned int &_side, const GLuint &_source);
	void calculateReflectance(const std::shared_ptr<EnvironmentProbe> &_environmentProbe);
	void calculateIrradiance(const std::shared_ptr<EnvironmentProbe> &_environmentProbe);
	std::shared_ptr<Texture> calculateAtmosphere(const AtmosphereParams &_atmosphereParams);
	GLuint getEnvironmentMap() const;

private:
	// shaders
	std::shared_ptr<ShaderProgram> atmosphereShader;
	std::shared_ptr<ShaderProgram> reflectanceShader;
	std::shared_ptr<ShaderProgram> irradianceShader;
	std::shared_ptr<ShaderProgram> blitShader;

	// fullscreenTriangle
	std::shared_ptr<Mesh> fullscreenTriangle;

	GLuint environmentFbo;
	GLuint convolutionFbo;

	GLuint environmentMap;

	// blit
	GLint uScreenTextureBlit;

	// irradiance
	Uniform<GLint> uEnvironmentMapI = Uniform<GLint>("uEnvironmentMap");
	Uniform<glm::mat3> uRotationI = Uniform<glm::mat3>("uRotation"); 
	Uniform<glm::mat4> uInverseProjectionI = Uniform<glm::mat4>("uInverseProjection");

	// reflectance
	Uniform<GLint> uEnvironmentMapR = Uniform<GLint>("uEnvironmentMap");
	Uniform<GLint> uEnvironmentResolutionR = Uniform<GLint>("uEnvironmentResolution");
	Uniform<GLfloat> uRoughness = Uniform<GLfloat>("uRoughness");
	Uniform<glm::mat3> uRotationR = Uniform<glm::mat3>("uRotation");
	Uniform<glm::mat4> uInverseProjectionR = Uniform<glm::mat4>("uInverseProjection");

	// atmosphere
	Uniform<glm::mat3> uRotationA = Uniform<glm::mat3>("uRotation");
	Uniform<glm::mat4> uInverseProjectionA = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec3> uLightDirA = Uniform<glm::vec3>("uLightDir");
	Uniform<GLfloat> uRayleighBrightnessA = Uniform<GLfloat>("uRayleighBrightness");
	Uniform<GLfloat> uMieBrightnessA = Uniform<GLfloat>("uMieBrightness");
	Uniform<GLfloat> uMieDistributionA = Uniform<GLfloat>("uMieDistribution");
	Uniform<GLfloat> uSpotBrightnessA = Uniform<GLfloat>("uSpotBrightness");
	Uniform<GLfloat> uSurfaceHeightA = Uniform<GLfloat>("uSurfaceHeight");
	Uniform<GLint> uStepCountA = Uniform<GLint>("uStepCount");
	Uniform<glm::vec3> uIntensityA = Uniform<glm::vec3>("uIntensity");
	Uniform<GLfloat> uScatterStrengthA = Uniform<GLfloat>("uScatterStrength");
	Uniform<GLfloat> uRayleighStrengthA = Uniform<GLfloat>("uRayleighStrength");
	Uniform<GLfloat> uMieStrengthA = Uniform<GLfloat>("uMieStrength");
	Uniform<GLfloat> uRayleighCollectionPowerA = Uniform<GLfloat>("uRayleighCollectionPower");
	Uniform<GLfloat> uMieCollectionPowerA = Uniform<GLfloat>("uMieCollectionPower");

	// orientations
	glm::mat3 rotations[6];

};

