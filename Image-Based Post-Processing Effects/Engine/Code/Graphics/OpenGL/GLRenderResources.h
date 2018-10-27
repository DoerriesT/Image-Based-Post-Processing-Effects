#pragma once
#include <glad/glad.h>
#include <memory>

class GLRenderer;

struct GLRenderResources
{
	friend GLRenderer;
public:
	GLRenderResources(const GLRenderResources &) = delete;
	GLRenderResources(const GLRenderResources &&) = delete;
	GLRenderResources &operator= (const GLRenderResources &) = delete;
	GLRenderResources &operator= (const GLRenderResources &&) = delete;
	~GLRenderResources();
	void resize(unsigned int width, unsigned int height);

private:

	// FBOs
	GLuint m_gBufferFbo;
	GLuint m_ssaoFbo;
	GLuint m_shadowFbo;
	GLuint m_ppFullResolutionFbo;
	GLuint m_ppHalfResolutionFbo;
	GLuint m_smaaFbo;
	GLuint m_velocityFbo;
	GLuint m_cocFbo;
	GLuint m_debugFbo;

	// textures
	GLuint m_brdfLUT;
	GLuint m_gAlbedoTexture;
	GLuint m_gNormalTexture;
	GLuint m_gMRASTexture; // metallic | roughness | ambient occlusion | shaded
	GLuint m_gLightColorTextures[2];
	GLuint m_gVelocityTexture;
	GLuint m_gDepthStencilTexture;
	GLuint m_ssaoTextureA;
	GLuint m_ssaoTextureB;
	GLuint m_ssaoTextureC;
	GLuint m_noiseTexture;
	GLuint m_noiseTexture2;
	GLuint m_luminanceHistogramIntermediary;
	GLuint m_luminanceHistogram;
	GLuint m_luminanceTempTexture;
	GLuint m_luminanceTexture[2];
	GLuint m_fullResolutionTextureA;
	GLuint m_fullResolutionTextureB;
	GLuint m_fullResolutionHdrTexture;
	GLuint m_fullResolutionCocTexture;
	GLuint m_fullResolutionDofTexA;
	GLuint m_fullResolutionDofTexB;
	GLuint m_fullResolutionDofTexC;
	GLuint m_fullResolutionDofTexD;
	GLuint m_fullResolutionSmaaEdgesTex;
	GLuint m_fullResolutionSmaaBlendTex;
	GLuint m_fullResolutionSmaaMLResultTex[2];
	GLuint m_fullResolutionSmaaResultTex;
	GLuint m_halfResolutionHdrTexA;
	GLuint m_halfResolutionHdrTexB;
	GLuint m_halfResolutionHdrTexC;
	GLuint m_halfResolutionCocTexA;
	GLuint m_halfResolutionCocTexB;
	GLuint m_halfResolutionDofTexA;
	GLuint m_halfResolutionDofTexB;
	GLuint m_halfResolutionDofTexC;
	GLuint m_halfResolutionDofTexD;
	GLuint m_halfResolutionDofDoubleTex;
	GLuint m_halfResolutionGodRayTexA;
	GLuint m_halfResolutionGodRayTexB;
	GLuint m_velocityTexTmp;
	GLuint m_velocityMaxTex;
	GLuint m_velocityNeighborMaxTex;
	GLuint m_cocTexTmp;
	GLuint m_cocMaxTex;
	GLuint m_cocNeighborMaxTex;
	GLuint m_anamorphicPrefilter;
	GLuint m_anamorphicChain[6];

	explicit GLRenderResources(unsigned int width, unsigned int height);
	void createResizableTextures(unsigned int width, unsigned int height);
	void createAllTextures(unsigned int width, unsigned int height);
	void deleteResizableTextures();
	void deleteAllTextures();
};