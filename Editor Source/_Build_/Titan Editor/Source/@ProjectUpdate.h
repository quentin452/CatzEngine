#pragma once
/******************************************************************************/
/******************************************************************************/
class ProjectUpdate {
    Memc<UID> texs_update_base1, texs_remove_srgb;
    Project *proj;
    SyncLock lock;

    int total() C;

    static void Error(C Str &error);
    static void ErrorLoad(C Str &path);
    static void ErrorSave(C Str &path);

    static void UpdateBase1Tex(UID &tex_id, ProjectUpdate &pu, int thread_index);
    static void RemoveSRGB(UID &tex_id, ProjectUpdate &pu, int thread_index);
    void updateTex(C UID &tex_id, bool old_update);
    void start(Project &proj, Threads &threads);
    void stop(Threads &threads);

  public:
    ProjectUpdate();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
