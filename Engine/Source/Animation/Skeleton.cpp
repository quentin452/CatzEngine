﻿/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_SKEL CC4('S','K','E','L')

// prioritize type so if we want to share animation between multiple skeletons, and one skeleton has bones: "Hips" BONE_SPINE 0:0, "Spine" BONE_SPINE 0:1, and another has "Spine" BONE_SPINE 0:0, "Spine1" BONE_SPINE 0:1, then we will match by type/index first
#define FIND_BONE_PRIORITY_NAME 1
#define FIND_BONE_PRIORITY_TYPE 2

#define BONE_FRAC 0.3f
/******************************************************************************/
Cache<Skeleton> Skeletons("Skeleton");
/******************************************************************************/
static Str SkelName(C Skeleton *skel)
{
   if(CChar *name=Skeletons.name(skel))return S+" \""+name+'"';
   return S;
}
static Bool Singular(BONE_TYPE type) // bones which usually occur single
{
   switch(type)
   {
      case BONE_UNKNOWN:
      case BONE_SPINE  :
      case BONE_NECK   :
      case BONE_HEAD   :
      case BONE_JAW    :
      case BONE_TONGUE :
      case BONE_NOSE   :
      case BONE_TAIL   :
      case BONE_CAPE   : return true;
      default          : return false;
   }
}
static CChar8* BoneName_t[]=
{
   "Unknown",
   "Spine",
   "Shoulder",
   "UpperArm",
   "Forearm",
   "Hand",
   "Finger",
   "UpperLeg",
   "LowerLeg",
   "Foot",
   "Toe",
   "Neck",
   "Head",
   "Jaw",
   "Tongue",
   "Nose",
   "Eye",
   "Eyelid",
   "Eyebrow",
   "Ear",
   "Breast",
   "Butt",
   "Tail",
   "Wing",
   "Cape",
   "Hair",
}; ASSERT(Elms(BoneName_t)==BONE_NUM);
CChar8* BoneName(BONE_TYPE type) {return InRange(type, BoneName_t) ? BoneName_t[type] : null;}
/******************************************************************************/
// SKELETON SLOT
/******************************************************************************/
SkelSlot::SkeletonSlot()
{
   dir .set (0, 0, 1);
   perp.set (0, 1, 0);
   pos .zero(       );

   name[0]=0;
   setParent(BONE_NULL);
}
SkelSlot& SkelSlot::operator*=(Flt f)
{
   pos*=f;
   return T;
}
void SkelSlot::save(TextNode &node, C Skeleton *owner)C
{
   node.set(name);
   super::save(node.nodes);
   if(owner)
   {
      if(InRange(bone , owner->bones))node.nodes.New().set("Bone" , owner->bones[bone ].name);
      if(InRange(bone1, owner->bones))node.nodes.New().set("Bone1", owner->bones[bone1].name);
   }
}
/******************************************************************************/
// BONE ID
/******************************************************************************/
BoneID& BoneID::set(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)
{
   Set(T.name, name); T.type=type; T.type_index=type_index; T.type_sub=type_sub; return T;
}
Bool BoneID::operator==(C BoneID &id)C
{
   return Equal(name, id.name)
       && type      ==id.type
       && type_index==id.type_index
       && type_sub  ==id.type_sub;
}
/******************************************************************************/
// SKELETON BONE
/******************************************************************************/
SkelBone::SkeletonBone()
{
   dir .set (0, 0, 1);
   perp.set (0, 1, 0);
   pos .zero(       );

   parent=BONE_NULL;
   children_offset=children_num=0;
   flag  =0;
   length=0.3f;
   width =0.2f;
   offset.zero();
   shape .set(radius(), length, Vec(0, 0, 0.5f));
}
void SkelBone::draw(C Color &color)C
{
   Vec drf=pos+dir*length*BONE_FRAC,
       to =pos+dir*length;
   Vec p =      perp; p*=radius()*SQRT2_2;
   Vec pp=Cross(dir , p);
   Vec p1=drf+p+pp,
       p2=drf+p-pp,
       p3=drf-p-pp,
       p4=drf-p+pp;

   VI.color(color);
   VI.line(to, p1); VI.line(pos, p1);
   VI.line(to, p2); VI.line(pos, p2);
   VI.line(to, p3); VI.line(pos, p3);
   VI.line(to, p4); VI.line(pos, p4);
   VI.line(p1, p2);
   VI.line(p2, p3);
   VI.line(p3, p4);
   VI.line(p4, p1);
   VI.line(pos, pos+p);
   VI.end();
}
void SkelBone::save(TextNode &node, C Skeleton *owner)C
{
   node.set(name);
   super::save(node.nodes);
   node.nodes.New().set("Length", length);
   node.nodes.New().set("Width" , width );
   node.nodes.New().set("Flag"  , flag  );
   if(owner && InRange(parent, owner->bones))node.nodes.New().set("Parent", owner->bones[parent].name);
}
/******************************************************************************/
SkelBone& SkelBone::operator+=(C Vec &v)
{
   pos  +=v;
   shape+=v;
   return T;
}
SkelBone& SkelBone::operator-=(C Vec &v)
{
   pos  -=v;
   shape-=v;
   return T;
}
SkelBone& SkelBone::operator*=(Flt f)
{
   pos   *=f;
   length*=f;
   offset*=f;
   shape *=f;
   return T;
}
SkelBone& SkelBone::operator*=(C Vec &v)
{
   pos   *=v;
   dir   *=v;
   perp  *=v;
   length*=dir.normalize();
   offset*=v;
   shape *=v;
   fixPerp();
   return T;
}
/******************************************************************************/
SkelBone& SkelBone::operator*=(C Matrix3 &matrix)
{
   pos   *=matrix;
   dir   *=matrix;
   perp  *=matrix;
   length*=dir.normalize();
   offset*=matrix;
   shape *=matrix;
   fixPerp();
   return T;
}
SkelBone& SkelBone::operator/=(C Matrix3 &matrix)
{
   pos   /=matrix;
   dir   /=matrix;
   perp  /=matrix;
   length*=dir.normalize(); // 'length' should indeed be multiplied here because 'dir' already got transformed in correct way
   offset/=matrix;
   shape /=matrix;
   fixPerp();
   return T;
}
/******************************************************************************/
SkelBone& SkelBone::operator*=(C Matrix &matrix)
{
   pos   *=matrix;
   dir   *=matrix.orn();
   perp  *=matrix.orn();
   length*=dir   .normalize();
   offset*=matrix.orn(); // !! position should not be applied here !!
   shape *=matrix;
   fixPerp();
   return T;
}
SkelBone& SkelBone::operator/=(C Matrix &matrix)
{
   pos   /=matrix;
   dir   /=matrix.orn();
   perp  /=matrix.orn();
   length*=dir   .normalize(); // 'length' should indeed be multiplied here because 'dir' already got transformed in correct way
   offset/=matrix.orn(); // !! position should not be applied here !!
   shape /=matrix;
   fixPerp();
   return T;
}
/******************************************************************************/
SkelBone& SkelBone::operator*=(C MatrixM &matrix)
{
   pos   *=matrix;
   dir   *=matrix.orn();
   perp  *=matrix.orn();
   length*=dir   .normalize();
   offset*=matrix.orn(); // !! position should not be applied here !!
   shape *=matrix;
   fixPerp();
   return T;
}
SkelBone& SkelBone::operator/=(C MatrixM &matrix)
{
   pos   /=matrix;
   dir   /=matrix.orn();
   perp  /=matrix.orn();
   length*=dir   .normalize(); // 'length' should indeed be multiplied here because 'dir' already got transformed in correct way
   offset/=matrix.orn(); // !! position should not be applied here !!
   shape /=matrix;
   fixPerp();
   return T;
}
/******************************************************************************/
SkelBone& SkelBone::mirrorX    () {super::mirrorX    (); CHS       (offset.x); return T;}
SkelBone& SkelBone::mirrorY    () {super::mirrorY    (); CHS       (offset.y); return T;}
SkelBone& SkelBone::mirrorZ    () {super::mirrorZ    (); CHS       (offset.z); return T;}
SkelBone& SkelBone::rightToLeft() {super::rightToLeft(); offset.rightToLeft(); return T;}
/******************************************************************************/
SkelBone& SkelBone::setFromTo(C Vec &from, C Vec &to)
{
   Vec  f=from; // operate on copy in case 'from' is set to 'pos' or 'dir'
   dir   =to-f; // set 'dir' first because it uses 2 variables
   pos   =   f; // set 'pos' next and from the backup variable in case 'from' was set to 'dir' which is now different
   length=dir.normalize();
   fixPerp();
   return T;
}
/******************************************************************************/
// SKELETON
/******************************************************************************/
Skeleton::Skeleton() : _skel_anims("Animation", 8)
{
  _skel_anims.setLoadUser(this);
}
Skeleton::Skeleton(C Skeleton &src) : Skeleton() {T=src;}
Skeleton& Skeleton::del()
{
  _skel_anims.del();
   bones     .del();
   slots     .del();
   return T;
}
Skeleton& Skeleton::operator=(C Skeleton &src)
{
   if(this!=&src)
   {
     _skel_anims.del();

      bones=src.bones;
      slots=src.slots;
   }
   return T;
}
/******************************************************************************/
Skeleton& Skeleton::move(C Vec &move)
{
   REPAO(bones)+=move;
   REPAO(slots)+=move;
   return T;
}
Skeleton& Skeleton::scale(Flt scale)
{
   REPAO(bones)*=scale;
   REPAO(slots)*=scale;
   return T;
}
Skeleton& Skeleton::scale(C Vec &scale)
{
   REPAO(bones)*=scale;
   REPAO(slots)*=scale;
   return T;
}
Skeleton& Skeleton::scaleMove(C Vec &scale, C Vec &move)
{
   REPA(bones)(bones[i]*=scale)+=move;
   REPA(slots)(slots[i]*=scale)+=move;
   return T;
}
Skeleton& Skeleton::transform(C Matrix3 &matrix)
{
   REPAO(bones)*=matrix;
   REPAO(slots)*=matrix;
   return T;
}
Skeleton& Skeleton::transform(C Matrix &matrix)
{
   REPAO(bones)*=matrix;
   REPAO(slots)*=matrix;
   return T;
}
Skeleton& Skeleton::animate(C AnimatedSkeleton &skel)
{
   if(bones.elms()==skel.bones.elms())REPAO(bones)*=skel.bones[i].matrix();
   if(slots.elms()==skel.slots.elms())REPA (slots)
   {
      SkelSlot &slot=slots[i];
      slot=skel.slots[i];
      slot.fix(); // fix in case 'skel.slot' was scaled or not orthogonal
   }
   return T;
}
Skeleton& Skeleton::mirrorX()
{
   REPAO(bones).mirrorX();
   REPAO(slots).mirrorX();
   return T;
}
Skeleton& Skeleton::mirrorY()
{
   REPAO(bones).mirrorY();
   REPAO(slots).mirrorY();
   return T;
}
Skeleton& Skeleton::mirrorZ()
{
   REPAO(bones).mirrorZ();
   REPAO(slots).mirrorZ();
   return T;
}
Skeleton& Skeleton::rightToLeft()
{
   REPAO(bones).rightToLeft();
   REPAO(slots).rightToLeft();
   return T;
}
/******************************************************************************/
SkelBone* Skeleton::findBone (BONE_TYPE type, Int type_index, Int type_sub)  {return bones.addr(findBoneI(type, type_index, type_sub));}
Byte      Skeleton::findBoneB(BONE_TYPE type, Int type_index, Int type_sub)C {Int i=findBoneI(type, type_index, type_sub); return InRange(i, 256) ? i : 255;}
Int       Skeleton::findBoneI(BONE_TYPE type, Int type_index, Int type_sub)C
{
   if(type)REPA(bones)
   {
    C SkelBone &bone=bones[i];
      if(bone.type==type && bone.type_index==type_index && bone.type_sub==type_sub)return i;
   }
   return -1;
}
SkelBone* Skeleton::findBone (CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub) {return bones.addr(findBoneI(name, type, type_index, type_sub));}
Int       Skeleton::findBoneI(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)C
{
   Int  bone_index=-1, best_match=0;
   REPA(bones)
   {
    C SkelBone &bone=bones[i];
      Int match=0;
      if(Equal(bone.name, name))match+=FIND_BONE_PRIORITY_NAME;
      if(type && bone.type==type && bone.type_index==type_index && bone.type_sub==type_sub)match+=FIND_BONE_PRIORITY_TYPE; // check this only if 'type' is specified
      if(match>best_match)
      {
         bone_index=i;
         best_match=match;
         if(match==FIND_BONE_PRIORITY_NAME+FIND_BONE_PRIORITY_TYPE)break; // if found highest possible match then stop
      }
   }
   return bone_index;
}
Int Animation::findBoneI(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)C
{
   Int  bone_index=-1, best_match=0;
   REPA(bones)
   {
    C AnimBone &bone=bones[i];
      Int match=0;
      if(Equal(bone.name, name))match+=FIND_BONE_PRIORITY_NAME;
      if(type && bone.type==type && bone.type_index==type_index && bone.type_sub==type_sub)match+=FIND_BONE_PRIORITY_TYPE; // check this only if 'type' is specified
      if(match>best_match)
      {
         bone_index=i;
         best_match=match;
         if(match==FIND_BONE_PRIORITY_NAME+FIND_BONE_PRIORITY_TYPE)break; // if found highest possible match then stop
      }
   }
   return bone_index;
}
Int BoneMap::find(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)C
{
   Int  bone_index=-1, best_match=0;
   REP(_bones)
   {
    C Bone &bone=_bone[i];
      Int match=0;
      if(Equal(T.name(i), name))match+=FIND_BONE_PRIORITY_NAME;
      if(type && bone.type==type && bone.type_index==type_index && bone.type_sub==type_sub)match+=FIND_BONE_PRIORITY_TYPE; // check this only if 'type' is specified
      if(match>best_match)
      {
         bone_index=i;
         best_match=match;
         if(match==FIND_BONE_PRIORITY_NAME+FIND_BONE_PRIORITY_TYPE)break; // if found highest possible match then stop
      }
   }
   return bone_index;
}

Int       Skeleton::findBoneI(CChar8 *name                                )C {if(Is(name))REPA(bones)if(Equal(bones[i].name, name))return i; return -1;}
Int       Skeleton::findSlotI(CChar8 *name                                )C {if(Is(name))REPA(slots)if(Equal(slots[i].name, name))return i; return -1;}
Byte      Skeleton::findBoneB(CChar8 *name                                )C {Int i=findBoneI(name                      ); return InRange(i, 256) ? i : 255;}
Byte      Skeleton::findSlotB(CChar8 *name                                )C {Int i=findSlotI(name                      ); return InRange(i, 256) ? i : 255;}
Int       Skeleton:: getBoneI(BONE_TYPE type, Int type_index, Int type_sub)C {Int i=findBoneI(type, type_index, type_sub); if(i<0)Exit(S+"Bone "+BoneName(type)+':'+type_index+':'+type_sub+" not found in skeleton"+SkelName(this)+'.'); return i;}
Int       Skeleton:: getBoneI(CChar8 *name                                )C {Int i=findBoneI(name                      ); if(i<0)Exit(S+"Bone \""+  name                                +"\" not found in skeleton"+SkelName(this)+'.'); return i;}
Int       Skeleton:: getSlotI(CChar8 *name                                )C {Int i=findSlotI(name                      ); if(i<0)Exit(S+"Slot \""+  name                                +"\" not found in skeleton"+SkelName(this)+'.'); return i;}
SkelBone* Skeleton::findBone (CChar8 *name                                )  {return bones.addr(findBoneI(name));}
SkelSlot* Skeleton::findSlot (CChar8 *name                                )  {return slots.addr(findSlotI(name));}
SkelBone& Skeleton:: getBone (BONE_TYPE type, Int type_index, Int type_sub)  {return bones     [ getBoneI(type, type_index, type_sub)];}
SkelBone& Skeleton:: getBone (CChar8 *name                                )  {return bones     [ getBoneI(name)];}
SkelSlot& Skeleton:: getSlot (CChar8 *name                                )  {return slots     [ getSlotI(name)];}
/******************************************************************************/
Int Skeleton::parentlessBones()C
{
   Int    i=0; for(; i<bones.elms(); i++)if(bones[i].parent!=BONE_NULL)break;
   return i;
}
Bool Skeleton::contains(Int parent, Int child)C
{
   if(parent<0)parent=BONE_NULL;
   if(child <0)child =BONE_NULL;
   if(parent==child)return true; // if contains self ('parent' contains 'child' where both are the same)
   for(; InRange(child, bones); )
   {
      Int child_parent=bones[child].parent; // get 'child' parent
      if( child_parent==parent)return true;
      if( child_parent>=child )break; // proceed only if the parent has a smaller index (this solves the issue of never ending loops with incorrect data)
      child=child_parent;
   }
   return false;
}
Flt  Skeleton::volume()C {Flt vol=0; REPA(bones)vol+=bones[i].volume(); return vol;}
UInt Skeleton::memUsage()C
{
   return bones.memUsage()
         +slots.memUsage();
}
Int Skeleton::boneRoot(Int bone)C
{
   if(InRange(bone, bones))
   {
   again:
      Int parent=boneParent(bone); if(parent>=0){bone=parent; goto again;}
      return bone;
   }
   return -1;
}
Int Skeleton::boneParent(Int bone)C
{
   if(InRange(bone, bones))
   {
      Int parent=bones[bone].parent;
      if(InRange(parent, bones) && parent<bone)return parent; // parent index must always be smaller than of the bone
   }
   return -1;
}
Int Skeleton::boneParents(Int bone)C
{
   Int parents=0;
   for(;; parents++)
   {
      bone=boneParent(bone);
      if(bone<0)break;
   }
   return parents;
}
Int Skeleton::boneLevel(Int bone)C {return boneParents(bone)+InRange(bone, bones);}
Int Skeleton::findParent(Int bone, BONE_TYPE type)C
{
   for(;;)
   {
      bone=boneParent(bone); if(bone<0)break;
      if(bones[bone].type==type)return bone;
   }
   return -1;
}
Int Skeleton::findRagdollParent(Int bone_index)C
{
   for(; InRange(bone_index, bones); )
   {
    C SkelBone &bone=bones[bone_index];
      if(bone.flag&BONE_RAGDOLL)return bone_index;
      if(bone.parent>=bone_index)break; // proceed only if the parent has a smaller index (this solves the issue of never ending loops with incorrect data)
      bone_index=bone.parent;
   }
   return -1;
}
struct BoneParents
{
   BoneType bone, parents;
};
Int Skeleton::bonesSharedParent(MemPtrN<BoneType, 256> bones)C
{
   MemtN<BoneParents, 256> bone_parents;
   bone_parents.setNum(bones.elms());
   Int min_parents=-1;
   REPA(bones)
   {
      BoneParents &bp=bone_parents[i];
      bp.bone   =bones[i];
      bp.parents=boneParents(bp.bone);
      if(min_parents<0)min_parents=bp.parents;else MIN(min_parents, bp.parents);
   }
   if(min_parents>0) // there's at least one parent
   {
      // make all bones to be at the same depth
      REPA(bone_parents)
      {
         BoneParents &bp=bone_parents[i];
         REP(bp.parents-min_parents+1)bp.bone=T.bones[bp.bone].parent; // +1 to set all bones to their parents
      }
      // keep checking each parent for all bones if it's the same (starting from leaf to root)
      for(;;)
      {
         Int parent=bone_parents.last().bone;
         REP(bone_parents.elms()-1)if(bone_parents[i].bone!=parent)goto different;
         return parent; // all parents are the same
      different:
         if(!--min_parents)break;
         REPA(bone_parents)bone_parents[i].bone=T.bones[bone_parents[i].bone].parent;
      }
   }
   return BONE_NULL;
}
Int Skeleton::hierarchyDistance(Int bone_a, Int bone_b)C
{
   Int a_level=boneLevel(bone_a),
       b_level=boneLevel(bone_b);
   if(!a_level)return b_level;
   if(!b_level)return a_level;

   // put bones to the same depth
   Int min_level=Min(a_level, b_level);
   Int dist=a_level-min_level; REP(dist)bone_a=bones[bone_a].parent;
   Int d   =b_level-min_level; REP(d   )bone_b=bones[bone_b].parent; dist+=d;

   // find a shared bone
   for(;;)
   {
      if(bone_a==bone_b)break;
      dist+=2; // add 2 because each bone moves once
      if(!--min_level)break;
      bone_a=bones[bone_a].parent;
      bone_b=bones[bone_b].parent;
   }
   return dist;
}
/******************************************************************************/
void Skeleton::getSkin(C Vec &pos, VecB4 &blend, VtxBone &matrix)C
{
#if 0
   Int find[2]={-1, -1};
   Flt dist[2]={ 0,  0};
   REPA(bones)
   {
    C SkelBone &bone=bones[i];
      if(DistPointPlane(pos, bone.pos, bone.dir)>=-0.5f*bone.length*BONE_FRAC &&
         DistPointLine (pos, bone.pos, bone.dir)<= 2.5f*bone.radius())
      {
         Flt d=DistPointEdge(pos, bone.pos, bone.to());
         if(find[0]<0 || d<dist[0])
         {
            find[1]=find[0];
            dist[1]=dist[0];
            find[0]=i;
            dist[0]=d;
         }else
         if(find[1]<0 || d<dist[1])
         {
            find[1]=i;
            dist[1]=d;
         }
      }
   }
   if(find[0]<0)
   {
      matrix=0;
      blend .set(255, 0, 0, 0);
   }else
   if(find[1]<0)
   {
    C SkelBone &bone=bones[find[0]];
      matrix.c[0]=(                               1+find[0]    );
      matrix.c[1]=((bone.parent==BONE_NULL) ? 0 : 1+bone.parent);
      matrix.c[2]=0;
      matrix.c[3]=0;
      Byte b=RoundU(Sat(DistPointPlane(pos, bone.pos, bone.dir)/(BONE_FRAC*bone.length)+0.5f)*255);
      blend.set(b, 255-b, 0, 0);
   }else
   if(bones[find[0]].parent==find[1] || bones[find[1]].parent==find[0])
   {
      Int parent, child; MinMax(find[0], find[1], parent, child);
    C SkelBone &bone=bones[child];
      matrix.c[0]=1+child;
      matrix.c[1]=1+parent;
      matrix.c[2]=0;
      matrix.c[3]=0;
      Byte b=RoundU(Sat(DistPointPlane(pos, bone.pos, bone.dir)/(BONE_FRAC*bone.length)+0.5f)*255);
      blend.set(b, 255-b, 0, 0);
   }else
   /*if(smooth)
   {
      matrix.c[0]=1+find[0];
      matrix.c[1]=1+find[1];
      matrix.c[2]=0;
      matrix.c[3]=0;
      Byte b=RoundU(Sat(dist[0]/(dist[0]+dist[1]))*255);
      blend.set(b, 255-b, 0, 0);
   }else*/
   {
    C SkelBone &bone=bones[find[0]];
      matrix.c[0]=(                               1+find[0]    );
      matrix.c[1]=((bone.parent==BONE_NULL) ? 0 : 1+bone.parent);
      matrix.c[2]=0;
      matrix.c[3]=0;
      Byte b=RoundU(Sat(DistPointPlane(pos, bone.pos, bone.dir)/(BONE_FRAC*bone.length)+0.5f)*255);
      blend.set(b, 255-b, 0, 0);
   }
#else
   MemtN<IndexWeight, 256> skin;
   REPA(bones)
   {
    C SkelBone &bone=bones[i];
      Flt dist=DistPointPlane(pos, bone.pos, bone.dir);
      Flt frac=dist/bone.length;
      Flt intensity=1.5f-Abs(frac-0.5f);
      if(intensity>0)skin.New().set(i+VIRTUAL_ROOT_BONE, intensity);
   }
   SetSkin(skin, matrix, blend, this);
#endif
}
/******************************************************************************/
void Skeleton::boneRemap(C CMemPtrN<BoneType, 256> &old_to_new) // !! this does not modify 'children_offset' and 'children_num' !!
{
#if 1 // clear out of range
   REPA(bones){BoneType &b =bones[i].parent; b =(InRange(b , old_to_new) ? old_to_new[b ] : BONE_NULL);}
   REPA(slots){BoneType &b =slots[i].bone  ; b =(InRange(b , old_to_new) ? old_to_new[b ] : BONE_NULL);
               BoneType &b1=slots[i].bone1 ; b1=(InRange(b1, old_to_new) ? old_to_new[b1] : BONE_NULL);}
#else // keep out of range
   if(old_to_new.elms())
   {
      REPA(bones){BoneType &b =bones[i].parent; if(InRange(b , old_to_new))b =old_to_new[b ];}
      REPA(slots){BoneType &b =slots[i].bone  ; if(InRange(b , old_to_new))b =old_to_new[b ];
                  BoneType &b1=slots[i].bone1 ; if(InRange(b1, old_to_new))b1=old_to_new[b1];}
   }
#endif
}
Bool Skeleton::removeBone(Int i, MemPtrN<BoneType, 256> old_to_new)
{
   if(InRange(i, bones))
   {
      SkelBone &bone=bones[i];

   #if 0 // this is not done, instead 'setBoneTypes' is called to handle 'type_index' as well
      // adjust 'type_sub', performing this is optional
      if(bone.type)for(Int j=i+1; j<bones.elms(); j++)
      {
         SkelBone &test=bones[j];
         if(test.type==bone.type && test.type_index==bone.type_index && test.type_sub>bone.type_sub)test.type_sub--;
      }
   #endif

      MemtN<BoneType, 256> otn_rem; otn_rem.setNum(bones.elms());
      FREPD(j, i)                        otn_rem[j]=j          ; // all before 'i' are not changed
                                         otn_rem[i]=bone.parent; // 'i' is changed to parent of removed bone (parent index is always smaller than that bone)
      for(Int j=i+1; j<bones.elms(); j++)otn_rem[j]=j-1        ; // all after 'i' are set to index-1

      bones.remove(i, true);
      boneRemap(otn_rem);

      // since we're assigning children of 'bone' to its parent, we need to sort the bones
      MemtN<BoneType, 256> otn_sort; sortBones(otn_sort);

      setBoneTypes(); // call this because 'type_index' and 'type_sub' could have changed

      if(old_to_new)
      {
         old_to_new.setNum(otn_rem.elms());
         REPA(old_to_new)
         {
            BoneType otn=otn_rem[i];
            old_to_new[i]=(InRange(otn, otn_sort) ? otn_sort[otn] : BONE_NULL);
         }
      }
      return true;
   }
   old_to_new.clear();
   return false;
}
Skeleton& Skeleton::add(C Skeleton &src, MemPtrN<BoneType, 256> old_to_new) // !! assumes that skeletons have different bone names !!
{
   if(&src==this)return T;
   Int offset=bones.elms();
   FREPA(src.bones) // copy in the same order
   {
    C SkelBone &s=src.bones[i];
      SkelBone &d=    bones.New();
      d=s; if(d.parent!=BONE_NULL)d.parent+=offset;
   }
   FREPA(src.slots) // copy in the same order
   {
    C SkelSlot &s=src.slots[i];
      SkelSlot &d=    slots.New();
      d=s;
      if(d.bone !=BONE_NULL)d.bone +=offset;
      if(d.bone1!=BONE_NULL)d.bone1+=offset;
   }
   return sortBones(old_to_new).setBoneTypes();
}
Skeleton& Skeleton::addSlots(C Skeleton &src)
{
   if(&src==this)return T;
   FREPA(src.slots) // copy in the same order
   {
    C SkelSlot &s=src.slots[i];
      SkelSlot &d=    slots.New();
      d=s;

      d.bone=BONE_NULL;
      if(C SkelBone *bone=src.bones.addr(s.bone)) // get bone in 'src' skeleton that 's' slot is attached to
      {
         Int i=findBoneI(bone->name, bone->type, bone->type_index, bone->type_sub); // find that bone in this skeleton
         if(i>=0)d.bone=i;
      }

      d.bone1=BONE_NULL;
      if(C SkelBone *bone=src.bones.addr(s.bone1)) // get bone in 'src' skeleton that 's' slot is attached to
      {
         Int i=findBoneI(bone->name, bone->type, bone->type_index, bone->type_sub); // find that bone in this skeleton
         if(i>=0)d.bone1=i;
      }
   }
   return T;
}
/******************************************************************************/
static void AddChildren(Skeleton &skeleton, MemtN<Int, 256> &order, BoneType parent)
{
   Int start=order.elms(), // number of bones added at this point
       elms =Min(skeleton.bones.elms(), BONE_NULL); // don't process bone with index BONE_NULL because this is assumed to be <null> and would trigger adding bones from the start (never ending loop)
   SkelBone *parent_bone=skeleton.bones.addr(parent); // get parent bone (if any) for further processing
   if(       parent_bone)parent_bone->children_offset=order.elms(); // set parent children offset
   FREP(elms) // process forward, to try preserving existing order, this is very important as some codes may need this
      if(skeleton.bones[i].parent==parent) // if bone is a child of current parent
   {
      if(parent_bone)parent_bone->children_num++; // increase children number
      order.add(i); // add i-th bone to the order of added bones
   }
   Int end=order.elms(); // number of bones added at this point !! keep it as a variable because calls to 'AddChildren' below will change the order.elms !!
   for(Int i=start; i<end; i++) // process forward, to try preserving existing order
      AddChildren(skeleton, order, order[i]); // add children of bones that were added in above step
}
Skeleton& Skeleton::sortBones(MemPtrN<BoneType, 256> old_to_new)
{
   REPA(bones){SkelBone &bone=bones[i]; bone.children_offset=bone.children_num=0;} // reset data first

   MemtN<Int     , 256> order; AddChildren(T, order, BONE_NULL);
   MemtN<BoneType, 256> otn  ; otn .setNum(bones.elms()); REPAO(otn)=BONE_NULL;
   Mems <SkelBone     > temp ; temp.setNum(order.elms());

   REPA(order)
   {
      Int  old =order[i];
      otn [old]=i;
      temp[i  ]=bones[old];
   }
   Swap(temp, bones);
   boneRemap(otn);
   if(old_to_new)old_to_new=otn;

   return T;
}
Skeleton& Skeleton::setBoneLengths()
{
   const Flt min_bone_length=0.005f;

   // maximize lengths of bones which have children
   FREPA(bones) // process in order because we're first clearing the bone lengths
   {
      SkelBone &bone=bones[i]; bone.length=0;
      if(InRange(bone.parent, bones))
      {
         SkelBone &parent=bones[bone.parent];
         MAX(parent.length, DistPointPlane(bone.pos, parent.pos, parent.dir));
      }
   }

   // calculate average bone length
   Flt avg_length=0;
   Int avg_length_num=0;
   REPA(bones)if(Flt length=bones[i].length)
   {
      avg_length+=length;
      avg_length_num++;
   }

   if(avg_length_num)avg_length/=avg_length_num;else
   if(bones.elms()  )
   {
      Box box=bones[0].pos; REPA(bones)box.include(bones[i].pos);
      avg_length=box.size().length()*0.08f;
   }
   MAX(avg_length, min_bone_length);

   // set lengths for bones which weren't set yet
   Flt min_length=Max(min_bone_length, avg_length*0.05f);
   FREPA(bones) // process in order because we're checking parents, so we want to process them first
   {
      SkelBone &bone=bones[i];
      if(bone.length<=min_length)bone.length=(InRange(bone.parent, bones) ? bones[bone.parent].length : avg_length);
   }
   return T;
}
/******************************************************************************/
static inline Bool Lower(Char8 c) {return c>='a' && c<='z';}
static inline Bool Upper(Char8 c) {return c>='A' && c<='Z';}

static Bool BoneName(C SkelBone &bone, CChar8 *name) // this works as 'Contains' with extra checking for previous/next characters
{
   if(Int length=Length(name))
      for(CChar8 *start=bone.name; start=TextPos(start, name); )
   {
      Bool all_upper=true; REP(length)if(!Upper(start[i])){all_upper=false; break;} // check if name is all uppercase character
      if(start!=bone.name) // check previous character (if there's any)
      {
         if(all_upper && Upper(start[-1]))goto next; // bone name is all uppercase and previous character is also uppercase then fail, to reject "WHIP" as "Hip"
         if(Lower(*start)) // if found text starts from a lowercase character
         {
            Char8 p=start[-1]; // get previous character
            if(Lower(p) || Upper(p))goto next; // if it's a character
         }
         Bool lower=false; REP(length)if(Lower(start[i])){lower=true; break;} // check if there's at least one lowercase character
         if(  lower) // check only if at least one lowercase, don't check if all uppercase
            REP(length)
               if(Upper(start[i])  // if this is an uppercase character
               && Lower(name [i])) // but the requested character is lowercase (uppercase not allowed) then fail, this is for things like "HipSHJnt" to be detected as "Hip" instead of "Hips"
                  goto next;
      }
      if(             Lower(start[length]))goto next; // if next character is lowercase then fail
      if(all_upper && Upper(start[length]))goto next; // bone name is all uppercase and next character is also uppercase then fail, to reject "HANDLE" as "Hand"

      return true;
   next:
      start++;
   }
   return false;
}
struct BoneTypeIndexSetter
{
   struct Bone
   {
      Bool     start, side_bool;
      SByte    side;
      BoneType bone_i, depth,
             //child_i,
               bone_start_i, // index in which initially it was added to the 'bone_start'
               parent_start_i; // index of parent in 'bone_start'
      Flt      order;
   };
   static Int CompareChild(C Bone &a, C Bone &b)
   {
      if(Int c=Compare(a.side_bool, b.side_bool))return c;
        return Compare(a.order    , b.order);
   }
   static Int CompareGlobal(C Bone &a, C Bone &b)
   {
      if(Int c=Compare(a.side_bool     , b.side_bool     ))return c;
      if(Int c=Compare(a.parent_start_i, b.parent_start_i))return c;
      if(Int c=Compare(a.depth         , b.depth         ))return c;
      if(Int c=Compare(a.bone_start_i  , b.bone_start_i  ))return c;
    //if(Int c=Compare(a.child_i       , b.child_i       ))return c;
    //if(Int c=Compare(a.order         , b.order         ))return c;
      return 0;
   }

   MemtN<Bone, 256> bone_start;
   Skeleton &skel;

   void flood(BoneType parent_i, BoneType parent_start_i, BoneType depth, SByte side) // 'side'=-1 negative, 0=unknown, 1=positive
   {
      SkelBone  *parent=skel.bones.addr(parent_i);
      Matrix     parent_matrix;
      Int        bone_i, child_end;
      if(parent){bone_i=parent->children_offset; child_end=bone_i+parent->children_num; parent_matrix=*parent;}
      else      {bone_i=                      0; child_end=skel.parentlessBones();}
      MemtN<Bone, 256> children;
      for(; bone_i<child_end; bone_i++)
      {
         SkelBone &bone=skel.bones[bone_i];
         Bool  singular=Singular(bone.type);

         Bone &child=children.New();
         child.bone_i=bone_i;
         child.depth =depth;
         child.start =(bone.type && !bone.type_sub);
         child.side  =side;

         Vec pos=((bone.type==BONE_SHOULDER) ? bone.to() : bone.pos); // for shoulders use target location because they can be located close to the center (or in case of some models even on the other side)
         if(parent)
         {
            pos.divNormalized(parent_matrix);
            switch(parent->type)
            {
               case BONE_SPINE:
               case BONE_NECK :
               case BONE_HEAD :
                  pos.swapYZ(); CHS(pos.x); // swap YZ because spine is expected to have perp Y pointed forward, and dir Z pointed up, change X sign because cross X points left
               break;
            }
         }
         if(singular || !child.start)
         {
            // keep current 'side' from parent
            child.side_bool=true; // use positive indexes by default
         }else
         {
            if(!child.side)child.side=(pos.x>=0 ? 1 : -1); // if side unknown then calculate it
                child.side_bool=(child.side>=0);
            if(!child.side_bool)pos.chsX(); // mirror order for negative side
         }
         pos.normalize();
         if(bone.type==BONE_FINGER || bone.type==BONE_TOE)
         {
            child.order=pos.x; // start with left
         }else
         {
            child.order=pos.x-pos.y-pos.z; // start with left top front
         }
      }
      children.sort(CompareChild);
      REPA(children) // add in reversed order because later we're processing bones from the end
      {
         Bone &child=children[i];
         if(child.start)
         {
          //child.child_i=i;
            child.bone_start_i=bone_start.elms();
            child.parent_start_i=parent_start_i;
            bone_start.add(child);
         }
      }
      FREPA(children)
      {
         Bone &child=children[i];
         if(!child.start)flood(child.bone_i, parent_start_i, depth+1, child.side);
      }
   }
   BoneTypeIndexSetter(Skeleton &skel) : skel(skel)
   {
      flood(BONE_NULL, BONE_NULL, 0, 0);
      if(Int end=bone_start.elms())
      {
         Int start=0;
         do
         {
            ASSERT(bone_start.Continuous); Sort(&bone_start[start], end-start, CompareGlobal);
            do
            {
               Bone &bone=bone_start[start];
               flood(bone.bone_i, start, bone.depth+1, bone.side);
            }while(++start<end);
            end=bone_start.elms();
         }while(start<end); // if added new elements then process them
      }

      BoneType bone_num[BONE_NUM][2]; Zero(bone_num); // number of bones for each side
      REPA(bone_start) // process bones from the end to list furthest shoulders/arms/legs as first (smallest index)
      {
         Bone     &bone=bone_start[i];
         SkelBone &sbon=skel.bones[bone.bone_i];
         BoneType &bones=bone_num[sbon.type][bone.side_bool];
         sbon.type_index=(bone.side_bool ? bones : -1-bones);
         bones++;
      }
      FREPA(skel.bones)
      {
         SkelBone &bone=skel.bones[i]; if(bone.type_sub)
         {
            Int parent=skel.findParent(i, bone.type);
            if( parent>=0)bone.type_index=skel.bones[parent].type_index; // set from parent
            else          bone.type_index=0;
         }
      }
   }
};
Skeleton& Skeleton::setBoneTypes()
{
   // STEP 1 - set 'type'
   REPA(bones)
   {
      SkelBone &bone=bones[i];
      BONE_TYPE type=BONE_UNKNOWN;

      // leave not unique names (such as "arm", "leg", "eye" and "head") at the end
      if(BoneName(bone, "Wing") || BoneName(bone, "Wings"))type=BONE_WING;else // "Wings" used by "Bat", check this at the start because there are many wings that have hand/finger, but we want the type to be Wing

      if(BoneName(bone, "Spine") || BoneName(bone, "Pelvis") || BoneName(bone, "Hips") || BoneName(bone, "Chest") || BoneName(bone, "Torso") || BoneName(bone, "Body") || BoneName(bone, "Ribs") || BoneName(bone, "RibCage") || BoneName(bone, "Rib Cage") || BoneName(bone, "Rib_Cage"))type=BONE_SPINE;else

      if(BoneName(bone, "Shoulder") || BoneName(bone, "Clavicle") || BoneName(bone, "CollarBone"))type=BONE_SHOULDER;else
      if(BoneName(bone, "ForeArm") || BoneName(bone, "LowerArm") || BoneName(bone, "Elbow"))type=BONE_LOWER_ARM;else
      if(BoneName(bone, "Finger") || BoneName(bone, "Fingers"))type=BONE_FINGER;else // "Fingers" used by "Troll"
      if(BoneName(bone, "Hand") || BoneName(bone, "Wrist") || BoneName(bone, "Palm") && !BoneName(bone, "LegPalm"))type=BONE_HAND;else

      if(BoneName(bone, "Calf") || BoneName(bone, "Crus") || BoneName(bone, "Shin") || BoneName(bone, "LowerLeg") || BoneName(bone, "HorseLink") || BoneName(bone, "Knee"))type=BONE_LOWER_LEG;else
      if(BoneName(bone, "Toe") || BoneName(bone, "Toes") || Equal(bone.name, "FootL0") || Equal(bone.name, "FootR0"))type=BONE_TOE;else // "Toes" used by "Cyclop", "FootL0/FootR0" is from the EE recommended naming system
      if(BoneName(bone, "Foot") || BoneName(bone, "Feet") || BoneName(bone, "Ankle") || BoneName(bone, "LegPalm"))type=BONE_FOOT;else // "LegPalm" used by "Wolf"

      if(BoneName(bone, "Neck"))type=BONE_NECK;else
      if(BoneName(bone, "Jaw"))type=BONE_JAW;else
      if(BoneName(bone, "Tongue"))type=BONE_TONGUE;else
      if(BoneName(bone, "Nose") || BoneName(bone, "Snout"))type=BONE_NOSE;else
    //if(BoneName(bone, "Mouth") || BoneName(bone, "Lips") || BoneName(bone, "Lip"))type=BONE_MOUTH;else
      if(BoneName(bone, "EyeLid") || BoneName(bone, "EyeLids"))type=BONE_EYELID;else
      if(BoneName(bone, "EyeBrow") || BoneName(bone, "EyeBrows") || BoneName(bone, "Brows"))type=BONE_EYEBROW;else // "brows" used by BitGem animals
      if(BoneName(bone, "Ear"))type=BONE_EAR;else
      if(BoneName(bone, "Hair"))type=BONE_HAIR;else

      if(BoneName(bone, "Breast") || BoneName(bone, "Boob") || BoneName(bone, "Boobs"))type=BONE_BREAST;else
      if(BoneName(bone, "Butt") || BoneName(bone, "Buttock") || BoneName(bone, "Buttocks"))type=BONE_BUTT;else

      if(BoneName(bone, "Tail") && !BoneName(bone, "PonyTail") && !BoneName(bone, "Pony Tail") && !BoneName(bone, "Pony_Tail"))type=BONE_TAIL;else
      if(BoneName(bone, "Cape") || BoneName(bone, "Cloak"))type=BONE_CAPE;else

      // not unique names
      if(BoneName(bone, "Eye"))type=BONE_EYE;else
      if(BoneName(bone, "Head"))type=BONE_HEAD;else // this can be "HeadJaw"
      if(BoneName(bone, "Arm") || BoneName(bone, "UpArm") || BoneName(bone, "UpperArm"))type=BONE_UPPER_ARM;else // this can be both BONE_UPPER_ARM and BONE_LOWER_ARM
      if(BoneName(bone, "Thigh") || BoneName(bone, "Leg") || BoneName(bone, "UpLeg") || BoneName(bone, "UpperLeg") || BoneName(bone, "Hip"))type=BONE_UPPER_LEG;else // this can be both BONE_UPPER_LEG and BONE_LOWER_LEG
         {}

      bone.type=type;
   }

   // set fingers/toes
   FREPA(bones)
   {
      SkelBone &bone=bones[i];
      if(bone.type==BONE_UNKNOWN // process unknown bones
      || bone.type==BONE_HAND || bone.type==BONE_UPPER_ARM  // or hands/arms, in case they're named like "ArmHandThumb" which falls into BONE_HAND category above
      || bone.type==BONE_FOOT || bone.type==BONE_UPPER_LEG) // or feet /legs, in case they're named like "FootMiddle"   which falls into BONE_FOOT category above
         for(Int parent_i=i; ; )
      {
         parent_i=boneParent(parent_i); if(parent_i<0)break;
         SkelBone &parent=bones[parent_i]; if(parent.type) // find first parent with a known type
         {
            BONE_TYPE type=BONE_UNKNOWN;
            if(parent.type==BONE_HAND || parent.type==BONE_FINGER)type=BONE_FINGER;else
            if(parent.type==BONE_FOOT || parent.type==BONE_TOE   )type=BONE_TOE   ;
            if(type)
            {
               if(BoneName(bone, "Thumb" )
               || BoneName(bone, "Index" )
               || BoneName(bone, "Middle")
               || BoneName(bone, "Ring"  )
               || BoneName(bone, "Pinky" ) || BoneName(bone, "Pinkie" ) || BoneName(bone, "Little") // "Little" used by "Fire Ice Elemental", "Pinkie" used by "Barbarian (Poker)"
               || BoneName(bone, "Digit" )                                                          //          used by "Barbarian" and "Wolf"
               || BoneName(bone, "Ball"  )                                                          //          used by "Barbarian" and "Wolf"
               )bone.type=type;
            }
            break;
         }
      }
   }

   // convert BONE_UPPER_ARM/BONE_UPPER_LEG to BONE_LOWER_ARM/BONE_LOWER_LEG
   REPA(bones)
   {
      SkelBone &bone=bones[i];
      if(bone.type==BONE_UPPER_ARM)
      {
         if(SkelBone *parent=bones.addr(bone.parent))if(parent->type==BONE_UPPER_ARM) // parent is also UPPER_ARM
            REP(bone.children_num)if(SkelBone *child=bones.addr(bone.children_offset+i))if(child->type==BONE_HAND) // any child is a HAND
         {
            bone.type=BONE_LOWER_ARM;
            break;
         }
      }else
      if(bone.type==BONE_UPPER_LEG)
      {
         if(SkelBone *parent=bones.addr(bone.parent))if(parent->type==BONE_UPPER_LEG) // parent is also UPPER_LEG
            REP(bone.children_num)if(SkelBone *child=bones.addr(bone.children_offset+i))if(child->type==BONE_FOOT) // any child is a FOOT
         {
            bone.type=BONE_LOWER_LEG;
            break;
         }
      }
   }

   // STEP 2 - set 'type_sub', needed for assigning 'type_index'
   struct Link
   {
      BoneType top_parent, // index of the top parent that has the same type
               total_subs; // this is used only for top parents, counter of how many children bones with same type were encountered so far
   };
   MemtN<Link, 256> links; links.setNum(bones.elms());
   FREPA(bones) // go from the start to assign parents first
   {
      SkelBone &bone=bones[i];
      if(bone.type)
      {
         Link &bone_link=links[i];
         Int parent_i=findParent(i, bone.type); // find first parent with same type
         if( parent_i>=0)
         {
            SkelBone &parent=bones[parent_i];
            Int top_parent=links[parent_i].top_parent; // get top parent according to parent
            bone_link.top_parent=top_parent; // set this bone's top_parent to be 'parent.top_parent'
          //bone_link.total_subs=0; can be ignored because this is used only for the top parent, and since here we've found a parent, then this is not a top parent
            bone.type_sub=++links[top_parent].total_subs; // increase top parent total subs
         }else
         {
            bone_link.top_parent=i; // set top parent to be self
            bone_link.total_subs=0; // have 0 subs
            bone.type_sub=0;
         }
      }else
      {
         bone.type_sub=0;
         bone.type_index=0;
      }
   }

   // STEP 3 - set 'type_index'
   BoneTypeIndexSetter(T);

   return T;
}
static Bool ChildOK(C SkelBone &parent, C SkelBone &child)
{
   Flt parent_radius=parent.radius(),
       x=DistPointLine (child.pos, parent.pos, parent.dir)/parent_radius,
       y=DistPointPlane(child.pos, parent.pos, parent.dir)/parent.length;
   return y > x*x + 0.5; // +0.5 because we want to test points at least half way from parent start to end
}
static void NextChild(C Skeleton &skel, Int i, Vec &to)
{
   if(InRange(i, skel.bones))
   {
    C SkelBone &bone=skel.bones[i];
      Int children_ok=0, child_i=-1; REP(bone.children_num) // iterate all children
      {
         Int ci=bone.children_offset+i;
         if(ChildOK(bone, skel.bones[ci])){children_ok++; child_i=ci;} // remember last ok child index
      }
      if(children_ok==1)
      {
       C SkelBone &child=skel.bones[child_i];
         if(child.flag&BONE_RAGDOLL)
         {
            to=child.pos;
         }else
         {
            to=child.to();
            NextChild(skel, child_i, to);
         }
      }
   }
}
static Flt ColRadius(C SkelBone &bone)
{
   return Min(bone.radius(), bone.length*0.5f);
}
static Bool AlwaysConnect(C SkelBone &bone) // always connect these bones, because they're usually always forced up, to allow animation compatibility
{
   switch(bone.type)
   {
      case BONE_SPINE:
      case BONE_NECK:
      case BONE_HEAD:
         return true;
   }
   return false;
}
Skeleton& Skeleton::setBoneShapes()
{
   REPA(bones)
   {
      SkelBone &bone=bones[i];
      Bool      always_connect=AlwaysConnect(bone);
      Vec       from=bone.pos, to=bone.to();
      Flt       radius=bone.radius(), col_rad=ColRadius(bone);
      if(bone.flag&BONE_RAGDOLL)NextChild(T, i, to);

      const Bool reduce_merge_by_dist=true;

      Vec center=Avg(from, to)+bone.offset, dir=to-from; Flt length=Max(dir.normalize(), 2*radius);
      if(bone.children_num)
      {
         Vec to_offset; if(reduce_merge_by_dist)to_offset=to+bone.offset;
         Flt fwd=0; REP(bone.children_num)if(SkelBone *child=bones.addr(bone.children_offset+i))if(Equal(to, child->pos) || always_connect && AlwaysConnect(*child))
         {
            Flt r=Min(col_rad, ColRadius(*child));
            if(reduce_merge_by_dist)r-=Dist(to_offset, child->pos+child->offset);
            MAX(fwd, r);
         }
         if(fwd>0)
         {
            length+= fwd;
            center+=(fwd*0.5)*dir;
         }
      }
      if(SkelBone *parent=bones.addr(bone.parent))
      {
         Flt bck;
         if(!bone.type_sub && // always apply full back extend for first bone of following types
         (   bone.type==BONE_TAIL
          || bone.type==BONE_EAR
          || bone.type==BONE_UPPER_LEG
         ))
         {
            bck=col_rad; goto apply_bck;
         }
         if(Equal(parent->to(), from) || always_connect && AlwaysConnect(*parent))
         {
            bck=Min(col_rad, ColRadius(*parent));
            if(reduce_merge_by_dist)bck-=Dist(from+bone.offset, parent->to()+parent->offset);
         apply_bck:
            if(bck>0)
            {
               length+= bck;
               center-=(bck*0.5)*dir;
            }
         }
      }

      bone.shape.set(radius, length, center, dir);
   }
   return T;
}
Bool Skeleton::setBoneParent(Int child, Int parent, MemPtrN<BoneType, 256> old_to_new)
{
   if(InRange(child, bones) && child!=parent) // can't be a child of itself
   {
      if(!InRange(parent, bones))parent=BONE_NULL; // set <null> if parent is invalid
      BoneType &bone_parent =bones[child].parent;
      if(       bone_parent!=parent) // if different
      {
         bone_parent=parent; // set new parent
         sortBones(old_to_new).setBoneTypes(); // sort because we need to rebuild 'children_offset' and 'children_num', and in case child has an index smaller than parent
         return true;
      }
   }
   old_to_new.clear();
   return false;
}
/******************************************************************************/
void Skeleton::recreateSkelAnims()
{
   CacheLock lock(_skel_anims); REPA(_skel_anims)
   {
      SkelAnim &skel_anim=_skel_anims.lockedData(i); skel_anim.create(T, *skel_anim.animation());
   }
}
/******************************************************************************/
void Skeleton::draw(C Color &bone_color, C Color &slot_color, Flt slot_size)C
{
   if(bone_color.a)REPAO(bones).draw(bone_color);
   if(slot_color.a)REPAO(slots).draw(slot_color, slot_size);
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Skeleton::save(File &f)C
{
   f.putMulti(UInt(CC4_SKEL), Byte(8)); // version
   bones.saveRaw(f); // if in the future bones are saved manually, then use 'File.putStr' for their names
   slots.saveRaw(f); // if in the future slots are saved manually, then use 'File.putStr' for their names
   return f.ok();
}
static BoneType OldBone(File &f) {Byte b; f>>b; return (b==0xFF) ? BONE_NULL : b;}
Bool Skeleton::load(File &f)
{
  _skel_anims.del();

   Flt b_frac;
   if(f.getUInt()==CC4_SKEL)switch(f.decUIntV()) // version
   {
      case 8:
      {
         bones.loadRaw(f);
         slots.loadRaw(f);
         if(f.ok())return true;
      }break;

      case 7:
      {
         bones.setNum(f.decUIntV()); FREPA(bones){SkelBone &b=bones[i]; f>>SCAST(OrientP, b)>>SCAST(BoneID, b); b.parent=OldBone(f); b.children_offset=f.getByte(); f>>b.children_num>>b.flag; f.skip(1); f>>b.length>>b.width>>b.offset>>b.shape;}
         slots.setNum(f.decUIntV()); FREPA(slots){SkelSlot &s=slots[i]; f>>SCAST(OrientP, s)>>s.name; s.bone=OldBone(f); s.bone1=OldBone(f); f.skip(2);}
         if(f.ok())return true;
      }break;

      case 6:
      {
         bones.setNum(f.decUIntV()); FREPA(bones){SkelBone &b=bones[i]; f>>SCAST(OrientP, b)>>SCAST(BoneID, b); b.parent=OldBone(f); b.children_offset=f.getByte(); f>>b.children_num>>b.flag; f.skip(1); f>>b.length>>b.width>>b_frac>>b.offset>>b.shape;}
         slots.setNum(f.decUIntV()); FREPA(slots){SkelSlot &s=slots[i]; f>>SCAST(OrientP, s)>>s.name; s.bone=s.bone1=OldBone(f); f.skip(3);}
         if(f.ok())return true;
      }break;

      case 5:
      {
         bones.setNum(f.decUIntV());
         slots.setNum(f.decUIntV());
         FREPA(bones){SkelBone &b=bones[i]; f>>SCAST(OrientP, b)>>b.name; b.parent=OldBone(f); b.children_offset=f.getByte(); f>>b.children_num>>b.flag>>b.type>>b.type_index>>b.type_sub; f.skip(1); f>>b.length>>b.width>>b_frac>>b.offset>>b.shape;}
         FREPA(slots){SkelSlot &s=slots[i]; f>>SCAST(OrientP, s)>>s.name; s.bone=s.bone1=OldBone(f); f.skip(3);}
         if(f.ok())return true;
      }break;

      case 4:
      {
         bones.setNum(f.decUIntV(), 0); // reset because not all params are set
         slots.setNum(f.decUIntV());
         FREPA(bones){SkelBone &b=bones[i]; f>>SCAST(OrientP, b)>>b.name; b.parent=OldBone(f); b.children_offset=f.getByte(); f>>b.children_num>>b.flag>>b.length>>b.width>>b_frac>>b.offset>>b.shape;}
         FREPA(slots){SkelSlot &s=slots[i]; f>>SCAST(OrientP, s)>>s.name; s.bone=s.bone1=OldBone(f); f.skip(3);}
         if(f.ok())
         {
            setBoneTypes();
            return true;
         }
      }break;

      case 3:
      {
         bones.setNum(f.decUIntV(), 0); // reset because not all params are set
         slots.setNum(f.decUIntV());
         FREPA(bones){SkelBone &b=bones[i]; f>>SCAST(OrientP, b)>>b.name; b.parent=OldBone(f); f>>b.flag; f.skip(2); f>>b.length>>b.width>>b_frac>>b.offset>>b.shape;}
         FREPA(slots){SkelSlot &s=slots[i]; f>>SCAST(OrientP, s)>>s.name; s.bone=s.bone1=OldBone(f); f.skip(3);}
         if(f.ok())
         {
            sortBones().setBoneTypes(); // sort to calculate 'children..', do this before 'setBoneTypes' because it needs that date
            return true;
         }
      }break;

      case 2:
      {
         bones.setNum(f.getUShort(), 0); // reset because not all params are set
         slots.setNum(f.getUShort());
         FREPA(bones){SkelBone &b=bones[i]; f>>SCAST(OrientP, b)>>b.name; b.parent=OldBone(f); f>>b.flag; f.skip(2); f>>b.length>>b.width>>b_frac;}
         FREPA(slots){SkelSlot &s=slots[i]; f>>SCAST(OrientP, s)>>s.name; s.bone=s.bone1=OldBone(f); f.skip(3);}
         if(f.ok())
         {
            sortBones().setBoneTypes().setBoneShapes(); // sort to calculate 'children..', do this before 'setBoneTypes,setBoneShapes' because they need that data
            return true;
         }
      }break;

      case 1:
      {
         bones.setNum(f.getUShort(), 0); // reset because not all params are set
         slots.setNum(f.getUShort(), 0); // reset because not all params are set
         f.skip(2); // old body bones
         FREPA(bones){SkelBone &b=bones[i]; f>>SCAST(OrientP, b); f.get(b.name, 16); b.parent=OldBone(f); f>>b.flag; f.skip(2); f>>b.length>>b.width>>b_frac;}
         FREPA(slots){SkelSlot &s=slots[i]; f>>SCAST(OrientP, s); f.get(s.name, 16); s.bone=s.bone1=OldBone(f); f.skip(3);}
         if(f.ok())
         {
            sortBones().setBoneTypes().setBoneShapes(); // sort to calculate 'children..', do this before 'setBoneTypes,setBoneShapes' because they need that data
            return true;
         }
      }break;

      case 0:
      {
         f.skip(1); // old version byte
         bones.setNum(f.getUShort(), 0); // reset because not all params are set
         slots.setNum(f.getUShort(), 0); // reset because not all params are set
         FREPA(bones){SkelBone &b=bones[i]; f>>SCAST(OrientP, b); f.get(b.name, 16); b.parent=OldBone(f); f>>b.flag; f.skip(2); f>>b.length>>b.width>>b_frac;}
         FREPA(slots){SkelSlot &s=slots[i]; f>>SCAST(OrientP, s); f.get(s.name, 16); s.bone=s.bone1=OldBone(f); f.skip(3);}
         if(f.ok())
         {
            sortBones().setBoneTypes().setBoneShapes(); // sort to calculate 'children..', do this before 'setBoneTypes,setBoneShapes' because they need that data
            return true;
         }
      }break;
   }
   del(); return false;
}
/******************************************************************************/
Bool Skeleton::save(C Str &name)C
{
   File f; if(f.write(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Skeleton::load(C Str &name)
{
   File f; if(f.read(name))return load(f);
   del(); return false;
}
Bool Skeleton::reload(C Str &name)
{
   Cache<SkelAnim> temp; Swap(temp, _skel_anims);
   Bool ok=load(name);
   Swap(temp, _skel_anims);
   recreateSkelAnims();
   return ok;
}
void Skeleton::operator=(C UID &id  ) {T=_EncodeFileName(id);}
void Skeleton::operator=(C Str &name)
{
   if(!load(name))Exit(MLT(S+"Can't load Skeleton \""        +name+"\"",
                       PL,S+u"Nie można wczytać Szkieletu \""+name+"\""));
}
/******************************************************************************/
void Skeleton::save(MemPtr<TextNode> nodes)C
{
   if(bones.elms())
   {
      TextNode &node=nodes.New(); node.name="Bones";
      FREPAO(bones).save(node.nodes.New(), this);
   }
   if(slots.elms())
   {
      TextNode &node=nodes.New(); node.name="Slots";
      FREPAO(slots).save(node.nodes.New(), this);
   }
}
/******************************************************************************/
// BONE MAP
/******************************************************************************/
BoneMap::BoneMap(C BoneMap &src) : BoneMap() {T=src;}
void BoneMap::del()
{
   Free(_bone); _bones=0;
}
Int BoneMap::alloc(Int bones, Int name_size)
{
   del();
   Int size=SIZE(Bone)*bones + name_size;
  _bones=bones;
  _bone =(Bone*)Alloc(size);
   return size;
}
void BoneMap::operator=(C BoneMap &src)
{
   if(this!=&src)
   {
      Int size=alloc(src._bones, src.nameSize());
      CopyFast(_bone, src._bone, size);
   }
}
void BoneMap::create(C Skeleton &skeleton)
{
   Int name_size=0; REPA(skeleton.bones)name_size+=Length(skeleton.bones[i].name)+1; // calculate memory needed for names
   alloc(skeleton.bones.elms(), name_size);
   name_size=0;
   Char8 *bone_name=nameStart();
   FREP(_bones)
   {
    C SkelBone &sbon=skeleton.bones[i];
          Bone &bone=        _bone [i];
      bone.type       =sbon.type;
      bone.type_index =sbon.type_index;
      bone.type_sub   =sbon.type_sub;
      bone.parent     =sbon.parent;
      bone.name_offset=name_size;
      Int length_1=Length(sbon.name)+1;
      Set(bone_name, sbon.name, length_1);
      bone_name+=length_1;
      name_size+=length_1;
   }
}

 Int    BoneMap::nameSize (     )C {return _bones ? _bone[_bones-1].name_offset + Length(name(_bones-1)) + 1 : 0;}
 Char8* BoneMap::nameStart(     )C {return (Char8*)(_bone+_bones);}
CChar8* BoneMap::name     (Int i)C {return InRange(i, _bones) ? nameStart()+_bone[i].name_offset : null;}

Int BoneMap::find(CChar8 *name)C {REP(_bones)if(Equal(T.name(i), name))return i; return -1;}

Bool BoneMap::same(C Skeleton &skeleton)C
{
   if (_bones!=skeleton.bones.elms())return false; // if bone number doesn't match
   REP(_bones)
   {
    C SkelBone &skel_bone=skeleton.bones[i];
        C Bone &bone=             _bone [i];
      if(bone.type      !=skel_bone.type
      || bone.type_index!=skel_bone.type_index
      || bone.type_sub  !=skel_bone.type_sub
      || bone.parent    !=skel_bone.parent
      ||  !Equal(name(i), skel_bone.name)
      )return false; // if any i-th bone name is different from i-th skeleton bone
   }
   return true;
}

/*Bool    rename(C Str8 &src, C Str8 &dest); // rename 'src' bone to 'dest', returns true if any change was made
Bool BoneMap::rename(C Str8 &src, C Str8 &dest)
{
   this ignores adjusting type type_index type_sub
   REPD(b, _bones)if(Equal(name(b), src))
   {
      BoneMap temp; temp.alloc(_bones, nameSize() + (dest.length()-src.length()));
      Char8 *bone_name  =temp.nameStart();
      Int    name_offset=0;
      FREP(temp._bones)
      {
         CChar8 *name=((i==b) ? dest() : T.name(i));
         Int     length_1=Length(name)+1;
         Set(bone_name, name, length_1);
         temp._bone[i].parent     =T._bone[i].parent;
         temp._bone[i].name_offset=name_offset;
         bone_name  +=length_1;
         name_offset+=length_1;
      }
      Swap(temp, T);
      return true;
   }
   return false;
}*/

void BoneMap::remap(C CMemPtrN<BoneType, 256> &old_to_new)
{
   if(_bones) // process only if this already has some bones, this is important so we don't set a new map from empty data
   {
      MemtN<Int, 256> new_to_old; // create a 'new_to_old' remap
      // Warning: Multiple OLD elements can point to the same NEW element, in that case pick the OLD with the smallest index (the parent), for that we need to process from end to start, so parents are processed last and overwrite results
      REPAD(old, old_to_new) // order is important! process from end to start as noted above
      {
         Int _new=old_to_new[old];
         if( _new!=BONE_NULL)new_to_old(_new)=old+1; // for the moment use +1 values because 0 are created when using () operator, which below will be converted to -1
      }
      REPAO(new_to_old)--; // now correct the "+1" indexes, invalid indexes will now be set to "-1"

      Int name_size=0; REPA(new_to_old){Int old=new_to_old[i]; name_size+=Length(name(old))+1;} // calculate memory needed for names

      BoneMap temp; temp.alloc(new_to_old.elms(), name_size);
      Char8  *bone_name=temp.nameStart();
      Int     name_offset=0;
      FREP(temp._bones)
      {
         Bone &bone=temp._bone[i];
         Int    old=new_to_old[i];
         if(InRange(old, _bones))
         {
          C Bone &old_bone=_bone[old]; BoneType old_parent=old_bone.parent;
            bone.type      =old_bone.type      ;
            bone.type_index=old_bone.type_index;
            bone.type_sub  =old_bone.type_sub  ;
            bone.parent    =(InRange(old_parent, old_to_new) ? old_to_new[old_parent] : BONE_NULL);
         }else
         {
            bone.type      =BONE_UNKNOWN;
            bone.type_index=0;
            bone.type_sub  =0;
            bone.parent    =BONE_NULL;
         }
         CChar8 *name=T.name(old);
         Int     length_1=Length(name)+1;
         Set(bone_name, name, length_1);
         bone.name_offset=name_offset;
         bone_name  +=length_1;
         name_offset+=length_1;
      }
      Swap(temp, T);
   }
}
void BoneMap::setRemap(C Skeleton &skeleton, MemPtrN<BoneType, 256> old_to_new, Bool by_name)C
{
   old_to_new.clear();
   FREP(_bones) // process in order
   {
    C Bone &bone=_bone[i]; auto bone_name=name(i);
      Int skel_bone=(by_name ? skeleton.findBoneI(bone_name) : skeleton.findBoneI(bone_name, bone.type, bone.type_index, bone.type_sub)); // find i-th bone in skeleton
      if( skel_bone<0) // not found
      {
         BoneType parent_index=_bone[i].parent;
         skel_bone=(InRange(parent_index, old_to_new) ? old_to_new[parent_index] : BONE_NULL); // set "new bone" as the same as "old parents new bone"
      }
      old_to_new.add(skel_bone);
   }
}

Bool BoneMap::save(File &f)C
{
   f.cmpUIntV(_bones); if(_bones)
   {
      Int name_size=nameSize(); f.cmpUIntV(name_size);
      f.put(_bone, SIZE(Bone)*_bones + name_size);
   }
   return f.ok();
}
Bool BoneMap::load(File &f)
{
   if(Int bones=f.decUIntV())
   {
      Int name_size=f.decUIntV(),
         total_size=alloc(bones, name_size);
      f.getFast(_bone, total_size);
   }else del();
   if(f.ok())return true;
   del();    return false;
}

Bool BoneMap::loadOld2(File &f)
{
   if(Int bones=f.decUIntV())
   {
      Int name_size=f.decUIntV();
      alloc(bones, name_size);
      FREP( bones){Bone &bone=_bone[i]; f>>bone.type; f>>bone.type_index; f>>bone.type_sub; bone.parent=OldBone(f); f>>bone.name_offset;}
      f.getFast(nameStart(), name_size);
   }else del();
   if(f.ok())return true;
   del();    return false;
}
Bool BoneMap::loadOld1(File &f)
{
   if(Int bones=f.decUIntV())
   {
      Int name_size=f.decUIntV();
      alloc(bones, name_size);
      FREP( bones){Bone &bone=_bone[i]; bone.type=BONE_UNKNOWN; bone.type_index=bone.type_sub=0; bone.parent=OldBone(f); bone.name_offset=f.decUIntV();}
      f.getFast(nameStart(), name_size);
   }else del();
   if(f.ok())return true;
   del();    return false;
}
Bool BoneMap::loadOld(File &f)
{
   if(Int bones=f.getInt())
   {
      Int name_size=f.getInt();
      alloc(bones, name_size);
      FREP( bones){Bone &bone=_bone[i]; bone.type=BONE_UNKNOWN; bone.type_index=bone.type_sub=0; bone.parent=OldBone(f); bone.name_offset=f.getInt();}
      f.getFast(nameStart(), name_size);
   }else del();
   if(f.ok())return true;
   del();    return false;
}
/******************************************************************************/
}
/******************************************************************************/
