#include "body/CIrrBPRigidBody.h"
#include "animator/CIrrBPAnimator.h"

CIrrBPRigidBody::CIrrBPRigidBody()
{
	m_objType = RIGID_BODY;
	kinematic = false;
	m_Shape = NULL;
	m_MotionState = NULL;
	m_RigidBody = NULL;
}
CIrrBPRigidBody::~CIrrBPRigidBody()
{
	if(m_Shape)
	delete m_Shape;
	
	m_Shape = NULL;

	if(m_MotionState)
	delete m_MotionState;

	m_MotionState = NULL;

	if(m_RigidBody)
	delete m_RigidBody;
	
	m_RigidBody = NULL;
}
void CIrrBPRigidBody::applyCentralImpulse(const irr::core::vector3df &impulse)
{
	m_RigidBody->applyCentralImpulse(irrVectorToBulletVector(impulse));
}
void CIrrBPRigidBody::applyCentralForce(const vector3df &force)
{
	m_RigidBody->applyCentralForce(irrVectorToBulletVector(force));
}
void  CIrrBPRigidBody::applyForce (const vector3df &force, const vector3df &rel_pos)
{
	m_RigidBody->applyForce(irrVectorToBulletVector(force),irrVectorToBulletVector(rel_pos));
}
void  CIrrBPRigidBody::applyTorqueImpulse (const vector3df &torque)
{
	m_RigidBody->applyTorqueImpulse(irrVectorToBulletVector(torque));
}
void  CIrrBPRigidBody::applyImpulse (const vector3df &impulse, const vector3df &rel_pos) 
{
	m_RigidBody->applyImpulse(irrVectorToBulletVector(impulse),irrVectorToBulletVector(rel_pos));
}
void CIrrBPRigidBody::applyTorque (const vector3df &torque)
{
	m_RigidBody->applyTorque(irrVectorToBulletVector(torque));
}

irr::f32 CIrrBPRigidBody::getAutomaticCCDSSR()
{
	vector3df realExt = m_IrrSceneNode->getBoundingBox().getExtent() * m_IrrSceneNode->getScale();
	irr::f32 ccdSSR = 0.0f;
	irr::f32 tolerance = 2.0f;
	if(realExt.X < realExt.Y && realExt.X < realExt.Z)
		ccdSSR = realExt.X / tolerance;
	else if(realExt.Y < realExt.X && realExt.Y < realExt.Z)
		ccdSSR = realExt.Y / tolerance;
	else
		ccdSSR = realExt.Z / tolerance;
	return ccdSSR;
}

irr::f32 CIrrBPRigidBody::getAutomaticCCDMT()
{
	vector3df realExt = m_IrrSceneNode->getBoundingBox().getExtent() * m_IrrSceneNode->getScale();
	irr::f32 ccdMT = 0.0f;
	irr::f32 tolerance = 1.2f;
	if(realExt.X < realExt.Y && realExt.X < realExt.Z)
		ccdMT = realExt.X / tolerance;
	else if(realExt.Y < realExt.X && realExt.Y < realExt.Z)
		ccdMT = realExt.Y / tolerance;
	else
		ccdMT = realExt.Z / tolerance;
	return ccdMT;
}


void CIrrBPRigidBody::setAutomaticCCD()
{
	this->setCcdMotionThreshold(this->getAutomaticCCDMT());
	this->setCcdSweptSphereRadius(this->getAutomaticCCDSSR());
}

vector3df CIrrBPRigidBody::getPosition()
{
	return bulletVectorToIrrVector(m_RigidBody->getWorldTransform().getOrigin());
}

void CIrrBPRigidBody::setPosition(const vector3df & newPos)
{
	m_IrrSceneNode->setPosition(newPos);
	btTransform  tr = m_RigidBody->getWorldTransform();
	tr.setOrigin(irrVectorToBulletVector(newPos));	
	m_RigidBody->setWorldTransform(tr);
}

vector3df CIrrBPRigidBody::getRotation()
{
	return bulletTransformToIrrRotation(m_RigidBody->getWorldTransform());
}

void CIrrBPRigidBody::setRotation(const vector3df & newPos)
{
	m_IrrSceneNode->setRotation(newPos);
	btTransform bt;
	bt = irrRotationToBulletTransform(newPos);
	bt.setOrigin(m_RigidBody->getWorldTransform().getOrigin());
	
	m_RigidBody->setWorldTransform(bt);
}

void CIrrBPRigidBody::setKinematic(bool isKinematic)
{
	kinematic = isKinematic;
	if(kinematic)
		this->m_RigidBody->setCollisionFlags(m_RigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	else
		this->m_RigidBody->setCollisionFlags(m_RigidBody->getCollisionFlags() ^ btCollisionObject::CF_KINEMATIC_OBJECT);

}
bool CIrrBPRigidBody::isKinematic()
{
	return kinematic;
}
void CIrrBPRigidBody::setDeactivation(bool value)
{
	deactivate = value;
	
	if(value)
		m_RigidBody->setActivationState(WANTS_DEACTIVATION);
	else
		m_RigidBody->setActivationState(DISABLE_DEACTIVATION);
}