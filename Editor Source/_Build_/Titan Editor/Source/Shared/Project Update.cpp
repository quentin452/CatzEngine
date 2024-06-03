/******************************************************************************/
#include "../../stdafx.h"
/******************************************************************************/

/******************************************************************************/
int ProjectUpdate::total() C { return texs_update_base1.elms() + texs_remove_srgb.elms(); }
void ProjectUpdate::Error(C Str &error) { Gui.msgBox(S, error); }
void ProjectUpdate::ErrorLoad(C Str &path) { Error(S + "Can't load image:\n\"" + path + '"'); }
void ProjectUpdate::ErrorSave(C Str &path) { Error(S + "Can't save image:\n\"" + path + '"'); }
void ProjectUpdate::UpdateBase1Tex(UID &tex_id, ProjectUpdate &pu, int thread_index) { pu.updateTex(tex_id, true); }
void ProjectUpdate::RemoveSRGB(UID &tex_id, ProjectUpdate &pu, int thread_index) { pu.updateTex(tex_id, false); }
void ProjectUpdate::updateTex(C UID &tex_id, bool old_update) {
    Str path = proj->texPath(tex_id);
    Image img;

    // load
    File f, temp, *src = &f;

    if (!f.read(path)) {
        ErrorLoad(path);
        goto error;
    }
    if (IsServer) {
        if (!Decompress(*src, temp, true)) {
            ErrorLoad(path);
            goto error;
        }
        src = &temp;
        src->pos(0);
    }
    ThreadMayUseGPUData();
    if (!img.load(*src)) {
        ErrorLoad(path);
        goto error;
    }
    f.del();

    // convert
    if (old_update) {
        if (!UpdateMtrlBase1Tex(img, img)) {
            Error("Can't convert texture");
            goto error;
        }
    } else {
        if (!img.copy(img, -1, -1, -1, ImageTypeExcludeSRGB(img.type()))) {
            Error("Can't convert texture");
            goto error;
        }
    }

    // save
    if (IsServer) // server
    {
        if (!img.save(temp.writeMem())) {
            ErrorSave(path);
            goto error;
        }
        temp.pos(0);
        if (!Compress(temp, f.writeMem(), ClientNetworkCompression, ClientNetworkCompressionLevel, false)) {
            ErrorSave(path);
            goto error;
        }
        f.pos(0);
        if (!SafeOverwrite(f, path)) {
            ErrorSave(path);
            goto error;
        }
    } else if (!Save(img, path)) {
        ErrorSave(path);
        goto error;
    } // client

    {
        SyncLocker locker(lock);
        proj->texs_update_base1.binaryExclude(tex_id);
        proj->texs_remove_srgb.binaryExclude(tex_id);
    }
error:;
}
void ProjectUpdate::start(Project &proj, Threads &threads) {
    T.proj = &proj;
    T.texs_update_base1 = proj.texs_update_base1; // copy because 'proj.texs_update_base1' will be dynamically updated
    T.texs_remove_srgb = proj.texs_remove_srgb;   // copy because 'proj.texs_remove_srgb'  will be dynamically updated
    // Warning: with method below there's a risk of processing the same texture from different functions, to remove that risk, the textures are placed to either container, but not both
    REPA(texs_update_base1)
    threads.queue(texs_update_base1[i], UpdateBase1Tex, T);
    REPA(texs_remove_srgb)
    threads.queue(texs_remove_srgb[i], RemoveSRGB, T);
}
void ProjectUpdate::stop(Threads &threads) {
    REPA(texs_update_base1)
    threads.cancel(texs_update_base1[i], UpdateBase1Tex, T);
    REPA(texs_remove_srgb)
    threads.cancel(texs_remove_srgb[i], RemoveSRGB, T);
    REPA(texs_update_base1)
    threads.wait(texs_update_base1[i], UpdateBase1Tex, T);
    REPA(texs_remove_srgb)
    threads.wait(texs_remove_srgb[i], RemoveSRGB, T);
    T.proj = null;
}
ProjectUpdate::ProjectUpdate() : proj(null) {}

/******************************************************************************/
