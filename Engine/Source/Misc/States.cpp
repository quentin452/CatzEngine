/******************************************************************************/
#include "../../stdafx.h"
namespace EE {
/******************************************************************************/
// this cannot be done on OpenGL (on Windows it was tested, even with 'glFlush' significant stuttering occured, like previous frames were rendered sometimes)
#define MT_FLIP (DX11 || DX12) // if use Multi-Threaded flipping which performs 'D.flip' on the secondary thread, allowing the CPU 'Update' to run while flipping is still executing, this will improve performance only if the 'Update' is time consuming, however the performance benefit will not be full if the user tries to modify the GPU data (by using 'D._lock', which will have to wait until flip has finished)
#define MT_SPIN 0              // spin is ~2x faster (0.000015s vs 0.000030s), however may cause slow downs if other threads catch sync lock before 'DisplayFlipThread' does
/******************************************************************************/
#if MT_FLIP
#if MT_SPIN
static Bool DisplayFlipLocked;
#else
static SyncEvent DisplayFlipLocked;
#endif
static SyncEvent DisplayFlipDo;
static Thread DisplayFlipThread;
static Bool DisplayFlipFunc(Thread &thread) {
    for (; DisplayFlipDo.wait() && !DisplayFlipThread.wantStop();) {
        SyncLocker locker(D._lock);
#if MT_SPIN
        DisplayFlipLocked = true;
#else
        DisplayFlipLocked.on();
#endif
#if GL
        // ThreadMayUseGPUData(); glFlush(); didn't help
        // glFinish(); didn't help
#endif
        D.flip();
#if GL
        // glFlush(); didn't help
        // glFinish(); didn't help
#endif
    }
    return false;
}
#endif
/******************************************************************************/
State StateExit(null, null);
/******************************************************************************/
State StateMain(Update, Draw),
    *StateActive,
    *StateNext = &StateMain;
/******************************************************************************/
void State::set() {
    StateNext = this;
}
void State::set(Flt fade_time, Bool fade_immediate) {
    if (StateNext != this) {
        D.setFade(fade_time, fade_immediate); // 'setFade' before changing state
        set();
    }
}
/******************************************************************************/
State::State(Bool (*update)(), void (*draw)(), Bool (*init)(), void (*shut)()) {
    T.update = update;
    T.draw = draw;
    T.init = init;
    T.shut = shut;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
void InitState() {
#if MT_FLIP
    DisplayFlipThread.create(DisplayFlipFunc, null, 3, false, "EE.Display.flip");
#endif
}
void ShutState() {
    if (StateActive) {
        StateActive->shutDo();
        StateActive = null;
    }
#if MT_FLIP
    DisplayFlipThread.stop();
    DisplayFlipDo.on();
    DisplayFlipThread.del();
#endif
}
Bool UpdateState() {
    // change states
    if (StateNext != StateActive) {
        if (StateActive)
            StateActive->shutDo();
        StateActive = StateNext;
        Time._st = 0;
        Time.skipUpdate(); // zero state time before 'StateInit'
        if (!StateActive || !StateActive->initDo())
            return false;
        D.resetEyeAdaptation();
    }

    // update state & draw
    if (StateActive && StateActive->update) {
        // update caches (process in order from parents to base elements)
        DelayRemoveUpdate();
        Environments.update();
        Objects.update();
        Meshes.update();
        PhysBodies.update();
        WaterMtrls.update();
        Materials.update();
        GuiSkins.update();
        Panels.update();
        PanelImages.update();
        TextStyles.update();
        Fonts.update();
        ImageAtlases.update();
        Images.update();

        Physics.step(); // try step, since we're after 'DrawState' any stepping could already be called, so possible lack of synchronization is inevitable
        Dbl t = Time.curTime();
        Bool update = StateActive->update();
        UpdateSound();
        UpdateThreads(); // enable sound update event immediately after state update has finished so latest sounds can be played and changes can be applied
        Time._state_update = Time.curTime() - t;
        return update;
    }
    return false;
}
Bool DrawState() {
    if (App.minimized() || (D.full() && !App.activeOrBackFull()) || !D.created()) {
        UpdateStreamLoads();
        return true;
    } // needed for APP_ALLOW_NO_GPU/APP_ALLOW_NO_XDISPLAY, 'UpdateStreamLoads' to release memory
    if (StateActive && StateActive->draw) {
        // draw
        Dbl start_time, flip_time;
        {
            SyncLockerEx locker(D._lock);
            LockedUpdateStreamLoads();
            if (Renderer._t_measure)
                D.finish();
            start_time = Time.curTime();
            D._flip = Renderer._cur_main; // this will call 'discard', this is needed to hold ref count until 'D.flip' is called
            {
                ImageRTPtr ds = Renderer._cur_main_ds; // this will call 'discard', this is needed to hold ref count until DS is no longer needed
                D._view_main.setViewport();            // user may have called 'D.viewRect' during 'Update', in which setting the viewport can be ignored, so force it always here
                StateActive->draw();
                Physics.step(); // step after all drawing completed (in case it used current state of physics), call this ASAP so physics can continue as fast as possible
                Renderer.cleanup1();
            } // <- this will call 'Renderer._cur_main_ds.discard', because 'ds' is being deleted
            D.fadeDraw();
#if SWITCH
            NS::DrawInput();
#endif
            Gui.dragDraw();
            Ms.draw();
            VR.draw(); // !! THIS WILL AUTOMATICALLY APPLY HDR BOOST #AutoHdrBoost !!

            // flip
            if (Renderer._t_measure) {
                D.finish();
                flip_time = Time.curTime();
            }
#if MT_FLIP
            if (DisplayFlipThread.active()
#if WINDOWS_OLD && DX11
                && SwapChainDesc.Windowed // on DX10+ we can't do this when in full screen (only in windowed), because Alt+Tab will freeze the app, https://docs.microsoft.com/en-au/windows/win32/direct3ddxgi/d3d10-graphics-programming-guide-dxgi#multithread-considerations
#endif
            ) {
#if GL
                glFlush();
#endif
                locker.off();       // release lock
                DisplayFlipDo.on(); // enable flip
                                    // wait for flip to enter lock
#if MT_SPIN
                for (; !DisplayFlipLocked;)
                    Yield();
                DisplayFlipLocked = false;
#else
                DisplayFlipLocked.wait();
#endif
            } else
#endif
            {
                D.flip();
                if (Renderer._t_measure)
                    D.finish();
            }
        }

        Dbl cur_time = Time.curTime();
        Time._state_draw = cur_time - start_time;
        if (Renderer._t_measure) {
            Renderer._t_measures[1]++;
            Renderer._t_gpu_wait[1] += cur_time - flip_time;
        }
        Physics.step(); // flipping could take some time, so try stepping again

        return true;
    }
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
