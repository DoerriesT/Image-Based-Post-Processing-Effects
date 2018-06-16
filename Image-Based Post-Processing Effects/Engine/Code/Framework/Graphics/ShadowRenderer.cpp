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
	:splits{ 0.05f, 0.15f, 0.5f, 1.0f }
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
	createFboAttachments(std::make_pair(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION));

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}


void ShadowRenderer::renderShadows(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects, const std::shared_ptr<Camera> &_camera)
{
	// bind shadow shader
	shadowShader->bind();

	glViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);

	// render shadowmaps for all light sources that cast shadows
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	AxisAlignedBoundingBox sceneAABB = calculateSceneAABB(_scene);

	for (const std::shared_ptr<DirectionalLight> &directionalLight : _level->lights.directionalLights)
	{
		if (directionalLight->isRenderShadows())
		{
			directionalLight->setViewProjectionMatrix(calculateLightViewProjection(_renderData, sceneAABB, directionalLight->getDirection(), 0.1f, 15.0f, true));

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
	shadowBlurShader->setUniform(uHeight, (int)SHADOW_MAP_RESOLUTION);
	shadowBlurShader->setUniform(uWidth, (int)SHADOW_MAP_RESOLUTION);

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

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

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->modelComponent
			|| !entityRenderData->transformationComponent
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


		// skip this mesh if its transparent
		if (entityRenderData->transparencyComponent && contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		// we're good to go: render this mesh-entity instance
		glm::mat4 modelMatrix;
		modelMatrix = glm::translate(modelMatrix, entityRenderData->transformationComponent->position)
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale));

		shadowShader->setUniform(uModelViewProjectionMatrix, _viewProjectionMatrix * modelMatrix);

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

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

void ShadowRenderer::blur(GLuint _textureToBlur)
{
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureToBlur);

	shadowBlurShader->setUniform(uDirection, false);
	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindTexture(GL_TEXTURE_2D, shadowTextureB);
	glDrawBuffer(GL_COLOR_ATTACHMENT2);

	shadowBlurShader->setUniform(uDirection, true);
	fullscreenTriangle->getSubMesh()->render();
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

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
	fullscreenTriangle->getSubMesh()->render();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

glm::mat4 ShadowRenderer::calculateLightViewProjection(const RenderData &_renderData, const std::shared_ptr<Camera> &_camera, const glm::vec3 &_lightDir, float _nearPlane, float _farPlane)
{
	glm::vec3 frustumMin(std::numeric_limits<float>::max());
	glm::vec3 frustumMax(std::numeric_limits<float>::lowest());

	const float aspectRatio = static_cast<float>(_renderData.resolution.first) / _renderData.resolution.second;
	const float heightScale = 2.0f * tan(glm::radians(_renderData.fov) * 0.5f);
	const float nearHeight = heightScale * _nearPlane;
	const float nearWidth = nearHeight * aspectRatio;
	const float farHeight = heightScale * _farPlane;
	const float farWidth = farHeight * aspectRatio;
	const glm::vec3 camPos = _renderData.cameraPosition;
	const glm::vec3 camForward = _camera->getForwardDirection();
	const glm::vec3 camUp = _camera->getUpDirection();
	const glm::vec3 camRight = glm::cross(camForward, camUp);
	const glm::vec3 nc = camPos + camForward * _nearPlane; // near center
	const glm::vec3 fc = camPos + camForward * _farPlane; // far center

	// Vertices in a world space.
	glm::vec3 vertices[8] =
	{
		nc - camUp * nearHeight * 0.5f - camRight * nearWidth * 0.5f, // nbl (near, bottom, left)
		nc - camUp * nearHeight * 0.5f + camRight * nearWidth * 0.5f, // nbr
		nc + camUp * nearHeight * 0.5f + camRight * nearWidth * 0.5f, // ntr
		nc + camUp * nearHeight * 0.5f - camRight * nearWidth * 0.5f, // ntl
		fc - camUp * farHeight  * 0.5f - camRight * farWidth * 0.5f, // fbl (far, bottom, left)
		fc - camUp * farHeight  * 0.5f + camRight * farWidth * 0.5f, // fbr
		fc + camUp * farHeight  * 0.5f + camRight * farWidth * 0.5f, // ftr
		fc + camUp * farHeight  * 0.5f - camRight * farWidth * 0.5f, // ftl
	};

	// center of frustum in world space
	glm::vec3 center(0.0f);
	for (size_t i = 0; i < 8; ++i)
	{
		center += vertices[i];
	}
	center /= 8.0f;

	const float distance = -10.0f;

	glm::mat4 lightView = glm::lookAt(center + _lightDir * distance, center, glm::abs(_lightDir) != glm::vec3(0.0, 1.0, 0.0) ? glm::vec3(0.0, 1.0, 0.0) : glm::vec3(0.0, 1.0, 1.0));

	glm::mat3 rotMat(lightView);
	glm::vec3 d(lightView[3]);

	glm::vec3 retVec = (-d * rotMat) / distance;

	for (size_t i = 0; i < 8; ++i)
	{
		// transform to light space
		glm::vec4 v = lightView * glm::vec4(vertices[i], 1.0);
		v /= v.w;
		vertices[i] = v;

		// Update bounding box.
		frustumMin = glm::min(frustumMin, vertices[i]);
		frustumMax = glm::max(frustumMax, vertices[i]);
	}

	glm::mat4 lightProjection = glm::ortho(frustumMin.x, frustumMax.x, frustumMin.y, frustumMax.y, 0.0f, frustumMin.z);

	return lightProjection * lightView;
}

glm::mat4 ShadowRenderer::calculateLightViewProjection(const RenderData & _renderData, const AxisAlignedBoundingBox &_sceneAABB, const glm::vec3 &_lightDir, float _nearPlane, float _farPlane, bool _useAABB)
{
	glm::mat4 cameraProjection = glm::perspective(glm::radians(_renderData.fov), _renderData.resolution.first / (float)_renderData.resolution.second, _nearPlane, _farPlane);
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

	// transform corners to world space and calculate frustum center
	glm::vec3 frustumCenter;
	for (glm::vec4 &corner : frustumCorners)
	{
		corner = invProjectionViewMatrix * corner;
		corner /= corner.w;
		frustumCenter += glm::vec3(corner);
	}
	frustumCenter /= 8.0f;
	float distance = glm::distance(frustumCorners[0], frustumCorners[7]);

	glm::vec3 lightPos = _lightDir * distance + frustumCenter;
	glm::vec3 upDir(0.0f, 1.0f, 0.0f);

	// choose different up vector if light direction would be linearly dependent otherwise
	if (abs(_lightDir.x) < 0.001 && abs(_lightDir.z) < 0.001)
	{
		upDir = glm::vec3(1.0f, 1.0f, 0.0f);
	}

	glm::mat4 lightView = glm::lookAt(lightPos, frustumCenter, upDir);

	glm::vec3 minCorner(std::numeric_limits<float>::max());
	glm::vec3 maxCorner(std::numeric_limits<float>::lowest());
	for (glm::vec4 corner : frustumCorners)
	{
		corner = lightView * corner;
		corner /= corner.w;
		glm::vec3 corner3 = glm::vec3(corner);
		minCorner = glm::min(corner3, minCorner);
		maxCorner = glm::max(corner3, maxCorner);
	}

	if (_useAABB)
	{
		glm::vec4 viewAABBMin = glm::vec4(_sceneAABB.min, 1.0f);
		glm::vec4 viewAABBMax = glm::vec4(_sceneAABB.max, 1.0f);
		viewAABBMin = lightView * viewAABBMin;
		viewAABBMax = lightView * viewAABBMax;
		viewAABBMin /= viewAABBMin.w;
		viewAABBMax /= viewAABBMax.w;

		viewAABBMin = glm::min(viewAABBMin, viewAABBMax);
		viewAABBMax = glm::max(viewAABBMin, viewAABBMax);

		if (viewAABBMin.x > minCorner.x && viewAABBMin.x < maxCorner.x)
		{
			minCorner.x = viewAABBMin.x;
		}
		if (viewAABBMin.y > minCorner.y && viewAABBMin.y < maxCorner.y)
		{
			minCorner.y = viewAABBMin.y;
		}
		if (viewAABBMax.z < maxCorner.z && viewAABBMax.z > minCorner.z)
		{
			maxCorner.z = viewAABBMax.z;
		}

		if (viewAABBMax.x < maxCorner.x && viewAABBMax.x > minCorner.x)
		{
			maxCorner.x = viewAABBMax.x;
		}
		if (viewAABBMax.y < maxCorner.y && viewAABBMax.y > minCorner.y)
		{
			maxCorner.y = viewAABBMax.y;
		}
		/*if (viewAABBMax.z < maxCorner.z && viewAABBMax.z > minCorner.z)
		{
			maxCorner.z = viewAABBMax.z;
		}*/
	}

	float distz = maxCorner.z - minCorner.z;
	float projFarPlane = distz + distance;
	return glm::ortho(minCorner.x, maxCorner.x, minCorner.y, maxCorner.y, 0.0f, projFarPlane) * lightView;
}

AxisAlignedBoundingBox ShadowRenderer::calculateSceneAABB(const Scene &_scene)
{
	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	glm::vec3 minCorner(std::numeric_limits<float>::max());
	glm::vec3 maxCorner(std::numeric_limits<float>::lowest());

	for (size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (entityRenderData->customTransparencyShaderComponent ||
			!entityRenderData->modelComponent ||
			!entityRenderData->transformationComponent)
		{
			continue;
		}

		std::shared_ptr<SubMesh> &currentMesh = entityRenderData->mesh;

		// skip this mesh if its transparent
		if (entityRenderData->transparencyComponent && contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh->isValid())
		{
			continue;
		}

		glm::mat4 modelMatrix = glm::translate(entityRenderData->transformationComponent->position)
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale));

		AxisAlignedBoundingBox meshAABB = currentMesh->getAABB();
		glm::vec4 meshAABBMin = glm::vec4(meshAABB.min, 1.0);
		glm::vec4 meshAABBMax = glm::vec4(meshAABB.max, 1.0);

		meshAABBMin = modelMatrix * meshAABBMin;
		meshAABBMin /= meshAABBMin.w;
		meshAABBMax = modelMatrix * meshAABBMax;
		meshAABBMax /= meshAABBMax.w;

		minCorner = glm::min(glm::vec3(meshAABBMin), minCorner);
		maxCorner = glm::max(glm::vec3(meshAABBMax), maxCorner);
	}

	return { minCorner , maxCorner };
}
