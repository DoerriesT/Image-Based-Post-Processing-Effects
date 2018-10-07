#pragma once
#include <glad/glad.h>
#include <memory>
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\RSMRenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\LightInjectionRenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\GeometryInjectionRenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\LightPropagationRenderPass.h"
#include "Graphics\Volume.h"

class RSMRenderPass;
class LightInjectionRenderPass;
class LightPropagationRenderPass;
class RenderPass;
struct RenderData;
struct Level;
class Scene;

class LightPropagationVolumes
{
public:
	void init();
	void render(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);
	GLuint getRedVolume() const;
	GLuint getGreenVolume() const;
	GLuint getBlueVolume() const;
	GLuint get2DVolume() const;
	Volume getVolume() const;

private:
	GLuint rsmFbo;
	GLuint rsmDepth;
	GLuint rsmFlux;
	GLuint rsmNormal;

	GLuint propagationFbo;
	
	// 2d textures
	GLuint propagation2DAccumVolumeRed;
	GLuint propagation2DAccumVolumeGreen;
	GLuint propagation2DAccumVolumeBlue;
	GLuint geometry2DVolume;
	GLuint propagation2DVolumeRed0;
	GLuint propagation2DVolumeGreen0;
	GLuint propagation2DVolumeBlue0;
	GLuint propagation2DVolumeRed1;
	GLuint propagation2DVolumeGreen1;
	GLuint propagation2DVolumeBlue1;

	Volume propagationVolume;

	// renderpasses
	std::unique_ptr<RSMRenderPass> rsmRenderPass;
	std::unique_ptr<LightInjectionRenderPass> lightInjectionRenderPass;
	std::unique_ptr<GeometryInjectionRenderPass> geometryInjectionRenderPass;
	std::unique_ptr<LightPropagationRenderPass> lightPropagationRenderPass;
};