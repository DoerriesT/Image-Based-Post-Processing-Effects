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
	const Entity *entity;
	std::shared_ptr<SubMesh> mesh;
	Material *material;
	TransformationComponent *transformationComponent;
	ModelComponent *modelComponent;
	OutlineComponent *outlineComponent;
	TransparencyComponent *transparencyComponent;
	TextureAtlasIndexComponent *textureAtlasIndexComponent;
	CustomOpaqueShaderComponent *customOpaqueShaderComponent;
	CustomTransparencyShaderComponent *customTransparencyShaderComponent;
};