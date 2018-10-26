#include "RenderPass.h"

void RenderPass::begin(const RenderPass *_previousRenderPass)
{
	// depth
	{
		if (!_previousRenderPass || _previousRenderPass->m_state.m_depthState.m_enabled != m_state.m_depthState.m_enabled)
		{
			m_state.m_depthState.m_enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		}
		if (!_previousRenderPass || _previousRenderPass->m_state.m_depthState.m_mask != m_state.m_depthState.m_mask)
		{
			glDepthMask(m_state.m_depthState.m_mask);
		}
		if (!_previousRenderPass || _previousRenderPass->m_state.m_depthState.m_func != m_state.m_depthState.m_func)
		{
			glDepthFunc(m_state.m_depthState.m_func);
		}
	}

	// stencil
	{
		if (!_previousRenderPass || _previousRenderPass->m_state.m_stencilState.m_enabled != m_state.m_stencilState.m_enabled)
		{
			m_state.m_stencilState.m_enabled ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
		}
		//if (state.stencilState.enabled)
		{
			if (!_previousRenderPass
				|| _previousRenderPass->m_state.m_stencilState.m_frontFunc != m_state.m_stencilState.m_frontFunc
				|| _previousRenderPass->m_state.m_stencilState.m_frontRef != m_state.m_stencilState.m_frontRef
				|| _previousRenderPass->m_state.m_stencilState.m_frontMask != m_state.m_stencilState.m_frontMask)
			{
				glStencilFuncSeparate(GL_FRONT, m_state.m_stencilState.m_frontFunc, m_state.m_stencilState.m_frontRef, m_state.m_stencilState.m_frontMask);
			}
			if (!_previousRenderPass
				|| _previousRenderPass->m_state.m_stencilState.m_backFunc != m_state.m_stencilState.m_backFunc
				|| _previousRenderPass->m_state.m_stencilState.m_backRef != m_state.m_stencilState.m_backRef
				|| _previousRenderPass->m_state.m_stencilState.m_backMask != m_state.m_stencilState.m_backMask)
			{
				glStencilFuncSeparate(GL_BACK, m_state.m_stencilState.m_backFunc, m_state.m_stencilState.m_backRef, m_state.m_stencilState.m_backMask);
			}

			if (!_previousRenderPass
				|| _previousRenderPass->m_state.m_stencilState.m_frontOpFail != m_state.m_stencilState.m_frontOpFail
				|| _previousRenderPass->m_state.m_stencilState.m_frontOpZfail != m_state.m_stencilState.m_frontOpZfail
				|| _previousRenderPass->m_state.m_stencilState.m_frontOpZpass != m_state.m_stencilState.m_frontOpZpass)
			{
				glStencilOpSeparate(GL_FRONT, m_state.m_stencilState.m_frontOpFail, m_state.m_stencilState.m_frontOpZfail, m_state.m_stencilState.m_frontOpZpass);
			}

			if (!_previousRenderPass
				|| _previousRenderPass->m_state.m_stencilState.m_backOpFail != m_state.m_stencilState.m_backOpFail
				|| _previousRenderPass->m_state.m_stencilState.m_backOpZfail != m_state.m_stencilState.m_backOpZfail
				|| _previousRenderPass->m_state.m_stencilState.m_backOpZpass != m_state.m_stencilState.m_backOpZpass)
			{
				glStencilOpSeparate(GL_BACK, m_state.m_stencilState.m_backOpFail, m_state.m_stencilState.m_backOpZfail, m_state.m_stencilState.m_backOpZpass);
			}
		}
	}

	// cull face
	{
		if (!_previousRenderPass || _previousRenderPass->m_state.m_cullFaceState.m_enabled != m_state.m_cullFaceState.m_enabled)
		{
			m_state.m_cullFaceState.m_enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		}
		//if (state.cullFaceState.enabled)
		{
			if (!_previousRenderPass || _previousRenderPass->m_state.m_cullFaceState.m_face != m_state.m_cullFaceState.m_face)
			{
				glCullFace(m_state.m_cullFaceState.m_face);
			}
		}
		
	}

	// blend
	{
		if (!_previousRenderPass || _previousRenderPass->m_state.m_blendState.m_enabled != m_state.m_blendState.m_enabled)
		{
			m_state.m_blendState.m_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		}

		//if (state.blendState.enabled)
		{
			if (!_previousRenderPass
				|| _previousRenderPass->m_state.m_blendState.m_sFactor != m_state.m_blendState.m_sFactor
				|| _previousRenderPass->m_state.m_blendState.m_dFactor != m_state.m_blendState.m_dFactor)
			{
				glBlendFunc(m_state.m_blendState.m_sFactor, m_state.m_blendState.m_dFactor);
			}
		}
	}

	// fbo
	{
		if (!_previousRenderPass || _previousRenderPass->m_fbo != m_fbo)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		}

		// check if previous pass used same fbo and drawbuffers
		bool updateDrawBuffers = _previousRenderPass && m_drawBuffers.size() != _previousRenderPass->m_drawBuffers.size();

		if (!updateDrawBuffers && _previousRenderPass)
		{
			for (size_t i = 0; i < m_drawBuffers.size(); ++i)
			{
				if (m_drawBuffers[i] != _previousRenderPass->m_drawBuffers[i])
				{
					updateDrawBuffers = true;
					break;
				}
			}
		}

		if (!_previousRenderPass || updateDrawBuffers || _previousRenderPass->m_fbo != m_fbo)
		{
			glDrawBuffers(static_cast<GLsizei>(m_drawBuffers.size()), m_drawBuffers.data());
		}
	}

	// viewport
	{
		if (!_previousRenderPass 
			|| _previousRenderPass->m_state.m_viewportState.m_x != m_state.m_viewportState.m_x
			|| _previousRenderPass->m_state.m_viewportState.m_y != m_state.m_viewportState.m_y
			|| _previousRenderPass->m_state.m_viewportState.m_width != m_state.m_viewportState.m_width
			|| _previousRenderPass->m_state.m_viewportState.m_height != m_state.m_viewportState.m_height)
		{
			glViewport(m_state.m_viewportState.m_x, m_state.m_viewportState.m_y, m_state.m_viewportState.m_width, m_state.m_viewportState.m_height);
		}
	}
}

void RenderPass::resize(unsigned int _width, unsigned int _height)
{
	m_state.m_viewportState = { 0, 0, static_cast<GLint>(_width), static_cast<GLint>(_height) };
}
