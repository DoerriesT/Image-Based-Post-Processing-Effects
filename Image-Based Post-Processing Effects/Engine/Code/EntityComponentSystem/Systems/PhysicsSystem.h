#pragma once
#include <vector>
#include ".\..\IOnComponentAddedListener.h"
#include ".\..\IOnComponentRemovedListener.h"
#include ".\..\IOnEntityDestructionListener.h"
#include ".\..\System.h"
#include <memory>

class btDiscreteDynamicsWorld;
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class EntityManager;

class PhysicsSystem : public System<PhysicsSystem>, IOnComponentAddedListener, IOnComponentRemovedListener, IOnEntityDestructionListener
{
public:
	explicit PhysicsSystem();
	void init() override;
	void input(double _currentTime, double _timeDelta) override;
	void update(double _currentTime, double _timeDelta) override;
	void render() override;
	void onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent) override;
	void onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent) override;
	void onDestruction(const Entity *_entity) override;

private:
	EntityManager &entityManager;
	std::vector<const Entity *> managedEntities;
	std::vector<const Entity *> entitiesToRemove;
	std::vector<const Entity *> entitiesToAdd;
	std::vector<std::uint64_t> validBitMaps;

	// TODO: do proper cleanup of these
	btDiscreteDynamicsWorld *dynamicsWorld;
	btBroadphaseInterface *broadphase;
	btDefaultCollisionConfiguration *collisionConfiguration;
	btCollisionDispatcher *dispatcher;
	btSequentialImpulseConstraintSolver *solver;

	bool validate(std::uint64_t _bitMap);
};