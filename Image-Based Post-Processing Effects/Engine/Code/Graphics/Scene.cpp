#include "Scene.h"
#include <cassert>
#include <algorithm>
#include ".\..\EntityComponentSystem\Component.h"
#include ".\..\EntityComponentSystem\EntityManager.h"
#include "EntityRenderData.h"
#include ".\..\Graphics\Material.h"

const std::vector<std::unique_ptr<EntityRenderData>> &Scene::getData() const
{
	return m_meshEntityData;
}

void Scene::add(const Entity *_entity)
{
	assert(std::find_if(m_meshEntityData.begin(), m_meshEntityData.end(), [&](const std::unique_ptr<EntityRenderData> &_item) { return _item->m_entity == _entity; }) == m_meshEntityData.end());

	EntityManager &entityManager = EntityManager::getInstance();

	ModelComponent *mc = entityManager.getComponent<ModelComponent>(_entity);

	for (std::size_t i = 0; i < mc->m_model.size(); ++i)
	{
		std::unique_ptr<EntityRenderData> renderData = std::make_unique<EntityRenderData>();
		renderData->m_entity = _entity;
		renderData->m_mesh = mc->m_model[i].first;
		renderData->m_material = &mc->m_model[i].second;
		renderData->m_transformationComponent = entityManager.getComponent<TransformationComponent>(_entity);
		renderData->m_modelComponent = mc;
		renderData->m_outlineComponent = entityManager.getComponent<OutlineComponent>(_entity);
		renderData->m_transparencyComponent = entityManager.getComponent<TransparencyComponent>(_entity);
		renderData->m_textureAtlasIndexComponent = entityManager.getComponent<TextureAtlasIndexComponent>(_entity);
		renderData->m_customOpaqueShaderComponent = entityManager.getComponent<CustomOpaqueShaderComponent>(_entity);
		renderData->m_customTransparencyShaderComponent = entityManager.getComponent<CustomTransparencyShaderComponent>(_entity);

		if (renderData->m_transparencyComponent)
		{
			++m_transparencyCount;
		}
		if (renderData->m_outlineComponent)
		{
			++m_outlineCount;
		}
		if (renderData->m_customOpaqueShaderComponent)
		{
			++m_customOpaqueCount;
		}
		if (renderData->m_customTransparencyShaderComponent)
		{
			++m_customTransparencyCount;
		}

		m_meshEntityData.push_back(std::move(renderData));
	}
}

void Scene::remove(const Entity *_entity)
{
	auto start = std::remove_if(m_meshEntityData.begin(), m_meshEntityData.end(), [&](const std::unique_ptr<EntityRenderData> &_item) 
	{ 
		bool result = _item->m_entity == _entity;
		if (result)
		{
			if (_item->m_transparencyComponent)
			{
				--m_transparencyCount;
			}
			if (_item->m_outlineComponent)
			{
				--m_outlineCount;
			}
			if (_item->m_customOpaqueShaderComponent)
			{
				--m_customOpaqueCount;
			}
			if (_item->m_customTransparencyShaderComponent)
			{
				--m_customTransparencyCount;
			}
		}
		return result;
	});
	m_meshEntityData.erase(start, m_meshEntityData.end());
}

void Scene::update(const Entity *_entity)
{
	remove(_entity);
	add(_entity);
}

void Scene::sort()
{
	std::sort(m_meshEntityData.begin(), m_meshEntityData.end(), [](const std::unique_ptr<EntityRenderData> &_lhv, const std::unique_ptr<EntityRenderData> &_rhv) { return _lhv->m_mesh < _rhv->m_mesh; });
}

size_t Scene::getOutlineCount() const
{
	return m_outlineCount;
}

size_t Scene::getTransparencyCount() const
{
	return m_transparencyCount;
}

size_t Scene::getCustomOpaqueCount() const
{
	return m_customOpaqueCount;
}

size_t Scene::getCustomTransparencyCount() const
{
	return m_customTransparencyCount;
}
