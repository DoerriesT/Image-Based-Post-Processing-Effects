#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <glm\gtc\matrix_transform.hpp>
#include "GraphicsFramework.h"
#include "Utilities\Utility.h"
#include "Engine.h"
#include "Input\UserInput.h"
#include <gli\gli.hpp>
#include "Graphics\EnvironmentProbe.h"
#include "Window\Window.h"
#include "Graphics\Camera.h"
#include "Graphics\Effects.h"
#include "Graphics\Mesh.h"
#include "RenderData.h"
#include "Graphics\Texture.h"
#include "Level.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

GraphicsFramework::GraphicsFramework(std::shared_ptr<Window> _window)
	:m_renderer(),
	m_window(_window),
	m_environmentRenderer()
{
	m_window->addResizeListener(this);
}

void APIENTRY  MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

void GraphicsFramework::init()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	// seamless cubemap interpolation
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDisable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glEnable(GL_CULL_FACE);

	//glEnable(GL_MULTISAMPLE);

	glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif // _DEBUG

	m_renderer.init(m_window->getWidth(), m_window->getHeight());
	m_environmentRenderer.init();

	m_blitShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Shared/blit.frag");

	m_uScreenTexture = m_blitShader->createUniform("uScreenTexture");
	m_uRedToWhite = m_blitShader->createUniform("uRedToWhite");
	m_uScale = m_blitShader->createUniform("uScale");
	m_uNormalMode = m_blitShader->createUniform("uNormalMode");
	m_uInvViewMatrix = m_blitShader->createUniform("uInvViewMatrix");
	m_uPower = m_blitShader->createUniform("uPower");
	m_uPowerValue = m_blitShader->createUniform("uPowerValue");

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

static glm::mat3 invViewMat;
static RenderData renderData;

extern bool freeze;
bool debugDraw = false;

void GraphicsFramework::render(const std::shared_ptr<Camera> &_camera, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects)
{
	static glm::mat4 prevViewProjectionMatrix;
	static glm::mat4 prevInvJitter;

	glm::vec2 jitters[] =
	{
		glm::vec2(0.25f, -0.25f),
		glm::vec2(-0.25f, 0.25f)
	};

	++m_frame;

	glm::mat4 jitterMatrix = _effects.m_smaa.m_enabled && _effects.m_smaa.m_temporalAntiAliasing ?
		glm::translate(glm::mat4(), glm::vec3(jitters[m_frame % 2].x / float(m_window->getWidth()), jitters[m_frame % 2].y / float(m_window->getHeight()), 0.0f))
		: glm::mat4();

	if (!freeze)
	{
		renderData = {};
		renderData.m_frustum = _camera->getFrustum();
		renderData.m_invJitter = glm::inverse(jitterMatrix);
		renderData.m_prevInvJitter = prevInvJitter;
		renderData.m_projectionMatrix = jitterMatrix * m_window->getProjectionMatrix();
		renderData.m_invProjectionMatrix = glm::inverse(renderData.m_projectionMatrix);
		renderData.m_viewMatrix = _camera->getViewMatrix();
		renderData.m_invViewMatrix = glm::inverse(renderData.m_viewMatrix);
		renderData.m_viewProjectionMatrix = renderData.m_projectionMatrix * renderData.m_viewMatrix;
		renderData.m_invViewProjectionMatrix = glm::inverse(renderData.m_viewProjectionMatrix);
		renderData.m_prevViewProjectionMatrix = prevViewProjectionMatrix;
		renderData.m_resolution = std::make_pair(m_window->getWidth(), m_window->getHeight());
		renderData.m_shadows = _effects.m_shadowQuality != ShadowQuality::OFF;
		renderData.m_time = (float)Engine::getTime();
		renderData.m_cameraPosition = _camera->getPosition();
		renderData.m_viewDirection = _camera->getForwardDirection();
		renderData.m_fov = m_window->getFieldOfView();
		renderData.m_nearPlane = Window::NEAR_PLANE;
		renderData.m_farPlane = Window::FAR_PLANE;
		renderData.m_frame = m_frame;
		renderData.m_bake = false;

		invViewMat = renderData.m_invViewMatrix;

		prevViewProjectionMatrix = renderData.m_viewProjectionMatrix;
		prevInvJitter = renderData.m_invJitter;
	}
	

	m_renderer.render(renderData, _scene, _level, _effects, false, debugDraw);

	//glBindFramebuffer(GL_FRAMEBUFFER, debugFbo);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessRenderer.getFinishedTexture(), 0);
	//RenderPass *prev = nullptr;
	//boundingBoxRenderPass->render(renderData, _level, _scene, &prev);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	blitToScreen();
}

void GraphicsFramework::bake(const Scene &_scene, const std::shared_ptr<Level> &_level, unsigned int _bounces, bool _reflections, bool _irradianceVolume)
{
	RenderData renderData = {};

	renderData.m_projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, Window::NEAR_PLANE, Window::FAR_PLANE);
	renderData.m_invProjectionMatrix = glm::inverse(renderData.m_projectionMatrix);
	renderData.m_resolution = std::make_pair(ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE);
	renderData.m_shadows = true;
	renderData.m_time = (float)Engine::getTime();
	renderData.m_frame = 1;
	renderData.m_fov = 90.0f;
	renderData.m_bake = true;

	Effects effects = {};
	effects.m_ambientOcclusion = AmbientOcclusion::HBAO;
	effects.m_shadowQuality = ShadowQuality::NORMAL;
	effects.m_diffuseAmbientSource = DiffuseAmbientSource::IRRADIANCE_VOLUMES;

	m_renderer.resize(ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE);

	for (unsigned int bounce = 0; bounce < _bounces; ++bounce)
	{
		if (_reflections)
		{
			// bake reflections
			for (std::shared_ptr<EnvironmentProbe> environmentProbe : _level->m_environment.m_environmentProbes)
			{
				glm::vec3 position = environmentProbe->getPosition();
				glm::mat4 viewMatrices[] =
				{
					glm::lookAt(position, position + glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
					glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
					glm::lookAt(position, position + glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
					glm::lookAt(position, position + glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
					glm::lookAt(position, position + glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
					glm::lookAt(position, position + glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
				};


				renderData.m_cameraPosition = position;

				for (unsigned int i = 0; i < 6; ++i)
				{
					renderData.m_viewMatrix = viewMatrices[i];
					renderData.m_invViewMatrix = glm::inverse(renderData.m_viewMatrix);
					renderData.m_viewProjectionMatrix = renderData.m_projectionMatrix * renderData.m_viewMatrix;
					renderData.m_prevViewProjectionMatrix = renderData.m_viewProjectionMatrix;
					renderData.m_invViewProjectionMatrix = glm::inverse(renderData.m_viewProjectionMatrix);
					renderData.m_viewDirection = -glm::transpose(renderData.m_viewMatrix)[2];
					renderData.m_frustum.update(renderData.m_viewProjectionMatrix);

					m_renderer.render(renderData, _scene, _level, effects, true);
					m_environmentRenderer.updateCubeSide(i, m_renderer.getColorTexture());
				}
				m_environmentRenderer.generateMipmaps();
				m_environmentRenderer.calculateReflectance(environmentProbe);
				//environmentRenderer.calculateIrradiance(environmentProbe);
			}
		}
		

		// bake irradiance volume
		std::shared_ptr<IrradianceVolume> volume = _level->m_environment.m_irradianceVolume;
		if (_irradianceVolume && volume)
		{
			glm::ivec3 dims = volume->getDimensions();
			glm::vec3 origin = volume->getOrigin();
			float spacing = volume->getSpacing();

			for (int z = 0; z < dims.z; ++z)
			{
				for (int y = 0; y < dims.y; ++y)
				{
					for (int x = 0; x < dims.x; ++x)
					{
						glm::vec3 position = origin + glm::vec3(x, y, z) * spacing;
						glm::mat4 viewMatrices[] =
						{
							glm::lookAt(position, position + glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
							glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
							glm::lookAt(position, position + glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
							glm::lookAt(position, position + glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
							glm::lookAt(position, position + glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
							glm::lookAt(position, position + glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
						};

						renderData.m_cameraPosition = position;
						for (unsigned int i = 0; i < 6; ++i)
						{
							renderData.m_viewMatrix = viewMatrices[i];
							renderData.m_invViewMatrix = glm::inverse(renderData.m_viewMatrix);
							renderData.m_viewProjectionMatrix = renderData.m_projectionMatrix * renderData.m_viewMatrix;
							renderData.m_prevViewProjectionMatrix = renderData.m_viewProjectionMatrix;
							renderData.m_invViewProjectionMatrix = glm::inverse(renderData.m_viewProjectionMatrix);
							renderData.m_viewDirection = -glm::transpose(renderData.m_viewMatrix)[2];
							renderData.m_frustum.update(renderData.m_viewProjectionMatrix);

							m_renderer.render(renderData, _scene, _level, effects, true);
							m_environmentRenderer.updateCubeSide(i, m_renderer.getColorTexture());
						}
						m_environmentRenderer.calculateIrradiance(volume, glm::ivec3(x, y, z));
					}
				}
			}
			volume->flushToGpu();
		}
	}

	m_renderer.resize(m_window->getWidth(), m_window->getHeight());
}

std::shared_ptr<Texture> GraphicsFramework::render(const AtmosphereParams &_params)
{
	return m_environmentRenderer.calculateAtmosphere(_params);
}

void save2DTextureToFile(GLuint _texture, unsigned int _width, unsigned int _height)
{
	std::unique_ptr<uint32_t[]> textureData = std::make_unique<uint32_t[]>(static_cast<size_t>(_width * _height));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.get());

	std::string filename = "screenshot-" + Utility::getFormatedTime() + ".png";
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filename.c_str(), _width, _height, 4, textureData.get(), 0);

}

GBufferDisplayMode displayMode = GBufferDisplayMode::SHADED;

void GraphicsFramework::blitToScreen()
{
	// bind default framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	m_blitShader->bind();
	m_blitShader->setUniform(m_uScreenTexture, 0);

	GLuint texture = 0;

	switch (displayMode)
	{
	case GBufferDisplayMode::SHADED:
		texture = m_renderer.getFinishedTexture();
		m_blitShader->setUniform(m_uScale, 1.0f);
		m_blitShader->setUniform(m_uRedToWhite, false);
		m_blitShader->setUniform(m_uNormalMode, false);
		m_blitShader->setUniform(m_uPower, false);
		break;
	case GBufferDisplayMode::ALBEDO:
		texture = m_renderer.getAlbedoTexture();
		m_blitShader->setUniform(m_uScale, 1.0f);
		m_blitShader->setUniform(m_uRedToWhite, false);
		m_blitShader->setUniform(m_uNormalMode, false);
		m_blitShader->setUniform(m_uPower, false);
		break;
	case GBufferDisplayMode::NORMAL:
		texture = m_renderer.getNormalTexture();
		m_blitShader->setUniform(m_uScale, 1.0f);
		m_blitShader->setUniform(m_uRedToWhite, false);
		m_blitShader->setUniform(m_uNormalMode, true);
		m_blitShader->setUniform(m_uInvViewMatrix, invViewMat);
		m_blitShader->setUniform(m_uPower, false);
		break;
	case GBufferDisplayMode::MATERIAL:
		texture = m_renderer.getMaterialTexture();
		m_blitShader->setUniform(m_uScale, 1.0f);
		m_blitShader->setUniform(m_uRedToWhite, false);
		m_blitShader->setUniform(m_uNormalMode, false);
		m_blitShader->setUniform(m_uPower, false);
		break;
	case GBufferDisplayMode::DEPTH:
		texture = m_renderer.getDepthStencilTexture();
		m_blitShader->setUniform(m_uScale, 1.0f);
		m_blitShader->setUniform(m_uRedToWhite, true);
		m_blitShader->setUniform(m_uNormalMode, false);
		m_blitShader->setUniform(m_uPower, true);
		m_blitShader->setUniform(m_uPowerValue, 30.0f);
		break;
	case GBufferDisplayMode::VELOCITY:
		texture = m_renderer.getVelocityTexture();
		m_blitShader->setUniform(m_uScale, 10.0f);
		m_blitShader->setUniform(m_uRedToWhite, false);
		m_blitShader->setUniform(m_uNormalMode, false);
		m_blitShader->setUniform(m_uPower, false);
		break;
	case GBufferDisplayMode::AMBIENT_OCCLUSION:
		texture = m_renderer.getAmbientOcclusionTexture();
		m_blitShader->setUniform(m_uScale, 1.0f);
		m_blitShader->setUniform(m_uRedToWhite, true);
		m_blitShader->setUniform(m_uNormalMode, false);
		m_blitShader->setUniform(m_uPower, false);
		break;
	default:
		break;
	}

	// bind finished frame texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);


	// draw to back buffer
	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
	m_fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	static double lastPressed = Engine::getTime();

	if (UserInput::getInstance().isKeyPressed(InputKey::F11) && Engine::getTime() - lastPressed > 0.5)
	{
		lastPressed = Engine::getTime();
		save2DTextureToFile(m_renderer.getFinishedTexture(), m_window->getWidth(), m_window->getHeight());
	}
}

GLuint GraphicsFramework::getFinishedFrameTexture()
{
	return m_renderer.getFinishedTexture();
}

void GraphicsFramework::onResize(unsigned int _width, unsigned int _height)
{
	m_renderer.resize(_width, _height);
}
