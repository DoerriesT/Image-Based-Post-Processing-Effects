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
	m_fbo = _fbo;
	m_drawBuffers = { GL_NONE };
	m_state.m_blendState.m_enabled = false;
	m_state.m_cullFaceState.m_enabled = true;
	m_state.m_cullFaceState.m_face = GL_BACK;
	m_state.m_depthState.m_enabled = true;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_TRUE;
	m_state.m_stencilState.m_enabled = false;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_ALWAYS;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 1;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = m_state.m_stencilState.m_backOpZfail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(_width, _height);

	m_shadowShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shadow/shadow.vert", "Resources/Shaders/Shadow/shadow.frag");

	m_uModelViewProjectionMatrix.create(m_shadowShader);
}

void ShadowRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene & _scene, bool _cascadeSkipOptimization, RenderPass ** _previousRenderPass)
{
	if (!_renderData.m_shadows)
	{
		return;
	}
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	m_shadowShader->bind();

	unsigned int frameCounter = _renderData.m_frame % SHADOW_CASCADES;

	float splits[SHADOW_CASCADES];
	float nearPlane = Window::NEAR_PLANE;
	float farPlane = Window::FAR_PLANE * 0.1f;
	float blendWeight = 0.7f;
	for (unsigned int i = 1; i < SHADOW_CASCADES; ++i)
	{
		float logSplit = nearPlane * pow(farPlane / nearPlane, i / float(SHADOW_CASCADES));
		float uniSplit = nearPlane + (farPlane - nearPlane) * (i / float(SHADOW_CASCADES));
		splits[i - 1] = blendWeight * logSplit + (1.0f - blendWeight) * uniSplit;
	}
	splits[SHADOW_CASCADES - 1] = farPlane;

	for (const std::shared_ptr<DirectionalLight> &directionalLight : _level->m_lights.m_directionalLights)
	{
		if (directionalLight->isRenderShadows())
		{
			glm::mat4 lightViewProjections[SHADOW_CASCADES];
			for (unsigned int i = 0; i < SHADOW_CASCADES; ++i)
			{
				if (i <= frameCounter || !_cascadeSkipOptimization)
				{
					lightViewProjections[i] = calculateLightViewProjection(_renderData, directionalLight->getDirection(), i == 0 ? 0.05f : splits[i - 1], splits[i], directionalLight->getShadowMapResolution());
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

	for (const std::shared_ptr<SpotLight> &spotLight : _level->m_lights.m_spotLights)
	{
		if (spotLight->isRenderShadows() && _renderData.m_frustum.testSphere(spotLight->getBoundingSphere()))
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

	for (const std::shared_ptr<PointLight> &pointLight : _level->m_lights.m_pointLights)
	{
		if (pointLight->isRenderShadows() && _renderData.m_frustum.testSphere(pointLight->getBoundingSphere()))
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
	glViewport(m_state.m_viewportState.m_x, m_state.m_viewportState.m_y, m_state.m_viewportState.m_width, m_state.m_viewportState.m_height);
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
		if (!entityRenderData->m_modelComponent
			|| !entityRenderData->m_transformationComponent
			|| entityRenderData->m_customTransparencyShaderComponent
			|| entityRenderData->m_customOpaqueShaderComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->m_mesh)
		{
			currentMesh = entityRenderData->m_mesh;
			enabledMesh = false;
		}

		// skip this mesh if its transparent
		if (entityRenderData->m_transparencyComponent && ContainerUtility::contains(entityRenderData->m_transparencyComponent->m_transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		// we're good to go: render this mesh-entity instance

		m_uModelViewProjectionMatrix.set(_viewProjectionMatrix * entityRenderData->m_transformationComponent->m_transformation);

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArraysPositionOnly();
		}

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(currentMesh->getIndices().size()), GL_UNSIGNED_INT, NULL);
	}
}

glm::mat4 ShadowRenderPass::calculateLightViewProjection(const RenderData & _renderData, const glm::vec3 & _lightDir, float _nearPlane, float _farPlane, unsigned int _shadowMapSize)
{
	glm::mat4 cameraProjection = glm::perspective(glm::radians(_renderData.m_fov), _renderData.m_resolution.first / (float)_renderData.m_resolution.second, _nearPlane, _farPlane);
	glm::mat4 invProjection = glm::inverse(cameraProjection);

	glm::vec3 frustumCorners[8];
	frustumCorners[0] = glm::vec3(-1.0f, -1.0f, -1.0f); // xyz
	frustumCorners[1] = glm::vec3(1.0f, -1.0f, -1.0f); // Xyz
	frustumCorners[2] = glm::vec3(-1.0f, 1.0f, -1.0f); // xYz
	frustumCorners[3] = glm::vec3(1.0f, 1.0f, -1.0f); // XYz
	frustumCorners[4] = glm::vec3(-1.0f, -1.0f, 1.0f); // xyZ
	frustumCorners[5] = glm::vec3(1.0f, -1.0f, 1.0f); // XyZ
	frustumCorners[6] = glm::vec3(-1.0f, 1.0f, 1.0f); // xYZ
	frustumCorners[7] = glm::vec3(1.0f, 1.0f, 1.0f); // XYZ

	AxisAlignedBoundingBox bb = { glm::vec3(std::numeric_limits<float>::max()) , glm::vec3(std::numeric_limits<float>::lowest()) };

	for (size_t i = 0; i < 8; ++i)
	{
		glm::vec4 corner4 = invProjection * glm::vec4(frustumCorners[i], 1.0f);
		glm::vec3 corner = corner4 /= corner4.w;
		bb.m_min = glm::min(bb.m_min, corner);
		bb.m_max = glm::max(bb.m_max, corner);
	}

	float radius = glm::distance(bb.m_min, bb.m_max) * 0.5f;
	glm::vec3 sphereCenter = (bb.m_min + bb.m_max) * 0.5f;
	glm::vec3 target = _renderData.m_invViewMatrix * glm::vec4(sphereCenter, 1.0f);

	glm::vec3 upDir(0.0f, 1.0f, 0.0f);

	// choose different up vector if light direction would be linearly dependent otherwise
	if (abs(_lightDir.x) < 0.001 && abs(_lightDir.z) < 0.001)
	{
		upDir = glm::vec3(1.0f, 1.0f, 0.0f);
	}

	glm::mat4 lightView = glm::lookAt(target + _lightDir * 150.0f, target - _lightDir * 150.0f, upDir);

	lightView[3].x -= fmodf(lightView[3].x, (radius / static_cast<float>(_shadowMapSize)) * 2.0f);
	lightView[3].y -= fmodf(lightView[3].y, (radius / static_cast<float>(_shadowMapSize)) * 2.0f);

	return glm::ortho(-radius, radius, -radius, radius, 0.0f, 300.0f) * lightView;
}