#include "ShadowRenderer.h"
#include "Utilities\Utility.h"
#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include <cmath>
#define GLFW_INCLUDE_NONE
#include <GLFW\glfw3.h>
#include ".\..\..\Graphics\Mesh.h"
#include ".\..\..\EntityComponentSystem\Component.h"
#include "ShaderProgram.h"
#include ".\..\..\Graphics\Camera.h"
#include ".\..\..\Graphics\Scene.h"
#include "RenderData.h"
#include ".\..\..\Level.h"
#include ".\..\..\Graphics\Effects.h"
#include ".\..\..\Graphics\EntityRenderData.h"

ShadowRenderer::ShadowRenderer()
{
}

ShadowRenderer::~ShadowRenderer()
{	
	deleteAttachments();
	deleteFbos();
}

void ShadowRenderer::init()
{
	shadowShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shadows/shadow.vert", "Resources/Shaders/Shadows/shadow.frag");
	shadowBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Shadows/shadowBlur.frag");
	blitShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Shared/blit.frag");

	// create uniforms
	uModelViewProjectionMatrix = shadowShader->createUniform("uModelViewProjectionMatrix");

	uShadowMap = shadowBlurShader->createUniform("uShadowMap");
	uDirection = shadowBlurShader->createUniform("uDirection");
	uWidth = shadowBlurShader->createUniform("uWidth");
	uHeight = shadowBlurShader->createUniform("uHeight");

	uScreenTextureB = blitShader->createUniform("uScreenTexture");

	// create FBO
	glGenFramebuffers(1, &shadowFbo);
	createFboAttachments(std::make_pair(1024u, 1024u));

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.obj", true);
}

void ShadowRenderer::renderShadows(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects)
{
	if (_effects.shadowQuality == ShadowQuality::OFF)
	{
		return;
	}

	glm::mat4 cameraProjection = glm::perspective(glm::radians(_renderData.fov), _renderData.resolution.first / (float)_renderData.resolution.second, 0.01f, 25.0f);
	glm::mat4 invProjectionViewMatrix = glm::inverse(cameraProjection * _renderData.viewMatrix);

	// generate cube corners and transform them into worldspace and make them match view frustum corners
	glm::vec4 frustumCorners[8];
	frustumCorners[0] = glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f); // xyz
	frustumCorners[1] = glm::vec4(1.0f, -1.0f, -1.0f, 1.0f); // Xyz
	frustumCorners[2] = glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f); // xYz
	frustumCorners[3] = glm::vec4(1.0f, 1.0f, -1.0f, 1.0f); // XYz
	frustumCorners[4] = glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f); // xyZ
	frustumCorners[5] = glm::vec4(1.0f, -1.0f, 1.0f, 1.0f); // XyZ
	frustumCorners[6] = glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f); // xYZ
	frustumCorners[7] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // XYZ

	// calculate frustum center and frustum length
	glm::vec3 centroid;
	float maxZ = -999999999999.0f;
	float minZ = 999999999999.0f;
	for (glm::vec4 &corner : frustumCorners)
	{
		corner = invProjectionViewMatrix * corner;
		corner /= corner.w;
		glm::vec3 corner3 = glm::vec3(corner);
		centroid += corner3;
		minZ = std::min(minZ, corner3.z);
		maxZ = std::max(maxZ, corner3.z);
	}
	centroid /= 8.0f;
	float distance = maxZ - minZ;

	// bind shadow shader
	shadowShader->bind();

	glViewport(0, 0, 1024, 1024);

	// render shadowmaps for all light sources that cast shadows
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	for (const std::shared_ptr<DirectionalLight> &directionalLight : _level->lights.directionalLights)
	{
		if (directionalLight->isRenderShadows())
		{
			glm::vec3 lightDir = directionalLight->getDirection();
			glm::vec3 lightPos = lightDir * distance + centroid;
			glm::vec3 upDir(0.0f, 1.0f, 0.0f);

			if (abs(lightDir.x) < 0.001 && abs(lightDir.z) < 0.001)
			{
				upDir = glm::vec3(1.0f, 1.0f, 0.0f);
			}

			glm::mat4 lightView = glm::lookAt(lightPos, centroid, upDir);

			float minX = 999999999999.0f;
			float maxX = -999999999999.0f;
			float minY = 999999999999.0f;
			float maxY = -999999999999.0f;
			minZ = 999999999999.0f;
			maxZ = -999999999999.0f;
			for (glm::vec4 corner : frustumCorners)
			{
				corner = lightView * corner;
				corner /= corner.w;
				glm::vec3 corner3 = glm::vec3(corner);
				minX = std::min(corner3.x, minX);
				maxX = std::max(corner3.x, maxX);
				minY = std::min(corner3.y, minY);
				maxY = std::max(corner3.y, maxY);
				minZ = std::min(corner3.z, minZ);
				maxZ = std::max(corner3.z, maxZ);
			}
			float distz = maxZ - minZ;
			directionalLight->setViewProjectionMatrix(glm::ortho(minX, maxX, minY, maxY, 0.0f, distz + distance) * lightView);
			

			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			render(directionalLight->getViewProjectionMatrix(), _scene);

			blit(directionalLight->getShadowMap());
		}
	}

	for (const std::shared_ptr<SpotLight> &spotLight : _level->lights.spotLights)
	{
		if (spotLight->isRenderShadows())
		{
			// render into multisampled framebuffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			render(spotLight->getViewProjectionMatrix(), _scene);

			// blit into texture
			blit(spotLight->getShadowMap());
		}
	}

	for (const std::shared_ptr<PointLight> &pointLight : _level->lights.pointLights)
	{
		if (pointLight->isRenderShadows())
		{
			// render into multisampled framebuffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			render(pointLight->getViewProjectionMatrix(), _scene);

			// blit into texture
			blit(pointLight->getShadowMap());
		}
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	if (_effects.shadowQuality < ShadowQuality::HIGH)
	{
		return;
	}

	// blur all shadowmaps
	shadowBlurShader->bind();
	shadowBlurShader->setUniform(uShadowMap, 0);
	shadowBlurShader->setUniform(uHeight, 1024);
	shadowBlurShader->setUniform(uWidth, 1024);

	fullscreenTriangle->enableVertexAttribArrays();

	for (const std::shared_ptr<DirectionalLight> &directionalLight : _level->lights.directionalLights)
	{
		if (directionalLight->isRenderShadows())
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, directionalLight->getShadowMap(), 0);
			blur(directionalLight->getShadowMap());
		}
	}

	for (const std::shared_ptr<SpotLight> &spotLight : _level->lights.spotLights)
	{
		if (spotLight->isRenderShadows())
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, spotLight->getShadowMap(), 0);
			blur(spotLight->getShadowMap());
		}
	}

	for (const std::shared_ptr<PointLight> &pointLight : _level->lights.pointLights)
	{
		if (pointLight->isRenderShadows())
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, pointLight->getShadowMap(), 0);
			blur(pointLight->getShadowMap());
		}
	}
}

void ShadowRenderer::render(const glm::mat4 &_viewProjectionMatrix, const Scene &_scene)
{
	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<Mesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->modelComponent 
			|| !entityRenderData->transformationComponent
			|| entityRenderData->transparencyComponent
			|| entityRenderData->customTransparencyShaderComponent
			|| entityRenderData->customOpaqueShaderComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->mesh)
		{
			currentMesh = entityRenderData->mesh;
			enabledMesh = false;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		// we're good to go: render this mesh-entity instance
		glm::mat4 modelMatrix;
		modelMatrix = glm::translate(modelMatrix, entityRenderData->transformationComponent->position) 
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation) 
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale));

		shadowShader->setUniform(uModelViewProjectionMatrix, _viewProjectionMatrix * modelMatrix);

		currentMesh->render();
	}
}

void ShadowRenderer::createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution)
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);

	glGenTextures(1, &shadowTextureA);
	glBindTexture(GL_TEXTURE_2D, shadowTextureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowTextureA, 0);

	glGenTextures(1, &shadowTextureB);
	glBindTexture(GL_TEXTURE_2D, shadowTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, shadowTextureB, 0);

	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _resolution.first, _resolution.second);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Shadow Map FBO not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glErrorCheck("");
}

void ShadowRenderer::blur(const GLuint &_textureToBlur)
{
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureToBlur);
		
	shadowBlurShader->setUniform(uDirection, false);
	fullscreenTriangle->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindTexture(GL_TEXTURE_2D, shadowTextureB);
	glDrawBuffer(GL_COLOR_ATTACHMENT2);

	shadowBlurShader->setUniform(uDirection, true);
	fullscreenTriangle->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void ShadowRenderer::deleteAttachments()
{
	GLuint textures[] = { shadowTextureA, shadowTextureB };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
}

void ShadowRenderer::deleteFbos()
{
	GLuint frameBuffers[] = { shadowFbo };
	glDeleteFramebuffers(sizeof(frameBuffers) / sizeof(GLuint), frameBuffers);
}

void ShadowRenderer::blit(GLuint targetTexture)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// attach target shadow map to fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, targetTexture, 0);

	// blit into texture
	blitShader->bind();
	glDrawBuffer(GL_COLOR_ATTACHMENT2);

	blitShader->setUniform(uScreenTextureB, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTextureA);

	fullscreenTriangle->enableVertexAttribArrays();
	fullscreenTriangle->render();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}
