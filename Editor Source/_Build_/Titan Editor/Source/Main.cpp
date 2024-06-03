﻿/******************************************************************************/
#include "../../stdafx.h"
/******************************************************************************/
/******************************************************************************

      Objects must use lock/unlock before modifying its elements (because background-thread-world-building loads area object 'Object.phys' read-only)
       Meshes must use lock/unlock before modifying its elements (because background-thread-world-building accesses mesh        data       read-only)
   PhysBodies must use lock/unlock before modifying its elements (because background-thread-world-building accesses phys bodies data       read-only)

/******************************************************************************/
bool Initialized = false;
Str SettingsPath, RunAtExit;
Environment DefaultEnvironment;
Threads WorkerThreads, BuilderThreads, BackgroundThreads;
/******************************************************************************/
void ScreenChanged(flt old_width, flt old_height) {
    VidOpt.setScale();
    VidOpt.resize();
    Misc.resize();
    Proj.resize();
    Projs.resize();
    Mode.resize();
    MtrlEdit.resize();
    WaterMtrlEdit.resize(); // resize after 'Proj'
    ObjEdit.resize();
    AnimEdit.resize();
    WorldEdit.resize();
    GuiSkinEdit.resize();
    GuiEdit.resize();
    CodeEdit.resize();
    CopyElms.resize();
    GameScreenChanged();
    SetKbExclusive(); // call because it depends on fullscreen mode
}
void SetShader() {
    Game::World.setShader();
    WorldEdit.setShader();
    ObjEdit.setShader();
    TexDownsize.setShader();
}
void Drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos) {
    AppStore.drop(names, obj, screen_pos); // process this before CopyElms, so it can be uploaded to the Store
    CopyElms.drop(names, obj, screen_pos); // process this first so it will handle importing ProjectPackage's
    Proj.drop(names, obj, screen_pos);
    MtrlEdit.drop(names, obj, screen_pos);
    WaterMtrlEdit.drop(names, obj, screen_pos);
    ObjEdit.drop(names, obj, screen_pos);
    ImportTerrain.drop(names, obj, screen_pos);
    ImageEdit.drop(names, obj, screen_pos);
    Brush.drop(names, obj, screen_pos);
    AppPropsEdit.drop(names, obj, screen_pos);
    SplitAnim.drop(names, obj, screen_pos);
    CodeEdit.drop(names, obj, screen_pos);
    CreateMtrls.drop(names, obj, screen_pos);
    if (obj && obj->type() == GO_TEXTLINE && names.elms())
        obj->asTextLine().set(names[0]);
}
bool SaveChanges(void (*after_save_close)(bool all_saved, ptr user), ptr user) {
    Memc<Edit::SaveChanges::Elm> elms;
    CodeEdit.saveChanges(elms);
    CodeEdit.saveChanges(elms, after_save_close, user);
    return !elms.elms();
}
void SetExit(bool all_saved, ptr) {
    if (all_saved)
        StateExit.set();
}
void Quit() {
    if (StateActive == &StatePublish)
        PublishCancel();
    else
        SaveChanges(SetExit);
}
void Resumed() {
    if (StateActive == &StateProjectList && !(Projs.proj_list.contains(Gui.ms()) && Gui.ms()->type() == GO_CHECKBOX)) // if we're activating application by clicking on a project list checkbox, then don't refresh, because it would lose focus and don't get toggled, or worse (another project would be toggled due to refresh)
        Projs.refresh();                                                                                              // refresh list of project when window gets focused in case we've copied some projects to the Editor
}
void ReceiveData(cptr data, int size, C SysWindow &sender_window) {
    File f;
    f.readMem(data, size);
    Str s = f.getStr();
    if (GetExt(s) == ProjectPackageExt)
        CopyElms.display(s);
}
void SetTitle() {
    Str title;
    if (Mode() == MODE_CODE)
        title = CodeEdit.title();
    if (title.is())
        title = S + AppName + " - " + title;
    else
        title = AppName;
    App.name(title);
}
void SetKbExclusive() {
    Kb.exclusive(D.full() || StateActive == &StateGame || (StateActive == &StateProject && (Mode() == MODE_OBJ || Mode() == MODE_ANIM || Mode() == MODE_WORLD || Mode() == MODE_TEX_DOWN)) || Theater.visible());
}
void SetProjectState() {
    (Proj.valid() ? Proj.needUpdate() ? StateProjectUpdate : StateProject : StateProjectList).set(StateFadeTime); // if there's a project opened then go back to its editing, otherwise go to project list
}
Rect EditRect(bool modes) {
    Rect r = D.rect();
    if (modes && Mode.visibleTabs())
        MIN(r.max.y, Mode.rect().min.y + D.pixelToScreenSize(Vec2(0, 0.5f)).y);
    if (Proj.visible())
        MAX(r.min.x, Proj.rect().max.x - D.pixelToScreenSize(Vec2(0.5f, 0)).x);
    if (MtrlEdit.visible())
        MIN(r.max.x, MtrlEdit.rect().min.x);
    if (WaterMtrlEdit.visible())
        MIN(r.max.x, WaterMtrlEdit.rect().min.x);
    return r;
}
Environment &CurrentEnvironment() {
    if (Environment *env = EnvEdit.cur())
        return *env;
    return DefaultEnvironment;
}
/******************************************************************************/
void InitPre() {
    ASSERT(ELM_NUM == (int)Edit::ELM_NUM); // they must match exactly
    Str path = GetPath(App.exe()).tailSlash(true);

    App.x = App.y = 0;
    App.receive_data = ReceiveData;
    D.screen_changed = ScreenChanged;
    D.exclusive(false);

    App.flag |= APP_MINIMIZABLE | APP_MAXIMIZABLE | APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE | APP_WORK_IN_BACKGROUND | APP_RESIZABLE;
    INIT(false, false);
#if DEBUG
    App.flag |= APP_BREAKPOINT_ON_ERROR | APP_MEM_LEAKS | APP_CALLSTACK_ON_ERROR;
    Paks.add(ENGINE_DATA_PATH);
    Paks.add(GetPath(ENGINE_DATA_PATH).tailSlash(true) + "Editor.pak");
    SetFbxDllPath(GetPath(ENGINE_DATA_PATH).tailSlash(true) + "FBX32.dll", GetPath(ENGINE_DATA_PATH).tailSlash(true) + "FBX64.dll");
#endif

    SupportCompressAll();
    SupportFilterWaifu();
#if LINUX
    SettingsPath = SystemPath(SP_APP_DATA).tailSlash(true) + ".esenthel\\Editor\\";
#elif DESKTOP
    SettingsPath = SystemPath(SP_APP_DATA).tailSlash(true) + "Esenthel\\Editor\\";
#else
    SettingsPath = SystemPath(SP_APP_DATA).tailSlash(true);
#endif
    FCreateDirs(SettingsPath);
    ProjectsPath = MakeFullPath("Projects\\");
    CodeEdit.projectsBuildPath(ProjectsPath + ProjectsBuildPath);
    App.name(AppName);
    App.flag |= APP_FULL_TOGGLE;
    App.backgroundFull(true);
    App.drop = Drop;
    App.quit = Quit;
    App.resumed = Resumed;
    D.secondaryOpenGLContexts(Cpu.threads() * 3); // worker threads + importer threads + manually called threads
    D.drawNullMaterials(true);
    D.dofFocusMode(true);
    D.set_shader = SetShader;
    D.mode(App.desktopW() * 0.8f, App.desktopH() * 0.8f);
#if !DEBUG
    Paks.add("Bin/Engine.pak");
    Paks.add("Bin/Editor.pak");
#endif
    Sky.atmospheric();
    InitElmOrder();
    LoadSettings();
    VidOpt.ctor(); // init before applying video settings
    ApplyVideoSettings();

    WorkerThreads.create(true, Cpu.threads(), 0, "Editor.Worker");
    BuilderThreads.create(false, Cpu.threads(), 0, "Editor.Builder");
    BackgroundThreads.create(false, Cpu.threads() - 1, 0, "Editor.Background");
}
/******************************************************************************/
bool Init() {
    {
        if (!Physics.created())
            Physics.create().timestep(PHYS_TIMESTEP_VARIABLE);

        const flt delay_remove = 10;
        CachesDelayRemove(delay_remove);
        EditObjects.delayRemove(delay_remove);

        Images.mode(CACHE_DUMMY);
        ImageAtlases.mode(CACHE_DUMMY);
        Materials.mode(CACHE_DUMMY);
        Meshes.mode(CACHE_DUMMY);
        PhysMtrls.mode(CACHE_DUMMY);
        PhysBodies.mode(CACHE_DUMMY);
        Skeletons.mode(CACHE_DUMMY);
        Animations.mode(CACHE_DUMMY);
        ParticlesCache.mode(CACHE_DUMMY);
        Objects.mode(CACHE_DUMMY);
        EditObjects.mode(CACHE_DUMMY);
        Fonts.mode(CACHE_DUMMY);
        Enums.mode(CACHE_DUMMY);
        WaterMtrls.mode(CACHE_DUMMY);
        PanelImages.mode(CACHE_DUMMY);
        Panels.mode(CACHE_DUMMY);
        TextStyles.mode(CACHE_DUMMY);
        GuiSkins.mode(CACHE_DUMMY);
        Environments.mode(CACHE_DUMMY);

        InitGui();
#if 0
      LicenseCheck.create(); // create before 'Buy'
      Buy.create(); // create after 'LicenseCheck'
#endif
        ReloadElm.create();
        SplitAnim.create();
        CopyElms.create();
        SizeStats.create();
        CompareProjs.create();
        ProjSettings.create();
        Publish.create();
        EraseRemoved.create();
        ElmProps.create();
        ObjClassEdit.create();
        RenameSlot.create();
        RenameBone.create();
        RenameEvent.create();
        NewWorld.create();
        Projs.create();
        Importer.create();
        ImportTerrain.create();
        EnumEdit.create();
        FontEdit.create();
        PanelImageEdit.create();
        PhysMtrlEdit.create();
        TextStyleEdit.create();
        AppPropsEdit.create();
        SoundEdit.create();
        VideoEdit.create();
        ImageEdit.create();
        ImageAtlasEdit.create();
        MiniMapEdit.create();
        IconSettsEdit.create();
        IconEdit.create();
        PanelEdit.create();
        EnvEdit.create();
        SetMtrlColor.create();
        MulSoundVolume.create();
        Gui += Calculator.create(Rect_C(0, 0, 1.0f, 0.235f)).hide();
        AppStore.create();
        MSM.create();
        DST.create();
        CreateMtrls.create();
        ConvertToAtlas.create();
        ConvertToDeAtlas.create();
        MtrlEdit.create();
        WaterMtrlEdit.create(); // create before 'Mode' so it's below Code Editor ouput
        Theater.create();       // create before 'Mode' so it's below Code Editor ouput
        Proj.create();          // create before 'Mode'

        // TODO FIX MEMORY CRASH WHEN EXITING THE APP
        Mode.create();      // create after 'MtrlEdit' // TODO INVESTIGATE WHY THIS CALL WHEN ENABLED REDUCE BY ALOT FPS
        Misc.create();      // create after 'MtrlEdit' so 'preview_big' doesn't overlap
                            // FINISH
        Preview.create();   // create after all elements to display preview on top
        RenameElm.create(); // create after 'Preview'
        ReplaceName.create();

        Proj.outer_region.moveAbove(Mode.close); // put above Mode so it's above obj/world/code edit (but below code editor output), so project region resize can be detected better (so it's on top of fullscreen editors)
        MtrlEdit.preview_big.moveAbove(Mode.close);
        WaterMtrlEdit.preview_big.moveAbove(Mode.close);
        Theater.moveAbove(Mode.close); // put above Mode so it's above obj/world/code edit (but below code editor output)

        StateProjectList.set();
        Sun.image = "Gui/Misc/sun.img";
        DefaultEnvironment.get(); // before 'ApplySettings'
        ApplySettings();          // !! after 'DefaultEnvironment' !!
        VidOpt.create();          // !! after 'ApplySettings' !!
        SetKbExclusive();
        AssociateFileType(ProjectPackageExt, App.exe(), ENGINE_NAME ".Editor", ENGINE_NAME " Project", App.exe());
        Initialized = true;
    }
    ScreenChanged();
    return true;
}
void Shut() {
    LoggerThread::GetLoggerThread().logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Start Shut method from Main.cpp");
    // delete threaded objects
    NewLod.del();
    MeshAO.del();
    FontEdit.del();
    PanelImageEdit.del();

    ShutGui();
    Proj.close();
    if (Initialized)
        SaveSettings(); // save before deletion
    WorkerThreads.del();
    BuilderThreads.del();
    BackgroundThreads.del();
#if WINDOWS // TODO SUPPORT MORE OS
    auto &executor = EE::Edit::CmdExecutor::GetInstance();
    executor.stopCmdProcess();
#endif
    if (RunAtExit.is())
        Run(RunAtExit, S, false, App.elevated());
    LoggerThread::GetLoggerThread().logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Finish Shut method from Main.cpp + Exit Logger Thread");
    LoggerThread::GetLoggerThread().ExitLoggerThread();
    // FOR NOW NEED TO CALL EXIT LOGGER THREAD HERE BECAUSE MEMORY CRASH AT GAME EXIT TODO NEED TO FIX
}
/******************************************************************************/
bool Update() { return false; }
void Draw() {}
/******************************************************************************/

/******************************************************************************/
