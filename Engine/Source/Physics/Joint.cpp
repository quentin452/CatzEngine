/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
enum JOINT_TYPE : Byte
{
   JOINT_NONE     ,
   JOINT_FIXED    ,
   JOINT_HINGE    ,
   JOINT_SPHERICAL,
   JOINT_SLIDER   ,
   JOINT_DISTANCE ,
};
/******************************************************************************/
#if PHYSX
   #define BOUNCE    0.0f
   #define STIFFNESS 0.0f
   #define DAMPING   0.0f

   #define PROJECT     1
   #define PROJECT_POS 0.01f // 0..FLT_MAX
   #define PROJECT_ANG PI    // 0..PI

/*struct SphericalJoint : PxJoint, PxConstraintConnector
{
	static const PxU32 TYPE_ID = PxConcreteType::eFIRST_USER_EXTENSION;

   // PxJoint
	virtual void				setActors(PxRigidActor* actor0, PxRigidActor* actor1)override {}
	virtual void				getActors(PxRigidActor*& actor0, PxRigidActor*& actor1)const override {}
	virtual void				setLocalPose(PxJointActorIndex::Enum actor, const PxTransform& localPose)override {}
	virtual PxTransform		getLocalPose(PxJointActorIndex::Enum actor)const override {}
	virtual PxTransform		getRelativeTransform()const override {}
	virtual PxVec3				getRelativeLinearVelocity()const	override {}
	virtual PxVec3				getRelativeAngularVelocity()const override {}
	virtual void				setBreakForce(PxReal force, PxReal torque)override {}
	virtual void				getBreakForce(PxReal& force, PxReal& torque)const override {}
	virtual void				setConstraintFlags(PxConstraintFlags flags)override {}
	virtual void				setConstraintFlag(PxConstraintFlag::Enum flag, bool value)override {}
	virtual PxConstraintFlags	getConstraintFlags()const override {}
	virtual void				setInvMassScale0(PxReal invMassScale)override {}
	virtual PxReal				getInvMassScale0()const override {}
	virtual void				setInvInertiaScale0(PxReal invInertiaScale) override {}
	virtual PxReal				getInvInertiaScale0()const override {}
	virtual void				setInvMassScale1(PxReal invMassScale)override {}
	virtual PxReal				getInvMassScale1()const override {}
	virtual void				setInvInertiaScale1(PxReal invInertiaScale)override {}
	virtual PxReal				getInvInertiaScale1()const override {}
	virtual PxConstraint*	getConstraint()const	override {}
	virtual void				setName(const char* name)override {}
	virtual const char*		getName()const override {}
	virtual void				release()override {}
	virtual PxScene*			getScene()const override {}

   // PxConstraintConnector
	virtual void*			prepareData()override {}
	virtual bool			updatePvdProperties(physx::pvdsdk::PvdDataStream& pvdConnection, const PxConstraint* c, PxPvdUpdateType::Enum updateType)const override {}
	virtual void			onConstraintRelease()override {}
	virtual void			onComShift(PxU32 actor)override {}
	virtual void			onOriginShift(const PxVec3& shift)override {}
	virtual void*			getExternalReference(PxU32& typeID)override {}
	virtual PxBase* getSerializable()override {}
	virtual PxConstraintSolverPrep getPrep()const override {}
	virtual const void* getConstantBlock()const override {}
};*/
#else
struct btDistanceConstraint : btPoint2PointConstraint
{
   Bool   use_spring;
   Flt    min_dist, max_dist;
   Spring spring;

   btDistanceConstraint(btRigidBody& rbA, const btVector3& pivotInA, Flt min_dist, Flt max_dist, C Spring *spring) : btPoint2PointConstraint(rbA, pivotInA)
   {
      T.min_dist=min_dist;
      T.max_dist=max_dist;
      if(!spring)T.use_spring=false;else
      {
         T.use_spring=true;
         T.    spring=*spring;
      }
   }

   btDistanceConstraint(btRigidBody& rbA, btRigidBody& rbB, const btVector3& pivotInA, const btVector3& pivotInB, Flt min_dist, Flt max_dist, C Spring *spring) : btPoint2PointConstraint(rbA, rbB, pivotInA, pivotInB)
   {
      T.min_dist=min_dist;
      T.max_dist=max_dist;
      if(!spring)T.use_spring=false;else
      {
         T.use_spring=true;
         T.    spring=*spring;
      }
   }

   virtual void getInfo1(btConstraintInfo1 *info)
   {
      info->m_numConstraintRows=1;
      info->nub=5;
   }
   virtual void getInfo2(btConstraintInfo2 *info)
   {
      btVector3 relA = m_rbA.getCenterOfMassTransform().getBasis() * getPivotInA();
      btVector3 relB = m_rbB.getCenterOfMassTransform().getBasis() * getPivotInB();
      btVector3 posA = m_rbA.getCenterOfMassTransform().getOrigin() + relA;
      btVector3 posB = m_rbB.getCenterOfMassTransform().getOrigin() + relB;
      btVector3 del = posB - posA;
      btScalar currDist = btSqrt(del.dot(del));
      btVector3 ortho = (currDist ? del / currDist : btVector3(0, 0, 0));
      info->m_J1linearAxis[0] = ortho[0];
      info->m_J1linearAxis[1] = ortho[1];
      info->m_J1linearAxis[2] = ortho[2];
      btVector3 p, q;
      p = relA.cross(ortho);
      q = relB.cross(ortho);
      info->m_J1angularAxis[0] = p[0];
      info->m_J1angularAxis[1] = p[1];
      info->m_J1angularAxis[2] = p[2];
      info->m_J2angularAxis[0] = -q[0];
      info->m_J2angularAxis[1] = -q[1];
      info->m_J2angularAxis[2] = -q[2];
      btScalar rhs = (currDist - 0/*m_distance*/) * info->fps * info->erp;
      if(use_spring)rhs*=spring.spring*0.01f;
      info->m_constraintError[0] = rhs;
      info->cfm[0] = btScalar(0.f);
      info->m_lowerLimit[0] = -SIMD_INFINITY;
      info->m_upperLimit[0] = SIMD_INFINITY;
   }
};
#endif
/******************************************************************************/
Joint& Joint::del()
{
   if(_joint)
   {
      SafeWriteLock lock(Physics._rws);
      if(_joint)
      {
      #if PHYSX
         if(Physx.world)_joint->release(); _joint=null;
      #else
         if(_joint->needsFeedback())Bullet.breakables.exclude(_joint); // remove from breakable list
         if(Bullet.world)Bullet.world->removeConstraint(_joint); // remove from world
         Delete(_joint);
      #endif
      }
   }
   return T;
}
/******************************************************************************/
static Bool ValidActors(Actor &a0, Actor *a1) // PhysX will still create joint for null actors, and creating for static only will even create a crash
{
   return a0.is() && (a1 ? a1->is() : true) // make sure that both exist
   #if PHYSX
      && (a0.isDynamic() || (a1 && a1->isDynamic())) // for PhysX make sure that at least one is dynamic
   #endif
   ;
}
/******************************************************************************/
Joint& Joint::create(Actor &a0, Actor *a1)
{
   WriteLock lock(Physics._rws);

   del();

#if PHYSX
   if(Physx.world && ValidActors(a0, a1))
      if(PxFixedJoint *fixed=PxFixedJointCreate(*Physx.physics, a0._actor, PxTransform(PxIdentity), a1 ? a1->_actor : null, Physx.matrix(a1 ? a0.matrix().divNormalized(a1->matrix()) : a0.matrix()))) // when specifying world matrix use current a0 world position, when specifying local frame for a1 use current a0 position in a1 space
   {
      fixed->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, false);
     _joint=fixed;
   }
#else
   if(a0._actor)
   {
      btGeneric6DofConstraint *dof=null;
      if(a1 && a1->_actor)
      {
         dof=new btGeneric6DofConstraint(*a0._actor, *a1->_actor, btTransform::getIdentity(), Bullet.matrix(a0.massCenterMatrix()/a1->massCenterMatrix()), false);
      }else
      {
         dof=new btGeneric6DofConstraint(*a0._actor, btTransform::getIdentity(), false);
      }
      dof->setAngularLowerLimit(btVector3(0, 0, 0));
      dof->setAngularUpperLimit(btVector3(0, 0, 0));
      dof->setLinearLowerLimit (btVector3(0, 0, 0));
      dof->setLinearUpperLimit (btVector3(0, 0, 0));
      if(_joint=dof)
      {
         Flt f=FLT_MAX;
        _joint->setUserConstraintId((Int&)f);
        _joint->setUserConstraintType(0);
         if(Bullet.world)Bullet.world->addConstraint(_joint, true);
      }
   }
#endif
   return T;
}
/******************************************************************************/
Joint& Joint::createHinge(Actor &a0, Actor *a1, C Vec local_anchor[2], C Vec local_axis[2], C Vec local_normal[2], C Vec2 *angle_min_max, Bool collision)
{
   del();
#if PHYSX
   Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.y=local_normal[0]; m0.z=Cross(m0.x, m0.y);
   Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.y=local_normal[1]; m1.z=Cross(m1.x, m1.y);
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxRevoluteJoint *hinge=PxRevoluteJointCreate(*Physx.physics, a0._actor, Physx.matrix(m0), a1 ? a1->_actor : null, Physx.matrix(m1)))
   {
      if(PROJECT)
      {
         hinge->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
         hinge->setProjectionLinearTolerance (PROJECT_POS);
         hinge->setProjectionAngularTolerance(PROJECT_ANG);
      }

      hinge->setConstraintFlag   (PxConstraintFlag   ::eCOLLISION_ENABLED, collision          );
      hinge->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED    , angle_min_max!=null);

      PxJointAngularLimitPair limit(angle_min_max ? -angle_min_max->y : 0, angle_min_max ? -angle_min_max->x : 0); // here order is reversed to pass max first
      limit.restitution=BOUNCE; limit.stiffness=STIFFNESS; limit.damping=DAMPING; hinge->setLimit(limit);
     _joint=hinge;
   }
#else
   if(a0._actor)
   {
      Matrix m0; m0.pos=local_anchor[0]; m0.z=local_axis[0]; m0.x=local_normal[0]; m0.y=Cross(m0.z, m0.x);                     m0*=a0. _actor->offset;
      Matrix m1; m1.pos=local_anchor[1]; m1.z=local_axis[1]; m1.x=local_normal[1]; m1.y=Cross(m1.z, m1.x); if(a1 && a1->_actor)m1*=a1->_actor->offset;
      btHingeConstraint *hinge=null;
      if(a1 && a1->_actor)
      {
         hinge=new btHingeConstraint(*a0._actor, *a1->_actor, Bullet.matrix(m0), Bullet.matrix(m1));
      }else
      {
         hinge=new btHingeConstraint(*a0._actor, Bullet.matrix(m0)); hinge->getBFrame()=Bullet.matrix(m1);
      }
      if(_joint=hinge)
      {
         Flt f=FLT_MAX;
         hinge->setUserConstraintId((Int&)f);
         hinge->setUserConstraintType(0);
         if(angle_min_max)hinge->setLimit(angle_min_max->x, angle_min_max->y);

         WriteLock lock(Physics._rws);
         if(Bullet.world)Bullet.world->addConstraint(hinge, !collision);
      }
   }
#endif
   return T;
}
/******************************************************************************/
Joint& Joint::createSpherical(Actor &a0, Actor *a1, C Vec local_anchor[2], C Vec local_axis[2], C Vec local_normal[2], C Flt *twist, C Flt *swing_y, C Flt *swing_z, Bool collision)
{
   del();
#if PHYSX
#if 0 // PxSphericalJoint
   Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.y=local_normal[0]; m0.z=Cross(m0.x, m0.y);
   Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.y=local_normal[1]; m1.z=Cross(m1.x, m1.y);
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxSphericalJoint *spherical=PxSphericalJointCreate(*Physx.physics, a0._actor, Physx.matrix(m0), a1 ? a1->_actor : null, Physx.matrix(m1)))
   {
      spherical->setConstraintFlag    (PxConstraintFlag    ::eCOLLISION_ENABLED  , collision  );
      spherical->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED      , swing!=null);
      spherical->setSphericalJointFlag(PxSphericalJointFlag::eTWIST_LIMIT_ENABLED, twist!=null);
      Flt swing_val=(swing ? Mid(*swing, EPS, PI-EPS) : EPS);
      Flt twist_val=(twist ? Mid(*twist, EPS, PI-EPS) : EPS);
      PxJointLimitCone limit(swing_val, swing_val); limit.twist=twist_val; limit.restitution=BOUNCE; limit.stiffness=STIFFNESS; limit.damping=DAMPING;
      spherical->setLimitCone(limit);
     _joint=spherical;
   }
#else // PxD6Joint
   Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.y=local_normal[0]; m0.z=Cross(m0.x, m0.y);
   Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.y=local_normal[1]; m1.z=Cross(m1.x, m1.y);
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxD6Joint *spherical=PxD6JointCreate(*Physx.physics, a0._actor, Physx.matrix(m0), a1 ? a1->_actor : null, Physx.matrix(m1)))
   {
      if(PROJECT)
      {
         spherical->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
         spherical->setProjectionLinearTolerance (PROJECT_POS);
         spherical->setProjectionAngularTolerance(PROJECT_ANG);
      }

      spherical->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, collision);
      spherical->setMotion        (PxD6Axis::eTWIST , twist   ? (*twist  <=EPS) ? PxD6Motion::eLOCKED : PxD6Motion::eLIMITED : PxD6Motion::eFREE);
      spherical->setMotion        (PxD6Axis::eSWING1, swing_y ? (*swing_y<=EPS) ? PxD6Motion::eLOCKED : PxD6Motion::eLIMITED : PxD6Motion::eFREE);
      spherical->setMotion        (PxD6Axis::eSWING2, swing_z ? (*swing_z<=EPS) ? PxD6Motion::eLOCKED : PxD6Motion::eLIMITED : PxD6Motion::eFREE);
      if(twist)
      {
         Flt value=Mid(*twist, 0, PI);
         PxJointAngularLimitPair limit(-value, value); limit.restitution=BOUNCE; limit.stiffness=STIFFNESS; limit.damping=DAMPING;
         spherical->setTwistLimit(limit);
      }
      if(swing_y || swing_z)
      {
         PxJointLimitCone limit(swing_y ? Mid(*swing_y, EPS, PI-EPS) : PI_2,
                                swing_z ? Mid(*swing_z, EPS, PI-EPS) : PI_2); limit.restitution=BOUNCE; limit.stiffness=STIFFNESS; limit.damping=DAMPING;
         spherical->setSwingLimit(limit);
      }
     _joint=spherical;
   }
#endif
#else
   if(a0._actor)
   {
      if(!twist && !swing_y && !swing_z)
      {
         btPoint2PointConstraint *p2p=null;
         if(a1 && a1->_actor)
         {
            p2p=new btPoint2PointConstraint(*a0._actor, *a1->_actor, Bullet.vec(local_anchor[0]*a0._actor->offset), Bullet.vec(local_anchor[1]*a1->_actor->offset));
         }else
         {
            p2p=new btPoint2PointConstraint(*a0._actor, Bullet.vec(local_anchor[0]*a0._actor->offset)); p2p->setPivotB(Bullet.vec(local_anchor[1]));
         }
         if(_joint=p2p)
         {
            Flt f=FLT_MAX;
            p2p->setUserConstraintId((Int&)f);
            p2p->setUserConstraintType(0);

            WriteLock lock(Physics._rws);
            if(Bullet.world)Bullet.world->addConstraint(p2p, !collision);
         }
      }else
      {
         Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.z=local_normal[0]; m0.y=Cross(m0.z, m0.x);                     m0*=a0. _actor->offset;
         Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.z=local_normal[1]; m1.y=Cross(m1.z, m1.x); if(a1 && a1->_actor)m1*=a1->_actor->offset;
         btConeTwistConstraint *cone=null;
         if(a1 && a1->_actor)
         {
            cone=new btConeTwistConstraint(*a0._actor, *a1->_actor, Bullet.matrix(m0), Bullet.matrix(m1));
         }else
         {
            cone=new btConeTwistConstraint(*a0._actor, Bullet.matrix(m0)); cone->getBFrame()=Bullet.matrix(m1);
         }
         if(_joint=cone)
         {
            Flt f=FLT_MAX;
            cone->setUserConstraintId((Int&)f);
            cone->setUserConstraintType(0);
            cone->setLimit(swing_y ? *swing_y : FLT_MAX, swing_z ? *swing_z : FLT_MAX, twist ? *twist : FLT_MAX);

            WriteLock lock(Physics._rws);
            if(Bullet.world)Bullet.world->addConstraint(cone, !collision);
         }
      }
   }
#endif
   return T;
}
/******************************************************************************/
static void CreateSlider(Joint &joint, Actor &a0, Actor *a1, C Vec local_anchor[2], C Vec local_axis[2], C Vec local_normal[2], Flt min, Flt max, Bool collision)
{
   joint.del();
#if PHYSX
   Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.y=local_normal[0]; m0.z=Cross(m0.x, m0.y);
   Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.y=local_normal[1]; m1.z=Cross(m1.x, m1.y);
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxPrismaticJoint *slider=PxPrismaticJointCreate(*Physx.physics, a0._actor, Physx.matrix(m0), a1 ? a1->_actor : null, Physx.matrix(m1)))
   {
      slider->setConstraintFlag    (PxConstraintFlag    ::eCOLLISION_ENABLED, collision);
      slider->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED    , true     );
      PxJointLinearLimitPair limit(Physx.physics->getTolerancesScale(), -max, -min); slider->setLimit(limit);
      joint._joint=slider;
   }
#else
   if(a0._actor)
   {
      Matrix m0; m0.pos=local_anchor[0]; m0.x=local_axis[0]; m0.z=local_normal[0]; m0.y=Cross(m0.z, m0.x);                     m0*=a0. _actor->offset;
      Matrix m1; m1.pos=local_anchor[1]; m1.x=local_axis[1]; m1.z=local_normal[1]; m1.y=Cross(m1.z, m1.x); if(a1 && a1->_actor)m1*=a1->_actor->offset;
      btSliderConstraint *slider=null;
      if(a1 && a1->_actor)
      {
         slider=new btSliderConstraint(*a1->_actor, *a0._actor, Bullet.matrix(m1), Bullet.matrix(m0), true); // btSliderConstraint(bodyA, bodyB), because constructor below accepts B, to match order we need to swap here actor order
      }else
      {
         slider=new btSliderConstraint(*a0._actor, Bullet.matrix(m0), true); // btSliderConstraint(bodyB), this accepts B only
         slider->setFrames(Bullet.matrix(m1), Bullet.matrix(m0));
      }
      if(joint._joint=slider)
      {
         Flt f=FLT_MAX;
         slider->setUserConstraintId((Int&)f);
         slider->setUserConstraintType(0);
         slider->setLowerLinLimit(min);
         slider->setUpperLinLimit(max);

         WriteLock lock(Physics._rws);
         if(Bullet.world)Bullet.world->addConstraint(slider, !collision);
      }
   }
#endif
}
/******************************************************************************/
static void CreateDistance(Joint &joint, Actor &a0, Actor *a1, C Vec local_anchor[2], Flt min, Flt max, C Spring *spring, Bool collision)
{
   joint.del();
#if PHYSX
   WriteLock lock(Physics._rws);
   if(Physx.world && ValidActors(a0, a1))
      if(PxDistanceJoint *distance=PxDistanceJointCreate(*Physx.physics, a0._actor, Physx.matrix(local_anchor[0]), a1 ? a1->_actor : null, Physx.matrix(local_anchor[1])))
   {
      distance->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, collision);
      distance->setMinDistance   (min);
      distance->setMaxDistance   (max);
      if(spring)
      {
         distance->setStiffness(spring->spring );
         distance->setDamping  (spring->damping);
      }
      distance->setDistanceJointFlag(PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, true        );
      distance->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true        );
      distance->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED      , spring!=null);
      joint._joint=distance;
   }
#else
   if(a0._actor)
   {
      btDistanceConstraint *dist=null;
      if(a1 && a1->_actor)
      {
         dist=new btDistanceConstraint(*a0._actor, *a1->_actor, Bullet.vec(local_anchor[0]*a0._actor->offset), Bullet.vec(local_anchor[1]*a1->_actor->offset), min, max, spring);
      }else
      {
         dist=new btDistanceConstraint(*a0._actor, Bullet.vec(local_anchor[0]*a0._actor->offset), min, max, spring); dist->setPivotB(Bullet.vec(local_anchor[1]));
      }
      if(joint._joint=dist)
      {
         Flt f=FLT_MAX;
         dist->setUserConstraintId((Int&)f);
         dist->setUserConstraintType(0);

         WriteLock lock(Physics._rws);
         if(Bullet.world)Bullet.world->addConstraint(dist, !collision);
      }
   }
#endif
}
/******************************************************************************/
Joint& Joint::createHinge(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, Bool collision)
{
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    normal        =PerpN(axis),
          local_anchor[]={Vec().fromDivNormalized(anchor, m0      ), Vec().fromDivNormalized(anchor, m1      )},
          local_axis  []={Vec().fromDivNormalized(axis  , m0.orn()), Vec().fromDivNormalized(axis  , m1.orn())},
          local_normal[]={Vec().fromDivNormalized(normal, m0.orn()), Vec().fromDivNormalized(normal, m1.orn())};
   return createHinge(a0, a1, local_anchor, local_axis, local_normal, null, collision);
}
Joint& Joint::createHinge(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, Flt min_angle, Flt max_angle, Bool collision)
{
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    normal        =PerpN(axis),
          local_anchor[]={Vec().fromDivNormalized(anchor, m0      ), Vec().fromDivNormalized(anchor, m1      )},
          local_axis  []={Vec().fromDivNormalized(axis  , m0.orn()), Vec().fromDivNormalized(axis  , m1.orn())},
          local_normal[]={Vec().fromDivNormalized(normal, m0.orn()), Vec().fromDivNormalized(normal, m1.orn())};
   return createHinge(a0, a1, local_anchor, local_axis, local_normal, &Vec2(min_angle, max_angle), collision);
}
Joint& Joint::createSpherical(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, C Flt *twist, C Flt *swing, Bool collision)
{
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    normal        =PerpN(axis),
          local_anchor[]={Vec().fromDivNormalized(anchor, m0      ), Vec().fromDivNormalized(anchor, m1      )},
          local_axis  []={Vec().fromDivNormalized(axis  , m0.orn()), Vec().fromDivNormalized(axis  , m1.orn())},
          local_normal[]={Vec().fromDivNormalized(normal, m0.orn()), Vec().fromDivNormalized(normal, m1.orn())};
   return createSpherical(a0, a1, local_anchor, local_axis, local_normal, twist, swing, swing, collision);
}
Joint& Joint::createSpherical(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, C Vec &perp, C Flt *twist, C Flt *swing_y, C Flt *swing_z, Bool collision)
{
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    local_anchor[]={Vec().fromDivNormalized(anchor, m0      ), Vec().fromDivNormalized(anchor, m1      )},
          local_axis  []={Vec().fromDivNormalized(axis  , m0.orn()), Vec().fromDivNormalized(axis  , m1.orn())},
          local_normal[]={Vec().fromDivNormalized(perp  , m0.orn()), Vec().fromDivNormalized(perp  , m1.orn())};
   return createSpherical(a0, a1, local_anchor, local_axis, local_normal, twist, swing_y, swing_z, collision);
}
Joint& Joint::createSliding(Actor &a0, Actor *a1, C Vec &anchor, C Vec &dir, Flt min, Flt max, Bool collision)
{
   Matrix m0            =      a0. matrix(),
          m1            =(a1 ? a1->matrix() : MatrixIdentity);
   Vec    normal        =PerpN(dir),
          local_anchor[]={Vec().fromDivNormalized(anchor, m0      ), Vec().fromDivNormalized(anchor, m1      )},
          local_axis  []={Vec().fromDivNormalized(dir   , m0.orn()), Vec().fromDivNormalized(dir   , m1.orn())},
          local_normal[]={Vec().fromDivNormalized(normal, m0.orn()), Vec().fromDivNormalized(normal, m1.orn())};
   CreateSlider(T, a0, a1, local_anchor, local_axis, local_normal, min, max, collision);
   return T;
}
Joint& Joint::createDist(Actor &a0, Actor *a1, C Vec &anchor0, C Vec &anchor1, Flt min, Flt max, C Spring *spring, Bool collision)
{
   Vec local_anchor[]={anchor0, anchor1};
   CreateDistance(T, a0, a1, local_anchor, min, max, spring, collision);
   return T;
}
/******************************************************************************/
Matrix Joint::localAnchor(Bool index)C
{
#if PHYSX
   if(_joint)return Physx.matrix(_joint->getLocalPose(index ? PxJointActorIndex::eACTOR1 : PxJointActorIndex::eACTOR0));
#endif
   return MatrixIdentity;
}
void Joint::changed()
{
#if PHYSX
   if(_joint)if(PxConstraint *constraint=_joint->getConstraint())constraint->markDirty();
#endif
}
/******************************************************************************/
// BREAKABLE
/******************************************************************************/
Bool Joint::broken()C
{
#if PHYSX
   return _joint ? FlagOn((UInt)_joint->getConstraintFlags(), PxConstraintFlag::eBROKEN) : true;
#else
   return _joint ? _joint->getUserConstraintType()!=0 : true;
#endif
}
/******************************************************************************/
Joint& Joint::breakable(Flt max_force, Flt max_torque)
{
   if(_joint)
   {
   #if PHYSX
     _joint->setBreakForce((max_force <0) ? PX_MAX_REAL : max_force ,
                           (max_torque<0) ? PX_MAX_REAL : max_torque);
   #else
      Flt f;
      if(max_force >=FLT_MAX)max_force =-1; // need to adjust to -1 because depending on <0 we include/exclude in breakable list
      if(max_torque>=FLT_MAX)max_torque=-1; // need to adjust to -1 because depending on <0 we include/exclude in breakable list
      if(max_force>=0 || max_torque>=0)
      {
         Bullet.breakables.include(_joint); // add to breakable list
        _joint->enableFeedback(true);
         if(max_force>=0 && max_torque>=0)f=Avg(max_force, max_torque);else
         if(max_force>=0                 )f=    max_force             ;else
         if(                max_torque>=0)f=    max_torque            ;
      }else
      {
         Bullet.breakables.exclude(_joint); // remove from breakable list
        _joint->enableFeedback(false);
         f=FLT_MAX;
      }
     _joint->setUserConstraintId((Int&)f);
   #endif
   }
   return T;
}
/******************************************************************************/
// HINGE
/******************************************************************************/
Flt Joint::hingeAngle()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getAngle();
#endif
   return 0;
}
Flt Joint::hingeVel()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getVelocity();
#endif
   return 0;
}
Bool Joint::hingeDriveEnabled()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return FlagOn((UInt)hinge->getRevoluteJointFlags(), PxRevoluteJointFlag::eDRIVE_ENABLED);
#endif
   return false;
}
Joint& Joint::hingeDriveEnabled(Bool on)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, on);
#endif
   return T;
}
Bool Joint::hingeDriveFreeSpin()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return FlagOn((UInt)hinge->getRevoluteJointFlags(), PxRevoluteJointFlag::eDRIVE_FREESPIN);
#endif
   return false;
}
Joint& Joint::hingeDriveFreeSpin(Bool on)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, on);
#endif
   return T;
}
Flt Joint::hingeDriveVel()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getDriveVelocity();
#endif
   return 0;
}
Joint& Joint::hingeDriveVel(Flt vel)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setDriveVelocity(vel);
#endif
   return T;
}
Flt Joint::hingeDriveForceLimit()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getDriveForceLimit();
#endif
   return 0;
}
Joint& Joint::hingeDriveForceLimit(Flt limit)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setDriveForceLimit(limit);
#endif
   return T;
}
Flt Joint::hingeDriveGearRatio()C
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())return hinge->getDriveGearRatio();
#endif
   return 0;
}
Joint& Joint::hingeDriveGearRatio(Flt ratio)
{
#if PHYSX
   if(_joint)if(PxRevoluteJoint *hinge=_joint->is<PxRevoluteJoint>())hinge->setDriveGearRatio(ratio);
#endif
   return T;
}
/******************************************************************************/
// IO
/******************************************************************************/
enum JOINT_FLAG
{
   JOINT_COLLISION=1<<0,

   JOINT_SPHERICAL_LIMIT_TWIST  =1<<1,
   JOINT_SPHERICAL_LIMIT_SWING_Y=1<<2,
   JOINT_SPHERICAL_LIMIT_SWING_Z=1<<3,
};
Bool Joint::save(File &f)C // !! IF MAKING ANY CHANGE, UPDATE TO NEW VERSION BECAUSE THIS IS USED IN SAVE GAMES !!
{
   f.cmpUIntV(0); // version

   JOINT_TYPE type=JOINT_NONE;
   
   if(_joint && !broken())
   {
   #if PHYSX
      switch(_joint->getConcreteType())
      {
         case PxJointConcreteType::eFIXED    : type=JOINT_FIXED    ; break;
         case PxJointConcreteType::eREVOLUTE : type=JOINT_HINGE    ; break;
         case PxJointConcreteType::ePRISMATIC: type=JOINT_SLIDER   ; break;
         case PxJointConcreteType::eDISTANCE : type=JOINT_DISTANCE ; break;

         case PxJointConcreteType::eD6       :
         case PxJointConcreteType::eSPHERICAL: type=JOINT_SPHERICAL; break;
      }
   #else
      if(CAST(btDistanceConstraint   , _joint))type=JOINT_DISTANCE ;else // this must be checked as first, because 'btDistanceConstraint' extends 'btPoint2PointConstraint' checked below
      if(CAST(btGeneric6DofConstraint, _joint))type=JOINT_FIXED    ;else
      if(CAST(btHingeConstraint      , _joint))type=JOINT_HINGE    ;else
      if(CAST(btPoint2PointConstraint, _joint))type=JOINT_SPHERICAL;else
      if(CAST(btConeTwistConstraint  , _joint))type=JOINT_SPHERICAL;else
      if(CAST(btSliderConstraint     , _joint))type=JOINT_SLIDER   ;
   #endif
   }

   // save joint type
   f.putByte(type);

   // save joint data
   switch(type)
   {
      case JOINT_HINGE:
      {
         Vec  anchor[2], axis[2], normal[2];
         Bool collision, limit_angle;
         Vec2 angle;
      #if PHYSX
         PxRevoluteJoint &hinge=*_joint->is<PxRevoluteJoint>();
         Matrix m0=Physx.matrix(hinge.getLocalPose(PxJointActorIndex::eACTOR0)),
                m1=Physx.matrix(hinge.getLocalPose(PxJointActorIndex::eACTOR1));
         PxJointAngularLimitPair limit=hinge.getLimit();
         anchor[0]=m0.pos;
         anchor[1]=m1.pos;
         axis  [0]=m0.x;
         axis  [1]=m1.x;
         normal[0]=m0.y;
         normal[1]=m1.y;
         collision  =FlagOn((UInt)hinge.getConstraintFlags   (), PxConstraintFlag   ::eCOLLISION_ENABLED);
         limit_angle=FlagOn((UInt)hinge.getRevoluteJointFlags(), PxRevoluteJointFlag::eLIMIT_ENABLED    );
         angle.x    =-limit.upper; // min
         angle.y    =-limit.lower; // max
      #else
         btHingeConstraint *hinge=CAST(btHingeConstraint, _joint);
         Matrix m=Bullet.matrix(hinge->getAFrame()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))m.divNormalized(rb->offset); anchor[0]=m.pos; axis[0]=m.z; normal[0]=m.y;
                m=Bullet.matrix(hinge->getBFrame()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyB()))m.divNormalized(rb->offset); anchor[1]=m.pos; axis[1]=m.z; normal[1]=m.y;
         collision=true; btRigidBody &rb=_joint->getRigidBodyA(); REP(rb.getNumConstraintRefs())if(rb.getConstraintRef(i)==_joint){collision=false; break;}
         angle.x  =hinge->getLowerLimit();
         angle.y  =hinge->getUpperLimit();
         limit_angle=(angle.x<=angle.y);
      #endif
         f<<anchor<<axis<<normal<<collision<<limit_angle;
         if(limit_angle)f<<angle;
         f<<hingeDriveEnabled()<<hingeDriveFreeSpin()<<hingeDriveVel()<<hingeDriveForceLimit()<<hingeDriveGearRatio();
      }break;

      case JOINT_SPHERICAL:
      {
         Vec  anchor[2], axis[2], normal[2];
         Byte flag=0;
         Flt  twist, swing_y, swing_z;
      #if PHYSX
      #if 0
         PxSphericalJoint &spherical=*_joint->is<PxSphericalJoint>();
         Matrix m0=Physx.matrix(spherical.getLocalPose(PxJointActorIndex::eACTOR0)),
                m1=Physx.matrix(spherical.getLocalPose(PxJointActorIndex::eACTOR1));
         PxJointLimitCone limit=spherical.getLimitCone();
         anchor[0]=m0.pos;
         anchor[1]=m1.pos;
         axis  [0]=m0.x;
         axis  [1]=m1.x;
         normal[0]=m0.y;
         normal[1]=m1.y;
         collision  =FlagOn((UInt)spherical.getConstraintFlags    (), PxConstraintFlag    ::eCOLLISION_ENABLED  );
         limit_swing=FlagOn((UInt)spherical.getSphericalJointFlags(), PxSphericalJointFlag::eLIMIT_ENABLED      );
         limit_twist=FlagOn((UInt)spherical.getSphericalJointFlags(), PxSphericalJointFlag::eTWIST_LIMIT_ENABLED);
         swing      =limit.yAngle;
         twist      =limit.twistAngle;
      #else
         PxD6Joint &spherical=*_joint->is<PxD6Joint>();
         PxJointAngularLimitPair twist_limit=spherical.getTwistLimit();
	      PxJointLimitCone        swing_limit=spherical.getSwingLimit();
         Matrix m0=Physx.matrix(spherical.getLocalPose(PxJointActorIndex::eACTOR0)),
                m1=Physx.matrix(spherical.getLocalPose(PxJointActorIndex::eACTOR1));
         anchor[0]=m0.pos;
         anchor[1]=m1.pos;
         axis  [0]=m0.x;
         axis  [1]=m1.x;
         normal[0]=m0.y;
         normal[1]=m1.y;
         if(FlagOn((UInt)spherical.getConstraintFlags(), PxConstraintFlag::eCOLLISION_ENABLED))flag|=JOINT_COLLISION;
         switch(spherical.getMotion(PxD6Axis::eTWIST )){case PxD6Motion::eLOCKED: flag|=JOINT_SPHERICAL_LIMIT_TWIST  ; twist  =0; break;   case PxD6Motion::eLIMITED: flag|=JOINT_SPHERICAL_LIMIT_TWIST  ; twist  =twist_limit. upper; break;}
         switch(spherical.getMotion(PxD6Axis::eSWING1)){case PxD6Motion::eLOCKED: flag|=JOINT_SPHERICAL_LIMIT_SWING_Y; swing_y=0; break;   case PxD6Motion::eLIMITED: flag|=JOINT_SPHERICAL_LIMIT_SWING_Y; swing_y=swing_limit.yAngle; break;}
         switch(spherical.getMotion(PxD6Axis::eSWING2)){case PxD6Motion::eLOCKED: flag|=JOINT_SPHERICAL_LIMIT_SWING_Z; swing_z=0; break;   case PxD6Motion::eLIMITED: flag|=JOINT_SPHERICAL_LIMIT_SWING_Z; swing_z=swing_limit.zAngle; break;}
      #endif
      #else
         if(btPoint2PointConstraint *p2p=CAST(btPoint2PointConstraint, _joint))
         {
            anchor[0]=Bullet.vec(p2p->getPivotInA()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))anchor[0].divNormalized(rb->offset); axis[0].set(0, 0, 1); normal[0].set(1, 0, 0);
            anchor[1]=Bullet.vec(p2p->getPivotInB()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyB()))anchor[1].divNormalized(rb->offset); axis[1].set(0, 0, 1); normal[1].set(1, 0, 0);
         }else
         {
            btConeTwistConstraint *cone=CAST(btConeTwistConstraint, _joint);
            Matrix m=Bullet.matrix(cone->getAFrame()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))m.divNormalized(rb->offset); anchor[0]=m.pos; axis[0]=m.x; normal[0]=m.z;
                   m=Bullet.matrix(cone->getBFrame()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyB()))m.divNormalized(rb->offset); anchor[1]=m.pos; axis[1]=m.x; normal[1]=m.z;
            twist  =cone->getTwistSpan (); if(twist  !=FLT_MAX)flag|=JOINT_SPHERICAL_LIMIT_TWIST;
            swing_y=cone->getSwingSpan1(); if(swing_y!=FLT_MAX)flag|=JOINT_SPHERICAL_LIMIT_SWING_Y;
            swing_z=cone->getSwingSpan2(); if(swing_z!=FLT_MAX)flag|=JOINT_SPHERICAL_LIMIT_SWING_Z;
         }
         btRigidBody &rb=_joint->getRigidBodyA(); REP(rb.getNumConstraintRefs())if(rb.getConstraintRef(i)==_joint)goto no_collision; flag|=JOINT_COLLISION; no_collision:;
      #endif
         f<<anchor<<axis<<normal<<flag;
         if(flag&JOINT_SPHERICAL_LIMIT_TWIST  )f<<twist;
         if(flag&JOINT_SPHERICAL_LIMIT_SWING_Y)f<<swing_y;
         if(flag&JOINT_SPHERICAL_LIMIT_SWING_Z)f<<swing_z;
      }break;

      case JOINT_SLIDER:
      {
         Vec  anchor[2], axis[2], normal[2];
         Flt  min, max;
         Bool collision;
      #if PHYSX
         PxPrismaticJoint &slider=*_joint->is<PxPrismaticJoint>();
         Matrix m0=Physx.matrix(slider.getLocalPose(PxJointActorIndex::eACTOR0)),
                m1=Physx.matrix(slider.getLocalPose(PxJointActorIndex::eACTOR1));
         PxJointLinearLimitPair limit=slider.getLimit();
         anchor[0]=m0.pos;
         anchor[1]=m1.pos;
         axis  [0]=m0.x;
         axis  [1]=m1.x;
         normal[0]=m0.y;
         normal[1]=m1.y;
         collision=FlagOn((UInt)slider.getConstraintFlags(), PxConstraintFlag::eCOLLISION_ENABLED);
         min      =-limit.upper;
         max      =-limit.lower;
      #else
         btSliderConstraint *slider=CAST(btSliderConstraint, _joint);
         Matrix m=Bullet.matrix(slider->getFrameOffsetA()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))m.divNormalized(rb->offset); anchor[1]=m.pos; axis[1]=m.x; normal[1]=m.z;
                m=Bullet.matrix(slider->getFrameOffsetB()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyB()))m.divNormalized(rb->offset); anchor[0]=m.pos; axis[0]=m.x; normal[0]=m.z;
         collision=true; btRigidBody &rb=_joint->getRigidBodyA(); REP(rb.getNumConstraintRefs())if(rb.getConstraintRef(i)==_joint){collision=false; break;}
         min=slider->getLowerLinLimit();
         max=slider->getUpperLinLimit();
      #endif
         f<<anchor<<axis<<normal<<min<<max<<collision;
      }break;

      case JOINT_DISTANCE:
      {
         Vec    anchor[2];
         Flt    min, max;
         Bool   collision, use_spring;
      #if PHYSX
         PxDistanceJoint &distance=*_joint->is<PxDistanceJoint>();
         anchor[0] =Physx.vec(distance.getLocalPose(PxJointActorIndex::eACTOR0).p);
         anchor[1] =Physx.vec(distance.getLocalPose(PxJointActorIndex::eACTOR1).p);
         collision =FlagOn((UInt)distance.getConstraintFlags   (), PxConstraintFlag   ::eCOLLISION_ENABLED);
         use_spring=FlagOn((UInt)distance.getDistanceJointFlags(), PxDistanceJointFlag::eSPRING_ENABLED   );
         min       =distance.getMinDistance();
         max       =distance.getMaxDistance();
      #else
         btDistanceConstraint *dist=CAST(btDistanceConstraint, _joint);
         anchor[0] =Bullet.vec(dist->getPivotInA()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))anchor[0].divNormalized(rb->offset); 
         anchor[1] =Bullet.vec(dist->getPivotInB()); if(RigidBody *rb=CAST(RigidBody, &_joint->getRigidBodyA()))anchor[1].divNormalized(rb->offset); 
         min       =dist->min_dist;
         max       =dist->max_dist;
         use_spring=dist->use_spring;
         collision =true; btRigidBody &rb=_joint->getRigidBodyA(); REP(rb.getNumConstraintRefs())if(rb.getConstraintRef(i)==_joint){collision=false; break;}
	   #endif
	      f<<anchor<<min<<max<<collision<<use_spring;
	      if(use_spring)
         {
            Spring spring;
         #if PHYSX
            spring.spring =distance.getStiffness();
            spring.damping=distance.getDamping  ();
	      #else
            spring=dist->spring;
         #endif
            f<<spring;
         }
      }break;
   }

   // save breakable data
   if(type)
   {
      Flt max_force, max_torque;
   #if PHYSX
     _joint->getBreakForce(max_force, max_torque);
   #else
      Int force=_joint->getUserConstraintId(); max_force=max_torque=(Flt&)force;
   #endif
      f<<max_force<<max_torque;
   }
   return f.ok();
}
/******************************************************************************/
Bool Joint::load(File &f, Actor &a0, Actor *a1)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         JOINT_TYPE type=JOINT_TYPE(f.getByte());
         switch(    type)
         {
            default         : goto error; // unknown joint type
            case JOINT_NONE : del();          break;
            case JOINT_FIXED: create(a0, a1); break;

            case JOINT_HINGE:
            {
               Vec  anchor[2], axis[2], normal[2];
               Bool collision, limit_angle;
               Vec2 angle;
               f>>anchor>>axis>>normal>>collision>>limit_angle;
               if(limit_angle)f>>angle;

               createHinge(a0, a1, anchor, axis, normal, limit_angle ? &angle : null, collision);
               hingeDriveEnabled(f.getBool()); hingeDriveFreeSpin(f.getBool()); hingeDriveVel(f.getFlt()); hingeDriveForceLimit(f.getFlt()); hingeDriveGearRatio(f.getFlt());
            }break;

            case JOINT_SPHERICAL:
            {
               Vec  anchor[2], axis[2], normal[2];
               Byte flag;
               Flt  twist, swing_y, swing_z;
               f>>anchor>>axis>>normal>>flag;
               if(flag&JOINT_SPHERICAL_LIMIT_TWIST  )f>>twist;
               if(flag&JOINT_SPHERICAL_LIMIT_SWING_Y)f>>swing_y;
               if(flag&JOINT_SPHERICAL_LIMIT_SWING_Z)f>>swing_z;

               createSpherical(a0, a1, anchor, axis, normal, FlagOn(flag, JOINT_SPHERICAL_LIMIT_TWIST) ? &twist : null, FlagOn(flag, JOINT_SPHERICAL_LIMIT_SWING_Y) ? &swing_y : null, FlagOn(flag, JOINT_SPHERICAL_LIMIT_SWING_Z) ? &swing_z : null, FlagOn(flag, JOINT_COLLISION));
            }break;

            case JOINT_SLIDER:
            {
               Vec  anchor[2], axis[2], normal[2];
               Flt  min, max;
               Bool collision;
	            f>>anchor>>axis>>normal>>min>>max>>collision;

               CreateSlider(T, a0, a1, anchor, axis, normal, min, max, collision);
            }break;

            case JOINT_DISTANCE:
            {
               Vec    anchor[2];
               Flt    min, max;
               Bool   collision, use_spring;
               Spring spring;
	            f>>anchor>>min>>max>>collision>>use_spring;
	            if(use_spring)f>>spring;

               CreateDistance(T, a0, a1, anchor, min, max, use_spring ? &spring : null, collision);
            }break;
         }

         // load breakable data
         if(type)
         {
            Flt max_force, max_torque; f>>max_force>>max_torque; breakable(max_force, max_torque);
         }

         if(f.ok())return true;
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
