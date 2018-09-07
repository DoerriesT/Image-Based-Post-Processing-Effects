#include "RenderPass.h"

void RenderPass::begin(const RenderPass *_previousRenderPass)
{
	// depth
	{
		if (!_previousRenderPass || _previousRenderPass->state.depthState.enabled != state.depthState.enabled)
		{
			state.depthState.enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		}
		if (!_previousRenderPass || _previousRenderPass->state.depthState.mask != state.depthState.mask)
		{
			glDepthMask(state.depthState.mask);
		}
		if (!_previousRenderPass || _previousRenderPass->state.depthState.func != state.depthState.func)
		{
			glDepthFunc(state.depthState.func);
		}
	}

	// stencil
	{
		if (!_previousRenderPass || _previousRenderPass->state.stencilState.enabled != state.stencilState.enabled)
		{
			state.stencilState.enabled ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
		}
		//if (state.stencilState.enabled)
		{
			if (!_previousRenderPass
				|| _previousRenderPass->state.stencilState.frontFunc != state.stencilState.frontFunc
				|| _previousRenderPass->state.stencilState.frontRef != state.stencilState.frontRef
				|| _previousRenderPass->state.stencilState.frontMask != state.stencilState.frontMask)
			{
				glStencilFuncSeparate(GL_FRONT, state.stencilState.frontFunc, state.stencilState.frontRef, state.stencilState.frontMask);
			}
			if (!_previousRenderPass
				|| _previousRenderPass->state.stencilState.backFunc != state.stencilState.backFunc
				|| _previousRenderPass->state.stencilState.backRef != state.stencilState.backRef
				|| _previousRenderPass->state.stencilState.backMask != state.stencilState.backMask)
			{
				glStencilFuncSeparate(GL_BACK, state.stencilState.backFunc, state.stencilState.backRef, state.stencilState.backMask);
			}

			if (!_previousRenderPass
				|| _previousRenderPass->state.stencilState.frontOpFail != state.stencilState.frontOpFail
				|| _previousRenderPass->state.stencilState.frontOpZfail != state.stencilState.frontOpZfail
				|| _previousRenderPass->state.stencilState.frontOpZpass != state.stencilState.frontOpZpass)
			{
				glStencilOpSeparate(GL_FRONT, state.stencilState.frontOpFail, state.stencilState.frontOpZfail, state.stencilState.frontOpZpass);
			}

			if (!_previousRenderPass
				|| _previousRenderPass->state.stencilState.backOpFail != state.stencilState.backOpFail
				|| _previousRenderPass->state.stencilState.backOpZfail != state.stencilState.backOpZfail
				|| _previousRenderPass->state.stencilState.backOpZpass != state.stencilState.backOpZpass)
			{
				glStencilOpSeparate(GL_BACK, state.stencilState.backOpFail, state.stencilState.backOpZfail, state.stencilState.backOpZpass);
			}
		}
	}

	// cull face
	{
		if (!_previousRenderPass || _previousRenderPass->state.cullFaceState.enabled != state.cullFaceState.enabled)
		{
			state.cullFaceState.enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		}
		//if (state.cullFaceState.enabled)
		{
			if (!_previousRenderPass || _previousRenderPass->state.cullFaceState.face != state.cullFaceState.face)
			{
				glCullFace(state.cullFaceState.face);
			}
		}
		
	}

	// blend
	{
		if (!_previousRenderPass || _previousRenderPass->state.blendState.enabled != state.blendState.enabled)
		{
			state.blendState.enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		}

		//if (state.blendState.enabled)
		{
			if (!_previousRenderPass
				|| _previousRenderPass->state.blendState.sFactor != state.blendState.sFactor
				|| _previousRenderPass->state.blendState.dFactor != state.blendState.dFactor)
			{
				glBlendFunc(state.blendState.sFactor, state.blendState.dFactor);
			}
		}
	}

	// fbo
	{
		if (!_previousRenderPass || _previousRenderPass->fbo != fbo)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		}

		// check if previous pass used same fbo and drawbuffers
		bool updateDrawBuffers = _previousRenderPass && drawBuffers.size() != _previousRenderPass->drawBuffers.size();

		if (!updateDrawBuffers && _previousRenderPass)
		{
			for (unsigned int i = 0; i < drawBuffers.size(); ++i)
			{
				if (drawBuffers[i] != _previousRenderPass->drawBuffers[i])
				{
					updateDrawBuffers = true;
					break;
				}
			}
		}

		if (!_previousRenderPass || updateDrawBuffers || _previousRenderPass->fbo != fbo)
		{
			glDrawBuffers(drawBuffers.size(), drawBuffers.data());
		}
	}

	// viewport
	{
		if (!_previousRenderPass 
			|| _previousRenderPass->state.viewportState.x != state.viewportState.x
			|| _previousRenderPass->state.viewportState.y != state.viewportState.y
			|| _previousRenderPass->state.viewportState.width != state.viewportState.width
			|| _previousRenderPass->state.viewportState.height != state.viewportState.height)
		{
			glViewport(state.viewportState.x, state.viewportState.y, state.viewportState.width, state.viewportState.height);
		}
	}
}

void RenderPass::resize(unsigned int _width, unsigned int _height)
{
	state.viewportState = { 0, 0, static_cast<GLint>(_width), static_cast<GLint>(_height) };
}
