#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   'DisplayDraw' contains all the basic 2D drawing options.

/******************************************************************************/
struct DisplayDraw // Display Drawing Functions, this class methods can be called by the use of 'Display' object
{
    // draw dot
    static void dot(C Color &color, C Vec2 &v, Flt radius = 0.007f) { v.draw(color, radius); }
    static void dot(C Color &color, Flt x, Flt y, Flt radius = 0.007f) { Vec2(x, y).draw(color, radius); }
    static void dot(C Color &color, C Vec &v, Flt radius = 0.007f) { v.draw(color, radius); }
    static void dotP(C Color &color, C Vec &v, Flt radius = 0.007f) { v.drawP(color, radius); } // with perspective size depending on distance to camera

    // draw line
    static void line(C Color &color, C Edge2 &edge) { edge.draw(color); }
    static void line(C Color &color, C Vec2 &a, C Vec2 &b) { Edge2(a, b).draw(color); }
    static void line(C Color &color, Flt x0, Flt y0, Flt x1, Flt y1) { Edge2(x0, y0, x1, y1).draw(color); }
    static void line(C Color &color, C Edge &edge) { edge.draw(color); }
    static void line(C Color &color, C Vec &a, C Vec &b) { Edge(a, b).draw(color); }

    static void lineX(C Color &color, Flt y, Flt x0, Flt x1);     // draw horizontal line
    static void lineY(C Color &color, Flt x, Flt y0, Flt y1);     // draw vertical   line
    static void lines(C Color &color, C Vec2 *point, Int points); // draw continuous lines, 'point'=point array, 'points'=number of points

    // draw text
    static void text(C TextStyleParams &ts, Flt x, Flt y, CChar *t, C StrData *data = null, Int datas = 0);  // draw using custom  text style
    static void text(C TextStyleParams &ts, Flt x, Flt y, CChar8 *t, C StrData *data = null, Int datas = 0); // draw using custom  text style
    static void text(Flt x, Flt y, CChar *t, C StrData *data = null, Int datas = 0);                         // draw using default text style
    static void text(Flt x, Flt y, CChar8 *t, C StrData *data = null, Int datas = 0);                        // draw using default text style
    static void text(C TextStyleParams &ts, C Vec2 &p, CChar *t, C StrData *data = null, Int datas = 0) { text(ts, p.x, p.y, t, data, datas); }
    static void text(C TextStyleParams &ts, C Vec2 &p, CChar8 *t, C StrData *data = null, Int datas = 0) { text(ts, p.x, p.y, t, data, datas); }
    static void text(C Vec2 &p, CChar *t, C StrData *data = null, Int datas = 0) { text(p.x, p.y, t, data, datas); }
    static void text(C Vec2 &p, CChar8 *t, C StrData *data = null, Int datas = 0) { text(p.x, p.y, t, data, datas); }

    // draw text in rectangle area
    static void text(C TextStyleParams &ts, C Rect &rect, CChar *t, C StrData *data = null, Int datas = 0, Bool auto_line = false, C Flt *width = null);  // 'width'=optional width used for calculating lines (how much text can fit in 1 line) this should be similar to "rect.w()", this parameter is needed in case you've previously calculated text lines/size/rect and now you want to draw it with the calculated settings, because when moving 'rect' on the screen, its "rect.w()" might slightly change due to floating numerical precision issues and these small changes might result in text lines being split in a different way, using 'width' will guarantee that the width always remains the same and produce the same results, even when 'rect' is being moved.
    static void text(C TextStyleParams &ts, C Rect &rect, CChar8 *t, C StrData *data = null, Int datas = 0, Bool auto_line = false, C Flt *width = null); // 'width'=optional width used for calculating lines (how much text can fit in 1 line) this should be similar to "rect.w()", this parameter is needed in case you've previously calculated text lines/size/rect and now you want to draw it with the calculated settings, because when moving 'rect' on the screen, its "rect.w()" might slightly change due to floating numerical precision issues and these small changes might result in text lines being split in a different way, using 'width' will guarantee that the width always remains the same and produce the same results, even when 'rect' is being moved.
    static void text(C Rect &rect, CChar *t, C StrData *data = null, Int datas = 0, Bool auto_line = false);
    static void text(C Rect &rect, CChar8 *t, C StrData *data = null, Int datas = 0, Bool auto_line = false);

    // set text depth
    static void textDepth(Bool use, Flt depth = 0); // this function can be optionally called before drawing text, to specify depth of the text (Z value for the Depth Buffer), if enabled then the text will be drawn with depth buffer test enabled and will be occluded by objects with depth smaller than 'depth'

    // set text gamma
    static void textBackgroundAuto();  // specify auto text background
    static void textBackgroundBlack(); // specify that text will mostly be drawn on black background
    static void textBackgroundWhite(); // specify that text will mostly be drawn on white background
#if EE_PRIVATE
    static void textBackgroundReset(ShaderParam *&sp, Vec &col);
    static void textBackgroundSet(ShaderParam *sp, C Vec &col);
#endif

    // draw shadow
    static void drawShadowBorders(Byte alpha, C Rect &rect, Flt shadow_radius = 0.05f);
    static void drawShadow(Byte alpha, C Rect &rect, Flt shadow_radius = 0.05f);

    // backbuffer effects
    static void fxBegin();     // begin drawing to secondary render target, this can be called only outside of Render function, after calling this function a secondary render target will be set, it will not be initialized to any color, therefore you may want to clear it using 'D.clearCol'
    static ImageRTPtr fxEnd(); // end   drawing to secondary render target and restore the main render target, helper render target is returned which can be used as Image for drawing, including the use of custom shaders

#if !EE_PRIVATE
  private:
#endif
    Bool _text_depth;

    DisplayDraw();
};
/******************************************************************************/
void DrawKeyboardCursor(C Vec2 &pos, Flt height);
void DrawKeyboardCursorOverwrite(C Vec2 &pos, Flt height, Flt width);
/******************************************************************************/
