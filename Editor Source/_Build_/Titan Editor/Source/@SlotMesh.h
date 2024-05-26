/******************************************************************************/
/******************************************************************************

   Meshes are stored in:
      "Edit" (untransformed, skeleton not set but has BoneMap, parts not merged                          , only MeshBase   available)
      "Game" (  transformed, skeleton     set with    BoneMap, parts     merged by material and DrawGroup, only MeshRender available)

   Upon opening a mesh in Object Editor, "Edit" version is loaded (and MeshRender created)
      every usage of the mesh must be combined with transforming by its current "Game" matrix.

   Upon saving a mesh, it's saved as both "Edit" and "Game" versions.

/******************************************************************************/
class SlotMesh {
    MeshPtr mesh;
    Str8 name;
    flt scale;

    void set(C MeshPtr &mesh, C Str &name);
    void draw(C Skeleton &skel);
    void draw(C AnimSkel &skel);
    static void Set(MemPtr<SlotMesh> slot_meshes, cchar8 *name, C MeshPtr &mesh);

  public:
    SlotMesh();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
