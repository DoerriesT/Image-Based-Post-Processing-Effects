#include "PhysicsSystem.h"
#include ".\..\SystemManager.h"
#include ".\..\EntityManager.h"
#include <btBulletDynamicsCommon.h>
#include ".\..\..\Graphics\Mesh.h"

class MotionState : public btMotionState
{
public:
	explicit MotionState(TransformationComponent *_tc)
		:m_tc(_tc)
	{

	}

	void setWorldTransform(const btTransform &_worldTrans) override
	{
		const btVector3 &origin = _worldTrans.getOrigin();
		m_tc->m_position.x = origin.getX();
		m_tc->m_position.y = origin.getY();
		m_tc->m_position.z = origin.getZ();

		btQuaternion rotation = _worldTrans.getRotation();
		m_tc->m_rotation.x = rotation.getX();
		m_tc->m_rotation.y = rotation.getY();
		m_tc->m_rotation.z = rotation.getZ();
		m_tc->m_rotation.w = rotation.getW();
	}

	void getWorldTransform(btTransform &_worldTrans) const override
	{
		btVector3 &origin = _worldTrans.getOrigin();
		origin.setX(m_tc->m_position.x);
		origin.setY(m_tc->m_position.y);
		origin.setZ(m_tc->m_position.z);

		btQuaternion rotation(m_tc->m_rotation.x, m_tc->m_rotation.y, m_tc->m_rotation.z, m_tc->m_rotation.w);

		_worldTrans.setRotation(rotation);
	}

private:
	TransformationComponent *m_tc;

};

PhysicsSystem::PhysicsSystem()
	:m_entityManager(EntityManager::getInstance())
{
	m_validBitMaps.push_back(Component<PhysicsComponent>::getTypeId() | Component<TransformationComponent>::getTypeId() | Component<ModelComponent>::getTypeId());
}

void PhysicsSystem::init()
{
	m_entityManager.addOnComponentAddedListener(this);
	m_entityManager.addOnComponentRemovedListener(this);
	m_entityManager.addOnEntityDestructionListener(this);

	m_broadphase = new btDbvtBroadphase();

	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	m_solver = new btSequentialImpulseConstraintSolver;

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

	m_dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
}

void PhysicsSystem::input(double _currentTime, double _timeDelta)
{
}

void PhysicsSystem::update(double _currentTime, double _timeDelta)
{

	for (const Entity *entity : m_entitiesToRemove)
	{
		ContainerUtility::remove(m_managedEntities, entity);
	}
	m_entitiesToRemove.clear();

	for (const Entity *entity : m_entitiesToAdd)
	{
		m_managedEntities.push_back(entity);

		PhysicsComponent *pc = m_entityManager.getComponent<PhysicsComponent>(entity);
		ModelComponent *mc = m_entityManager.getComponent<ModelComponent>(entity);
		TransformationComponent *tc = m_entityManager.getComponent<TransformationComponent>(entity);

		pc->m_motionState = new MotionState(tc);

		if (pc->m_dynamic)
		{
			if (pc->m_sphere)
			{
				pc->m_collisionShape = new btSphereShape(tc->m_scale.x);
			}
			else
			{
				btConvexHullShape *hull = new btConvexHullShape();

				for (std::size_t i = 0; i < mc->m_model.size(); ++i)
				{
					const std::vector<glm::vec3> &vertices = mc->m_model[i].first->getVertices();

					for (std::size_t j = 0; j < vertices.size(); ++j)
					{
						const glm::vec3 &v = vertices[j];
						hull->addPoint(btVector3(v.x, v.y, v.z), false);
					}
				}
				hull->recalcLocalAabb();
				pc->m_collisionShape = hull;
			}

		}
		else
		{
			btTriangleIndexVertexArray *triArr = new btTriangleIndexVertexArray();

			for (std::size_t i = 0; i < mc->m_model.size(); ++i)
			{
				const std::vector<glm::vec3> &vertices = mc->m_model[i].first->getVertices();
				const std::vector<std::uint32_t> &indices = mc->m_model[i].first->getIndices();

				btIndexedMesh indexedMesh;
				indexedMesh.m_numTriangles = static_cast<int>(indices.size()) / 3;
				indexedMesh.m_triangleIndexBase = (unsigned char *)indices.data();
				indexedMesh.m_triangleIndexStride = sizeof(std::uint32_t) * 3;
				indexedMesh.m_numVertices = static_cast<int>(vertices.size());
				indexedMesh.m_vertexBase = (unsigned char *)vertices.data();
				indexedMesh.m_vertexStride = sizeof(glm::vec3);

				triArr->addIndexedMesh(indexedMesh);
			}


			btBvhTriangleMeshShape *triShape = new btBvhTriangleMeshShape(triArr, false);
			pc->m_collisionShape = new btScaledBvhTriangleMeshShape(triShape, btVector3(tc->m_scale.x, tc->m_scale.y, tc->m_scale.z));
		}


		btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo(pc->m_dynamic ? pc->m_mass : 0.0f, pc->m_motionState, pc->m_collisionShape);
		btRigidBody *rigidBody = new btRigidBody(rigidBodyInfo);
		rigidBody->setRestitution(pc->m_restitution);
		rigidBody->setFriction(0.1f);

		if (pc->m_kinematic)
		{
			rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			rigidBody->setActivationState(DISABLE_DEACTIVATION);
		}

		pc->m_rigidBody = rigidBody;

		m_dynamicsWorld->addRigidBody(rigidBody);
	}
	m_entitiesToAdd.clear();

	m_dynamicsWorld->stepSimulation(_timeDelta);
}

void PhysicsSystem::render()
{
}

void PhysicsSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(m_entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(m_entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(m_managedEntities, _entity) || ContainerUtility::contains(m_entitiesToRemove, _entity))
		{
			m_entitiesToAdd.push_back(_entity);
		}
	}
}

void PhysicsSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(m_entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

void PhysicsSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

bool PhysicsSystem::validate(std::uint64_t _bitMap)
{
	for (std::uint64_t configuration : m_validBitMaps)
	{
		if ((configuration & _bitMap) == configuration)
		{
			return true;
		}
	}
	return false;
}
