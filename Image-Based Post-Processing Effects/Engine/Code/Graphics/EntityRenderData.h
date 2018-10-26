#pragma once
#include <memory>

struct TransformationComponent;
struct OutlineComponent;
struct TextureAtlasIndexComponent;
struct CustomOpaqueShaderComponent;
struct CustomTransparencyShaderComponent;
struct TransparencyComponent;
struct ModelComponent;
struct Entity;
class SubMesh;
class Material;

struct EntityRenderData
{
	const Entity *m_entity;
	std::shared_ptr<SubMesh> m_mesh;
	Material *m_material;
	TransformationComponent *m_transformationComponent;
	ModelComponent *m_modelComponent;
	OutlineComponent *m_outlineComponent;
	TransparencyComponent *m_transparencyComponent;
	TextureAtlasIndexComponent *m_textureAtlasIndexComponent;
	CustomOpaqueShaderComponent *m_customOpaqueShaderComponent;
	CustomTransparencyShaderComponent *m_customTransparencyShaderComponent;
};