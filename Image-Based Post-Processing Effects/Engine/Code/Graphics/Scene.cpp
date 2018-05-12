#include "Scene.h"
#include <cassert>
#include <algorithm>
#include ".\..\EntityComponentSystem\Component.h"
#include ".\..\EntityComponentSystem\EntityManager.h"
#include "EntityRenderData.h"
#include ".\..\Graphics\Material.h"

const std::vector<std::unique_ptr<EntityRenderData>> &Scene::getData() const
{
	return meshEntityData;
}

void Scene::add(const Entity *_entity)
{
	assert(std::find_if(meshEntityData.begin(), meshEntityData.end(), [&](const std::unique_ptr<EntityRenderData> &_item) { return _item->entity == _entity; }) == meshEntityData.end());

	EntityManager &entityManager = EntityManager::getInstance();

	ModelComponent *mc = entityManager.getComponent<ModelComponent>(_entity);

	for (std::size_t i = 0; i < mc->model.size(); ++i)
	{
		std::unique_ptr<EntityRenderData> renderData = std::make_unique<EntityRenderData>();
		renderData->entity = _entity;
		renderData->mesh = mc->model[i].first;
		renderData->material = &mc->model[i].second;
		renderData->transformationComponent = entityManager.getComponent<TransformationComponent>(_entity);
		renderData->modelComponent = mc;
		renderData->outlineComponent = entityManager.getComponent<OutlineComponent>(_entity);
		renderData->transparencyComponent = entityManager.getComponent<TransparencyComponent>(_entity);
		renderData->textureAtlasIndexComponent = entityManager.getComponent<TextureAtlasIndexComponent>(_entity);
		renderData->customOpaqueShaderComponent = entityManager.getComponent<CustomOpaqueShaderComponent>(_entity);
		renderData->customTransparencyShaderComponent = entityManager.getComponent<CustomTransparencyShaderComponent>(_entity);

		if (renderData->transparencyComponent)
		{
			++transparencyCount;
		}
		if (renderData->outlineComponent)
		{
			++outlineCount;
		}
		if (renderData->customOpaqueShaderComponent)
		{
			++customOpaqueCount;
		}
		if (renderData->customTransparencyShaderComponent)
		{
			++customTransparencyCount;
		}

		meshEntityData.push_back(std::move(renderData));
	}
}

void Scene::remove(const Entity *_entity)
{
	auto start = std::remove_if(meshEntityData.begin(), meshEntityData.end(), [&](const std::unique_ptr<EntityRenderData> &_item) 
	{ 
		bool result = _item->entity == _entity;
		if (result)
		{
			if (_item->transparencyComponent)
			{
				--transparencyCount;
			}
			if (_item->outlineComponent)
			{
				--outlineCount;
			}
			if (_item->customOpaqueShaderComponent)
			{
				--customOpaqueCount;
			}
			if (_item->customTransparencyShaderComponent)
			{
				--customTransparencyCount;
			}
		}
		return result;
	});
	meshEntityData.erase(start, meshEntityData.end());
}

void Scene::update(const Entity *_entity)
{
	remove(_entity);
	add(_entity);
}

void Scene::sort()
{
	std::sort(meshEntityData.begin(), meshEntityData.end(), [](const std::unique_ptr<EntityRenderData> &_lhv, const std::unique_ptr<EntityRenderData> &_rhv) { return _lhv->mesh < _rhv->mesh; });
}

unsigned int Scene::getOutlineCount() const
{
	return outlineCount;
}

unsigned int Scene::getTransparencyCount() const
{
	return transparencyCount;
}

unsigned int Scene::getCustomOpaqueCount() const
{
	return customOpaqueCount;
}

unsigned int Scene::getCustomTransparencyCount() const
{
	return customTransparencyCount;
}
