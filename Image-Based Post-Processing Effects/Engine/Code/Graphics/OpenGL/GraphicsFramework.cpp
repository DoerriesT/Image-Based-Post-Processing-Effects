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

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

GraphicsFramework::GraphicsFramework(std::shared_ptr<Window> _window)
	:sceneRenderer(_window),
	postProcessRenderer(_window),
	window(_window),
	shadowRenderer(),
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

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	sceneRenderer.init();
	postProcessRenderer.init();
	shadowRenderer.init();
	environmentRenderer.init();
	
	blitShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Shared/blit.frag");

	uScreenTextureBlit = blitShader->createUniform("uScreenTexture");

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void GraphicsFramework::render(const std::shared_ptr<Camera> &_camera, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects)
{
	static glm::mat4 prevViewProjectionMatrix;

	RenderData renderData;
	renderData.projectionMatrix = window->getProjectionMatrix();
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

	prevViewProjectionMatrix = renderData.viewProjectionMatrix;

	if (renderData.shadows)
	{
		shadowRenderer.renderShadows(renderData, _scene, _level, _effects, _camera);
	}
	sceneRenderer.render(renderData, _scene, _level, _effects);
	postProcessRenderer.render(_effects, sceneRenderer.getColorTexture(), sceneRenderer.getDepthStencilTexture(), sceneRenderer.getVelocityTexture(), _camera);

	blitToScreen();
}

void GraphicsFramework::render(const std::shared_ptr<EnvironmentProbe> &_environmentProbe, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects)
{
	assert(window->getWidth() >= ENVIRONMENT_MAP_SIZE && window->getHeight() >= ENVIRONMENT_MAP_SIZE); // we're reusing the same fbo(s), we use for normal rendering, so make sure our cubemap rendering fits inside
	glm::vec3 position = _environmentProbe->getPosition();
	glm::mat4 viewMatrices[] =
	{
		glm::lookAt(position, position + glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(position, position + glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(position, position + glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(position, position + glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(position, position + glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	RenderData renderData;
	renderData.projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, Window::NEAR_PLANE, Window::FAR_PLANE);
	renderData.resolution = std::make_pair(512, 512);
	renderData.shadows = _effects.shadowQuality != ShadowQuality::OFF;
	renderData.time = (float)Engine::getTime();
	renderData.cameraPosition = position;
	renderData.fov = window->getFieldOfView();

	Effects effects = _effects;
	effects.ambientOcclusion = AmbientOcclusion::OFF;


	for (unsigned int i = 0; i < 6; ++i)
	{
		renderData.viewMatrix = viewMatrices[i];
		renderData.viewProjectionMatrix = renderData.projectionMatrix * renderData.viewMatrix;
		sceneRenderer.render(renderData, _scene, _level, effects);
		environmentRenderer.updateCubeSide(i, sceneRenderer.getColorTexture());
	}
	environmentRenderer.generateMipmaps();
	environmentRenderer.calculateReflectance(_environmentProbe);
	environmentRenderer.calculateIrradiance(_environmentProbe);
}

std::shared_ptr<Texture> GraphicsFramework::render(const AtmosphereParams &_params)
{
	return environmentRenderer.calculateAtmosphere(_params);
}

void save2DTextureToFile(GLuint _texture, unsigned int _width, unsigned int _height)
{
	unsigned char *textureData = new unsigned char[_width * _height * 4];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

	std::string filename = "screenshot-" + Util::getFormatedTime() + ".png";
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filename.c_str(), _width, _height, 4, textureData, 0);

	delete[] textureData;

}

void GraphicsFramework::blitToScreen()
{
	// bind default framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	// bind finished frame texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, postProcessRenderer.getFinishedTexture());
	blitShader->bind();
	blitShader->setUniform(uScreenTextureBlit, 0);

	// draw to back buffer
	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	static double lastPressed = Engine::getTime();

	if (UserInput::getInstance().isKeyPressed(InputKey::F11) && Engine::getTime() - lastPressed > 0.5)
	{
		lastPressed = Engine::getTime();
		save2DTextureToFile(postProcessRenderer.getFinishedTexture(), window->getWidth(), window->getHeight());
	}
}

void GraphicsFramework::setShadowQuality(const ShadowQuality &_shadowQuality)
{

}

GLuint GraphicsFramework::getFinishedFrameTexture()
{
	return postProcessRenderer.getFinishedTexture();
}

void GraphicsFramework::onResize(unsigned int _width, unsigned int _height)
{
	std::pair<unsigned int, unsigned int> resolution = std::make_pair(_width, _height);
	sceneRenderer.resize(resolution);
	postProcessRenderer.resize(resolution);
}
