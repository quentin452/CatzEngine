/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
int Compare(C IDReplace &a, C IDReplace &b) { return Compare(a.from, b.from); }
int Compare(C IDReplace &a, C UID &b) { return Compare(a.from, b); }
/******************************************************************************/
MergeSimilarMaterials MSM;
Memc<IDReplace> ReplaceIDs;
State IDReplaceState(UpdateIDReplace, DrawIDReplace, InitIDReplace, ShutIDReplace);
C IDReplace *ReplaceID(C UID &src, Memc<IDReplace> &replace) { return src.valid() ? replace.binaryFind(src, Compare) : null; }
/******************************************************************************/
bool ThreadIDReplace(Thread &thread) {
    ThreadMayUseGPUData();
    FREPA(Proj.elms) {
        if (thread.wantStop())
            return false;
        Elm &elm = Proj.elms[i];
        if (elm.data) {
            if (ReplaceID(elm.id))
                elm.setRemoved(true); // if this element is replaced with another one, then remove it
            if (C IDReplace *parent = ReplaceID(elm.parent_id))
                elm.setParent(parent->to); // if parent of this element is being removed, then assign this element to the replacement
            FREPA(ReplaceIDs)
            if (elm.data->mayContain(ReplaceIDs[i].from)) {
                bool changed = false;
                switch (elm.type) {
                case ELM_MESH: {
                    Mesh mesh;
                    if (Load(mesh, Proj.editPath(elm), Proj.game_path)) {
                        REPD(l, mesh.lods())
                        REPAD(p, mesh.lod(l)) {
                            MeshPart &part = mesh.lod(l).parts[p];
                            bool changed_multi_mtrl = false;
                            MaterialPtr mtrls[4];
                            REPA(mtrls) {
                                MaterialPtr &mtrl = mtrls[i];
                                mtrl = part.multiMaterial(i);
                                if (C IDReplace *id = ReplaceID(mtrl.id())) {
                                    changed_multi_mtrl = true;
                                    mtrl = Proj.gamePath(id->to);
                                }
                            }
                            if (changed_multi_mtrl) {
                                changed = true;
                                part.multiMaterial(mtrls[0], mtrls[1], mtrls[2], mtrls[3]);
                            }
                            REP(part.variations())
                            if (i)
                                if (C IDReplace *id = ReplaceID(part.variation(i).id())) {
                                    changed = true;
                                    part.variation(i, Proj.gamePath(id->to));
                                }
                        }
                        if (changed) {
                            ElmMesh *mesh_data = elm.meshData();
                            mesh_data->newVer();
                            mesh_data->fromMtrl(mesh);
                            mesh_data->file_time.getUTC();
                            Skeleton *body_skel;
                            Proj.getMeshSkels(mesh_data, null, &body_skel);
                            Mesh game;
                            EditToGameMesh(mesh, game, body_skel, Proj.getEnum(mesh_data->draw_group_id), &NoTemp(mesh_data->transform()));
                            Save(mesh, Proj.editPath(elm), Proj.game_path);
                            Save(game, Proj.gamePath(elm));
                            Proj.savedGame(elm);
                        }
                    }
                } break;

                case ELM_OBJ:
                case ELM_OBJ_CLASS: {
                    EditObject obj;
                    if (obj.load(Proj.editPath(elm))) {
                        REPA(obj) {
                            EditParam &param = obj[i];
                            if (ParamTypeID(param.type))
                                REP(param.IDs())
                            if (ReplaceID(param.asID(i))) // if any ID needs to be replaced
                            {
                                Memt<UID> ids;
                                FREP(param.IDs()) // list in original order
                                {
                                    UID id = param.asID(i);
                                    if (C IDReplace *replace = ReplaceID(id))
                                        id = replace->to;
                                    ids.add(id);
                                }
                                changed = true;
                                param.setAsIDArray(ids);
                                break;
                            }
                        }
                        if (changed) {
                            elm.data->newVer();
                            Save(obj, Proj.editPath(elm));
                            Proj.makeGameVer(elm);
                        }
                    }
                } break;

                case ELM_WORLD: {
                    // TODO: replace terrain materials (watch out for heightmap material palette having both replacement and original)
                    // TODO: replace world object params
                } break;
                }
                if (changed)
                    Proj.elmChanged(elm); // this may cause conflicts with 'Gui.update' (if needed then move to 'ShutIDReplace')
                break;
            }
        }
        UpdateProgress.set(i, Proj.elms.elms());
    }
    return false;
}
bool InitIDReplace() {
    SetKbExclusive();
    Proj.pause();
    UpdateProgress.create(Rect_C(0, -0.05f, 1, 0.045f));
    UpdateThread.create(ThreadIDReplace);
    return true;
}
void ShutIDReplace() {
    UpdateThread.del();
    UpdateProgress.del();
    Proj.refresh().resume();
    App.stateNormal().flash();
}
/******************************************************************************/
bool UpdateIDReplace() {
    if (Kb.bp(KB_ESC)) {
        SetProjectState();
        Gui.msgBox(S, "Merging breaked on user request");
    }
    if (!UpdateThread.active())
        SetProjectState();

    App.stateProgress(UpdateProgress());
    Time.wait(1000 / 30);
    //Gui.update(); this may cause conflicts with 'Proj.elmChanged'
    Server.update(null, true);
    if (Ms.bp(MS_MAXIMIZE))
        App.window().toggle();
    return true;
}
/******************************************************************************/
void DrawIDReplace() {
    D.clear(BackgroundColor());
    D.text(0, 0.05f, "Merging Elements");
    GuiPC gpc;
    gpc.visible = gpc.enabled = true;
    gpc.client_rect = gpc.clip = D.rect();
    gpc.offset.zero();
    UpdateProgress.draw(gpc);
    D.clip();
    Draw();
}
/******************************************************************************/

/******************************************************************************/
void IDReplace::set(C UID &from, C UID &to) {
    T.from = from;
    T.to = to;
}
bool MergeSimilarMaterials::Mtrl::similar(C Mtrl &m) C {
    if (MSM.name && name != m.name)
        return false;
    if (MSM.color_name && color_name != m.color_name)
        return false;
    if (MSM.color_tex && base_0_tex != m.base_0_tex)
        return false;
    if (MSM.normal_tex && base_1_tex != m.base_1_tex)
        return false;
    if (MSM.base2_tex && base_2_tex != m.base_2_tex)
        return false;
    if (MSM.detail_tex && detail_tex != m.detail_tex)
        return false;
    if (MSM.macro_tex && macro_tex != m.macro_tex)
        return false;
    if (MSM.emissive_tex && emissive_tex != m.emissive_tex)
        return false;
    if (MSM.cull && cull != m.cull)
        return false;
    if (MSM.tech && tech != m.tech)
        return false;
    if (MSM.color_value_on() && Abs(col_s - m.col_s).max() > MSM.color_value)
        return false;
    if (MSM.bump_value_on() && Abs(bump - m.bump) > MSM.bump_value)
        return false;
    if (MSM.smooth_value_on() && Abs(smooth - m.smooth) > MSM.smooth_value)
        return false;
    if (MSM.reflect_value_on() && Abs(reflect - m.reflect) > MSM.reflect_value)
        return false;
    if (MSM.glow_value_on() && Abs(glow - m.glow) > MSM.glow_value)
        return false;
    if (MSM.uv_scale_on() && Abs(uv_scale - m.uv_scale) > MSM.uv_scale)
        return false;
    return true;
}
::MergeSimilarMaterials::Mtrl &MergeSimilarMaterials::Mtrl::set(C UID &elm_id, C EditMaterial &m) {
    if (Elm *elm = Proj.findElm(elm_id))
        name = elm->name;
    T.elm_id = elm_id;
    color_name = m.color_map;
    base_0_tex = m.base_0_tex;
    base_1_tex = m.base_1_tex;
    base_2_tex = m.base_2_tex;
    detail_tex = m.detail_tex;
    macro_tex = m.macro_tex;
    emissive_tex = m.emissive_tex;
    cull = m.cull;
    tech = m.tech;
    col_s = m.color_s;
    bump = m.bump;
    smooth = m.smooth;
    reflect = m.reflect_min; // TODO: only reflect_min is checked
    glow = m.glow;
    uv_scale = m.uv_scale;
    return T;
}
int MergeSimilarMaterials::CompareMtrl(C Mtrl &a, C Mtrl &b) { return ComparePathNumber(Proj.elmFullName(b.elm_id), Proj.elmFullName(a.elm_id)); }
void MergeSimilarMaterials::Detect(MergeSimilarMaterials &msm) {
    MtrlEdit.flush(); // flush Material Editor changes first

    Memc<Mtrl> mtrls;
    Memc<Memc<Mtrl>> similar;

    // first load all existing materials
    FREPA(Proj.elms) {
        Elm &elm = Proj.elms[i];
        if (elm.type == ELM_MTRL && elm.finalExists()) {
            EditMaterial edit_mtrl;
            if (edit_mtrl.load(Proj.editPath(elm)))
                if (!msm.color_is || edit_mtrl.base_0_tex.valid())
                    mtrls.New().set(elm.id, edit_mtrl);
        }
    }
    mtrls.sort(CompareMtrl);

    // detect similar ones
    for (; mtrls.elms();) {
        Mtrl &mtrl = mtrls.last();

        // first check in already merged groups
        REPA(similar) {
            Memc<Mtrl> &group = similar[i];
            if (mtrl.similar(group[0])) // compare with the first one in the group only
            {
                Swap(group.New(), mtrl);
                mtrls.removeLast();
                goto merged;
            }
        }

        // now check all single materials
        REPD(i, mtrls.elms() - 1) {
            Mtrl &test = mtrls[i];
            if (mtrl.similar(test)) // 2 materials are similar
            {
                Memc<Mtrl> &group = similar.New(); // create a new group
                Swap(group.New(), mtrl);
                Swap(group.New(), test);
                mtrls.removeLast();    // first remove the one with higher index
                mtrls.remove(i, true); // now   remove the one with lower  index
                goto merged;
            }
        }

        mtrls.removeLast(); // no similar were detected so just remove it

    merged:;
    }

    // setup data
    MSM.replace.clear();
    MSM.data.clear();
    FREPA(similar) {
        C Memc<Mtrl> &src = similar[i];
        for (int i = 1; i < src.elms(); i++)
            MSM.replace.New().set(src[i].elm_id, src[0].elm_id);
        if (MSM.data.elms())
            MSM.data.New();
        FREPA(src)
        MSM.data.New().id = src[i].elm_id;
    }
    MSM.replace.sort(Compare);
    MSM.list.setData(MSM.data);
}
void MergeSimilarMaterials::display(C MemPtr<UID> &elm_ids) {
    Memc<UID> mtrl_ids;
    REPA(elm_ids)
    if (Elm *mtrl = Proj.findElm(elm_ids[i], ELM_MTRL))
        mtrl_ids.add(mtrl->id);
    if (mtrl_ids.elms() > 1) {
        replace.clear();
        data.clear();
        mtrl_ids.sort(CompareProjPath);
        FREPA(mtrl_ids) {
            if (i)
                replace.New().set(mtrl_ids[i], mtrl_ids[0]);
            data.New().id = mtrl_ids[i];
        }
        replace.sort(Compare);
        list.setData(data);
        activate();
    }
}
void MergeSimilarMaterials::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos) {
    if (contains(focus_obj)) {
        display(elms);
        elms.clear();
    }
}
void MergeSimilarMaterials::Merge(MergeSimilarMaterials &msm) {
    if (msm.replace.elms()) {
        ReplaceIDs = msm.replace;
        msm.clearProj();
        IDReplaceState.set(StateFadeTime);
    }
}
Str MergeSimilarMaterials::Data::AsText(C Data &data) { return Proj.elmFullName(data.id); }
void MergeSimilarMaterials::clearProj() {
    replace.clear();
    data.clear();
    list.clear();
}
void MergeSimilarMaterials::create() {
    add("Require Same:");
    add("Material Name", MEMBER(MergeSimilarMaterials, name)).desc("Materials will be tested against their name in the project");
    add("Color Source Name", MEMBER(MergeSimilarMaterials, color_name)).desc("Materials will be tested against the source file name of the color texture");
    add("Color Texture", MEMBER(MergeSimilarMaterials, color_tex)).desc("Materials will be tested against the actual color texture image");
    add("Normal Texture", MEMBER(MergeSimilarMaterials, normal_tex));
    add("Base2 Texture", MEMBER(MergeSimilarMaterials, base2_tex));
    add("Detail Texture", MEMBER(MergeSimilarMaterials, detail_tex));
    add("Macro Texture", MEMBER(MergeSimilarMaterials, macro_tex));
    add("Emissive Texture", MEMBER(MergeSimilarMaterials, emissive_tex));
    add("Technique", MEMBER(MergeSimilarMaterials, tech));
    add("Cull", MEMBER(MergeSimilarMaterials, cull));
    add("Color Value", MEMBER(MergeSimilarMaterials, color_value)).desc("Tolerance").range(0, 2);
    add("Bump Value", MEMBER(MergeSimilarMaterials, bump_value)).desc("Tolerance").range(0, 1);
    add("Smooth Value", MEMBER(MergeSimilarMaterials, smooth_value)).desc("Tolerance").range(0, 1);
    add("Reflect Value", MEMBER(MergeSimilarMaterials, reflect_value)).desc("Tolerance").range(0, 1);
    add("Glow Value", MEMBER(MergeSimilarMaterials, glow_value)).desc("Tolerance").range(0, 1);
    add("UV Scale", MEMBER(MergeSimilarMaterials, uv_scale)).desc("Tolerance").min(0);
    add();
    add("Merge Only If:");
    add("Color Texture Exists", MEMBER(MergeSimilarMaterials, color_is)).desc("When this option is selected, then Materials will not be merged if they don't have a color texture image set");
    flt h = 0.043f;
    super::create("Merge Similar Materials", Vec2(0.02f, -0.02f), 0.036f, h, 0.15f);
    button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
    int c = 0;
    FREPA(props)
    if (props[i].textline.is()) {
        CheckBox *check = null;
        switch (c++) {
        case 0:
            check = &color_value_on;
            break;
        case 1:
            check = &bump_value_on;
            break;
        case 2:
            check = &smooth_value_on;
            break;
        case 3:
            check = &reflect_value_on;
            break;
        case 4:
            check = &glow_value_on;
            break;
        case 5:
            check = &uv_scale_on;
            break;
        }
        if (check)
            T += check->create(Rect_LU(props[i].textline.pos(), props[i].textline.size().y), true);
        props[i].textline.move(Vec2(h, 0));
        props[i].button.move(Vec2(h, 0));
    }
    autoData(this);
    Vec2 params = rect().size();
    params.x += h;
    Vec2 size = params;
    size.x += 1.3f;
    size.y += 0.6f;
    rect(Rect_C(0, size));
    T += detected.create(Vec2(Avg(params.x, size.x) - 0.05f, -0.04f), "Detected:", &ts);
    T += detect.create(Rect_D(size.x * 1 / 3, -clientHeight() + 0.04f, 0.3f, 0.06f), "Detect").func(Detect, T).desc("Detect similar materials");
    T += merge.create(Rect_D(size.x * 2 / 3, -clientHeight() + 0.04f, 0.3f, 0.06f), "Merge").func(Merge, T).desc("Merge detected materials");

    T += region.create(Rect(params.x, detect.rect().max.y + 0.04f, size.x - 0.02f, detected.pos().y - 0.03f));
    ListColumn lc[] =
        {
            ListColumn(Data::AsText, LCW_DATA, "File"),
        };
    region += list.create(lc, Elms(lc), true).elmHeight(0.038f).textSize(0, 1);
    FlagDisable(list.flag, LIST_SORTABLE);
    list.cur_mode = LCM_MOUSE;
}
void MergeSimilarMaterials::update(C GuiPC &gpc) {
    super::update(gpc);
    if (gpc.visible && visible()) {
        if (Ms.tappedFirst(0) && Gui.ms() == &list)
            if (Data *data = list())
                if (Elm *elm = Proj.findElm(data->id))
                    MtrlEdit.toggle(elm);
    }
}
MergeSimilarMaterials::MergeSimilarMaterials() : name(false), color_name(false), color_is(false), color_tex(true), normal_tex(true), base2_tex(true), detail_tex(true), macro_tex(true), emissive_tex(true), tech(true), cull(true), color_value(0.1f), bump_value(0.1f), smooth_value(0.1f), reflect_value(0.1f), glow_value(0.1f), uv_scale(0.1f) {}

MergeSimilarMaterials::Data::Data() : id(UIDZero) {}

/******************************************************************************/
