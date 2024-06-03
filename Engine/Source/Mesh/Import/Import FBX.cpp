/******************************************************************************

   When suspecting a bug in importing Scales Animation keyframes for a Skeleton,
      please read comments in Animation.cpp file.

/******************************************************************************/
#include "../../../stdafx.h"
#if DEBUG && 0
#define FBX_LINK_TYPE FBX_LINK_LIB
#endif
#include "../../../../ThirdPartyLibs/FBX/FBX Shared.h"

#if FBX_LINK_TYPE == FBX_LINK_LIB
#include "../../../../ThirdPartyLibs/FBX/FBX.cpp"
#endif

namespace EE {
/******************************************************************************/
#if FBX_LINK_TYPE == FBX_LINK_DLL
static Bool Tried = false;
static DLL FbxDll;
static Str FbxDllPath = (X64 ? "Bin/FBX64.dll" : "Bin/FBX32.dll");
static SyncLock Lock;
static void(__cdecl *ImportFBXFree)(CPtr data);
static CPtr(__cdecl *ImportFBXData)(CChar *name, Int &size, UInt mask);
#endif
/******************************************************************************/
void SetFbxDllPath(C Str &dll_32, C Str &dll_64) {
#if FBX_LINK_TYPE == FBX_LINK_DLL
    FbxDllPath = (X64 ? dll_64 : dll_32);
#endif
}
Bool ImportFBX(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XAnimation> animations, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index, XSkeleton *xskeleton, Bool all_nodes_as_bones) {
#if FBX_LINK_TYPE == FBX_LINK_DLL
    if (mesh)
        mesh->del();
    if (skeleton)
        skeleton->del();
    if (xskeleton)
        xskeleton->del();
    animations.clear();
    materials.clear();
    part_material_index.clear();

    {
        SyncLocker locker(Lock);
        if (!Tried) {
            Tried = true;
            if (FbxDll.createFile(FbxDllPath))
                if (Int(__cdecl * ImportFBXVer)() = (Int(__cdecl *)())FbxDll.getFunc("ImportFBXVer"))
                    if (ImportFBXVer() == 0)
                        if (ImportFBXFree = (void(__cdecl *)(CPtr data))FbxDll.getFunc("ImportFBXFree"))
                            ImportFBXData = (CPtr(__cdecl *)(CChar * name, Int & size, UInt mask)) FbxDll.getFunc("ImportFBXData");
        }
    }

    if (ImportFBXData) {
        Int size = 0;
        if (CPtr data = ImportFBXData(name, size, (mesh ? FBX_MESH : 0) | (skeleton ? FBX_SKEL : 0) | (animations ? FBX_ANIM : 0) | (materials ? FBX_MTRL : 0) | (part_material_index ? FBX_PMI : 0) | (xskeleton ? FBX_XSKEL : 0) | (all_nodes_as_bones ? FBX_ALL_NODES_AS_BONES : 0))) {
            File f;
            f.readMem(data, size);

            // copy data from the DLL memory
            Bool ok = LoadFBXData(f, mesh, skeleton, animations, materials, part_material_index, xskeleton);

            // free memory allocated by the DLL on the DLL
            ImportFBXFree(data);

            return ok;
        }
    }

    return false;
#elif FBX_LINK_TYPE == FBX_LINK_LIB
    return _ImportFBX(name, mesh, skeleton, animations, materials, part_material_index, xskeleton, all_nodes_as_bones, S8);
#endif
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
