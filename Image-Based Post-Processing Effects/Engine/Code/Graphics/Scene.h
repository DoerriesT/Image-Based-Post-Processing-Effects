#pragma once
#include <vector>
#include <memory>

class ShaderProgram;
class SubMesh;
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
	size_t getOutlineCount() const;
	size_t getTransparencyCount() const;
	size_t getCustomOpaqueCount() const;
	size_t getCustomTransparencyCount() const;

private:
	std::vector<std::unique_ptr<EntityRenderData>> m_meshEntityData;
	size_t m_outlineCount;
	size_t m_transparencyCount;
	size_t m_customOpaqueCount;
	size_t m_customTransparencyCount;
};