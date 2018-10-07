#include "ShadowRenderPass.h"
#include "Window\Window.h"
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Scene.h"
#include "Graphics\EntityRenderData.h"
#include "Utilities\ContainerUtility.h"
#include "Graphics\Mesh.h"
#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>

ShadowRenderPass::ShadowRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_NONE };
	state.blendState.enabled = false;
	state.cullFaceState.enabled = true;
	state.cullFaceState.face = GL_BACK;
	state.depthState.enabled = true;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_TRUE;
	state.stencilState.enabled = false;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_ALWAYS;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	shadowShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shadow/shadow.vert", "Resources/Shaders/Shadow/shadow.frag");

	uModelViewProjectionMatrix.create(shadowShader);
}

void ShadowRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene & _scene, bool _cascadeSkipOptimization, RenderPass ** _previousRenderPass)
{
	if (!_renderData.shadows)
	{
		return;
	}
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	shadowShader->bind();

	unsigned int frameCounter = _renderData.frame % SHADOW_CASCADES;

	AxisAlignedBoundingBox sceneAABB = calculateSceneAABB(_scene);

	float splits[SHADOW_CASCADES];
	float nearPlane = Window::NEAR_PLANE;
	float farPlane = Window::FAR_PLANE * 0.25f;
	float blendWeight = 0.7f;
	for (unsigned int i = 1; i < SHADOW_CASCADES; ++i)
	{
		float logSplit = nearPlane * pow(farPlane / nearPlane, i / float(SHADOW_CASCADES));
		float uniSplit = nearPlane + (farPlane - nearPlane) * (i / float(SHADOW_CASCADES));
		splits[i - 1] = blendWeight * logSplit + (1.0f - blendWeight) * uniSplit;
	}
	splits[SHADOW_CASCADES - 1] = farPlane;

	for (const std::shared_ptr<DirectionalLight> &directionalLight : _level->lights.directionalLights)
	{
		if (directionalLight->isRenderShadows())
		{
			glm::mat4 lightViewProjections[SHADOW_CASCADES];
			for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
			{
				if (i <= frameCounter || !_cascadeSkipOptimization)
				{
					lightViewProjections[i] = calculateLightViewProjection(_renderData, sceneAABB, directionalLight->getDirection(), i == 0 ? 0.05f : splits[i - 1], splits[i], true);
				}
				else
				{
					lightViewProjections[i] = directionalLight->getViewProjectionMatrices()[i];
				}
			}
			directionalLight->setViewProjectionMatrices(lightViewProjections);
			directionalLight->setSplits(splits);

			unsigned int shadowMapResolution = directionalLight->getShadowMapResolution();
			glViewport(0, 0, shadowMapResolution, shadowMapResolution);

			for (unsigned int i = 0; i < SHADOW_CASCADES && (i <= frameCounter || !_cascadeSkipOptimization); ++i)
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, directionalLight->getShadowMap(), 0, i);

				glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
				glClear(GL_DEPTH_BUFFER_BIT);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				renderShadows(*(directionalLight->getViewProjectionMatrices() + i), _scene);
			}
		}
	}

	for (const std::shared_ptr<SpotLight> &spotLight : _level->lights.spotLights)
	{
		if (spotLight->isRenderShadows() && _renderData.frustum.testSphere(spotLight->getBoundingSphere()))
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, spotLight->getShadowMap(), 0);
			unsigned int shadowMapResolution = spotLight->getShadowMapResolution();
			glViewport(0, 0, shadowMapResolution, shadowMapResolution);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			glm::mat4 viewProj = spotLight->getViewProjectionMatrix();
			renderShadows(viewProj, _scene);
		}
	}

	for (const std::shared_ptr<PointLight> &pointLight : _level->lights.pointLights)
	{
		if (pointLight->isRenderShadows() && _renderData.frustum.testSphere(pointLight->getBoundingSphere()))
		{
			unsigned int shadowMapResolution = pointLight->getShadowMapResolution();
			glViewport(0, 0, shadowMapResolution, shadowMapResolution);

			for (unsigned int i = 0; i < 6; ++i)
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointLight->getShadowMap(), 0, i);

				glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
				glClear(GL_DEPTH_BUFFER_BIT);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				renderShadows(*(pointLight->getViewProjectionMatrices() + i), _scene);
			}
		}
	}

	// reset these manually
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(state.viewportState.x, state.viewportState.y, state.viewportState.width, state.viewportState.height);
}

void ShadowRenderPass::renderShadows(const glm::mat4 & _viewProjectionMatrix, const Scene & _scene)
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
		if (entityRenderData->transparencyComponent && ContainerUtility::contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		// we're good to go: render this mesh-entity instance

		uModelViewProjectionMatrix.set(_viewProjectionMatrix * entityRenderData->transformationComponent->transformation);

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArraysPositionOnly();
		}

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(currentMesh->getIndices().size()), GL_UNSIGNED_INT, NULL);
	}
}

glm::mat4 ShadowRenderPass::calculateLightViewProjection(const RenderData & _renderData, const AxisAlignedBoundingBox & _sceneAABB, const glm::vec3 & _lightDir, float _nearPlane, float _farPlane, bool _useAABB)
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
	float distance = 100.0f;//glm::distance(frustumCorners[0], frustumCorners[7]);

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
		/*if (viewAABBMax.z < maxCorner.z && viewAABBMax.z > minCorner.z)
		{
		maxCorner.z = viewAABBMax.z;
		}*/

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
	return glm::ortho(minCorner.x, maxCorner.x, minCorner.y, maxCorner.y, 0.0f, 300.0f) * lightView;
}

AxisAlignedBoundingBox ShadowRenderPass::calculateSceneAABB(const Scene & _scene)
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
		if (entityRenderData->transparencyComponent && ContainerUtility::contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh->isValid())
		{
			continue;
		}

		glm::mat4 modelMatrix = entityRenderData->transformationComponent->transformation;

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

	return { minCorner, maxCorner };
}
