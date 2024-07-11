/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
WorldMapEditor WorldMapEdit;
bool WorldMapBuilding, WorldMapOk;
int WorldMapAreasLeft;
State StateWorldMap(UpdateWorldMap, DrawWorldMap, InitWorldMap, ShutWorldMap);
/******************************************************************************/
bool InitWorldMap() {
    SetKbExclusive();
    // Proj.pause(); we're not pausing so the 'WorldMapEdit' doesn't close and unload mini map element/data/game world
    Proj.pauseServer(); // instead just pause the server so no changes are made during mini map generation
    WorldMapEdit.init();
    UpdateProgress.create(Rect_C(0, -0.05f, 1, 0.045f));
    WorldMapBuilding = true;
    WorldMapOk = false;
    WorldMapAreasLeft = Proj.worldAreasToRebuild(&NoTemp(WorldMapEdit.worldID()));
    return true;
}
void ShutWorldMap() {
    UpdateProgress.del();
    // Proj.resume(); we're not resuming
    Proj.resumeServer(); // resume server
    App.stateNormal().flash();
    WorldMapEdit.reloadAreas();
    /*if(WorldMapOk)if(WorldMapVer *ver=WorldMapEdit.ver()) // send new mini map to the server only after full completion (this is not needed since we're resuming the server above)
   {
      Server.setWorldMapSettings(WorldMapEdit.elm_id, ver.settings, ver.time);
      REPA(ver.images)Synchronizer.setWorldMapImage(WorldMapEdit.elm_id, ver.images[i]);
   }*/
}
/******************************************************************************/
bool UpdateWorldMap() {
    if (Kb.bp(KB_ESC)) {
        SetProjectState();
        Gui.msgBox(S, "Mini Map generation breaked on user request");
    }

    Builder.update(!WorldMapBuilding, &NoTemp(WorldMapEdit.worldID()));
    if (WorldMapBuilding) {
        UpdateProgress.set(WorldMapAreasLeft - Proj.worldAreasToRebuild(&NoTemp(WorldMapEdit.worldID())), WorldMapAreasLeft);
        if (Builder.finished(&NoTemp(WorldMapEdit.worldID()))) {
            WorldMapBuilding = false;
            WorldEdit.flush();                         // flush any world areas that were built
            if (ElmWorldMap *data = WorldMapEdit.data()) // start creating mini maps
            {
                FDelInside(Proj.gamePath(WorldMapEdit.elm_id));
                Proj.createWorldMapPaths(WorldMapEdit.elm_id);

                Preview.elmChanged(WorldMapEdit.elm_id);
                if (WorldMapVer *ver = WorldMapEdit.ver()) {
                    data->copyTo(ver->settings);
                    ver->changed = true;
                    ver->time.getUTC();
                    ver->settings.area_size = Game::World.areaSize();
                    ver->settings.save(Proj.gamePath(WorldMapEdit.elm_id).tailSlash(true) + "Settings");
                    ver->images.clear();
                }
            }
            Memc<Game::WorldManager::AreaState> areas;
            Game::World.areaSetState(areas, true); // unload all areas in case they were changed during building
        }
        Time.wait(1000 / 30);
    }

    App.stateProgress(UpdateProgress());
    // Gui.update(); do not update gui as it's not hidden
    Server.update(null, true);
    if (Ms.bp(MS_MAXIMIZE))
        App.window().toggle();
    return true;
}
/******************************************************************************/
void DrawWorldMap() {
    D.clear(BackgroundColor());
    if (WorldMapBuilding) {
        D.text(0, 0.05f, S + "Waiting for " + Proj.worldAreasToRebuild(&NoTemp(WorldMapEdit.worldID())) + " world areas to finish building");
    } else {
        for (uint start = Time.curTimeMs();;) {
            UpdateProgress.set(WorldMapEdit.progress, WorldMapEdit.totalImages());
            App.stateProgress(UpdateProgress());

            if (WorldMapEdit.step()) {
                WorldMapEdit.setAreas();
                WorldMapEdit.draw(true);
                if ((Time.curTimeMs() - start) >= 1000 / 30)
                    break;
            } else {
                WorldMapOk = true;
                SetProjectState();
                break;
            }
        }
        D.viewRect(null);

        D.text(0, 0.05f, "Creating Mini Map Images");
    }
    GuiPC gpc;
    gpc.visible = gpc.enabled = true;
    gpc.client_rect = gpc.clip = D.rect();
    gpc.offset.zero();
    UpdateProgress.draw(gpc);
    D.clip();
    // Gui.draw(); do not draw gui as it's not hidden
    Draw();
}
/******************************************************************************/

/******************************************************************************/
cchar8 *WorldMapEditor::ImageSizes[] =
    {
        "32",
        "64",
        "128",
        "256",
        "512",
        "1024",
};
cchar8 *WorldMapEditor::AreasPerImage[] =
    {
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "12",
        "16",
        "24",
        "32",
        "64",
        "128",
        "256",
};
/******************************************************************************/
void WorldMapEditor::Change::create(ptr user) {
    if (ElmWorldMap *data = WorldMapEdit.data()) {
        T.data = *data;
        WorldMapEdit.undoVis();
    }
}
void WorldMapEditor::Change::apply(ptr user) {
    if (ElmWorldMap *data = WorldMapEdit.data()) {
        UID world = data->world_id;
        int areas = data->areas_per_image;
        data->undo(T.data);
        WorldMapEdit.setChanged(data->world_id != world, data->areas_per_image != areas);
        WorldMapEdit.toGui();
        WorldMapEdit.undoVis();
    }
}
void WorldMapEditor::undoVis() { SetUndo(undos, undo, redo); }
void WorldMapEditor::Render() { WorldMapEdit.render(); }
void WorldMapEditor::render() { Game::World.draw(); }
int WorldMapEditor::areasPerImage() {
    ElmWorldMap *data = T.data();
    return data ? data->areas_per_image : 4;
}
UID WorldMapEditor::worldID() {
    ElmWorldMap *data = T.data();
    return data ? data->world_id : UIDZero;
}
flt WorldMapEditor::WorldMapWidth() { return Game::World.areaSize() * areasPerImage(); }
int WorldMapEditor::totalImages() { return (images.w() + 1) * (images.h() + 1); }
flt WorldMapEditor::ShadowStep(int i, int num) {
    flt f = flt(i) / num;        // uniform fraction
    return Lerp(f, 0.5f, 0.97f); // because we'll set the camera so that most objects/heightmaps will be in the center of the viewport range, then move shadow split focus into the center (0.5)
}
void WorldMapEditor::Draw(Viewport &viewport) { WorldMapEdit.draw(); }
void WorldMapEditor::draw(bool final) {
    if (ElmWorldMap *data = T.data()) {
        // environment
        UID env_id = data->env_id;
        if (!env_id.valid())
            if (Elm *world = Proj.findElm(data->world_id))
                if (ElmWorld *world_data = world->worldData())
                    env_id = world_data->env_id;
        if (!env_id.valid())
            CurrentEnvironment().set();
        else {
            env = Proj.gamePath(env_id);
            env->set();
        }

        // graphics settings
        MOTION_MODE motion = D.motionMode();
        D.motionMode(MOTION_NONE);
        AMBIENT_MODE ambient = D.ambientMode();
        D.ambientMode(AMBIENT_ULTRA);
        flt ambient_res = D.ambientRes();
        if (final)
            D.ambientRes(1);
        flt density = D.density();
        if (final)
            D.density(1);
        EDGE_SOFTEN_MODE edge = D.edgeSoften();
        D.edgeSoften(EDGE_SOFTEN_SMAA);
        DOF_MODE dof = D.dofMode();
        D.dofMode(DOF_NONE);
        flt lod_factor = D.lodFactor();
        D.lodFactor(0);
        byte shd_soft = D.shadowSoft();
        D.shadowSoft(1);
        byte shd_num = D.shadowMapNum();
        D.shadowMapNum(6);
        bool shd_jitter = D.shadowJitter();
        D.shadowJitter(false);
        bool eye_adapt = D.eyeAdaptation();
        D.eyeAdaptation(false);
        bool null_mtrls = D.drawNullMaterials();
        if (final)
            D.drawNullMaterials(false);
        bool ocean = Water.draw;
        Water.draw = false;
        flt view_from = D.viewFrom(), view_range = D.viewRange(), view_fov = D.viewFov();
        flt (*shd_step)(int i, int num) = D.shadow_step;
        D.shadow_step = ShadowStep;

        RectI viewport;
        if (final)
            viewport.set(0, 0, D.resW(), D.resH());
        else
            viewport = D.screenToPixelI(D.viewRect());
        int border = pixel_border,
            res = data->image_size,             // image resolution
            res_border = res + border * 2;      // resolution with borders applied
        MIN(res_border, viewport.size().min()); // do not exceed available viewport resolution
        res = res_border - border * 2;          // recalculate resolution without borders
        if (final)
            res = FloorPow2(res);      // adjust it to be a power of 2
        res_border = res + border * 2; // recalculate resolution with borders

        bool downsample = false;
        if (viewport.w() >= res_border * 2 && viewport.h() >= res_border * 2 && final) {
            res *= 2;
            res_border *= 2;
            border *= 2;
            downsample = true;
        } // if viewport resolution is twice as big as rendering resolution, then render everything in 2x bigger size and later downsample for better quality

        if (final)
            D.viewForceSquarePixel(true);
        D.viewFov(DegToRad(0.4f), FOV_XY);                                                                                    // set low 'fov' to simulate orthogonal rendering, use 'FOV_XY' to set equal horizontal and vertical fov values to generate uniform scale textures
        D.viewRect(final ? &NoTemp(RectI(0, 0, res_border, res_border)) : &RectI(viewport.centerI()).extend(res_border / 2)); // set viewport to cover image map resolution with borders

        // set camera
        Cam.at = ((Vec2(image_pos) + 0.5f) * WorldMapWidth()).x0y(); // set camera at center of current map image
        Cam.yaw = 0;
        Cam.pitch = -PI_2;
        Cam.roll = 0;
        Cam.dist = WorldMapWidth() * 0.5f / Tan(D.viewFovX() / 2); // set distance to cover the selected areas
        Cam.dist *= flt(res_border) / res;                        // adjust for borders
        Cam.setSpherical().set();

        // update viewport
        D.viewFrom(ActiveCam.dist * 0.01f)  // set high 'from' to reduce depth buffer precision issues
            .viewRange(ActiveCam.dist * 2); // set viewport range to cover the range from sky to ground and ground to underground

        Renderer.allow_temporal = false;
        Renderer(WorldMapEditor::Render);
        Renderer.allow_temporal = true;

        if (!final)
            D.pixelToScreen(RectI(viewport.centerI()).extend(res / 2)).draw(RED, false);

        D.lodFactor(lod_factor);
        D.dofMode(dof);
        D.motionMode(motion);
        D.ambientMode(ambient).ambientRes(ambient_res);
        D.density(density);
        D.shadowSoft(shd_soft).shadowMapNum(shd_num).shadowJitter(shd_jitter).shadow_step = shd_step;
        D.edgeSoften(edge);
        D.eyeAdaptation(eye_adapt);
        D.viewForceSquarePixel(false).viewFrom(view_from).viewRange(view_range).viewFov(view_fov);
        D.drawNullMaterials(null_mtrls);
        Water.draw = ocean;

        if (final) {
            Image image;
            if (!Renderer.capture(image, -1, -1, IMAGE_R8G8B8_SRGB, IMAGE_SOFT)) {
                Gui.msgBox(S, "Error capturing screen data");
                SetProjectState();
            } else {
                image.crop(image, border, border, res, res);
                if (downsample)
                    image.downSample();
                image.copy(image, -1, -1, -1, IMAGE_BC1_SRGB, IMAGE_2D, 1);
                image.save(Proj.gamePath(WorldMapEdit.elm_id).tailSlash(true) + image_pos);
                if (WorldMapVer *ver = T.ver()) {
                    ver->images.binaryInclude(image_pos);
                    ver->changed = true;
                }
            }
        }
    }
}
void WorldMapEditor::Make(WorldMapEditor &mme) { mme.makeDo(); }
void WorldMapEditor::PreChanged(C Property &prop) { WorldMapEdit.undos.set(&prop); }
void WorldMapEditor::Changed(C Property &prop) { WorldMapEdit.setChanged(); }
void WorldMapEditor::World(WorldMapEditor &mme, C Str &text) {
    if (ElmWorldMap *data = mme.data()) {
        data->world_id = Proj.findElmID(text, ELM_WORLD);
        data->world_time.getUTC();
        mme.reloadWorld();
    }
}
Str WorldMapEditor::World(C WorldMapEditor &mme) {
    if (ElmWorldMap *data = mme.data())
        return Proj.elmFullName(data->world_id);
    return S;
}
void WorldMapEditor::Env(WorldMapEditor &mme, C Str &text) {
    if (ElmWorldMap *data = mme.data()) {
        data->env_id = Proj.findElmID(text, ELM_ENV);
        data->env_time.getUTC();
    }
}
Str WorldMapEditor::Env(C WorldMapEditor &mme) {
    if (ElmWorldMap *data = mme.data())
        return Proj.elmFullName(data->env_id);
    return S;
}
void WorldMapEditor::ArPerImg(WorldMapEditor &mme, C Str &text) {
    if (ElmWorldMap *data = mme.data()) {
        int i = TextInt(text);
        data->areas_per_image = (InRange(i, AreasPerImage) ? TextInt(AreasPerImage[i]) : 4);
        data->areas_per_image_time.getUTC();
        mme.reloadAreas();
    }
}
Str WorldMapEditor::ArPerImg(C WorldMapEditor &mme) {
    int nearest = -1, dist;
    if (ElmWorldMap *data = mme.data())
        REPA(AreasPerImage) {
            int d = Abs(data->areas_per_image - TextInt(AreasPerImage[i]));
            if (nearest < 0 || d < dist) {
                nearest = i;
                dist = d;
            }
        }
    return nearest;
}
void WorldMapEditor::Size(WorldMapEditor &mme, C Str &text) {
    if (ElmWorldMap *data = mme.data()) {
        int i = TextInt(text);
        data->image_size = (InRange(i, ImageSizes) ? TextInt(ImageSizes[i]) : 256);
        data->image_size_time.getUTC();
    }
}
Str WorldMapEditor::Size(C WorldMapEditor &mme) {
    int nearest = -1, dist;
    if (ElmWorldMap *data = mme.data())
        REPA(ImageSizes) {
            int d = Abs(data->image_size - TextInt(ImageSizes[i]));
            if (nearest < 0 || d < dist) {
                nearest = i;
                dist = d;
            }
        }
    return nearest;
}
void WorldMapEditor::Undo(WorldMapEditor &editor) { editor.undos.undo(); }
void WorldMapEditor::Redo(WorldMapEditor &editor) { editor.undos.redo(); }
void WorldMapEditor::Locate(WorldMapEditor &editor) { Proj.elmLocate(editor.elm_id); }
ElmWorldMap *WorldMapEditor::data() C { return elm ? elm->WorldMapData() : null; }
WorldMapVer *WorldMapEditor::ver() C { return elm ? Proj.WorldMapVerRequire(elm->id) : null; }
void WorldMapEditor::makeDo() {
    if (elm)
        if (ElmWorldMap *data = elm->WorldMapData())
            StateWorldMap.set(StateFadeTime);
}
void WorldMapEditor::init(bool center) {
    ElmWorldMap *data = T.data();
    RectI world_areas = Proj.getWorldAreas(worldID());
    if (center)
        world_areas = world_areas.centerI();
    int api = areasPerImage();
    progress = 0;
    images.set(DivFloor(world_areas.min.x, api),
               DivFloor(world_areas.min.y, api),
               DivFloor(world_areas.max.x, api),
               DivFloor(world_areas.max.y, api));
    image_pos = images.min;
    image_pos.x--; // go back 1 step
}
bool WorldMapEditor::step() {
    if (image_pos.y > images.max.y)
        return false; // if out of Y range then it means that we've processed all mini maps
                      // update mini map positions
    progress++;
    image_pos.x++;                  // move 1 step forward
    if (image_pos.x > images.max.x) // if out of X range, then reset X position and proceed to the next Y position
    {
        image_pos.x = images.min.x; // reset X
        image_pos.y++;              // proceed to next Y
        if (image_pos.y > images.max.y)
            return false; // if out of Y range then it means that we've processed all mini maps
    }
    return true;
}
void WorldMapEditor::create() {
    add("World", MemberDesc(DATA_STR).setFunc(World, World)).elmType(ELM_WORLD);
    add("Environment", MemberDesc(DATA_STR).setFunc(Env, Env)).elmType(ELM_ENV);
    add("Areas Per Image", MemberDesc().setFunc(ArPerImg, ArPerImg)).setEnum(AreasPerImage, Elms(AreasPerImage));
    add("Image Size", MemberDesc().setFunc(Size, Size)).setEnum(ImageSizes, Elms(ImageSizes));
    autoData(this);

    flt h = 0.043f;
    Rect r = super::create("Mini Map Editor", Vec2(0.02f, -0.07f), 0.036f, h, PropElmNameWidth);
    super::changed(Changed, PreChanged);
    T += undo.create(Rect_LU(0.02f, -0.01f, 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo");
    undo.image = "Gui/Misc/undo.img";
    T += redo.create(Rect_LU(undo.rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo");
    redo.image = "Gui/Misc/redo.img";
    T += locate.create(Rect_LU(redo.rect().ru() + Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
    prop_max_x = r.max.x;
    button[1].show();
    button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
    flag |= WIN_RESIZABLE;
    T += make.create(Rect_U(r.down() - Vec2(0, 0.04f), 0.3f, 0.055f), "Create").func(Make, T).desc("Create mini map using current settings");
    T += viewport.create(Draw);
    rect(Rect_C(0, 0, Min(1.7f, D.w() * 2), Min(1.12f, D.h() * 2)));
}
Rect WorldMapEditor::sizeLimit() C {
    Rect r = super::sizeLimit();
    r.min.set(0.9f, 0.5f);
    return r;
}
WorldMapEditor &WorldMapEditor::rect(C Rect &rect) {
    super::rect(rect);
    viewport.rect(Rect(prop_max_x, -clientHeight(), clientWidth(), 0).extend(-0.02f));
    return T;
}
void WorldMapEditor::flush() {
    if (elm && changed) {
        if (ElmWorldMap *data = elm->WorldMapData())
            data->newVer(); // modify just before saving/sending in case we've received data from server after edit
        Server.setElmShort(elm->id);
    }
    changed = false;
}
void WorldMapEditor::setAreas(int border) {
    Memc<Game::WorldManager::AreaState> areas;
    RectI recti = image_pos;
    recti.max++;
    recti *= areasPerImage();
    recti.max--;
    recti.extend(border); // for nearby shadow casting objects
    for (int y = recti.min.y; y <= recti.max.y; y++)
        for (int x = recti.min.x; x <= recti.max.x; x++)
            areas.New().set(VecI2(x, y), Game::AREA_ACTIVE);
    DataPath(Proj.game_path);
    Game::World.areaSetState(areas, true);
    DataPath(S);
}
void WorldMapEditor::reloadAreas() {
    if (ElmWorldMap *data = T.data()) {
        if (data->world_id == WorldEdit.elm_id)
            WorldEdit.flush();
        init(true);
        step();
        setAreas(1); // use low border because this is just a preview
        Time.skipUpdate();
    }
}
void WorldMapEditor::reloadWorld() {
    if (elm) {
        if (ElmWorldMap *data = T.data()) {
            InitGameObjContainers();
            Game::World.use_background_loading = false; // this must be disabled because we may write to world on the main thread
            DataPath(Proj.game_path);
            Game::World.mode(Game::WORLD_MANUAL).NewTry(Proj.gamePath(data->world_id));
            DataPath(S);
            reloadAreas();
        }
    } else {
        Game::World.del();
        Game::World.use_background_loading = true;
    }
}
void WorldMapEditor::setChanged(bool world, bool areas) {
    if (elm) {
        changed = true;
        if (ElmWorldMap *data = elm->WorldMapData())
            data->newVer();
        if (world)
            reloadWorld();
        else if (areas)
            reloadAreas();
        toGui();
    }
}
void WorldMapEditor::set(Elm *elm) {
    if (elm && elm->type != ELM_MINI_MAP)
        elm = null;
    if (elm && elm->type != ELM_WORLD_MAP)
        elm = null;
    if (T.elm != elm) {
        flush();
        undos.del();
        undoVis();
        T.elm = elm;
        T.elm_id = (elm ? elm->id : UIDZero);
        toGui();
        Proj.refresh(false, false);
        visible(T.elm != null).moveToTop();
        reloadWorld();
    }
}
void WorldMapEditor::activate(Elm *elm) {
    set(elm);
    if (T.elm)
        super::activate();
}
void WorldMapEditor::toggle(Elm *elm) {
    if (elm == T.elm)
        elm = null;
    set(elm);
}
WorldMapEditor &WorldMapEditor::hide() {
    if (visible()) {
        super::hide();
        set(null);
    }
    return T;
}
void WorldMapEditor::elmChanged(C UID &elm_id) {
    if (elm && elm->id == elm_id) {
        undos.set(null, true);
        reloadWorld();
        toGui();
    }
}
void WorldMapEditor::erasing(C UID &elm_id) {
    if (elm && elm->id == elm_id)
        set(null);
}
WorldMapEditor::WorldMapEditor() : pixel_border(32), prop_max_x(0), elm_id(UIDZero), elm(null), changed(false), progress(0), image_pos(0), images(0), undos(true) {}

/******************************************************************************/
