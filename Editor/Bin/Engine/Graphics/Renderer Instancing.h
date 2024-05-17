/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
#if EE_PRIVATE
/******************************************************************************/
// INSTANCE
/******************************************************************************/
#define INSTANCE_PTR 1 // if store 'MeshPart.Variation' as pointer or index (pointer uses 8 bytes of memory, index 4 bytes, pointer doesn't need 'if' in the draw call, while index needs it)
/******************************************************************************/
extern Memc<Material::MaterialShader> MaterialShaders;

struct MatrixPair
{
   Matrix cur, prev; // matrixes for current and previous frame

   void set(C Matrix &cur, C Matrix &prev) {T.cur=cur; T.prev=prev;}

   MatrixPair() {}
   MatrixPair(C Matrix &matrix) {cur=prev=matrix;}
};

struct EarlyZInstance
{
 C MeshRender *mesh;
   Matrix      view_matrix; // store as 'view_matrix' instead of 'obj_matrix' so we can use 'Matrix' instead of 'MatrixM'

   EarlyZInstance& set(C MeshRender &mesh);
};
extern Memc<EarlyZInstance> EarlyZInstances[2];

struct ShaderDraw
{
   Int         last_shader_material, first_shader_material; // keep 'last_shader_material' as first member, because it's used most often
   ShaderBase *shader;

   void unlink() {shader->unlink();}
};
extern Memc<ShaderDraw> ShaderDraws, MultiMaterialShaderDraws;

struct ShaderMaterial
{
   Int next_shader_material, last_shader_material_mesh, first_shader_material_mesh; // keep 'next_shader_material' as first member, because it's used most often
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   union
   {
      Material                *material;
      UniqueMultiMaterialData *umm;
   };
#else
   // no need to store the material because we can obtain it from 'ShaderMaterialMesh'
#endif

   void unlink();
};
extern Memc<ShaderMaterial> ShaderMaterials;

struct ShaderMaterialMesh
{
   Int       next_shader_material_mesh, first_instance; // keep 'next_shader_material_mesh' as first member, because it's used most often
 C MeshPart *mesh;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   void unlink() {mesh->unlink();} // have to unlink all variations because we don't store pointer to variation
#else
   #if INSTANCE_PTR
    C MeshPart::Variation *variation;
    C MeshPart::Variation& Variation()C {return *variation;}
   #else
      Int                  variation_1; // this is "variation-1"
    C MeshPart::Variation& Variation()C {return mesh->getVariation1(variation_1);}
   #endif
   void unlink() {Variation().unlink();}
#endif
};
extern Memc<ShaderMaterialMesh> ShaderMaterialMeshes;

struct OpaqueShaderMaterialMeshInstance
{
   Int                      next_instance; // index of next instance in the same shader/material/mesh group in 'OpaqueShaderMaterialMeshInstances' container, keep 'next_instance' as first member, because it's used most often
   MatrixPair               view_matrix; // store as 'view_matrix' instead of 'obj_matrix' so we can use 'Matrix' instead of 'MatrixM'
 C Memc<ShaderParamChange> *shader_param_changes;
   Byte                     stencil_value;

   OpaqueShaderMaterialMeshInstance& set();
};
extern Memc<OpaqueShaderMaterialMeshInstance> OpaqueShaderMaterialMeshInstances;

struct ShadowShaderMaterialMeshInstance
{
   Int                      next_instance; // index of next instance in the same shader/material/mesh group in 'ShadowShaderMaterialMeshInstances' container, keep 'next_instance' as first member, because it's used most often
   Matrix                   view_matrix; // store as 'view_matrix' instead of 'obj_matrix' so we can use 'Matrix' instead of 'MatrixM'
 C Memc<ShaderParamChange> *shader_param_changes;

   ShadowShaderMaterialMeshInstance& set();
};
extern Memc<ShadowShaderMaterialMeshInstance> ShadowShaderMaterialMeshInstances;

struct EmissiveInstance // emissive instances are stored in a simple way, without categorizing to shaders/materials, so we don't have to store one extra "MaterialShader _emissive_material_shader" in 'Material', also emissive instances are almost never used
{
 C MeshPart *mesh;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   Shader   *shader;
   Material *material;
   #if COUNT_MATERIAL_USAGE
     ~EmissiveInstance() {material->decUsage();} // for emissive, material will always be != null, because only materials with emissive value can create emissive instances
   #endif
#else
   #if INSTANCE_PTR
    C MeshPart::Variation *variation;
    C MeshPart::Variation& Variation()C {return *variation;}
   #else
      Int                  variation_1; // this is "variation-1"
    C MeshPart::Variation& Variation()C {return mesh->getVariation1(variation_1);}
   #endif
   #if COUNT_MATERIAL_USAGE
     ~EmissiveInstance() {Variation().material->decUsage();} // for emissive, material will always be != null, because only materials with emissive value can create emissive instances
   #endif
#endif
   Matrix                   view_matrix; // store as 'view_matrix' instead of 'obj_matrix' so we can use 'Matrix' instead of 'MatrixM'
 C Memc<ShaderParamChange> *shader_param_changes;

   EmissiveInstance& set(C MeshPart &mesh, C MeshPart::Variation &variation);
};
extern Memc<EmissiveInstance> EmissiveInstances;
/******************************************************************************/
// SKELETON
/******************************************************************************/
struct SkeletonShaderMaterial
{
 C Material *material; // keep this as first member, because it's used most often
   Int       next_skeleton_shader_material, first_mesh_instance, last_mesh_instance;

#if COUNT_MATERIAL_USAGE
  ~SkeletonShaderMaterial() {if(material)material->decUsage();}
#endif
};
extern Memc<SkeletonShaderMaterial> SkeletonShaderMaterials, SkeletonBlendShaderMaterials;

struct SkeletonShaderMaterialMeshInstance
{
   Int                      next_instance; // keep this as first member, because it's used most often
 C MeshRender              *mesh;
 C Memc<ShaderParamChange> *shader_param_changes;

   void set(C MeshRender &mesh);
   void set(C MeshPart   &mesh) {set(mesh.render);}
};
extern Memc<SkeletonShaderMaterialMeshInstance> SkeletonShadowShaderMaterialMeshInstances;

struct SkeletonOpaqueShaderMaterialMeshInstance : SkeletonShaderMaterialMeshInstance
{
   void set(C MeshRender &mesh);
   void set(C MeshPart   &mesh) {set(mesh.render);}
};
extern Memc<SkeletonOpaqueShaderMaterialMeshInstance> SkeletonOpaqueShaderMaterialMeshInstances;

struct SkeletonBlendShaderMaterialMeshInstance : SkeletonOpaqueShaderMaterialMeshInstance
{
   STENCIL_MODE stencil_mode;

   void set(C MeshRender &mesh);
   void set(C MeshPart   &mesh) {set(mesh.render);}
};
extern Memc<SkeletonBlendShaderMaterialMeshInstance> SkeletonBlendShaderMaterialMeshInstances;

struct SkeletonShader
{
   ShaderBase            *shader; // keep this as first member, because it's used most often
   SkeletonShaderMaterial material;
   Int                    next_skeleton_shader;
};
extern Memc<SkeletonShader> SkeletonShaders;

struct SkeletonBlendShader : SkeletonShader
{
   Byte type; // BlendInstance::TYPE
};
extern Memc<SkeletonBlendShader> SkeletonBlendShaders;

struct SkeletonInstance
{
 C AnimatedSkeleton *anim_skel;
   SkeletonShader    skel_shader;

   void set(C AnimatedSkeleton &anim_skel);
   void newInstance(ShaderBase &shader, C Material &material, Memc<SkeletonShaderMaterialMeshInstance> &instances);

   void unlinkOpaque() {anim_skel->_instance.opaque=-1;}
   void unlinkBlend () {anim_skel->_instance.blend =-1;}
   void unlinkShadow() {anim_skel->_instance.shadow=-1;}
};
struct SkeletonBlendInstance
{
 C AnimatedSkeleton   *anim_skel;
   SkeletonBlendShader skel_shader;

   void set(C AnimatedSkeleton &anim_skel);
   void newInstance(ShaderBase &shader, C Material &material, UInt type);

   void addBlend(Shader &shader, C Material &material, C MeshPart &mesh);
   void addBlend(BLST   &blst  , C Material &material, C MeshPart &mesh);
   void addFur  (Shader &shader, C Material &material, C MeshPart &mesh);

   void unlinkOpaque() {anim_skel->_instance.opaque=-1;}
   void unlinkBlend () {anim_skel->_instance.blend =-1;}
   void unlinkShadow() {anim_skel->_instance.shadow=-1;}
};

struct SkeletonInstances : Memc<SkeletonInstance>
{
          SkeletonInstance& getSkeletonInstance      (C AnimatedSkeleton &anim_skel, Int &instance_index);
   INLINE SkeletonInstance& getSkeletonInstanceOpaque(C AnimatedSkeleton &anim_skel) {return getSkeletonInstance(anim_skel, anim_skel._instance.opaque);}
   INLINE SkeletonInstance& getSkeletonInstanceShadow(C AnimatedSkeleton &anim_skel) {return getSkeletonInstance(anim_skel, anim_skel._instance.shadow);}
};
extern SkeletonInstances SkeletonOpaqueInstances, SkeletonShadowInstances;

struct SkeletonEmissiveInstance // emissive instances are stored in a simple way, without categorizing to shaders/materials, so we don't have to store one extra "Int _emissive" in 'AnimatedSkeleton', also emissive instances are almost never used
{
 C MeshPart *mesh;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   Shader   *shader;
 C Material *material;
   #if COUNT_MATERIAL_USAGE
     ~SkeletonEmissiveInstance() {material->decUsage();} // for emissive, material will always be != null, because only materials with emissive value can create emissive instances
   #endif
#else
   #if INSTANCE_PTR
    C MeshPart::Variation *variation;
    C MeshPart::Variation& Variation()C {return *variation;}
   #else
      Int                  variation_1; // this is "variation-1"
    C MeshPart::Variation& Variation()C {return mesh->getVariation1(variation_1);}
   #endif
   #if COUNT_MATERIAL_USAGE
     ~SkeletonEmissiveInstance() {Variation().material->decUsage();} // for emissive, material will always be != null, because only materials with emissive value can create emissive instances
   #endif
#endif
 C AnimatedSkeleton        *anim_skel;
 C Memc<ShaderParamChange> *shader_param_changes;

   void set(C MeshPart &mesh, C MeshPart::Variation &variation, C AnimatedSkeleton &anim_skel);
};
extern Memc<SkeletonEmissiveInstance> SkeletonEmissiveInstances;
/******************************************************************************/
// BLEND
/******************************************************************************/
struct BlendInstance
{
   enum TYPE
   {
      STATIC     ,
      STATIC_BLST,
      STATIC_FUR ,
      SKELETON   ,
      BLEND_OBJ  ,
      GAME_OBJ   ,
      GAME_AREA  ,
   }type;

   Flt z; // used for sorting

   struct Static // non-animated
   {
   #if 1
      union
      {
         Shader *shader;
         BLST   *blst  ;
      };
    C Material                *material;
   #else
    C MeshPart::Variation     *variation;
   #endif
    C MeshPart                *mesh;
      MatrixPair               view_matrix; // store as 'view_matrix' instead of 'obj_matrix' so we can use 'Matrix' instead of 'MatrixM'
    C Memc<ShaderParamChange> *shader_param_changes;
      STENCIL_MODE             stencil_mode;
   };

   union // Data
   {
      Static                stat      ; // static
      SkeletonBlendInstance skeleton  ; // skeleton
      BlendObject          *blend_obj ; // blend object
      Game::Obj            * game_obj ; //  game object
      Game::Area::Data     * game_area; //  game area
   };

   void setViewZ(Flt z) {T.z=z;}
   void setZ    (C VecD &pos);

   void viewMatrixChanged() {setViewZ(stat.view_matrix.cur.pos.z);}
   void setViewMatrix(C Matrix  &view_matrix                             ) {stat.view_matrix.cur=view_matrix;                                         viewMatrixChanged();} // here 'stat.view_matrix.prev' is ignored because it's assumed to be unused
   void setViewMatrix(C Matrix  &view_matrix, C Matrix  &view_matrix_prev) {stat.view_matrix.cur=view_matrix; stat.view_matrix.prev=view_matrix_prev; viewMatrixChanged();}
   void setMatrix    (C MatrixM &     matrix);
   void setMatrix    (C MatrixM &     matrix, C MatrixM &matrix_prev);

   void unlink();

  ~BlendInstance();
   BlendInstance() {} // needed because of union
};
struct BlendInstancesClass : Memc<BlendInstance>
{
   BlendInstance& add   (Shader &shader, C Material &material, C MeshPart &mesh, C MeshPart::Variation &variation);
   BlendInstance& add   (BLST   &blst  , C Material &material, C MeshPart &mesh, C MeshPart::Variation &variation);
   BlendInstance& addFur(Shader &shader, C Material &material, C MeshPart &mesh, C MeshPart::Variation &variation);

   void add(BlendObject        &blend_obj, C VecD &pos);
   void add(  Game::Obj        & game_obj );
   void add(  Game::Area::Data & game_area);

   SkeletonBlendInstance& getSkeletonInstance(C AnimatedSkeleton &anim_skel);
};
/******************************************************************************/
// CLOTH
/******************************************************************************
struct ClothInstance
{
 C Cloth      *cloth;
   ShaderBase *shader;
 C Material   *material;

#if COUNT_MATERIAL_USAGE
  ~ClothInstance() {if(material)material->decUsage();}
#endif
};
struct ClothInstances : Memc<ClothInstance>
{
   void addShadow(C Cloth &cloth, Shader &shader, C Material &material);
   void add      (C Cloth &cloth, Shader &shader, C Material &material);
   void add      (C Cloth &cloth, FRST   &frst  , C Material &material);
};
/******************************************************************************/
// MISC
/******************************************************************************/
struct GameObjects : Memc<Game::Obj*>
{
};
struct GameAreas : Memc<Game::Area::Data*>
{
};
/******************************************************************************/
// VARIABLES
/******************************************************************************/
extern BlendInstancesClass BlendInstances;
//extern ClothInstances      OpaqueClothInstances, ShadowClothInstances;
extern    GameObjects      PaletteObjects, Palette1Objects, OverlayObjects, OpaqueObjects, EmissiveObjects, OutlineObjects, BehindObjects;
extern    GameAreas        PaletteAreas  , Palette1Areas;
/******************************************************************************/
Bool   HasEarlyZInstances();
void  DrawEarlyZInstances();
void ClearEarlyZInstances();

void  DrawOpaqueInstances();
void ClearOpaqueInstances();

        Int             Compare(C BlendInstance &a, C BlendInstance &b);
inline void  SortBlendInstances() {BlendInstances.sort(Compare);}
       void  DrawBlendInstances();
       void ClearBlendInstances();

void  SortEmissiveInstances();
void  DrawEmissiveInstances();
void ClearEmissiveInstances();

void PrepareShadowInstances();
void    DrawShadowInstances();

void     DrawPaletteObjects();
void    DrawPalette1Objects();
void     DrawOverlayObjects();
void     DrawOutlineObjects();
void      DrawBehindObjects();

void  ShutInstances();
void  InitInstances();
void ClearInstances();
/******************************************************************************/
#endif
/******************************************************************************/
