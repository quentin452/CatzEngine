/******************************************************************************

   Rendering to VR works in a following way:
      -3D is rendered into RenderTexture
      -2D is drawn    into     UITexture (assumed to have pixel_aspect=1)
      -the textures are submitted to HMD ('ovr_SubmitFrame' OculusRift, 'Submit' OpenVR)
      -the textures are also drawn to the System Window (without any warp shaders):
         -RenderTexture is drawn with FIT_FILL for the Left Eye only (with correct scale and position so that the eye focus is at the window center)
         -    UITexture is drawn with FIT_FULL

   Since VR UITexture can be set to a custom size, most likely it will be different than System Window size/resolution.
   Because of that, most display members are set based on VR UITexture, and not Window size, this includes:
      D.w, D.h, D.w2, D.h2, D.pixelToScreen, D.pixelToScreenSize, D.screenToPixel, D.screenToPixelSize, ..
   But DOES NOT INCLUDE:
      D.mode, D.res, D.resW, D.resH

   RenderTexture for simplicity, uses the same D.w and D.h values as UITexture.
      In both cases range (-D.w, -D.h) .. (D.w, D.h) covers the entire textures.
      However since the textures are not displayed with the same scale, visible screen position in one texture is not the same as in the other.
      To compensate for different texture sizes, 'D.viewFovTan' is calculated differently for RenderTexture/UITexture,
         to do correct mapping between 2D<->3D space.

   Just before doing drawing to the System Window, D.w, D.h are recalculated based on window size,
      to maintain correct aspect. Once window drawing is finished, D.w D.h are restored to the VR UITexture values.

   D.windowPixelToScreen is used to convert pixel in the System Window, to screen position on VR UITexture.
      This is used for converting mouse/touch pointer positions.

   To detect if we're rendering into VR, use:
      VR.active()      - this remains true, as long as VR is connected
      Renderer._stereo - this is set at the start of rendering, based on viewport being full and VR Active

/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
namespace EE {
/******************************************************************************/
static struct VirtualRealityDummyApi : VirtualRealityApi {
    virtual Bool init() override {
        Set(VR._name, "Dummy VR");
        VR._res.set(1280, 720);
        VR._refresh = 60;
        VR._eye_dist = 0.064f;
        VR.setFOVTan(1.39f, 1.24f, 1.47f, 1.47f);
        _active = true;
        VR.connected(); // set active before calling connected
        return true;
    }
    virtual void shut() override {
        VR.delImages(); // !! need to call 'VR.delImages' and not 'T.delImages' !!
        _active = false;
        VR.disconnected(); // set active before calling connected
    }

    virtual Bool active() C override { return _active; }
    virtual Matrix matrixCur() C override { return MatrixIdentity; }
    virtual void recenter() override {}
    virtual void changedUIDepth() override {}
    virtual void changedUISize() override {}
    virtual void update() override {}
    virtual void draw() override {}

    virtual void delImages() override {
        _render.del();
        _ui.del();
    }
    virtual Bool createUIImage() override { return _ui.create(VR.guiRes(), LINEAR_GAMMA ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8); }
    virtual Bool createRenderImage() override { return _render.create(VecI2(1280, 720), LINEAR_GAMMA ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8); }

    virtual ImageRTC *getNewRender() override { return _render.is() ? &_render : null; }
    virtual ImageRTC *getNewUI() override { return _ui.is() ? &_ui : null; }

  private:
    Bool _active;
    ImageRTC _render, _ui;
} VirtualRealityDummy;
/******************************************************************************/
VirtualReality VR;
static VirtualRealityApi VrNull;
/******************************************************************************/
static void ClampTexSize(Int &w, Int &h) {
    if (D.maxTexSize() > 0) {
        if (w > D.maxTexSize()) {
            h = RoundPos(Flt(D.maxTexSize()) / w * h);
            w = D.maxTexSize();
        }
        if (h > D.maxTexSize()) {
            w = RoundPos(Flt(D.maxTexSize()) / h * w);
            h = D.maxTexSize();
        }
    }
    MAX(w, 1);
    MAX(h, 1);
}
/******************************************************************************/
VirtualReality::VirtualReality() {
    draw_2d = true;
    _name[0] = '\0';
    _eye_dist = 0.064f;
    _density = 1;
    _refresh = 60;
    _gui_depth = _gui_size = 1;
    _res.zero();
    _gui_res.set(1024, 1024);
    _fov = DegToRad(70);
    _matrix.identity();
    _left.identity();
    _right.identity();
    _adapter_id = 0;
    _api = &VrNull; // !! '_api' may never be null !!
#if 0               // not needed at start, also 'VR' is global so it will always be set to zero
  _left_eye_tex_aspect=1;
  _left_eye_tex_rect  .zero();
#endif
}
void VirtualReality::DummyInit() { VR.init(VirtualRealityDummy); }
Bool VirtualReality::init(VirtualRealityApi &api) {
    if (_api == &api)
        return true; // already using this API
    shut();          // need to fully shut down existing API first
    _api = &api;     // need to set this first in case 'init' will call 'VR._api' methods
    if (api.init())  // if API initialized OK
    {
        // put to API what was set earlier
        api.changedUIDepth();
        api.changedUISize();
        return true;
    }
    _api = &VrNull;
    return false; // if failed then revert back to null
}
void VirtualReality::shut() {
    if (_api != &VrNull) {
        _api->shut();
        _api = &VrNull;  // !! '_api' may never be null !!
        _adapter_id = 0; // disable forcing VR adapter
    }
}
Bool VirtualReality::connected() {
    if (createImages()) {
        Ms.resetCursor();
        Frustum.set();
#if GL // for OpenGL we need to adjust screen synchronization, DirectX can control sync real-time in the 'Present' function
        D.setSync();
#endif
        return true;
    }
    return false;
}
void VirtualReality::disconnected() {
    if (D.created()) {
        Renderer.setMain();
        D.aspectRatioEx();
#if GL // for OpenGL we need to adjust screen synchronization, DirectX can control sync real-time in the 'Present' function
        D.setSync();
#endif
        Ms.clipUpdateConditional(); // call 'clipUpdate' after 'aspectRatioEx', as display size affects mouse clip rect
        Ms.resetCursor();
        Frustum.set();
    }
}
/******************************************************************************/
Bool VirtualReality::active() C { return _api->active(); }
Matrix VirtualReality::matrixCur() C { return _api->matrixCur(); }
void VirtualReality::recenter() { _api->recenter(); }
void VirtualReality::update() { _api->update(); }
void VirtualReality::draw() { // #AutoHdrBoost
    Bool boost = (D.outputPrecision() > IMAGE_PRECISION_8 && D.whiteLum() != 1);
    Vec4 boost_col;
    if (boost)
        boost_col.set(D.whiteLum(), D.whiteLum(), D.whiteLum(), 1);

    // !! it's very important to clear '_render' and '_ui' !!
    if (active()) {
        _api->draw(); // submit VR layers (Render+UI)

        const Bool use_ds = false; // no need for depth buffer because we will only copy VR results
        Renderer._ui = Renderer._cur_main = &Renderer._main;
        Renderer._ui_ds = Renderer._cur_main_ds = &Renderer._main_ds;

        D._flip = Renderer._cur_main; // this will call 'discard', this is needed to hold ref count until 'D.flip' is called
        {
            ImageRTPtr ds;
            if (use_ds)
                ds = Renderer._cur_main_ds;              // this will call 'discard', this is needed to hold ref count until DS is no longer needed
            Renderer.set(Renderer._cur_main, ds, false); // set RT's after discarding

            D._allow_stereo = false;
            D.aspectRatioEx(true, true);
            Frustum.set(); // !! call in this order !!

            // D.viewRect(null); // reset viewport, not needed since we've reset this already above in 'Renderer.set'
            if (_render) {
                ALPHA_MODE alpha = D.alpha(ALPHA_NONE);
#if DEBUG && 1
                if (Kb.b(KB_NPMUL)) {
                    D.clearCol();
                    _render->drawFs(FIT_FULL, FILTER_LINEAR);
                } else
#endif
                    if (boost)
                    _render->drawPart(boost_col, Vec4Zero, Fit(_left_eye_tex_aspect, D.rect(), FIT_FILL), _left_eye_tex_rect);
                else
                    _render->drawPart(Fit(_left_eye_tex_aspect, D.rect(), FIT_FILL), _left_eye_tex_rect);
                D.alpha(alpha);
                _render.clear(); // !! clear because we no longer need it, this is very important because it allows to select the new RT in the VR swapchain !!
            } else
                D.clearCol(); // clear because UI may not cover the entire window

            if (_ui) {
                if (draw_2d) {
                    Rect screen = _ui->fit(D.rect(), FIT_FULL);
                    if (boost)
                        _ui->draw(boost_col, Vec4Zero, screen);
                    else
                        _ui->draw(screen);
                }
                _ui.clear(); // !! clear because we no longer need it, this is very important because it allows to select the new RT in the VR swapchain !!
            }
        } // <- this will call 'Renderer._cur_main_ds.discard', because 'ds' is being deleted

        Renderer.setMain(); // restore the main RT (that includes advancing to the next VR texture) after discarding '_cur_main_ds'
        D._allow_stereo = true;
        D.aspectRatioEx(true, true);
        Frustum.set(); // !! call in this order !!
    } else if (boost) {
        ALPHA_MODE alpha = D.alpha(ALPHA_MUL);
        D.rect().draw(boost_col);
        D.alpha(alpha);
    }
}
/******************************************************************************/
void VirtualReality::setFOVTan(Flt left, Flt right, Flt up, Flt down) {
    VR._fov.set(2 * Atan(Avg(left, right)),
                2 * Atan(Avg(up, down)));

    Flt proj_center = left / (left + right),
        proj_center_offset = Lerp(-1.0f, 1.0f, proj_center);
    ProjMatrixEyeOffset[0] = proj_center_offset;
    ProjMatrixEyeOffset[1] = -proj_center_offset;

    VR._left_eye_tex_aspect = Min(left, right) / Min(up, down);
    VR._left_eye_tex_rect.setC(proj_center * 0.5f, 0.5f, Min(proj_center, 1 - proj_center), 1); // calculate width based on how much we can go to the left and right edge of the texture
}
/******************************************************************************/
VirtualReality &VirtualReality::pixelDensity(Flt density) {
    Clamp(density, 0, 2);
    if (T._density != density) {
        T._density = density;
        createRenderImage();
    }
    return T;
}
VirtualReality &VirtualReality::guiRes(Int w, Int h) {
    ClampTexSize(w, h);
    if (_gui_res.x != w || _gui_res.y != h) {
        _gui_res.set(w, h);
        createUIImage();
    }
    return T;
}
VirtualReality &VirtualReality::guiDepth(Flt depth) {
    MAX(depth, 0);
    if (_gui_depth != depth) {
        _gui_depth = depth;
        _api->changedUIDepth();
    }
    return T;
}
VirtualReality &VirtualReality::guiSize(Flt size) {
    MAX(size, 0);
    if (_gui_size != size) {
        _gui_size = size;
        _api->changedUISize();
        D.setViewFovTan();
    }
    return T;
}

void VirtualReality::delImages() {
    _render.clear();
    _ui.clear();
    _ui_ds.del();
    _api->delImages();
}
Bool VirtualReality::createUIImage() {
    if (D.created() && active()) {
        _ui.clear();
        ClampTexSize(_gui_res.x, _gui_res.y);
        if (_api->createUIImage() && (_ui_ds.create(guiRes(), IMAGE_D24S8, IMAGE_DS) || _ui_ds.create(guiRes(), IMAGE_D32, IMAGE_DS))) {
            SyncLockerEx locker(D._lock);
            Renderer.setMain();
            D.aspectRatioEx();
            Ms.clipUpdateConditional(); // call in this order, as display size affects mouse clip rect
            return true;
        }
        shut();
        return false;
    }
    return true;
}
Bool VirtualReality::createRenderImage() {
    if (D.created() && active()) {
        _render.clear();
        if (_api->createRenderImage()) {
            return true;
        }
        shut();
        return false;
    }
    return true;
}
Bool VirtualReality::createImages() {
    return createRenderImage() && createUIImage();
}

ImageRTC *VirtualReality::getRender() {
    if (!_render)
        _render = _api->getNewRender();
    return _render;
} // this will call 'discard', have to keep as 'ImageRTPtr' as long as we need it
ImageRTC *VirtualReality::getUI() {
    if (!_ui)
        _ui = _api->getNewUI();
    return _ui;
} // this will call 'discard', have to keep as 'ImageRTPtr' as long as we need it
/******************************************************************************/
} // namespace EE
/******************************************************************************/
