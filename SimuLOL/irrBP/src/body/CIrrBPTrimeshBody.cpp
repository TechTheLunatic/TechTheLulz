#include "body/CIrrBPTrimeshBody.h"
CIrrBPTrimesh::CIrrBPTrimesh(IAnimatedMeshSceneNode * node,irr::f32 mass, irr::s32 bodyId)
{
	m_IrrSceneNode = node;
	m_BodyId = bodyId;
	initializeMesh(node->getMesh(),node->getPosition(),node->getScale(),node,mass);
}
CIrrBPTrimesh::CIrrBPTrimesh(IMeshSceneNode * node,irr::f32 mass, irr::s32 bodyId)
{
	m_IrrSceneNode = node;
	m_BodyId = bodyId;
	initializeMesh(node->getMesh(),node->getPosition(),node->getScale(),node,mass);
}


void CIrrBPTrimesh::initializeMesh(IMesh * mesh, const vector3df & pos, const vector3df & scale, void * nodePtr,irr::f32 mass)
{
	m_indexVertexArrays = irrMeshToBulletTriangleMesh(mesh,scale);
	m_trimeshShape = new btGImpactMeshShape(m_indexVertexArrays);




	m_MotionState = new CMotionState(this,getTransformFromIrrlichtNode(m_IrrSceneNode));		
	m_trimeshShape->updateBound();
	
	//Calculate Inertia
	btVector3 LocalInertia;
    m_trimeshShape->calculateLocalInertia(mass, LocalInertia);

	m_RigidBody = new btRigidBody(mass, m_MotionState, m_trimeshShape,LocalInertia);
	
    m_RigidBody->setUserPointer(nodePtr);
	//m_RigidBody->setCollisionFlags( m_RigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	//m_RigidBody->setActivationState(DISABLE_DEACTIVATION);
	collisionObj = m_RigidBody;
	setAutomaticCCD();
	
}

CIrrBPTrimesh::~CIrrBPTrimesh()
{

	delete this->m_trimeshShape;
	this->m_trimeshShape = NULL;
	delete this->m_indexVertexArrays;
	this->m_indexVertexArrays = NULL;
	delete this->m_MotionState;
	this->m_MotionState = NULL;
	delete this->m_RigidBody;
	this->m_RigidBody = NULL;

}