#include "CIrrBPWorld.h"

struct collisionCallback : public btCollisionWorld::ContactResultCallback
{
	bool collided;
	//Position World On First Body
	btVector3 pos;
	//Position World On Second Body
	btVector3 pos2;
	btCollisionObject* obj;
	btCollisionObject* obj2;
   collisionCallback(btCollisionObject* colObj,btCollisionObject* colObj2 = NULL)
   {
		collided=false;
		obj = colObj;
		obj2 = colObj2;
   }
   virtual   btScalar   addSingleResult(btManifoldPoint& cp,   const btCollisionObject* colObj0,int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1)
   {
	 if(obj2 == NULL)
	 {
		if(colObj0 == obj)
		{
			pos = cp.getPositionWorldOnA();
			pos2 = cp.getPositionWorldOnB();
		}
		else
		{
			pos = cp.getPositionWorldOnB();
			pos2 = cp.getPositionWorldOnA();
		}
	 }
	 else
	 {
		 pos = cp.getPositionWorldOnA();
		 pos2 = cp.getPositionWorldOnB();
	 }
	 collided=true;
     return 0;
   }
};

CIrrBPWorld::~CIrrBPWorld()
{
	isClosing=true;
	cout<<"# Cleaning IrrBP' pointers..."<<endl;
	m_worldInfo.m_sparsesdf.GarbageCollect();
	m_worldInfo.m_sparsesdf.Reset();

	clear();

	if(dDrawer)
		delete dDrawer;
	delete World;
	delete constraintSolver;
	delete pairCache;
	delete dispatcher;
	delete CollisionConfiguration;


	cout<<"# IrrBP closed successfully!"<<endl;

}
CIrrBPWorld::CIrrBPWorld(irr::IrrlichtDevice *device,const vector3df & Gravity)
{
	cout<<"# # # IrrBP - Version "<<IrrBP_MAJOR_VERSION<<"."<<IrrBP_MINOR_VERSION<<"."<<IrrBP_REVISION_VERSION<<" # # #"<<endl;
	cout<<"# Initializing new Irr-Bullet World..."<<endl;
	this->device = device;
	this->driver = device->getVideoDriver();

	this->Gravity = irrVectorToBulletVector(Gravity);

	CollisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
	//CollisionConfiguration->setConvexConvexMultipointIterations();
	dispatcher = new btCollisionDispatcher(CollisionConfiguration);


	pairCache = new btDbvtBroadphase();

	constraintSolver = new btSequentialImpulseConstraintSolver();

    World = new btSoftRigidDynamicsWorld(dispatcher, pairCache,
        constraintSolver, CollisionConfiguration);

	btGImpactCollisionAlgorithm::registerAlgorithm(/*(btCollisionDispatcher*)*/dispatcher);

	irrTimer = device->getTimer();
	World->setGravity(this->Gravity);
	isClosing = false;

	dDrawer = NULL;

	/*Set soft body informer*/

	m_worldInfo.m_broadphase = pairCache;
    m_worldInfo.m_dispatcher = dispatcher;
	m_worldInfo.m_sparsesdf.Initialize();

    m_worldInfo.m_gravity.setValue(0,-10.0,0);
    m_worldInfo.air_density = 1.0f;
    m_worldInfo.water_density = 0;
    m_worldInfo.water_offset = 0;
    m_worldInfo.water_normal = btVector3(0,0,0);

	timestep = 1.0f/100.0f;
	maxSubSteps = 50;
}
void CIrrBPWorld::clear()
{

	/*Delete all constraints*/
	for(u32 i=0;i<this->rigidBodiesConst.size();i++)
	{
		World->removeConstraint(rigidBodiesConst[i]->getConstraintPtr());
		rigidBodiesConst[i]->drop();
	}
	rigidBodiesConst.set_used(0);

	/*Delete all objects*/
	for(u32 i=0;i<this->collisionObj.size();i++)
	{
		World->removeCollisionObject(collisionObj[i]->getPtr());
		collisionObj[i]->drop();
	}
	collisionObj.set_used(0);

	/*Delete all action intefaces*/
	for(u32 i=0;i<this->actionObj.size();i++)
	{
		World->removeAction(actionObj[i]->getPtr());
		actionObj[i]->drop();
	}

	actionObj.set_used(0);
}
bool CIrrBPWorld::isPairColliding(CIrrBPCollisionObject *body1,CIrrBPCollisionObject *body2, contactPoint * dCP, bool returnSecondPoint)
{
	if(!body1)
		assert(!body1);
	if(!body2)
		assert(!body2);
	collisionCallback cBack(body1->getPtr(),body2->getPtr());
	World->contactPairTest(body1->getPtr(),body2->getPtr(),cBack);

	if(dCP)
	{
		if(!returnSecondPoint)
			dCP->point = bulletVectorToIrrVector(cBack.pos);
		else
			dCP->point = bulletVectorToIrrVector(cBack.pos2);
		dCP->contact = cBack.collided;
	}

	return cBack.collided;

}
bool CIrrBPWorld::getBodyCollidingPoint(CIrrBPCollisionObject *body, contactPoint * dCP)
{
	if(!body)
		assert(!body);
	if(!dCP)
		assert(!dCP);
	collisionCallback cBack(body->getPtr());
	World->contactTest(body->getPtr(),cBack);
	dCP->point = bulletVectorToIrrVector(cBack.pos);
	dCP->contact = cBack.collided;
	return cBack.collided;
}
bool CIrrBPWorld::isBodyColliding(CIrrBPCollisionObject *body)
{
	const int numManifolds = World->getDispatcher()->getNumManifolds();
	for (int i=0;i<numManifolds;i++)
	{
		btPersistentManifold* contactManifold =  World->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
		if(obA == body->getPtr() || obB == body->getPtr())
		{
			int numContacts = contactManifold->getNumContacts();

			if (numContacts == 0)
				return false;

			for (int j=0;j<numContacts;j++)
			{
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				if (pt.getDistance()>0.f)
					return false;
			}
			return true;
		}
	}
	return false;
}
void CIrrBPWorld::addSoftBody(CIrrBPSoftBody * sbody)
{
	collisionObj.push_back(sbody);
	World->addSoftBody(sbody->getBodyPtr());
	sbody->setValidStatus(true);
	#ifdef IRRBP_DEBUG_TEXT
	cout<<"# Added new Soft Body"<<endl;
	#endif

}
void CIrrBPWorld::addRigidBody(CIrrBPRigidBody *body)
{

	collisionObj.push_back(body);
	World->addRigidBody(body->getBodyPtr());
	body->setValidStatus(true);

	#ifdef IRRBP_DEBUG_TEXT
	cout<<"# Added new Body "<<endl<<"## Body ID: "<<body->getID()<<endl<<"## Absolute Body ID: "<<body->getUniqueID()<<endl;
	#endif

}

void CIrrBPWorld::addCollisionObject(CIrrBPCollisionObject * cobj)
{
	switch(cobj->getObjectType())
	{
		case RIGID_BODY:
			this->addRigidBody(dynamic_cast<CIrrBPRigidBody*>(cobj));
		break;
		case SOFT_BODY:
			this->addSoftBody(dynamic_cast<CIrrBPSoftBody*>(cobj));
		break;
	}
}
void CIrrBPWorld::addRigidBodyConstraint(CIrrBPConstraint * constraint)
{
	rigidBodiesConst.push_back(constraint);
	World->addConstraint(constraint->getConstraintPtr());
	#ifdef IRRBP_DEBUG_TEXT
	cout<<"# Added new constraint"<<endl<<"## Body ID (A): "<<constraint->getBodyA()->getID()<<endl<<"## Body UID (A): "<<constraint->getBodyA()->getUniqueID()<<endl;

	if(constraint->getBodyB())
		cout<<"## Body ID (B): "<<constraint->getBodyB()->getID()<<endl<<"## Body UID (B): "<<constraint->getBodyB()->getUniqueID()<<endl;
	#endif
}

void CIrrBPWorld::addAction(CIrrBPActionInterface * action)
{
	actionObj.push_back(action);
	World->addAction(action->getPtr());
	#ifdef IRRBP_DEBUG_TEXT
	cout<<"# Added new action "<<endl<<"## Action ID: "<<action->getID()<<endl<<"## Absolute Action ID: "<<action->getUniqueID()<<endl;
	#endif

}

void CIrrBPWorld::removeAction(CIrrBPActionInterface * action)
{
	for(irr::u32 i=0;i<actionObj.size();i++)
	{
		if(action == actionObj[i])
		{
			action->setValidStatus(false);
			World->removeAction(action->getPtr());
			actionObj[i]->drop();
			actionObj.erase(i);
			#ifdef IRRBP_DEBUG_TEXT
			cout<<"# Deleted action "<<endl<<"## Action ID: "<<action->getID()<<endl<<"## Absolute Action ID: "<<action->getUniqueID()<<endl;
			#endif
			return;
		}
	}
}

void CIrrBPWorld::removeCollisionObject(CIrrBPCollisionObject * cobj)
{
	for(irr::u32 i = 0;i<collisionObj.size(); i++)
	{
		if(cobj == collisionObj[i])
		{
			cobj->setValidStatus(false);
			World->removeCollisionObject(cobj->getPtr());
			collisionObj[i]->drop();
			collisionObj.erase(i);
			#ifdef IRRBP_DEBUG_TEXT
			cout<<"# Deleted Body"<<endl<<"## Body ID: "<<cobj->getID()<<endl;
			#endif

			return;
		}
	}
	#ifdef IRRBP_DEBUG_TEXT
	cout<<"# Error while deleting body...Body not found!";
	#endif
}
void CIrrBPWorld::removeRigidBody(CIrrBPRigidBody *body)
{
	this->removeCollisionObject(body);

}
void CIrrBPWorld::autoMaxSubSteps(int minFPS)
{

	//Equation: timestamp < m.s.s. * f.t.s
	// 1/FPS <  m.s.s. * f.t.s
	// m.s.s. > (1/FPS)/f.t.s
	float eq = (1.0f/((float)minFPS))/timestep;
	maxSubSteps = irr::core::ceil32(eq);
}
void CIrrBPWorld::stepSimulation()
{

	DeltaTime = irrTimer->getRealTime();
	#ifdef IRRBP_DEBUG_TEXT
		if((DeltaTime-TimeStamp) / 1000.0f >= (10*timestep))
		cout<<"You must fix your physics parameters"<<endl;
	#endif

	World->stepSimulation((DeltaTime-TimeStamp) / 1000.0f,maxSubSteps,timestep);
	TimeStamp = irrTimer->getRealTime();

	//m_worldInfo.m_sparsesdf.GarbageCollect();
	updateObjects();


};

void CIrrBPWorld::updateObjects()
{
	array<CIrrBPAnimator *> anims;
	for(irr::u32 i=0;i<collisionObj.size();)
	{
		if(collisionObj[i]->isValid() == false)
		{
			removeCollisionObject(collisionObj[i]);
			continue;
		}
		anims = collisionObj[i]->getAnimators();
		for(irr::u32 k=0;k<anims.size();k++)
			anims[k]->animate();


		if(collisionObj[i]->getObjectType() == SOFT_BODY)
			static_cast<CIrrBPSoftBody*>(collisionObj[i])->update();
		i++;
	}
}

CIrrBPCollisionObject * CIrrBPWorld::getBodyFromUId(irr::u32 uid)
{
	for(irr::u32 i=0;i<this->collisionObj.size();i++)
		if(collisionObj[i]->getUniqueID() == uid)
			return collisionObj[i];

	return NULL;
}
CIrrBPCollisionObject * CIrrBPWorld::getBodyFromId(irr::s32 id)
{

	for(irr::u32 i=0;i<this->collisionObj.size();i++)
		if(collisionObj[i]->getID() == id)
			return collisionObj[i];

	return NULL;
}
CIrrBPCollisionObject * CIrrBPWorld::getBodyFromName(irr::c8* name)
{
	for(irr::u32 i=0;i<this->collisionObj.size();i++)
		if(strcmp(collisionObj[i]->getName(),name)==0)
			return collisionObj[1];

	return NULL;
}

CIrrBPActionInterface * CIrrBPWorld::getActionFromId(irr::s32 id)
{
	for(irr::u32 i=0;i<this->actionObj.size();i++)
		if(actionObj[i]->getID() == id)
			return actionObj[i];

	return NULL;
}
CIrrBPActionInterface * CIrrBPWorld::getActionFromUId(irr::u32 uid)
{

	for(irr::u32 i=0;i<this->actionObj.size();i++)
		if(actionObj[i]->getUniqueID() == uid)
			return actionObj[i];

	return NULL;
}
CIrrBPActionInterface * CIrrBPWorld::getActionFromName(irr::c8* name)
{
	for(irr::u32 i=0;i<this->actionObj.size();i++)
		if(strcmp(actionObj[i]->getName(),name)==0)
			return actionObj[i];

	return NULL;
}

void CIrrBPWorld::createDebugDrawer()
{
	dDrawer = new CIrrBPDebugDrawer(this->device->getVideoDriver());
	if(World)
		World->setDebugDrawer(dDrawer);

	mat.Lighting = false;
	//mat.Thickness = 3;
}

void CIrrBPWorld::stepDebugDrawer()
{
	driver->setTransform(ETS_WORLD,matrix4());
	driver->setMaterial(mat);
	World->debugDrawWorld();
}

void CIrrBPWorld::setDebugDrawerFlags(int flags)
{
	if(dDrawer)
		dDrawer->setDebugMode(flags);
}

btSoftBodyWorldInfo& CIrrBPWorld::getSoftBodyWorldInfo()
{
	return m_worldInfo;
}

void CIrrBPWorld::setERP(irr::f32 erp)
{
	World->getSolverInfo().m_erp = erp;
}
void CIrrBPWorld::setERP2(irr::f32 erp2)
{
	World->getSolverInfo().m_erp2 = erp2;
}

void CIrrBPWorld::setCFM(irr::f32 cfm)
{
	World->getSolverInfo().m_globalCfm = cfm;
}
