/******************************************************************************

   Use 'Import' functions to import data from external file formats.

/******************************************************************************/
struct XMaterial // Material stored in external formats
{
   Bool               cull     , // enable face culling
                  flip_normal_y; // if flip normal map Y channel
   MATERIAL_TECHNIQUE technique; // material technique
   Vec4               color    ; // color                   (0,0,0,0) .. (1,1,1,1), default=(1,1,1,1)
   Vec            emissive     ; // emissive                  (0,0,0) .. (1,1,1)  , default=(0,0,0)
   Flt            emissive_glow, // emissive     glow               0 .. 1        , default=0
                      rough_mul, // roughness    from       texture 0 .. 1        , default=0
                      rough_add, // roughness    base               0 .. 1        , default=1
                    reflect_mul, // reflectivity from metal texture 0 .. 1        , default=1-MATERIAL_REFLECT
                    reflect_add, // reflectivity base               0 .. 1        , default=  MATERIAL_REFLECT
                      glow     , // glow amount                     0 .. 1        , default=0
                      normal   , // normal map sharpness            0 .. 1        , default=0
                      bump     , // bumpiness                       0 .. 1        , default=0
                      det_power, // detail     power                0 .. 1        , default=0.3
                   det_uv_scale, // detail  UV scale                0 .. Inf      , default=4
                       uv_scale; // texture UV scale                0 .. Inf      , default=1, this is used mainly for World terrain textures scaling
   Str                color_map, // color         texture file name
                      alpha_map, // alpha         texture file name
                       bump_map, // bump          texture file name
                       glow_map, // glow          texture file name
                   emissive_map, // emissive      texture file name
                     normal_map, // normal        texture file name
                     smooth_map, // smoothness    texture file name
                      metal_map, // metallic      texture file name
               detail_color_map, // detail color  texture file name
                detail_bump_map, // detail bump   texture file name
              detail_normal_map, // detail normal texture file name
              detail_smooth_map, // detail smooth texture file name
                           name; // material name

   void create(C Material &src); // create from 'src' material

   Flt reflect   ()C {return reflect_add;}   void reflect(Flt reflect     ); // get/set reflectivity, 0..1, default=MATERIAL_REFLECT
   Flt reflectMax()C;                        void reflect(Flt min, Flt max); // advanced
#if EE_PRIVATE
   void     del();
   void fixPath(Str path);
   Bool    save(File &f)C;
   Bool    load(File &f) ;
#endif

   XMaterial();
};
/******************************************************************************/
struct XAnimation // Animation stored in external formats
{
   Flt       fps  , // animation frames per second (0 if unknown)
             start; // animation start
   Str       name ; // animation name
   Animation anim ; // animation data

   XAnimation() {fps=start=0;}

#if EE_PRIVATE
   void del() {fps=start=0; name.del(); anim.del();}
   Bool save(File &f)C;
   Bool load(File &f);
   Bool save(C Str &name)C;
   Bool load(C Str &name);

   void save(MemPtr<TextNode> nodes)C; // save as text
#endif
};
/******************************************************************************/
struct XSkeleton // contains information about imported skeleton bones and their mapping to original nodes
{
   struct Node // original node
   {
      Str     name; // original node name (can be an empty string)
      Int     parent; // parent node index, -1=none
      OrientP orient_pos; // original node orientation and position

   #if EE_PRIVATE
      Bool save(File &f)C;
      Bool load(File &f);
   #endif
   };
   struct Bone // links 'SkelBone' -> 'Node'
   {
      Str8 name; // 'SkelBone' name
      Int  node; // node index

   #if EE_PRIVATE
      Bool save(File &f)C;
      Bool load(File &f);
   #endif
   };

   Mems<Node> nodes;
   Mems<Bone> bones;

   Bool is()C {return nodes.elms() || bones.elms();} // if has any data

#if EE_PRIVATE
   void del();

   void mirrorX();
   void scale(Flt scale);

   Bool save(File &f)C;
   Bool load(File &f);
#endif
};
/******************************************************************************/
// Autodesk FilmBox - FBX
void SetFbxDllPath(C Str &dll_32, C Str &dll_64); // set custom path to FBX DLL files (including path and file name) for 32-bit and 64-bit dll versions (default="Bin/FBX32.dll" and "Bin/FBX64.dll"), this function is used only on Windows platform
Bool ImportFBX(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XAnimation> animations, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index, XSkeleton *xskeleton, Bool all_nodes_as_bones=false); // import data from FBX .fbx 'name' file, false on fail

// Collada - DAE
Bool ImportDAE(C Str &name, Mesh *mesh, Skeleton *skeleton, XAnimation *animation, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from Collada .dae 'name' file, false on fail

// Microsoft DirectX - X
Bool ImportX(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XAnimation> animations, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from DirectX .x 'name' file, false on fail

// 3D Studio Max Ascii Scene Export - ASE
Bool ImportASE(C Str &name, Mesh *mesh, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from Ascii Scene Export 'name' file, false on fail

// Wavefront - OBJ
Bool ImportOBJ(C Str &name,   Mesh    *mesh, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from Wavefront Obj 'name' file, false on fail
Bool ExportOBJ(C Str &name, C MeshLod &mesh); // export to Wavefront Obj 'name' file

// 3D Studio - 3DS
Bool Import3DS(C Str &name, Mesh *mesh, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from 3D Studio 'name' file, false on fail

// Blitz3D - B3D
Bool ImportB3D(C Str &name, Mesh *mesh, Skeleton *skeleton, XAnimation *animation, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from Blitz3D 'name' file, false on fail

// Biovision Motion Capture - BVH
Bool ImportBVH(C Str &name, Skeleton *skeleton, XAnimation *animation); // import data from BVH 'name' file, false on fail

// MilkShape 3D - MS3D
Bool ImportMS3D(C Str &name, Mesh *mesh, Skeleton *skeleton, XAnimation *animation, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from MS3D 'name' file, false on fail

// Unreal Engine Mesh - PSK
Bool ImportPSK(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from PSK 'name' file, false on fail

// Unreal Engine Animation - PSA
Bool ImportPSA(C Str &name, Skeleton *skeleton, MemPtr<XAnimation> animations); // import data from PSA 'name' file, false on fail

// Xna Posing Studio - XPS
Bool ImportXPSBinary(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index);
Bool ImportXPSText  (C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index);
Bool ImportXPS      (C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index); // import data from XPS 'name' file, false on fail

Bool Import(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XAnimation> animations, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index, XSkeleton *xskeleton=null, Bool all_nodes_as_bones=false); // import data according to file extension, false on fail

#if EE_PRIVATE
Int  LodIndex(Str &name);
void CleanMesh(Mesh &mesh);
void ProcessBoneNames(MemPtrN<Str8, 256> names);
#endif
/******************************************************************************/
