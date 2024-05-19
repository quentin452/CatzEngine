/******************************************************************************/
#include "stdafx.h"

#include "../../../ThirdPartyLibs/begin.h"

#if SUPPORT_THEORA
#include "../../../ThirdPartyLibs/Theora/include/theora/theora.h"
#endif

#if SUPPORT_VORBIS || SUPPORT_OPUS || SUPPORT_VP // may be used by Vorbis/Opus/VP inside WEBM
#pragma warning(push)
#pragma warning(disable : 4996) // strcpy
#include "../../../ThirdPartyLibs/VP/libvpx/third_party/libwebm/mkvparser/mkvparser.cc"
#include "../../../ThirdPartyLibs/VP/libvpx/third_party/libwebm/mkvparser/mkvparser.h"
#pragma warning(pop)
#endif

#if SUPPORT_VP
#include "../../../ThirdPartyLibs/VP/libvpx/vpx/vp8dx.h"
#include "../../../ThirdPartyLibs/VP/libvpx/vpx/vpx_decoder.h"
#include "../../../ThirdPartyLibs/VP/libvpx/webmdec.cc" // always include to avoid having to do separate WindowsUniversal libs for 32/64
#include "../../../ThirdPartyLibs/VP/libvpx/webmdec.h"
#undef UNUSED
#endif

#include "../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE {
/******************************************************************************/
#if SUPPORT_THEORA
struct Theora : VideoCodec {
    Int p;
    ogg_sync_state oy;
    ogg_page og;
    ogg_stream_state vo;
    ogg_stream_state to;
    ogg_packet op;
    theora_info ti;
    theora_comment tc;
    theora_state td;

    void zero() {
        p = 0;
        Zero(oy);
        Zero(og);
        Zero(vo);
        Zero(to);
        Zero(op);
        Zero(ti);
        Zero(tc);
        Zero(td);
    }
    void del() {
        if (p) {
            ogg_stream_clear(&to);
            theora_clear(&td);
            theora_comment_clear(&tc);
            theora_info_clear(&ti);
        }
        ogg_sync_clear(&oy);
        zero();
    }
    virtual Bool create(Video &video) {
        del();

        ogg_sync_init(&oy);

        theora_comment_init(&tc);
        theora_info_init(&ti);

        for (;;) {
            if (!buffer_data(video))
                break;
            Bool read = false;
            while (ogg_sync_pageout(&oy, &og) > 0) {
                ogg_stream_state test;

                if (!ogg_page_bos(&og)) {
                    queue_page();
                    goto done;
                }

                ogg_stream_init(&test, ogg_page_serialno(&og));
                ogg_stream_pagein(&test, &og);
                ogg_stream_packetout(&test, &op);

                if (!p && theora_decode_header(&ti, &tc, &op) >= 0) {
                    CopyFast(&to, &test, SIZE(test));
                    p = 1;
                } else {
                    ogg_stream_clear(&test);
                }
                read = true;
            }
            if (!read)
                break;
        }
    done:;

        while (p && p < 3) {
            int ret;
            while (p && p < 3 && (ret = ogg_stream_packetout(&to, &op))) {
                if (ret < 0 || theora_decode_header(&ti, &tc, &op))
                    return false; // error parsing Theora stream headers; corrupt stream?
                if (++p == 3)
                    break;
            }

            if (ogg_sync_pageout(&oy, &og) > 0)
                queue_page();
            else if (!buffer_data(video))
                return false; // end of file while searching for codec headers
        }

        if (p) {
            theora_decode_init(&td, &ti);
        } else {
            theora_info_clear(&ti);
            theora_comment_clear(&tc);
        }

        while (ogg_sync_pageout(&oy, &og) > 0)
            queue_page();
        return p != 0;
    }

    void queue_page() {
        if (p)
            ogg_stream_pagein(&to, &og);
    }

    Int buffer_data(Video &video) {
        int bytes = video._file.getReturnSize(ogg_sync_buffer(&oy, 4096), 4096);
        ogg_sync_wrote(&oy, bytes);
        return bytes;
    }
    virtual RESULT nextFrame(Video &video, Flt &time) {
        for (;;) {
            for (; p;) {
                if (ogg_stream_packetout(&to, &op) <= 0)
                    break;
                theora_decode_packetin(&td, &op);
                time = theora_granule_time(&td, td.granulepos);
                return OK;
            }
            if (video._file.end())
                return END;
            for (buffer_data(video); ogg_sync_pageout(&oy, &og) > 0;)
                queue_page();
        }
    }
    virtual void frameToImage(Video &video) {
        yuv_buffer yuv;
        theora_decode_YUVout(&td, &yuv);
        if (yuv.y_width > 0 && yuv.y_height > 0) {
            Int offset_x = ti.offset_x, offset_x_uv = (offset_x * yuv.uv_width) / yuv.y_width,
                offset_y = ti.offset_y, offset_y_uv = (offset_y * yuv.uv_height) / yuv.y_height,
                width = ti.frame_width, width_uv = (width * yuv.uv_width) / yuv.y_width,
                height = ti.frame_height, height_uv = (height * yuv.uv_height) / yuv.y_height;

            yuv.y += offset_x + offset_y * yuv.y_stride;
            yuv.u += offset_x_uv + offset_y_uv * yuv.uv_stride;
            yuv.v += offset_x_uv + offset_y_uv * yuv.uv_stride;

            video.frameToImage(width, height, width_uv, height_uv, yuv.y, yuv.u, yuv.v, yuv.y_stride, yuv.uv_stride, yuv.uv_stride);
        }
    }

    ~Theora() { del(); }
    Theora() { zero(); }
};
#endif
/******************************************************************************/
#if SUPPORT_VP
struct MkvReader : mkvparser::IMkvReader {
    File &f;

    MkvReader(File &f) : f(f) {}

    virtual int Read(long long pos, long len, unsigned char *buf) override // 0=OK, -1=error
    {
        if (len == 0)
            return 0;
        if (!f.pos(pos))
            return -1;
        return f.getFast(buf, len) ? 0 : -1;
    }
    virtual int Length(long long *total, long long *available) override // 0=OK, -1=error
    {
        if (total)
            *total = f.size();
        if (available)
            *available = f.size();
        return 0;
    }
};
#if SWITCH
#include "../../../NintendoSwitch/VP.cpp"
#else
struct VP : VideoCodec {
    Bool valid;
    uint8_t *webm_buffer;
    size_t webm_buffer_size;
    VpxInputContext input;
    WebmInputContext webm;
    vpx_codec_ctx_t decoder;

    void zero() {
        valid = false;
        webm_buffer = null;
        webm_buffer_size = 0;
        Zero(input);
        Zero(webm);
        Zero(decoder);
    }
    void del() {
        vpx_codec_destroy(&decoder);
        webm_free(&webm);
        DeleteN(webm_buffer);
        zero();
    }
    virtual Bool create(Video &video) {
        del();
        webm.reader = new MkvReader(video._file);
        if (file_is_webm(&webm, &input)) {
            input.file_type = FILE_TYPE_WEBM;
            if (input.fourcc == CC4('V', 'P', '9', '0'))
                if (!webm_guess_framerate(&webm, &input)) {
                    vpx_codec_dec_cfg_t cfg;
                    Zero(cfg);
                    cfg.threads = Min(2, Cpu.threads());
                    if (!vpx_codec_dec_init(&decoder, vpx_codec_vp9_dx(), &cfg, 0)) {
                        valid = true;
                        return true;
                    }
                }
        }
        del();
        return false;
    }
    virtual RESULT nextFrame(Video &video, Flt &time) {
        if (valid) {
            Int result = webm_read_frame(&webm, &webm_buffer, &webm_buffer_size); // 0=ok, 1=end, -1=error
            if (result == 1)
                return END;
            if (!result && !vpx_codec_decode(&decoder, webm_buffer, (unsigned int)webm_buffer_size, null, 0)) {
                time = webm.timestamp_ns / 1000000000.0;
                return OK;
            }
            del();
        }
        return ERROR;
    }
    virtual void frameToImage(Video &video) {
        if (valid) {
            vpx_codec_iter_t iter = null;
            if (vpx_image_t *image = vpx_codec_get_frame(&decoder, &iter)) {
                Int w = image->d_w, h = image->d_h;
                video.frameToImage(w, h, (w + 1) / 2, (h + 1) / 2, image->planes[0], image->planes[1], image->planes[2], image->stride[0], image->stride[1], image->stride[2]);
            }
        }
    }

    VP() { zero(); }
    ~VP() { del(); }
};
#endif
#endif
/******************************************************************************/
void Video::zero() {
    _codec = VIDEO_NONE;
    _loop = false;
    _mode = DEFAULT;
    _w = _h = _br = 0;
    _time = 0;
    _time_past = 0;
    _fps = 0;
    _d = null;
}
Video::Video() { zero(); }
void Video::release() {
    Delete(_d);
    _file.del();
}
void Video::del() {
    release();
    _lum.del();
    _u.del();
    _v.del();
    _tex.del();
    zero();
}
Bool Video::create(C Str &name, Bool loop, MODE mode) {
    del();
    if (!name.is())
        return true;
    if (_file.read(name)) {
#if SUPPORT_VP
        VP vp;
        if (vp.create(T)) {
            _codec = VIDEO_VP9;
            _w = vp.input.width;
            _h = vp.input.height;
            _br = 0;
            _fps = Flt(vp.input.framerate.numerator) / vp.input.framerate.denominator;
            VP *dest = new VP;
            Swap(*dest, vp);
            _d = dest;
        } else
#endif
        {
#if SUPPORT_THEORA
            _file.pos(0); // reset file position after VP attempt
            Theora theora;
            if (theora.create(T)) {
                _codec = VIDEO_THEORA;
                _w = theora.ti.frame_width;
                _h = theora.ti.frame_height;
                _br = ((theora.ti.target_bitrate > 0) ? theora.ti.target_bitrate : (theora.ti.keyframe_data_target_bitrate > 0) ? theora.ti.keyframe_data_target_bitrate
                                                                                                                                : 0);
                _fps = Flt(theora.ti.fps_numerator) / theora.ti.fps_denominator;
                Theora *dest = new Theora;
                Swap(*dest, theora);
                _d = dest;
            }
#endif
        }
        if (_codec) {
            _loop = loop;
            _mode = mode;
            return true;
        }
    }
    del();
    return false;
}
Bool Video::create(C UID &id, Bool loop, MODE mode) { return create(id.valid() ? _EncodeFileName(id) : null, loop, mode); }
/******************************************************************************/
CChar8 *Video::codecName() C { return CodecName(codec()); }
/******************************************************************************/
Bool Video::createTex() {
    if (_mode == IMAGE) // if want to create a texture
    {
        if (_tex.size() != _lum.size())
            if (!_tex.create(_lum.size(), LINEAR_GAMMA ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8))
                return false;

        SyncLocker locker(D._lock); // needed for drawing in case this is called outside of Draw
        ImageRT *rt[Elms(Renderer._cur)], *rtz = Renderer._cur_ds;
        REPAO(rt) = Renderer._cur[i];
        Renderer.set(&_tex, null, false);
        ALPHA_MODE alpha = D.alpha(ALPHA_NONE);
        draw(D.rect()); // use specified rectangle without fitting via 'drawFs' or 'drawFit'
        D.alpha(alpha);
        Renderer.set(rt[0], rt[1], rt[2], rt[3], rtz, true);
    }
    return true;
}
Bool Video::frameToImage(Int w, Int h, Int w_uv, Int h_uv, CPtr lum_data, CPtr u_data, CPtr v_data, Int lum_pitch, Int u_pitch, Int v_pitch) {
    if (_mode == ALPHA) {
        if (!_lum.create2D(w, h, IMAGE_R8, 1, false))
            return false;
        _lum.setFaceData(lum_data, lum_pitch);
        return true;
    } else {
        if (!_lum.create2D(w, h, IMAGE_R8, 1, false))
            return false;
        _lum.setFaceData(lum_data, lum_pitch);
        if (!_u.create2D(w_uv, h_uv, IMAGE_R8, 1, false))
            return false;
        _u.setFaceData(u_data, u_pitch);
        if (!_v.create2D(w_uv, h_uv, IMAGE_R8, 1, false))
            return false;
        _v.setFaceData(v_data, v_pitch);
        return createTex();
    }
}
Bool Video::frameToImage(Int w, Int h, Int w_uv, Int h_uv, CPtr lum_data, CPtr uv_data, Int lum_pitch, Int uv_pitch) {
    if (_mode == ALPHA) {
        if (!_lum.create2D(w, h, IMAGE_R8, 1, false))
            return false;
        _lum.setFaceData(lum_data, lum_pitch);
        return true;
    } else {
        if (!_lum.create2D(w, h, IMAGE_R8, 1, false))
            return false;
        _lum.setFaceData(lum_data, lum_pitch);
        if (!_u.create2D(w_uv, h_uv, IMAGE_R8G8, 1, false))
            return false;
        _u.setFaceData(uv_data, uv_pitch);
        _v.del();
        return createTex();
    }
}
void Video::frameToImage() {
    if (_d)
        _d->frameToImage(T);
}
Bool Video::nextFrame() {
    if (_d) {
        Flt time;
        switch (_d->nextFrame(T, time)) {
        default:
        error:
            release();
            break;
        case VideoCodec::OK:
        ok:
            _time = time + _time_past;
            return true;
        case VideoCodec::END: // finished playing
        {
            if (_loop) {
                // restart
                _file.pos(0);
                _time_past = _time;
                if (_d->create(T))
                    if (_d->nextFrame(T, time) == VideoCodec::OK)
                        goto ok; // reset and set the first frame
            }
            goto error;
        } break;
        }
    }
    return false;
}
/******************************************************************************/
Bool Video::update(Flt time) {
    if (_d) {
        Bool added = false;
        for (; T.time() <= time;)
            if (nextFrame())
                added = true;
            else
                break; // use <= so update(0) will set first frame
        if (added)
            frameToImage();
        return added || _d; // if there was a frame added or if the video still exists
    }
    return false;
}
/******************************************************************************/
#define UV_MERGED SWITCH
void Video::drawAlphaFs(C Video &alpha, FIT_MODE fit) C { return drawAlpha(alpha, T.fit(D.rect(), fit)); }
void Video::drawAlphaFit(C Video &alpha, C Rect &rect) C { return drawAlpha(alpha, T.fit(rect)); }
void Video::drawAlpha(C Video &alpha, C Rect &rect) C {
    if (_lum.is() && Renderer._cur[0]) {
        Sh.ImgX[0]->set(_lum);
#if UV_MERGED
        Sh.ImgXY[0]->set(_u);
#else
        Sh.ImgX[1]->set(_u);
        Sh.ImgX[2]->set(_v);
#endif
        Sh.ImgX[3]->set(alpha._lum);
        Bool gamma = LINEAR_GAMMA, swap = (gamma && Renderer._cur[0]->canSwapRTV());
        if (swap) {
            gamma = false;
            Renderer._cur[0]->swapRTV();
            Renderer.set(Renderer._cur[0], Renderer._cur_ds, true);
        }
        Shader *&shader = Sh.YUV[gamma][true];
        if (!shader)
            AtomicSet(shader, Sh.get(S8 + "YUV" + gamma + 1 + UV_MERGED));
        VI.shader(shader);
        _lum.draw(rect); // Warning: this will result in a useless VI.image call inside, since we already set '_lum' above
        VI.clear();      // force clear to reset custom shader, in case 'draw' doesn't process drawing
        if (swap) {
            Renderer._cur[0]->swapRTV();
            Renderer.set(Renderer._cur[0], Renderer._cur_ds, true);
        } // restore
    }
}

void Video::drawFs(FIT_MODE fit) C { return draw(T.fit(D.rect(), fit)); }
void Video::drawFit(C Rect &rect) C { return draw(T.fit(rect)); }
void Video::draw(C Rect &rect) C {
    if (_lum.is() && Renderer._cur[0]) {
        Bool gamma = LINEAR_GAMMA, swap = (gamma && Renderer._cur[0]->canSwapRTV());
        if (swap) {
            gamma = false;
            Renderer._cur[0]->swapRTV();
            Renderer.set(Renderer._cur[0], Renderer._cur_ds, true);
        }
        Sh.ImgX[0]->set(_lum);
#if UV_MERGED
        Sh.ImgXY[0]->set(_u);
#else
        Sh.ImgX[1]->set(_u);
        Sh.ImgX[2]->set(_v);
#endif
        Shader *&shader = Sh.YUV[gamma][false];
        if (!shader)
            AtomicSet(shader, Sh.get(S8 + "YUV" + gamma + 0 + UV_MERGED));
        VI.shader(shader);
        _lum.draw(rect); // Warning: this will result in a useless VI.image call inside, since we already set '_lum' above
        VI.clear();      // force clear to reset custom shader, in case 'draw' doesn't process drawing
        if (swap) {
            Renderer._cur[0]->swapRTV();
            Renderer.set(Renderer._cur[0], Renderer._cur_ds, true);
        } // restore
    }
}
/******************************************************************************/
CChar8 *CodecName(VIDEO_CODEC codec) {
    switch (codec) {
    default:
        return null;
    case VIDEO_THEORA:
        return "Theora";
    case VIDEO_VP9:
        return "VP9";
    }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
