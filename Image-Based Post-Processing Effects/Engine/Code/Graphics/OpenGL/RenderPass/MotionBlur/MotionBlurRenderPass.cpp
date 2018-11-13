#include "MotionBlurRenderPass.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

static const char *MOTION_BLUR = "MOTION_BLUR";

MotionBlurRenderPass::MotionBlurRenderPass(GLuint fbo, unsigned int width, unsigned int height)
{
	m_fbo = fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT3 };
	m_state.m_blendState.m_enabled = false;
	m_state.m_blendState.m_sFactor = GL_ONE;
	m_state.m_blendState.m_dFactor = GL_ONE;
	m_state.m_cullFaceState.m_enabled = false;
	m_state.m_cullFaceState.m_face = GL_BACK;
	m_state.m_depthState.m_enabled = false;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_FALSE;
	m_state.m_stencilState.m_enabled = false;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_ALWAYS;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 1;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = m_state.m_stencilState.m_backOpZfail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(width, height);

	m_motionBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/MotionBlur/motionBlur.frag");

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

double motionBlurRenderTime;

void MotionBlurRenderPass::render(int motionBlurType, RenderPass **previousRenderPass)
{
	SCOPED_TIMER_QUERY(motionBlurRenderTime);
	RenderPass::begin(*previousRenderPass);
	*previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	// shader permutations
	{
		const auto curDefines = m_motionBlurShader->getDefines();

		int motionBlur = 0;

		for (const auto &define : curDefines)
		{
			if (std::get<0>(define) == ShaderProgram::ShaderType::FRAGMENT)
			{
				if (std::get<1>(define) == MOTION_BLUR)
				{
					motionBlur = std::get<2>(define);
				}
			}
		}

		if (motionBlur != motionBlurType)
		{
			m_motionBlurShader->setDefines({{ ShaderProgram::ShaderType::FRAGMENT, MOTION_BLUR, motionBlurType }});
		}
	}

	m_motionBlurShader->bind();

	m_fullscreenTriangle->getSubMesh()->render();
}
