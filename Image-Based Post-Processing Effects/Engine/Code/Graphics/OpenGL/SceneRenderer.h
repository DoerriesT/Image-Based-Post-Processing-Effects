#pragma once
#include <glad\glad.h>
#include "Uniform.h"
#include "Ocean.h"
#include "VolumetricLighting.h"
#include "GBuffer.h"

class Camera;
class Window;
class Scene;
class Mesh;
struct Effects;
struct Level;
class ShaderProgram;
struct RenderData;
struct AxisAlignedBoundingBox;
struct Water;
class ShadowRenderPass;
class GBufferRenderPass;
class GBufferCustomRenderPass;
class SSAOOriginalRenderPass;
class SSAORenderPass;
class HBAORenderPass;
class GTAORenderPass;
class GTAODenoiseRenderPass;
class SSAOBlurRenderPass;
class SSAOBilateralBlurRenderPass;
class SkyboxRenderPass;
class AmbientLightRenderPass;
class DirectionalLightRenderPass;
class StencilRenderPass;
class PointLightRenderPass;
class SpotLightRenderPass;
class ForwardRenderPass;
class ForwardCustomRenderPass;
class OutlineRenderPass;
class LightProbeRenderPass;

class SceneRenderer
{
public:
	explicit SceneRenderer(std::shared_ptr<Window> _window);
	SceneRenderer(const SceneRenderer &) = delete;
	SceneRenderer(const SceneRenderer &&) = delete;
	SceneRenderer &operator= (const SceneRenderer &) = delete;
	SceneRenderer &operator= (const SceneRenderer &&) = delete;
	~SceneRenderer();
	void init();
	void render(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects);
	void resize(const std::pair<unsigned int, unsigned int> &_resolution);
	GLuint getColorTexture() const;
	GLuint getAlbedoTexture() const;
	GLuint getNormalTexture() const;
	GLuint getDepthStencilTexture() const;
	GLuint getVelocityTexture() const;
	GLuint getAmbientOcclusionTexture() const;
	GLuint getBrdfLUT() const;

private:
	std::shared_ptr<Window> window;

	unsigned int frame;

	Ocean ocean;
	VolumetricLighting volumetricLighting;

	GLuint brdfLUT;

	// g-buffer
	GLuint gBufferFBO;
	GLuint gAlbedoRMSTexture; // albedo | roughness | metallic | shading model
	GLuint gNormalAoTexture; // normal | ao
	GLuint gLightColorTextures[2];
	GLuint gVelocityTexture;
	GLuint gDepthStencilTexture;

	GBuffer gbuffer;

	GLenum lightColorAttachments[2];

	ShadowRenderPass *shadowRenderPass;
	GBufferRenderPass *gBufferRenderPass;
	GBufferCustomRenderPass *gBufferCustomRenderPass;
	SSAOOriginalRenderPass *ssaoOriginalRenderPass;
	SSAORenderPass *ssaoRenderPass;
	HBAORenderPass *hbaoRenderPass;
	GTAORenderPass *gtaoRenderPass;
	GTAODenoiseRenderPass *gtaoDenoiseRenderPass;
	SSAOBlurRenderPass *ssaoBlurRenderPass;
	SSAOBilateralBlurRenderPass *ssaoBilateralBlurRenderPass;
	SkyboxRenderPass *skyboxRenderPass;
	AmbientLightRenderPass *ambientLightRenderPass;
	DirectionalLightRenderPass *directionalLightRenderPass;
	StencilRenderPass *stencilRenderPass;
	PointLightRenderPass *pointLightRenderPass;
	SpotLightRenderPass *spotLightRenderPass;
	ForwardRenderPass *forwardRenderPass;
	ForwardCustomRenderPass *forwardCustomRenderPass;
	OutlineRenderPass *outlineRenderPass;
	LightProbeRenderPass *lightProbeRenderPass;


	// ssao fbo
	GLuint ssaoFbo;
	GLuint ssaoTextureA;
	GLuint ssaoTextureB;
	GLuint ssaoTextureC;
	GLuint noiseTexture;
	GLuint noiseTexture2;

	GLuint shadowFbo;

	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
	void createSsaoAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
	void createBrdfLUT();
};

