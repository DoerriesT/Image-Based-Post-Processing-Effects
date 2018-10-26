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
	GLuint m_rsmFbo;
	GLuint m_rsmDepth;
	GLuint m_rsmFlux;
	GLuint m_rsmNormal;

	GLuint m_propagationFbo;
	
	// 2d textures
	GLuint m_propagation2DAccumVolumeRed;
	GLuint m_propagation2DAccumVolumeGreen;
	GLuint m_propagation2DAccumVolumeBlue;
	GLuint m_geometry2DVolume;
	GLuint m_propagation2DVolumeRed0;
	GLuint m_propagation2DVolumeGreen0;
	GLuint m_propagation2DVolumeBlue0;
	GLuint m_propagation2DVolumeRed1;
	GLuint m_propagation2DVolumeGreen1;
	GLuint m_propagation2DVolumeBlue1;

	Volume m_propagationVolume;

	// renderpasses
	std::unique_ptr<RSMRenderPass> m_rsmRenderPass;
	std::unique_ptr<LightInjectionRenderPass> m_lightInjectionRenderPass;
	std::unique_ptr<GeometryInjectionRenderPass> m_geometryInjectionRenderPass;
	std::unique_ptr<LightPropagationRenderPass> m_lightPropagationRenderPass;
};