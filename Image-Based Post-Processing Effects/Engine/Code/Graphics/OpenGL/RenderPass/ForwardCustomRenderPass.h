#pragma once
#include "RenderPass.h"
#include <memory>

struct RenderData;
class Scene;
struct Level;

class ForwardCustomRenderPass : public RenderPass
{
public:
	explicit ForwardCustomRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene, RenderPass **_previousRenderPass = nullptr);

};
