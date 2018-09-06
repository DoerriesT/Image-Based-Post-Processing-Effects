#pragma once
#include <glad/glad.h>
#include <memory>

class RSMRenderPass;
class LightInjectionRenderPass;
class LightPropagationRenderPass;
class RenderPass;
struct RenderData;
struct Level;

class LightPropagationVolumes
{
public:
	void init();
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);

private:
	GLuint rsmFbo;
	GLuint rsmDepth;
	GLuint rsmFlux;
	GLuint rsmNormal;

	// actual 3d textures
	GLuint propagationVolumeRed;
	GLuint propagationVolumeGreen;
	GLuint propagationVolumeBlue;

	GLuint propagationFbo;
	
	// 2d textures
	GLuint propagation2DVolumeRed0;
	GLuint propagation2DVolumeGreen0;
	GLuint propagation2DVolumeBlue0;
	GLuint propagation2DVolumeRed1;
	GLuint propagation2DVolumeGreen1;
	GLuint propagation2DVolumeBlue1;

	// renderpasses
	std::unique_ptr<RSMRenderPass> rsmRenderPass;
	std::unique_ptr<LightInjectionRenderPass> lightInjectionRenderPass;
	std::unique_ptr<LightPropagationRenderPass> lightPropagationRenderPass;
};