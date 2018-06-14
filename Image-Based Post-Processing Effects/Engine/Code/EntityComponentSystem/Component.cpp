#include "Component.h"
#include ".\..\Graphics\Mesh.h"
#include ".\..\Graphics\Material.h"


std::uint64_t BaseComponent::typeCount = 0;

const std::uint64_t BoundingBoxComponent::FAMILY_ID = BOUNDING_BOX_FAMILY;
const std::uint64_t CleanupComponent::FAMILY_ID = 64;
const std::uint64_t GrabbedComponent::FAMILY_ID = MOVEMENT_FAMILY;
const std::uint64_t ModelComponent::FAMILY_ID = MODEL_FAMILY;
const std::uint64_t MovementComponent::FAMILY_ID = MOVEMENT_FAMILY;
const std::uint64_t OutlineComponent::FAMILY_ID = OUTLINE_FAMILY;
const std::uint64_t PerpetualRotationComponent::FAMILY_ID = ROTATION_FAMILY;
const std::uint64_t RenderableComponent::FAMILY_ID = RENDERABLE_FAMILY;
const std::uint64_t RotationComponent::FAMILY_ID = ROTATION_FAMILY;
const std::uint64_t SoundComponent::FAMILY_ID = SOUND_FAMILY;
const std::uint64_t TextureAtlasIndexComponent::FAMILY_ID = TEXTURE_ATLAS_FAMILY;
const std::uint64_t TransformationComponent::FAMILY_ID = TRANSFORMATION_FAMILY;
const std::uint64_t TransparencyComponent::FAMILY_ID = TRANSPARENCY_FAMILY; 
const std::uint64_t MovementPathComponent::FAMILY_ID = MOVEMENT_FAMILY; 
const std::uint64_t CustomTransparencyShaderComponent::FAMILY_ID = TRANSPARENCY_SHADER_FAMILY;
const std::uint64_t CustomOpaqueShaderComponent::FAMILY_ID = OPAQUE_SHADER_FAMILY;
const std::uint64_t PhysicsComponent::FAMILY_ID = PHYSICS_FAMILY;