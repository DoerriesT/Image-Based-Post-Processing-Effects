#include "CombinedDofTileMaxComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

CombinedDofTileMaxComputePass::CombinedDofTileMaxComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	tileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofCombinedCocTileMax.comp");

	uFocalLength.create(tileMaxShader);
	uApertureSize.create(tileMaxShader);
	uNearFar.create(tileMaxShader);
	uDirection.create(tileMaxShader);
}

void CombinedDofTileMaxComputePass::execute(GLuint _depthTexture, GLuint _tmpTexture, GLuint _tileMaxTexture, unsigned int _tileSize, float _fieldOfView, float _nearPlane, float _farPlane)
{
	tileMaxShader->bind();

	const float filmWidth = 0.035f;
	const float apertureSize = 8.0f;

	float focalLength = (0.5f * filmWidth) / glm::tan(_fieldOfView * 0.5f);

	uFocalLength.set(focalLength);
	uApertureSize.set(apertureSize);
	uNearFar.set(glm::vec2(_nearPlane, _farPlane));

	// first pass (horizontal)
	{
		uDirection.set(false);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _depthTexture);

		glBindImageTexture(0, _tmpTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		GLUtility::glDispatchComputeHelper(width / _tileSize, height, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
	
	// second pass (vertical)
	{
		uDirection.set(true);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _tmpTexture);

		glBindImageTexture(0, _tileMaxTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		GLUtility::glDispatchComputeHelper(width / _tileSize, height / _tileSize, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

void CombinedDofTileMaxComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
