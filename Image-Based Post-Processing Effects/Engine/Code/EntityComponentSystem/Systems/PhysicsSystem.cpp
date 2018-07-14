#include "PhysicsSystem.h"
#include ".\..\SystemManager.h"
#include ".\..\EntityManager.h"
#include <btBulletDynamicsCommon.h>
#include ".\..\..\Graphics\Mesh.h"

class MotionState : public btMotionState
{
public:
	explicit MotionState(TransformationComponent *_tc)
		:tc(_tc)
	{

	}

	void setWorldTransform(const btTransform &_worldTrans) override
	{
		const btVector3 &origin = _worldTrans.getOrigin();
		tc->position.x = origin.getX();
		tc->position.y = origin.getY();
		tc->position.z = origin.getZ();

		btQuaternion rotation = _worldTrans.getRotation();
		tc->rotation.x = rotation.getX();
		tc->rotation.y = rotation.getY();
		tc->rotation.z = rotation.getZ();
		tc->rotation.w = rotation.getW();
	}

	void getWorldTransform(btTransform &_worldTrans) const override
	{
		btVector3 &origin = _worldTrans.getOrigin();
		origin.setX(tc->position.x);
		origin.setY(tc->position.y);
		origin.setZ(tc->position.z);

		btQuaternion rotation(tc->rotation.x, tc->rotation.y, tc->rotation.z, tc->rotation.w);

		_worldTrans.setRotation(rotation);
	}

private:
	TransformationComponent *tc;

};

PhysicsSystem::PhysicsSystem()
	:entityManager(EntityManager::getInstance())
{
	validBitMaps.push_back(Component<PhysicsComponent>::getTypeId() | Component<TransformationComponent>::getTypeId() | Component<ModelComponent>::getTypeId());
}

void PhysicsSystem::init()
{
	entityManager.addOnComponentAddedListener(this);
	entityManager.addOnComponentRemovedListener(this);
	entityManager.addOnEntityDestructionListener(this);

	broadphase = new btDbvtBroadphase();

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
}

void PhysicsSystem::input(double _currentTime, double _timeDelta)
{
}

void PhysicsSystem::update(double _currentTime, double _timeDelta)
{

	for (const Entity *entity : entitiesToRemove)
	{
		ContainerUtility::remove(managedEntities, entity);
	}
	entitiesToRemove.clear();

	for (const Entity *entity : entitiesToAdd)
	{
		managedEntities.push_back(entity);

		PhysicsComponent *pc = entityManager.getComponent<PhysicsComponent>(entity);
		ModelComponent *mc = entityManager.getComponent<ModelComponent>(entity);
		TransformationComponent *tc = entityManager.getComponent<TransformationComponent>(entity);

		pc->motionState = new MotionState(tc);

		if (pc->dynamic)
		{
			if (pc->sphere)
			{
				pc->collisionShape = new btSphereShape(tc->scale.x);
			}
			else
			{
				btConvexHullShape *hull = new btConvexHullShape();

				for (std::size_t i = 0; i < mc->model.size(); ++i)
				{
					const std::vector<glm::vec3> &vertices = mc->model[i].first->getVertices();

					for (std::size_t j = 0; j < vertices.size(); ++j)
					{
						const glm::vec3 &v = vertices[j];
						hull->addPoint(btVector3(v.x, v.y, v.z), false);
					}
				}
				hull->recalcLocalAabb();
				pc->collisionShape = hull;
			}

		}
		else
		{
			btTriangleIndexVertexArray *triArr = new btTriangleIndexVertexArray();

			for (std::size_t i = 0; i < mc->model.size(); ++i)
			{
				const std::vector<glm::vec3> &vertices = mc->model[i].first->getVertices();
				const std::vector<std::uint32_t> &indices = mc->model[i].first->getIndices();

				btIndexedMesh indexedMesh;
				indexedMesh.m_numTriangles = indices.size() / 3;
				indexedMesh.m_triangleIndexBase = (unsigned char *)indices.data();
				indexedMesh.m_triangleIndexStride = sizeof(std::uint32_t) * 3;
				indexedMesh.m_numVertices = vertices.size();
				indexedMesh.m_vertexBase = (unsigned char *)vertices.data();
				indexedMesh.m_vertexStride = sizeof(glm::vec3);

				triArr->addIndexedMesh(indexedMesh);
			}


			btBvhTriangleMeshShape *triShape = new btBvhTriangleMeshShape(triArr, false);
			pc->collisionShape = new btScaledBvhTriangleMeshShape(triShape, btVector3(tc->scale.x, tc->scale.y, tc->scale.z));
		}


		btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo(pc->dynamic ? pc->mass : 0.0f, pc->motionState, pc->collisionShape);
		btRigidBody *rigidBody = new btRigidBody(rigidBodyInfo);
		rigidBody->setRestitution(pc->restitution);
		rigidBody->setFriction(0.1f);

		if (pc->kinematic)
		{
			rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			rigidBody->setActivationState(DISABLE_DEACTIVATION);
		}

		pc->rigidBody = rigidBody;

		dynamicsWorld->addRigidBody(rigidBody);
	}
	entitiesToAdd.clear();

	dynamicsWorld->stepSimulation(_timeDelta);
}

void PhysicsSystem::render()
{
}

void PhysicsSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(managedEntities, _entity) || ContainerUtility::contains(entitiesToRemove, _entity))
		{
			entitiesToAdd.push_back(_entity);
		}
	}
}

void PhysicsSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

void PhysicsSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

bool PhysicsSystem::validate(std::uint64_t _bitMap)
{
	for (std::uint64_t configuration : validBitMaps)
	{
		if ((configuration & _bitMap) == configuration)
		{
			return true;
		}
	}
	return false;
}
