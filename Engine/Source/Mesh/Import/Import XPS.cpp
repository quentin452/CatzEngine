/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
static void Get(File &f, Char8 *name, Int name_len) {
    Int len = f.decUIntV();
    Int read = Min(len, name_len - 1);
    f.getFast(name, read);
    name[read] = '\0';
    f.skip(len - read);
}
template <Int elms>
static void Get(File &f, Char8 (&name)[elms]) { return Get(f, name, elms); }

Bool ImportXPSBinary(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index) {
    if (mesh)
        mesh->del();
    if (skeleton)
        skeleton->del();
    materials.clear();
    part_material_index.clear();

    File f;
    if (f.read(name)) {
        Char8 str[256];
        Bool variable_weights = false, tangent = true;
        UInt u = f.getUInt();
        if (u == 323232) {
            U16 major = f.getUShort();
            variable_weights = (major >= 3);
            U16 minor = f.getUShort();
            tangent = (major <= 2 && minor <= 12);
            Char8 author[256];
            Get(f, author);
            UInt settings_len = f.getUInt();
            Get(f, str); // MachineName
            Get(f, str); // UserName
            Get(f, str); // FileName
            f.skip(SIZE(UInt) * settings_len);
            u = f.getUInt();
        } // else goto error;
        UInt bones = u;
        if (InRange(bones, 1024)) {
            Skeleton temp, *skel = (skeleton ? skeleton : mesh ? &temp
                                                               : null); // if skel not specified, but we want mesh, then we have to process it
            if (skel)
                skel->bones.setNum(Min(BONE_NULL, bones));
            MemtN<Str8, 256> bone_names;
            FREP(bones) {
                SkelBone *bone = (skel ? skel->bones.addr(i) : null);
                Get(f, str);
                if (bone)
                    bone_names.add(str);
                U16 parent = f.getUShort();
                if (bone)
                    bone->parent = (InRange(parent, skel->bones) ? parent : BONE_NULL);
                if (bone)
                    f >> bone->pos;
                else
                    f.skip(SIZE(Vec));
            }
            ProcessBoneNames(bone_names);
            FREPA(bone_names)
            Set(skel->bones[i].name, bone_names[i]);
            MemtN<BoneType, 256> old_to_new;
            if (skel) {
                skel->mirrorX().sortBones(old_to_new).setBoneTypes();
                if (VIRTUAL_ROOT_BONE)
                    REPAO(old_to_new)
                ++; // 'mirrorX' must be called before 'setBoneTypes', 'sortBones' must be called before 'setBoneTypes' and 'SetSkin'
            }
            MemtN<IndexWeight, 256> skin;
            UInt parts = f.getUInt();
            if (InRange(parts, 256)) {
                UInt tan_size = (tangent ? SIZE(Vec4) : 0), uv_tan_size = SIZE(Vec2) + tan_size;
                if (part_material_index) {
                    part_material_index.setNum(parts);
                    REPAO(part_material_index) = i;
                }
                if (materials) {
                    materials.setNum(parts);
                    REPAO(materials).name = i;
                } // name is needed so Editor can create multiple materials instead of trying to reuse just one
                if (mesh)
                    mesh->parts.setNum(parts);
                if (mesh || materials)
                    FREP(parts) {
                        MeshPart *part = (mesh ? &mesh->parts[i] : null);
                        if (part)
                            Get(f, part->name);
                        else
                            Get(f, str);
                        UInt uvs = f.getUInt();
                        if (uvs >= 256)
                            goto error;
                        UInt textures = f.getUInt();
                        if (textures >= 256)
                            goto error;
                        FREPD(t, textures) {
                            Get(f, str); // tex file name
                            if (materials) {
                                XMaterial &mtrl = materials[i];
                                switch (t) {
                                case 0:
                                    mtrl.color_map = str;
                                    break;
                                case 1:
                                    mtrl.smooth_map = str;
                                    break;
                                case 2:
                                    mtrl.normal_map = str;
                                    break;
                                case 3:
                                    mtrl.smooth_map = str;
                                    break;
                                }
                            }
                            UInt uv_layer_index = f.getUInt();
                            if (uv_layer_index >= uvs)
                                goto error;
                        }
                        if (materials)
                            materials[i].fixPath(GetPath(name));
                        UInt vtxs = f.getUInt();
                        if (vtxs > 1 << 20)
                            goto error; // 1 mln
                        if (part) {
                            MeshBase &base = part->base.create(vtxs, 0, 0, 0, VTX_POS | VTX_NRM | VTX_COLOR | (bones ? VTX_SKIN : MESH_NONE) | ((uvs >= 1) ? VTX_TEX0 : MESH_NONE) | ((uvs >= 2) ? VTX_TEX1 : MESH_NONE));
                            FREP(vtxs) {
                                f >> base.vtx.pos(i);
                                Vec &nrm = base.vtx.nrm(i);
                                f >> nrm;
                                nrm.normalize();
                                f >> base.vtx.color(i);
                                if (uvs >= 1) {
                                    f >> base.vtx.tex0(i);
                                    if (tangent)
                                        f.skip(SIZE(Vec4));
                                    if (uvs >= 2) {
                                        f >> base.vtx.tex1(i);
                                        if (tangent)
                                            f.skip(SIZE(Vec4));
                                        f.skip(uv_tan_size * (uvs - 2));
                                    }
                                } // skip unprocessed
                                if (bones > 0) {
                                    U16 weights = (variable_weights ? f.getUShort() : 4);
                                    skin.setNumDiscard(weights);
                                    FREPA(skin) {
                                        Int bone = f.getUShort();
                                        skin[i].index = (InRange(bone, old_to_new) ? old_to_new[bone] : -1);
                                    }
                                    FREPA(skin)
                                    f >> skin[i].weight;
                                    SetSkin(skin, base.vtx.matrix(i), base.vtx.blend(i), skel);
                                }
                            }
                        } else
                            REP(vtxs) {
                                f.skip(SIZE(Vec) + SIZE(Vec) + SIZE(Color) + uv_tan_size * uvs);
                                if (bones > 0) {
                                    U16 weights = (variable_weights ? f.getUShort() : 4);
                                    f.skip(weights * (SIZE(U16) + SIZE(Flt)));
                                }
                            }

                        UInt tris = f.getUInt();
                        if (tris > 1 << 20)
                            goto error; // 1 mln
                        if (part) {
                            MeshBase &base = part->base;
                            base.tri._elms = tris;
                            base.include(TRI_IND);
                            Bool invalid = false;
                            FREP(tris) {
                                VecI &t = base.tri.ind(i);
                                f >> t;
                                t.reverse();
                                REPA(t)
                                if (!InRange(t.c[i], vtxs)) {
                                    t.zero();
                                    invalid = true;
                                    break;
                                }
                            }
                            if (invalid)
                                base.removeDegenerateFaces(0);
                            if (!base.vtx.nrm())
                                base.setNormalsAuto(EPS_NRM_AUTO, EPSD); // use small pos epsilon in case mesh is scaled down, call before 'setTanBin' as it depends on normals
                            if (!base.vtx.tan() || !base.vtx.bin())
                                base.setTanBin();                         // if(!base.vtx.tan())base.setTangents(); if(!base.vtx.bin())base.setBinormals(); // need to call before 'weldVtx' to don't remove too many vertexes
                            base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small pos epsilon in case mesh is scaled down
                        } else
                            f.skip(tris * SIZE(VecI));
                    }

                if (mesh) {
                    mesh->mirrorX().skeleton(skel).skeleton(null).setBox();
                    CleanMesh(*mesh);
                }
                if (skeleton) {
                    skel->setBoneShapes();
                    if (skeleton != skel)
                        Swap(*skeleton, *skel);
                }
                return true;
            }
        }
    }
error:
    if (mesh)
        mesh->del();
    return false;
}
/******************************************************************************/
static Vec2 _TextVec2(CChar *t) {
    CalcValue x, y;
    TextValue(_SkipWhiteChars(TextValue(t, x)), y);
    return Vec2(x.asFlt(), y.asFlt());
}
static Vec _TextVec(CChar *t) {
    CalcValue x, y, z;
    TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(t, x)), y)), z);
    return Vec(x.asFlt(), y.asFlt(), z.asFlt());
}
static Vec4 _TextVec4(CChar *t) {
    CalcValue x, y, z, w;
    TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(t, x)), y)), z)), w);
    return Vec4(x.asFlt(), y.asFlt(), z.asFlt(), w.asFlt());
}
static VecB4 _TextVecB4(CChar *t) {
    CalcValue x, y, z, w;
    TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(t, x)), y)), z)), w);
    return VecB4(x.asInt(), y.asInt(), z.asInt(), w.asInt());
}
static VecI _TextVecI(CChar *t) {
    CalcValue x, y, z;
    TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(t, x)), y)), z);
    return VecI(x.asInt(), y.asInt(), z.asInt());
}
static VecI4 _TextVecI4(CChar *t) {
    CalcValue x, y, z, w;
    TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(t, x)), y)), z)), w);
    return VecI4(x.asInt(), y.asInt(), z.asInt(), w.asInt());
}
static Color _TextColor(CChar *t) {
    CalcValue x, y, z, w;
    TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(_SkipWhiteChars(TextValue(t, x)), y)), z)), w);
    return Color(x.asInt(), y.asInt(), z.asInt(), w.asInt());
}

Bool ImportXPSText(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index) {
    if (mesh)
        mesh->del();
    if (skeleton)
        skeleton->del();
    materials.clear();
    part_material_index.clear();

    FileText f;
    if (f.read(name)) {
        Str s;
        f.fullLine(s);
        Int bones = TextInt(s);
        if (bones < 0)
            goto error;
        Skeleton temp, *skel = (skeleton ? skeleton : mesh ? &temp
                                                           : null); // if skel not specified, but we want mesh, then we have to process it
        if (skel)
            skel->bones.setNum(Min(BONE_NULL, bones));
        MemtN<Str8, 256> bone_names;
        FREP(bones) {
            SkelBone *bone = (skel ? skel->bones.addr(i) : null);
            f.fullLine(s);
            if (bone)
                bone_names.add(s);
            f.getLine(s);
            if (bone) {
                Int parent = TextInt(s);
                bone->parent = (InRange(parent, skel->bones) ? parent : BONE_NULL);
            }
            f.getLine(s);
            if (bone)
                bone->pos = _TextVec(s);
        }
        ProcessBoneNames(bone_names);
        FREPA(bone_names)
        Set(skel->bones[i].name, bone_names[i]);
        MemtN<BoneType, 256> old_to_new;
        if (skel) {
            skel->mirrorX().sortBones(old_to_new).setBoneTypes();
            if (VIRTUAL_ROOT_BONE)
                REPAO(old_to_new)
            ++; // 'mirrorX' must be called before 'setBoneTypes', 'sortBones' must be called before 'setBoneTypes' and 'SetSkin'
        }
        MemtN<IndexWeight, 256> skin;
        f.getLine(s);
        Int parts = TextInt(s);
        if (parts < 0)
            goto error;
        if (part_material_index) {
            part_material_index.setNum(parts);
            REPAO(part_material_index) = i;
        }
        if (materials) {
            materials.setNum(parts);
            REPAO(materials).name = i;
        } // name is needed so Editor can create multiple materials instead of trying to reuse just one
        if (mesh)
            mesh->parts.setNum(parts);
        if (mesh || materials)
            FREP(parts) {
                MeshPart *part = (mesh ? &mesh->parts[i] : null);
                f.fullLine(s);
                if (part)
                    Set(part->name, s);
                f.getLine(s);
                Int uv_layers = TextInt(s);
                if (uv_layers < 0)
                    goto error;
                f.getLine(s);
                Int textures = TextInt(s);
                if (textures < 0)
                    goto error;
                FREPD(t, textures) {
                    f.fullLine(s); // tex file name
                    if (materials) {
                        XMaterial &mtrl = materials[i];
                        switch (t) {
                        case 0:
                            mtrl.color_map = s;
                            break;
                        case 1:
                            mtrl.smooth_map = s;
                            break;
                        case 2:
                            mtrl.normal_map = s;
                            break;
                        case 3:
                            mtrl.smooth_map = s;
                            break;
                        }
                    }
                    f.getLine(s);
                    Int uv_layer_index = TextInt(s);
                    if (uv_layer_index < 0)
                        goto error;
                }
                if (materials)
                    materials[i].fixPath(GetPath(name));
                f.getLine(s);
                Int vtxs = TextInt(s);
                if (vtxs < 0)
                    goto error;
                if (part) {
                    MeshBase &base = part->base.create(vtxs, 0, 0, 0, VTX_POS | VTX_NRM | VTX_COLOR | (bones ? VTX_SKIN : MESH_NONE) | ((uv_layers >= 1) ? VTX_TEX0 : MESH_NONE) | ((uv_layers >= 2) ? VTX_TEX1 : MESH_NONE));
                    FREP(vtxs) {
                        f.getLine(s);
                        base.vtx.pos(i) = _TextVec(s);
                        f.getLine(s);
                        base.vtx.nrm(i) = !_TextVec(s);
                        f.getLine(s);
                        base.vtx.color(i) = _TextColor(s);
                        if (uv_layers >= 1) {
                            f.getLine(s);
                            base.vtx.tex0(i) = _TextVec2(s);
                        }
                        if (uv_layers >= 2) {
                            f.getLine(s);
                            base.vtx.tex1(i) = _TextVec2(s);
                        }
                        REP(uv_layers - 2)
                        f.skipLine(); // skip unprocessed
                        if (bones > 0) {
                            f.getLine(s);
                            VecI4 m = _TextVecI4(s);
                            f.getLine(s);
                            Vec4 b = _TextVec4(s);
                            FREPA(m)
                            if (b.c[i] && InRange(m.c[i], old_to_new))
                                skin.New().set(old_to_new[m.c[i]], b.c[i] /* /255.0f not needed because weight is normalized in 'SetSkin' */);
                            SetSkin(skin, base.vtx.matrix(i), base.vtx.blend(i), skel);
                            skin.clear();
                        }
                    }
                } else
                    REP(vtxs * (3 + uv_layers + (bones > 0) * 2))
                f.skipLine();

                f.getLine(s);
                Int tris = TextInt(s);
                if (tris < 0)
                    goto error;
                if (part) {
                    MeshBase &base = part->base;
                    base.tri._elms = tris;
                    base.include(TRI_IND);
                    Bool invalid = false;
                    FREP(tris) {
                        f.getLine(s);
                        VecI &t = base.tri.ind(i);
                        t = _TextVecI(s).reverse();
                        REPA(t)
                        if (!InRange(t.c[i], vtxs)) {
                            t.zero();
                            invalid = true;
                            break;
                        }
                    }
                    if (invalid)
                        base.removeDegenerateFaces(0);
                    if (!base.vtx.nrm())
                        base.setNormalsAuto(EPS_NRM_AUTO, EPSD); // use small pos epsilon in case mesh is scaled down, call before 'setTanBin' as it depends on normals
                    if (!base.vtx.tan() || !base.vtx.bin())
                        base.setTanBin();                         // if(!base.vtx.tan())base.setTangents(); if(!base.vtx.bin())base.setBinormals(); // need to call before 'weldVtx' to don't remove too many vertexes
                    base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small pos epsilon in case mesh is scaled down
                } else
                    REP(tris)
                f.skipLine();
            }

        if (mesh) {
            mesh->mirrorX().skeleton(skel).skeleton(null).setBox();
            CleanMesh(*mesh);
        }
        if (skeleton) {
            skel->setBoneShapes();
            if (skeleton != skel)
                Swap(*skeleton, *skel);
        }
        return true;
    }
error:
    if (mesh)
        mesh->del();
    return false;
}
/******************************************************************************/
Bool ImportXPS(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index) {
    return ImportXPSBinary(name, mesh, skeleton, materials, part_material_index) || ImportXPSText(name, mesh, skeleton, materials, part_material_index);
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
