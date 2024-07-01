/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************

   TODO: Ragdoll actor shape creation should be verified across many models,
      if no universal code is found, Ragdoll Editor should be added.

/******************************************************************************/
static Shape ShapeBone(C Vec &from, C Vec &to, Flt width) {
    Shape shape;
    if (width >= 0.5f) {
        shape.type = SHAPE_BALL;
        Ball &ball = shape.ball;
        ball.pos = Avg(from, to);
        ball.r = width * (to - from).length();
    } else {
        shape.type = SHAPE_CAPSULE;
        Capsule &capsule = shape.capsule;
        capsule.pos = Avg(from, to);
        capsule.up = to - from;
        capsule.h = capsule.up.normalize();
        capsule.r = Max(0.01f, width * capsule.h);

        Flt eps = capsule.r * 0.5f;
        capsule.pos -= eps * capsule.up;
        capsule.h += eps * 2;
    }
    return shape;
}
inline static Shape ShapeBone(C SkelBone &bone) { return ShapeBone(bone.pos, bone.to(), bone.width); } // return shape from bone
/******************************************************************************/
void Ragdoll::zero() {
    _scale = 0;
    _skel = null;
}
Ragdoll::Ragdoll() { zero(); }
Ragdoll &Ragdoll::del() {
    _joints.del();
    _bones.del();
    _resets.del();
    _aggr.del(); // delete aggregate after actors, so they won't be re-inserted into scene when aggregate is deleted
    zero();
    return T;
}
static void CreateHinge(Joint &joint, Actor &bone, Actor &parent, C Vec &anchor, C Vec &axis, Flt min_angle, Flt max_angle) {
    joint.createHinge(bone, &parent, anchor, axis, min_angle, max_angle, false);
}
static void CreateSpherical(Joint &joint, Actor &bone, Actor &parent, C Vec &anchor, C Vec &axis, Flt twist, Flt swing) {
    joint.createSpherical(bone, &parent, anchor, axis, &twist, &swing, false);
}
static void CreateSpherical(Joint &joint, Actor &bone, Actor &parent, C Vec &anchor, C Vec &axis, C Vec &perp, Flt twist, Flt swing_y, Flt swing_z) {
    joint.createSpherical(bone, &parent, anchor, axis, perp, &twist, &swing_y, &swing_z, false);
}
static void CreateSpherical(Joint &joint, Actor &bone, Actor &parent, C Vec &anchor, C Orient &bone_orn, C Orient &joint_orn, Flt twist, Flt swing_y, Flt swing_z) // 'bone_orn'=bone world orientation, 'joint_orn'=joint world orientation (the center of relaxed position, around which the bone can rotate)
{                                                                                                                                                                  // here 'bone' and 'parent' matrix is identity
    C Vec &axis = joint_orn.dir,
          &perp = joint_orn.perp;
    Matrix3 m;
    GetTransform(m, joint_orn, bone_orn);
    Vec local_anchor[] = {anchor, anchor},
        local_axis[] = {axis * m, axis},
        local_normal[] = {perp * m, perp};
    joint.createSpherical(bone, &parent, local_anchor, local_axis, local_normal, &twist, &swing_y, &swing_z, false);
}
Bool Ragdoll::createTry(C AnimatedSkeleton &anim_skel, Flt scale, Flt density, Bool kinematic) {
    del();

    if (T._skel = anim_skel.skeleton()) {
        T._scale = scale;

        Memt<Shape> shapes;
        C Skeleton &skel = *anim_skel.skeleton();
        Int body = -1;
        FREPA(skel.bones) // order is important, main bones should be added at start, in case the skeleton doesn't have "body" bone, and as main bone (zero) should be set some different
        {
            C SkelBone &sbon = skel.bones[i];
            if (sbon.flag & BONE_RAGDOLL) {
                Vec from = sbon.pos,
                    to = sbon.to();
                Flt width = sbon.width;

                if (sbon.type == BONE_FOOT) {
                    C SkelBone *b = skel.findBone(BONE_TOE, sbon.type_index);
                    if (b) {
                        from = Avg(sbon.pos, sbon.to());
                        to = b->to();
                        width = Avg(width, b->width) * 0.5f;
                        Vec down = sbon.dir * (width * Dist(from, to) * 0.5f);
                        from -= down;
                        to -= down;
                    } else
                        width *= 0.8f;
                } else if (sbon.type == BONE_HAND) {
                    C SkelBone *b = skel.findBone(BONE_FINGER, (sbon.type_index >= 0) ? 2 : -3); // find middle finger (2 for right, -3 for left hand)
                    if (b) {
                        to = b->to();
                        width *= 0.6f;
                    }
                } else if (sbon.type == BONE_SPINE && sbon.type_index == 0 && sbon.type_sub == 0) {
                    body = _bones.elms();
                    _resets.add(i); // add main bone for resetting
                }

                Shape &s = shapes.New();
                s = ShapeBone(from, to, width);
                Bone &rb = _bones.New();
                Set(rb.name, sbon.name);
                rb.skel_bone = i;
                rb.rbon_parent = BONE_NULL;
                if (!rb.actor.createTry(s * T._scale, density, &VecZero, kinematic))
                    return false;
            } else {
                _resets.add(i);
            }
        }

        // force 'body' bone to have index=0
        if (body > 0) {
            Swap(_bones[0], _bones[body]);
            Swap(shapes[0], shapes[body]);
        }

        // set parents, damping and solver iterations
        REPA(T) {
            // find first parent which has an actor
            Bone &rb = bone(i);
            C SkelBone &sb = skel.bones[rb.skel_bone];
            if (i) // skip the main bone
            {
                BoneType skel_bone_parent = sb.parent;
                if (skel_bone_parent != BONE_NULL) // if has a parent
                {
                    Int rbone = findBoneIndexFromSkelBone(skel_bone_parent); // find ragdoll bone assigned to skeleton parent bone
                    if (rbone >= 0)
                        rb.rbon_parent = rbone; // if exists, then set as ragdoll parent
                }
            }

            rb.actor.adamping(4.0f);
            rb.actor.damping(0.5f);
            rb.actor.sleepEnergy(0.01f);
#if PHYSX
            rb.actor._dynamic->setMaxDepenetrationVelocity(0.1f);
            rb.actor._dynamic->setSolverIterationCounts(6, 6);
            // rb.actor._dynamic->setStabilizationThreshold(1.0f); // may need enabling eENABLE_STABILIZATION
#endif
        }

        if (!kinematic) {
            // joints
            REPA(_bones)
            if (i) // skip the main bone
            {
                Bone &rb = bone(i);
                C SkelBone &sb = skel.bones[rb.skel_bone];
                BoneType rbon_parent = ((rb.rbon_parent == BONE_NULL) ? 0 : rb.rbon_parent); // if doesn't have a parent then use the main bone

                // if(rbon_parent!=BONE_NULL)
                {
                    Bone &rp = _bones[rbon_parent];
                    C SkelBone &sp = skel.bones[rp.skel_bone];
                    const Flt HeadNeckTwist = DegToRad(45);
                    const Flt HeadNeckPitch = DegToRad(45);
                    const Flt HeadNeckRoll = DegToRad(45);
                    switch (sb.type) {
                    case BONE_SPINE:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb.dir, sb.perp, DegToRad(45) / 2.5f, DegToRad(45) / 2.5f, DegToRad(60) / 2.5f);
                        break;
                    case BONE_NECK:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb.dir, sb.perp, HeadNeckTwist / 2, HeadNeckRoll / 2, HeadNeckPitch / 2);
                        break;
                    case BONE_HEAD:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb.dir, sb.perp, HeadNeckTwist / 2, HeadNeckRoll / 2, HeadNeckPitch / 2);
                        break;
                    case BONE_SHOULDER:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb.dir, DegToRad(0), DegToRad(15));
                        break;
                    case BONE_UPPER_ARM:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb, Orient(Vec((sb.type_index >= 0) ? SQRT2_2 : -SQRT2_2, 0, SQRT2_2), Vec(0, 1, 0)), DegToRad(45), DegToRad(45), DegToRad(90));
                        break;
                    case BONE_UPPER_LEG:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb, Orient(Vec(0, -SQRT2_2, SQRT2_2), Vec(0, SQRT2_2, SQRT2_2)), DegToRad(45), DegToRad(45), DegToRad(45));
                        break;
                    case BONE_HAND:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb.dir, sb.perp, DegToRad(0), DegToRad(45) / 2, DegToRad(60));
                        break;
                    case BONE_FOOT:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb.dir, sb.perp, DegToRad(30), DegToRad(5), DegToRad(45) / 2);
                        break;

                    case BONE_LOWER_ARM: {
                        Vec axis = sp.perp;
                        if (sb.type_index >= 0)
                            axis.chs();
#if 0
                     Flt angle=Angle(sb.dir, Cross(axis, sp.dir), sp.dir), angle_offset=angle-PI_2;
#else // optimized
                        Flt angle_offset = Angle(sb.dir, sp.dir, Cross(sp.dir, axis));
#endif
                        CreateHinge(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, axis, 0 + angle_offset, DegToRad(140) + angle_offset);
                    } break;

                    case BONE_LOWER_LEG: {
                        C Vec &axis = sp.cross();
#if 0
                     Flt angle=Angle(sb.dir, sp.perp, sp.dir), angle_offset=PI_2-angle;
#else // optimized
                        Flt angle_offset = Angle(sb.dir, sp.dir, sp.perp);
#endif
                        CreateHinge(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, axis, 0 + angle_offset, DegToRad(140) + angle_offset);
                    } break;

                    default:
                        CreateSpherical(_joints.New(), rb.actor, rp.actor, sb.pos * _scale, sb.dir, DegToRad(30), DegToRad(30));
                        break;
                    }
                }
            }

            // ignore
            REPA(T)
            REPD(j, i)
            if (Cuts(shapes[i], shapes[j]))
                bone(i).actor.ignore(bone(j).actor);
        }
        _aggr.create(bones());
        REPA(T)
        _aggr.add(bone(i).actor);
        return true;
    }
    return false;
}
/******************************************************************************/
Ragdoll &Ragdoll::create(C AnimatedSkeleton &anim_skel, Flt scale, Flt density, Bool kinematic) {
    if (!createTry(anim_skel, scale, density, kinematic)) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::INFO, __FILE__, __LINE__,
            "Cannot Create Skeleton");
        return T;
    }
    return T;
}
/******************************************************************************
void updateShapes(AnimatedSkeleton &anim_skel); // update 'shape_t' according to 'shape' and skeleton animation, 'anim_skel' must be set to the same skeleton which ragdoll was created from
void Ragdoll::updateShape(AnimatedSkeleton &anim_skel)
{
   if(T.skel==anim_skel.skeleton())REP(bones)shape_t[i]=shape[i]*anim_skel.bone(bone[i].skel_bone).matrix;
}
/******************************************************************************/
Ragdoll &Ragdoll::fromSkel(C AnimatedSkeleton &anim_skel, C Vec &vel, Bool immediate_even_for_kinematic_ragdoll) {
    if (_skel == anim_skel.skeleton()) {
        Bool scaled = (_scale != 1),
             kinematic = (immediate_even_for_kinematic_ragdoll ? false : T.kinematic());
        REPA(T) {
            Bone &rbon = bone(i);
            Actor &actor = rbon.actor;

#if 1
            Matrix matrix = anim_skel.bones[rbon.skel_bone]._matrix;
#else
            Matrix matrix = (i ? anim_skel.bone(rbon.skel_bone)._matrix : anim_skel.matrix);
#endif
            if (scaled)
                matrix.orn() /= _scale;

            if (kinematic)
                actor.kinematicMoveTo(matrix);
            else
                actor.matrix(matrix).vel(vel).angVel(VecZero);
        }
    }
    return T;
}
Ragdoll &Ragdoll::toSkel(AnimatedSkeleton &anim_skel) {
    if (_skel == anim_skel.skeleton() && bones()) {
        C Skeleton &skel = *_skel;

        // reset the orientation of non-ragdoll bones (and main and root) for default pose (the one from default Skeleton)
        anim_skel.root.clear();
        REPA(_resets)
        anim_skel.bones[_resets[i]].clear();
#if 0
      {
         BoneType  sbone    =_resets[i];
         Orient   &bone_orn = anim_skel.bone(sbone).orn;
         SkelBone &skel_bone=      skel.bone(sbone);
         BoneType  sparent  =      skel_bone.parent;
         if(sparent==BONE_NULL)bone_orn=GetAnimOrient(skel_bone);
         else                  bone_orn=GetAnimOrient(skel_bone, &skel.bone(sparent));
      }
#endif

        // set bone oriantation according to actors
        Matrix body = bone(0).actor.matrix();
        Matrix3 ibody;
        body.orn().inverse(ibody, true);
        for (Int i = 1; i < bones(); i++) // skip the main bone (zero) because it's set in the reset
        {
            Bone &rbon = bone(i);
            BoneType sbone = rbon.skel_bone,
                     rparent = rbon.rbon_parent;
            AnimSkelBone &asbon = anim_skel.bones[sbone];
            C SkelBone &skel_bone = skel.bones[sbone];

            asbon.clear();
            if (InRange(rparent, T))
                asbon.orn = GetAnimOrient(skel_bone, rbon.actor.orn(), &skel.bones[skel_bone.parent], &NoTemp(bone(rparent).actor.orn()));
            else
                asbon.orn = GetAnimOrient(skel_bone, rbon.actor.orn() * ibody);
        }

        body.scaleOrn(_scale);
        anim_skel.updateMatrix(body);
    }
    return T;
}
Ragdoll &Ragdoll::toSkelBlend(AnimatedSkeleton &anim_skel, Flt blend) {
    if (_skel == anim_skel.skeleton() && bones()) {
        Flt blend1 = 1 - blend;
        C Skeleton &skel = *_skel;

        // reset the orientation of non-ragdoll bones (and main) for default pose (the one from default Skeleton)
        REPA(_resets) {
            BoneType sbone = _resets[i];
            AnimSkelBone &asbon = anim_skel.bones[sbone];
            C SkelBone &skel_bone = skel.bones[sbone];
            BoneType sparent = skel_bone.parent;

            asbon.orn *= blend1;
            asbon.pos *= blend1;
#if HAS_ANIM_SKEL_ROT
            asbon.rot *= blend1;
#endif
            if (sparent == BONE_NULL)
                asbon.orn += blend * GetAnimOrient(skel_bone);
            else
                asbon.orn += blend * GetAnimOrient(skel_bone, &skel.bones[sparent]);
        }

        // set bone oriantation according to actors
        Matrix ragdoll_body = bone(0).actor.matrix();
        Matrix3 ibody;
        ragdoll_body.orn().inverse(ibody, true);
        for (Int i = 1; i < bones(); i++) // skip the main bone (zero) because it's set in the reset
        {
            Bone &rbon = bone(i);
            BoneType sbone = rbon.skel_bone,
                     rparent = rbon.rbon_parent;
            AnimSkelBone &asbon = anim_skel.bones[sbone];
            C SkelBone &skel_bone = skel.bones[sbone];

            asbon.orn *= blend1;
            asbon.pos *= blend1;
#if HAS_ANIM_SKEL_ROT
            asbon.rot *= blend1;
#endif
            if (InRange(rparent, T))
                asbon.orn += blend * GetAnimOrient(skel_bone, rbon.actor.orn(), &skel.bones[skel_bone.parent], &NoTemp(bone(rparent).actor.orn()));
            else
                asbon.orn += blend * GetAnimOrient(skel_bone, rbon.actor.orn() * ibody);
        }

        // convert root to matrix, and zero himself so it won't be taken into account
        Matrix temp = anim_skel.matrix();
        temp.normalize();
        anim_skel.root.clear();

        // blend matrix with ragdoll main bone matrix
        ragdoll_body *= blend;
        temp *= blend1;
        temp += ragdoll_body;
        temp.normalize().scaleOrn(_scale);

        // update skeleton according to obtained matrix
        anim_skel.updateMatrix(temp);
    }
    return T;
}
/******************************************************************************
Flt area  (); // total surface area of shapes
Flt volume(); // total volume       of shapes
Flt Ragdoll::area  (){Flt a=0; REP(_bones)a+=shape[i].area  (); return a*Sqr (scale);}
Flt Ragdoll::volume(){Flt v=0; REP(_bones)v+=shape[i].volume(); return v*Cube(scale);}
/******************************************************************************/
Ragdoll &Ragdoll::pos(C Vec &pos) {
    Vec delta = pos - T.pos();
    REPAO(_bones).actor.pos(bone(i).actor.pos() + delta);
    return T;
}
Ragdoll &Ragdoll::vel(C Vec &vel) {
    REPAO(_bones).actor.vel(vel);
    return T;
}
Ragdoll &Ragdoll::damping(Flt damping) {
    REPAO(_bones).actor.damping(damping);
    return T;
}
Ragdoll &Ragdoll::adamping(Flt damping) {
    REPAO(_bones).actor.adamping(damping);
    return T;
}
Ragdoll &Ragdoll::kinematic(Bool on) {
    REPAO(_bones).actor.kinematic(on);
    return T;
}
Ragdoll &Ragdoll::gravity(Bool on) {
    REPAO(_bones).actor.gravity(on);
    return T;
}
Ragdoll &Ragdoll::ray(Bool on) {
    REPAO(_bones).actor.ray(on);
    return T;
}
Ragdoll &Ragdoll::collision(Bool on) {
    REPAO(_bones).actor.collision(on);
    return T;
}
Ragdoll &Ragdoll::active(Bool on) {
    REPAO(_bones).actor.active(on);
    return T;
}
Ragdoll &Ragdoll::sleep(Bool sleep) {
    REPAO(_bones).actor.sleep(sleep);
    return T;
}
Ragdoll &Ragdoll::sleepEnergy(Flt energy) {
    REPAO(_bones).actor.sleepEnergy(energy);
    return T;
}
Ragdoll &Ragdoll::ccd(Bool on) {
    REPAO(_bones).actor.ccd(on);
    return T;
}
Ragdoll &Ragdoll::user(Ptr user) {
    REPAO(_bones).actor.user(user);
    return T;
}
Ragdoll &Ragdoll::obj(Ptr obj) {
    REPAO(_bones).actor.obj(obj);
    return T;
}
Ragdoll &Ragdoll::group(Byte group) {
    REPAO(_bones).actor.group(group);
    return T;
}
Ragdoll &Ragdoll::dominance(Byte dominance) {
    REPAO(_bones).actor.dominance(dominance);
    return T;
}
Ragdoll &Ragdoll::material(PhysMtrl *material) {
    REPAO(_bones).actor.material(material);
    return T;
}
Vec Ragdoll::pos() C { return bones() ? bone(0).actor.pos() : 0; }
Vec Ragdoll::vel() C { return bones() ? bone(0).actor.vel() : 0; }
Flt Ragdoll::damping() C { return bones() ? bone(0).actor.damping() : 0; }
Flt Ragdoll::adamping() C { return bones() ? bone(0).actor.adamping() : 0; }
Bool Ragdoll::kinematic() C { return bones() ? bone(0).actor.kinematic() : false; }
Bool Ragdoll::gravity() C { return bones() ? bone(0).actor.gravity() : false; }
Bool Ragdoll::ray() C { return bones() ? bone(0).actor.ray() : false; }
Bool Ragdoll::collision() C { return bones() ? bone(0).actor.collision() : false; }
Ptr Ragdoll::user() C { return bones() ? bone(0).actor.user() : null; }
Ptr Ragdoll::obj() C { return bones() ? bone(0).actor.obj() : null; }
Byte Ragdoll::group() C { return bones() ? bone(0).actor.group() : 0; }
Byte Ragdoll::dominance() C { return bones() ? bone(0).actor.dominance() : 0; }
PhysMtrl *Ragdoll::material() C { return bones() ? bone(0).actor.material() : null; }
Bool Ragdoll::sleep() C { return bones() ? bone(0).actor.sleep() : false; }
Flt Ragdoll::sleepEnergy() C { return bones() ? bone(0).actor.sleepEnergy() : 0; }
Bool Ragdoll::ccd() C { return bones() ? bone(0).actor.ccd() : false; }
/******************************************************************************/
Ragdoll &Ragdoll::ignore(Actor &actor, Bool ignore) {
    REPAO(_bones).actor.ignore(actor, ignore);
    return T;
}
/******************************************************************************/
Int Ragdoll::findBoneI(CChar8 *name) {
    REPA(T)
    if (Equal(bone(i).name, name))
        return i;
    return -1;
}
Ragdoll::Bone *Ragdoll::findBone(CChar8 *name) {
    Int i = findBoneI(name);
    return (i < 0) ? null : &bone(i);
}
Int Ragdoll::getBoneI(CChar8 *name) {
    Int i = findBoneI(name);
    if (i < 0)
        Exit(S + "Bone \"" + name + "\" not found in Ragdoll.");
    return i;
}
Ragdoll::Bone &Ragdoll::getBone(CChar8 *name) { return bone(getBoneI(name)); }
/******************************************************************************/
Int Ragdoll::findBoneIndexFromSkelBone(BoneType skel_bone_index) C {
    if (bones()) {
        if (_skel && InRange(skel_bone_index, _skel->bones))
            for (;;) {
                if (_skel->bones[skel_bone_index].flag & BONE_RAGDOLL) // if skeleton bone should contain a bone in the ragdoll
                {
                    REPA(T)
                    if (bone(i).skel_bone == skel_bone_index)
                        return i;
                }
                skel_bone_index = _skel->bones[skel_bone_index].parent;
                if (skel_bone_index == BONE_NULL)
                    break;
            }
        return 0;
    }
    return -1;
}
Int Ragdoll::findBoneIndexFromVtxMatrix(BoneType matrix_index) C {
    return findBoneIndexFromSkelBone(matrix_index - 1);
}
/******************************************************************************/
void Ragdoll::draw(C Color &col) C {
    REPAO(_bones).actor.draw(col);
}
/******************************************************************************/
#pragma pack(push, 1)
struct RagdollDesc {
    Byte group, dominance, material;
    UInt user, flag;
    Flt sleep_energy;
};
struct RagdollActorDesc {
    Vec vel, ang_vel;
    Matrix matrix;
};
#pragma pack(pop)

Bool Ragdoll::saveState(File &f, Bool include_matrix_vel) C {
    f.cmpUIntV(0);

    RagdollDesc desc;

    _Unaligned(desc.material, 0);
    Unaligned(desc.group, group());
    Unaligned(desc.dominance, dominance());
    _Unaligned(desc.user, (UIntPtr)user());
    Unaligned(desc.sleep_energy, sleepEnergy());

    UInt flag = 0;
    if (kinematic())
        flag |= ACTOR_KINEMATIC;
    if (gravity())
        flag |= ACTOR_GRAVITY;
    if (ray())
        flag |= ACTOR_RAY;
    if (collision())
        flag |= ACTOR_COLLISION;
    if (sleep())
        flag |= ACTOR_SLEEP;
    if (ccd())
        flag |= ACTOR_CCD;
    Unaligned(desc.flag, flag);

    f << desc;
    f.putBool(include_matrix_vel);
    if (include_matrix_vel) {
        f.putInt(bones());
        FREPA(T) {
            C Actor &actor = bone(i).actor;
            RagdollActorDesc ad;
            Unaligned(ad.vel, actor.vel());
            Unaligned(ad.ang_vel, actor.angVel());
            Unaligned(ad.matrix, actor.matrix());
            f << ad;
        }
    }
    return f.ok();
}
Bool Ragdoll::loadState(File &f) // don't delete on fail, as here we're loading only state
{
    switch (f.decUIntV()) {
    case 0: {
        RagdollDesc desc;
        if (f.getFast(desc)) {
            group(Unaligned(desc.group));
            dominance(Unaligned(desc.dominance));
            user(Ptr(Unaligned(desc.user)));
            sleepEnergy(Unaligned(desc.sleep_energy));

            kinematic(FlagOn(Unaligned(desc.flag), ACTOR_KINEMATIC));
            gravity(FlagOn(Unaligned(desc.flag), ACTOR_GRAVITY));
            ray(FlagOn(Unaligned(desc.flag), ACTOR_RAY));
            collision(FlagOn(Unaligned(desc.flag), ACTOR_COLLISION));
            sleep(FlagOn(Unaligned(desc.flag), ACTOR_SLEEP));
            ccd(FlagOn(Unaligned(desc.flag), ACTOR_CCD));

            if (f.getBool()) {
                Int bones = f.getInt();
                if (bones != T.bones()) // number of bones doesn't match (we're dealing with a different ragdoll than before)
                {
                    if (bones >= 1) // if there was at least one bone saved
                    {
                        RagdollActorDesc ad;
                        f >> ad;                        // load first bone
                        f.skip((bones - 1) * SIZE(ad)); // skip the rest
                        FREPA(T)                        // set all bones basing on the first
                        {
                            bone(i).actor.vel(Unaligned(ad.vel)).angVel(Unaligned(ad.ang_vel)).matrix(Unaligned(ad.matrix));
                        }
                    }
                } else
                    FREPA(T) {
                        RagdollActorDesc ad;
                        f >> ad;
                        bone(i).actor.vel(Unaligned(ad.vel)).angVel(Unaligned(ad.ang_vel)).matrix(Unaligned(ad.matrix));
                    }
            }
            if (f.ok())
                return true;
        }
    } break;
    }
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
