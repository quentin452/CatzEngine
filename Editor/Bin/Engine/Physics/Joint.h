#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Joint' to link 2 actors together, or one actor to fixed position in world.

/******************************************************************************/
struct Spring {
    Flt spring, damping;

    Spring(Flt spring = 0, Flt damping = 0) {
        T.spring = spring;
        T.damping = damping;
    }
};
/******************************************************************************/
struct Joint // a Joint between 2 actors, or one actor and fixed position in world
{
    // manage
    Joint &del();                                                                                                                                                                     // delete manually
    Joint &create(Actor &a0, Actor *a1);                                                                                                                                              // create default   joint
    Joint &createHinge(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, Bool collision = false);                                                                                     // create hinge     joint, 'anchor'=world space anchor position, 'axis'=world space joint axis, 'collision'=if allow for collisions between 'a0' and 'a1'
    Joint &createHinge(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, Flt min_angle, Flt max_angle, Bool collision = false);                                                       // create hinge     joint, 'anchor'=world space anchor position, 'axis'=world space joint axis, 'collision'=if allow for collisions between 'a0' and 'a1'
    Joint &createSpherical(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, C Flt *twist = null, C Flt *swing = null, Bool collision = false);                                       // create spherical joint, 'anchor'=world space anchor position, 'axis'=world space joint axis,                                              'twist'=maximum allowed twist angle rotation (0..PI) null for no limits, 'swing'  =maximum allowed swing angle rotation (0..PI) null for no limits
    Joint &createSpherical(Actor &a0, Actor *a1, C Vec &anchor, C Vec &axis, C Vec &perp, C Flt *twist = null, C Flt *swing_y = null, C Flt *swing_z = null, Bool collision = false); // create spherical joint, 'anchor'=world space anchor position, 'axis'=world space joint axis, 'perp'=world space joint axis perpendicular, 'twist'=maximum allowed twist angle rotation (0..PI) null for no limits, 'swing_y'=maximum allowed swing angle rotation (0..PI) null for no limits, 'swing_z'=maximum allowed swing angle rotation (0..PI) null for no limits
    Joint &createSliding(Actor &a0, Actor *a1, C Vec &anchor, C Vec &dir, Flt min, Flt max, Bool collision = false);                                                                  // create sliding   joint, 'anchor'=world space anchor position, 'dir' =world space sliding direction, 'min'=minumum allowed distance along 'dir' (-Inf..'max'), 'max'=maximum allowed distance along 'dir' ('min'..Inf)
    Joint &createDist(Actor &a0, Actor *a1, C Vec &anchor0, C Vec &anchor1, Flt min, Flt max, C Spring *spring = null, Bool collision = false);                                       // create distance based joint, here 'anchor0 anchor1' are in local spaces of actors

    // advanced
    Joint &createHinge(Actor &a0, Actor *a1, C Vec local_anchor[2], C Vec local_axis[2], C Vec local_normal[2], C Vec2 *angle_min_max, Bool collision);
    Joint &createSpherical(Actor &a0, Actor *a1, C Vec local_anchor[2], C Vec local_axis[2], C Vec local_normal[2], C Flt *twist, C Flt *swing_y, C Flt *swing_z, Bool collision);

    // get / set
    Bool is() C { return _joint != null; }           // if  created
    Bool broken() C;                                 // if  joint is broken
    Joint &breakable(Flt max_force, Flt max_torque); // set breakable parameters, if forces applied to the joint will exceed given parameters the joint will break (use <0 values for non-breakable joint)
#if EE_PRIVATE
    Matrix localAnchor(Bool index) C; // get anchor in actor local space (index=which actor), MatrixIdentity on fail
    void changed();
#endif

    // hinge joint specific (following methods are supported only on PhysX)
    Flt hingeAngle() C; // get joint angle in range (-PI, PI]
    Flt hingeVel() C;   // get joint velocity
    Bool hingeDriveEnabled() C;
    Joint &hingeDriveEnabled(Bool on); // get/set if drive is enabled
    Bool hingeDriveFreeSpin() C;
    Joint &hingeDriveFreeSpin(Bool on); // get/set if the existing velocity is beyond the drive velocity, do not add force
    Flt hingeDriveVel() C;
    Joint &hingeDriveVel(Flt vel); // get/set drive target velocity, 0..Inf, default=0
    Flt hingeDriveForceLimit() C;
    Joint &hingeDriveForceLimit(Flt limit); // get/set the maximum torque the drive can exert, setting this to a very large value if 'hingeDriveVel' is also very large may cause unexpected results, 0..Inf, default=Inf
    Flt hingeDriveGearRatio() C;
    Joint &hingeDriveGearRatio(Flt ratio); // get/set the gear ratio for the drive, when setting up the drive constraint, the velocity of the first actor is scaled by this value, and its response to drive torque is scaled down. So if the drive target velocity is zero, the second actor will be driven to the velocity of the first scaled by the gear ratio. 0..Inf, default=1

    // io
    Bool save(File &f) C;                     // save, does not save information about which actors are joined, false on fail
    Bool load(File &f, Actor &a0, Actor *a1); // load, 'a0' and 'a1' are actors which the joint should link, false on fail

    ~Joint() { del(); }
    Joint() { _joint = null; }

#if !EE_PRIVATE
  private:
#endif
#if EE_PRIVATE
    PHYS_API(PxJoint, btTypedConstraint) * _joint;
#else
    Ptr _joint;
#endif

    NO_COPY_CONSTRUCTOR(Joint);
};
/******************************************************************************/
