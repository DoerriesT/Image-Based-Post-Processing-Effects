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
	:renderer(),
	window(_window),
	environmentRenderer()
{
	window->addResizeListener(this);
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

	renderer.init(window->getWidth(), window->getHeight());
	environmentRenderer.init();

	blitShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Shared/blit.frag");

	uScreenTextureBlit = blitShader->createUniform("uScreenTexture");
	uRedToWhiteBlit = blitShader->createUniform("uRedToWhite");
	uScaleBlit = blitShader->createUniform("uScale");
	uNormalModeBlit = blitShader->createUniform("uNormalMode");
	uInvViewMatrixBlit = blitShader->createUniform("uInvViewMatrix");
	uPowerBlit = blitShader->createUniform("uPower");
	uPowerValueBlit = blitShader->createUniform("uPowerValue");

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
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

	++frame;

	glm::mat4 jitterMatrix = _effects.smaa.enabled && _effects.smaa.temporalAntiAliasing ?
		glm::translate(glm::mat4(), glm::vec3(jitters[frame % 2].x / float(window->getWidth()), jitters[frame % 2].y / float(window->getHeight()), 0.0f))
		: glm::mat4();

	if (!freeze)
	{
		renderData = {};
		renderData.frustum = _camera->getFrustum();
		renderData.invJitter = glm::inverse(jitterMatrix);
		renderData.prevInvJitter = prevInvJitter;
		renderData.projectionMatrix = jitterMatrix * window->getProjectionMatrix();
		renderData.invProjectionMatrix = glm::inverse(renderData.projectionMatrix);
		renderData.viewMatrix = _camera->getViewMatrix();
		renderData.invViewMatrix = glm::inverse(renderData.viewMatrix);
		renderData.viewProjectionMatrix = renderData.projectionMatrix * renderData.viewMatrix;
		renderData.invViewProjectionMatrix = glm::inverse(renderData.viewProjectionMatrix);
		renderData.prevViewProjectionMatrix = prevViewProjectionMatrix;
		renderData.resolution = std::make_pair(window->getWidth(), window->getHeight());
		renderData.shadows = _effects.shadowQuality != ShadowQuality::OFF;
		renderData.time = (float)Engine::getTime();
		renderData.cameraPosition = _camera->getPosition();
		renderData.viewDirection = _camera->getForwardDirection();
		renderData.fov = window->getFieldOfView();
		renderData.nearPlane = Window::NEAR_PLANE;
		renderData.farPlane = Window::FAR_PLANE;
		renderData.frame = frame;
		renderData.bake = false;

		invViewMat = renderData.invViewMatrix;

		prevViewProjectionMatrix = renderData.viewProjectionMatrix;
		prevInvJitter = renderData.invJitter;
	}
	

	renderer.render(renderData, _scene, _level, _effects, false, debugDraw);

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

	renderData.projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, Window::NEAR_PLANE, Window::FAR_PLANE);
	renderData.invProjectionMatrix = glm::inverse(renderData.projectionMatrix);
	renderData.resolution = std::make_pair(ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE);
	renderData.shadows = true;
	renderData.time = (float)Engine::getTime();
	renderData.frame = 1;
	renderData.fov = 90.0f;
	renderData.bake = true;

	Effects effects = {};
	effects.ambientOcclusion = AmbientOcclusion::HBAO;
	effects.shadowQuality = ShadowQuality::NORMAL;
	effects.diffuseAmbientSource = DiffuseAmbientSource::IRRADIANCE_VOLUMES;

	renderer.resize(ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE);

	for (unsigned int bounce = 0; bounce < _bounces; ++bounce)
	{
		if (_reflections)
		{
			// bake reflections
			for (std::shared_ptr<EnvironmentProbe> environmentProbe : _level->environment.environmentProbes)
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


				renderData.cameraPosition = position;

				for (unsigned int i = 0; i < 6; ++i)
				{
					renderData.viewMatrix = viewMatrices[i];
					renderData.invViewMatrix = glm::inverse(renderData.viewMatrix);
					renderData.viewProjectionMatrix = renderData.projectionMatrix * renderData.viewMatrix;
					renderData.prevViewProjectionMatrix = renderData.viewProjectionMatrix;
					renderData.invViewProjectionMatrix = glm::inverse(renderData.viewProjectionMatrix);
					renderData.viewDirection = -glm::transpose(renderData.viewMatrix)[2];
					renderData.frustum.update(renderData.viewProjectionMatrix);

					renderer.render(renderData, _scene, _level, effects, true);
					environmentRenderer.updateCubeSide(i, renderer.getColorTexture());
				}
				environmentRenderer.generateMipmaps();
				environmentRenderer.calculateReflectance(environmentProbe);
				//environmentRenderer.calculateIrradiance(environmentProbe);
			}
		}
		

		// bake irradiance volume
		std::shared_ptr<IrradianceVolume> volume = _level->environment.irradianceVolume;
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

						renderData.cameraPosition = position;
						for (unsigned int i = 0; i < 6; ++i)
						{
							renderData.viewMatrix = viewMatrices[i];
							renderData.invViewMatrix = glm::inverse(renderData.viewMatrix);
							renderData.viewProjectionMatrix = renderData.projectionMatrix * renderData.viewMatrix;
							renderData.prevViewProjectionMatrix = renderData.viewProjectionMatrix;
							renderData.invViewProjectionMatrix = glm::inverse(renderData.viewProjectionMatrix);
							renderData.viewDirection = -glm::transpose(renderData.viewMatrix)[2];
							renderData.frustum.update(renderData.viewProjectionMatrix);

							renderer.render(renderData, _scene, _level, effects, true);
							environmentRenderer.updateCubeSide(i, renderer.getColorTexture());
						}
						environmentRenderer.calculateIrradiance(volume, glm::ivec3(x, y, z));
					}
				}
			}
			volume->flushToGpu();
		}
	}

	renderer.resize(window->getWidth(), window->getHeight());
}

std::shared_ptr<Texture> GraphicsFramework::render(const AtmosphereParams &_params)
{
	return environmentRenderer.calculateAtmosphere(_params);
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

	blitShader->bind();
	blitShader->setUniform(uScreenTextureBlit, 0);

	GLuint texture = 0;

	switch (displayMode)
	{
	case GBufferDisplayMode::SHADED:
		texture = renderer.getFinishedTexture();
		blitShader->setUniform(uScaleBlit, 1.0f);
		blitShader->setUniform(uRedToWhiteBlit, false);
		blitShader->setUniform(uNormalModeBlit, false);
		blitShader->setUniform(uPowerBlit, false);
		break;
	case GBufferDisplayMode::ALBEDO:
		texture = renderer.getAlbedoTexture();
		blitShader->setUniform(uScaleBlit, 1.0f);
		blitShader->setUniform(uRedToWhiteBlit, false);
		blitShader->setUniform(uNormalModeBlit, false);
		blitShader->setUniform(uPowerBlit, false);
		break;
	case GBufferDisplayMode::NORMAL:
		texture = renderer.getNormalTexture();
		blitShader->setUniform(uScaleBlit, 1.0f);
		blitShader->setUniform(uRedToWhiteBlit, false);
		blitShader->setUniform(uNormalModeBlit, true);
		blitShader->setUniform(uInvViewMatrixBlit, invViewMat);
		blitShader->setUniform(uPowerBlit, false);
		break;
	case GBufferDisplayMode::MATERIAL:
		texture = renderer.getMaterialTexture();
		blitShader->setUniform(uScaleBlit, 1.0f);
		blitShader->setUniform(uRedToWhiteBlit, false);
		blitShader->setUniform(uNormalModeBlit, false);
		blitShader->setUniform(uPowerBlit, false);
		break;
	case GBufferDisplayMode::DEPTH:
		texture = renderer.getDepthStencilTexture();
		blitShader->setUniform(uScaleBlit, 1.0f);
		blitShader->setUniform(uRedToWhiteBlit, true);
		blitShader->setUniform(uNormalModeBlit, false);
		blitShader->setUniform(uPowerBlit, true);
		blitShader->setUniform(uPowerValueBlit, 30.0f);
		break;
	case GBufferDisplayMode::VELOCITY:
		texture = renderer.getVelocityTexture();
		blitShader->setUniform(uScaleBlit, 10.0f);
		blitShader->setUniform(uRedToWhiteBlit, false);
		blitShader->setUniform(uNormalModeBlit, false);
		blitShader->setUniform(uPowerBlit, false);
		break;
	case GBufferDisplayMode::AMBIENT_OCCLUSION:
		texture = renderer.getAmbientOcclusionTexture();
		blitShader->setUniform(uScaleBlit, 1.0f);
		blitShader->setUniform(uRedToWhiteBlit, true);
		blitShader->setUniform(uNormalModeBlit, false);
		blitShader->setUniform(uPowerBlit, false);
		break;
	default:
		break;
	}

	// bind finished frame texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);


	// draw to back buffer
	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	static double lastPressed = Engine::getTime();

	if (UserInput::getInstance().isKeyPressed(InputKey::F11) && Engine::getTime() - lastPressed > 0.5)
	{
		lastPressed = Engine::getTime();
		save2DTextureToFile(renderer.getFinishedTexture(), window->getWidth(), window->getHeight());
	}
}

GLuint GraphicsFramework::getFinishedFrameTexture()
{
	return renderer.getFinishedTexture();
}

void GraphicsFramework::onResize(unsigned int _width, unsigned int _height)
{
	renderer.resize(_width, _height);
}
