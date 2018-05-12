#pragma once
#include <vector>
#include <memory>

class ShaderProgram;
class Mesh;
struct EntityRenderData;
struct Entity;

class Scene
{
public:
	Scene() = default;
	const std::vector<std::unique_ptr<EntityRenderData>> &getData() const;
	void add(const Entity *_entity);
	void remove(const Entity *_entity);
	void update(const Entity *_entity);
	void sort();
	unsigned int getOutlineCount() const;
	unsigned int getTransparencyCount() const;
	unsigned int getCustomOpaqueCount() const;
	unsigned int getCustomTransparencyCount() const;

private:
	std::vector<std::unique_ptr<EntityRenderData>> meshEntityData;
	unsigned int outlineCount;
	unsigned int transparencyCount;
	unsigned int customOpaqueCount;
	unsigned int customTransparencyCount;
};