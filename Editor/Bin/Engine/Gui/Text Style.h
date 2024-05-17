/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'TextStyle'       to specify custom text style which can be used when drawing texts, use it      as a global/class variable for storage.
   Use 'TextStyleParams' to specify custom text style which can be used when drawing texts, use it only as a local        variable inside functions for drawing/calculation.

/******************************************************************************

   'TextStyle' and 'TextStyleParams' are mostly the same, as 'TextStyle' is based on 'TextStyleParams'.
      The only difference is that 'TextStyle' stores the 'font' information as a reference counted pointer,
         which allows releasing the 'font' when it's no longer used.
      Therefore 'TextStyle' should be used anywhere for storage purposes.

   'TextStyleParams' does NOT store the 'font' information as a reference counted pointer, but as a regular pointer.
      Therefore it cannot be used for storage globally or in classes.
      The purpose of 'TextStyleParams' is to be used only inside function for drawing or temporary processing.

   For example it is often needed to draw the text on the screen, based on an existing 'TextStyle' however with small modification.
      Normally we would do something like that:
         void draw()
         {
            TextStyle ts=some_other_existing_text_style; ts.size=0.1; D.text(ts, ..); // draw text using modified 'some_other_existing_text_style'
         }
      The above code however requires a copy constructor for the 'TextStyle' object,
         and since it has a 'font' member as a reference counted pointer, the copy operation does incur some performance penalty.
      Since we're not interested in storing the modified text style, but only to use it for drawing,
         instead of using 'TextStyle', 'TextStyleParams' can be used, like that:
         void draw()
         {
            TextStyleParams tsp=some_other_existing_text_style; tsp.size=0.1; D.text(tsp, ..); // draw text using modified 'some_other_existing_text_style'
         }
      When using 'TextStyleParams' the copy operation is much faster, and does not incur any performance penalty.

   In short:
      Keep 'TextStyle' globally or in classes:
         class Class
         {
            TextStyle ts;
         }

      Use 'TextStyleParams' inside drawing/processing functions when modification is required:
         void draw()
         {
            TextStyleParams tsp=some_other_existing_text_style; "modify tsp"; "draw using tsp";
         }

/******************************************************************************/
enum TEXT_INDEX_MODE : Byte // Text Index Mode
{
   TEXT_INDEX_DEFAULT  , // use when finding cursor position, with rounding
   TEXT_INDEX_OVERWRITE, // use when finding cursor position in overwrite mode
   TEXT_INDEX_FIT      , // use when finding how many elements can fit in given space
};
/******************************************************************************/
struct StrData
{
   enum TYPE : Byte
   {
      NONE,
      TEXT,
      IMAGE,
      COLOR,
      COLOR_OFF,
      SHADOW,
      SHADOW_OFF,
      FONT,
      PANEL,
      NUM,
   };
   TYPE type;
   union
   {
      Str           text;
      ImagePtr      image;
      Color         color;
      Byte          shadow;
      FontPtr       font;
      PanelImagePtr panel;
   };

   Bool visible()C;
   Bool operator==(C StrData &x)C;
   Bool operator!=(C StrData &x)C {return !(T==x);}

   void     del   ();
   StrData& create(TYPE type);

  ~StrData() {del();}
   StrData() {type=NONE;}
   StrData(C StrData &src)=delete;
   void operator=(C StrData &src);
};
struct StrEx : Mems<StrData> // Extended String, which can hold: Text, Images, Color/Shadow/Font information, Panel background
{
   // get
   Bool       is()C {return elms();} // if has any data
   Int    length()C; // get              length (where each image element has length=1)
   Int strLength()C; // get       string length (ignoring   image elements)
   Str str      ()C; // return as string        (ignoring   image, color, shadow, panel, font elements)

   StrEx& clear() {super::clear(); return T;}
   StrEx& del  () {super::del  (); return T;}

   void operator=(C Str &text);

   // add element
   StrEx& text  (   Char          chr   );
   StrEx& text  (   Char8         chr   );
   StrEx& text  (C CChar         *text  );
   StrEx& text  (C CChar8        *text  );
   StrEx& text  (C Str           &text  );
   StrEx& text  (C Str8          &text  );
   StrEx& image (C ImagePtr      &image );
   StrEx& color (C Color         &color );
   StrEx& color (C Color         *color ); // null disables custom color  and reverts to default
   StrEx& shadow(  Byte           shadow);
   StrEx& shadow(C Byte          *shadow); // null disables custom shadow and reverts to default
   StrEx& font  (C FontPtr       &font  ); // null disables custom font   and reverts to default
   StrEx& panel (C PanelImagePtr &panel );

   void   operator+=(C StrEx         &str   );
   void   operator+=(   Char          chr   ) {T.text  (chr  );}
   void   operator+=(   Char8         chr   ) {T.text  (chr  );}
   void   operator+=(C CChar         *text  ) {T.text  (text );}
   void   operator+=(C CChar8        *text  ) {T.text  (text );}
   void   operator+=(C Str           &text  ) {T.text  (text );}
   void   operator+=(C Str8          &text  ) {T.text  (text );}
   void   operator+=(C ImagePtr      &image ) {T.image (image);}
   StrEx& shadow    (  Int            shadow) {T.shadow(Byte(shadow)); return T;}
   StrEx& panelText (C PanelImagePtr &panel, C Str      &text ); // add text  inside a panel, same as "T.panel(panel); T+=text ; T.panel(null);"
   StrEx& panelImage(C PanelImagePtr &panel, C ImagePtr &image); // add image inside a panel, same as "T.panel(panel); T+=image; T.panel(null);"

   StrEx& space(); // add a space if string isn't empty and does not end with a new line or space
   StrEx& nbsp (); // add a nbsp  if string isn't empty and does not end with a new line or space
   StrEx& line (); // add a line  if string isn't empty and does not end with a new line

   // io
   Bool save(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
};
/******************************************************************************/
struct TextStyleParams // Text Style Params
{
   SPACING_MODE spacing    ; // spacing mode              , default=SPACING_NICE
   Bool         pixel_align; // pixel alignment           , default=true (if enabled then every character will be aligned per pixel, you can disable this if you'd like to have smooth movement on the screen at the cost of slightly more blurriness of the text)
   Byte         shadow     , // shadow              0..255, default=255
                shade      ; // shade               0..255, default=230
   Color        color      , //       color               , default=WHITE
                image_color, // image color               , default=WHITE
                selection  ; // selection background color, default=(51, 153, 255, 64)
   Vec2         align      , // aligning                  , default=(0   , 0   )
                size       , // size                      , default=(0.08, 0.08)
                space      ; // space                     , default=(0.06, 1   )
   TextEdit    *edit       ; // text edit settings        , default=null

   Font* font()C {return _font;}   void font(Font *font) {T._font=font;} // get/set font, default=null (if set to null then current value of 'Gui.skin.font' is used)

   // get
#if EE_PRIVATE
   Font* getFont()C;

   Flt  posY (Flt y             )C; // get actual position which will be used for drawing (including padding)
   void posY (Flt y, Vec2 &range)C; // get actual position which will be used for drawing (including padding)
   void posYI(Flt y, Vec2 &range)C; // get actual position which will be used for drawing (including padding)
#endif
   Bool spacingConst()C {return spacing==SPACING_CONST;}

   Flt  colWidth ()C {return size.x*space.x;} // get column width  (this is valid if "spacing==SPACING_CONST")
   Flt lineHeight()C {return size.y*space.y;} // get line   height

   Flt textWidth(C Str  &str , Int max_length=-1         )C; // get width of one line 'str'  text
   Flt textWidth(C Str8 &str , Int max_length=-1         )C; // get width of one line 'str'  text
   Flt textWidth(CChar  *text, Int max_length=-1         )C; // get width of one line 'text' text
   Flt textWidth(CChar8 *text, Int max_length=-1         )C; // get width of one line 'text' text
   Flt textWidth(CChar  *text, C StrData *data, Int datas)C; // get width of one line 'text' text

   Int textIndex(CChar  *text,                             Flt x,        TEXT_INDEX_MODE index_mode                                       )C; // get index of character at 'x'   position, returns "0 .. Length(text)"
   Int textIndex(CChar8 *text,                             Flt x,        TEXT_INDEX_MODE index_mode                                       )C; // get index of character at 'x'   position, returns "0 .. Length(text)"
   Int textIndex(CChar  *text,                             Flt x, Flt y, TEXT_INDEX_MODE index_mode, Flt  width, Bool auto_line, Bool &eol)C; // get index of character at 'x,y' position, returns "0 .. Length(text)"
   Int textIndex(CChar  *text, C StrData *data, Int datas, Flt x, Flt y, TEXT_INDEX_MODE index_mode, Flt  width, Bool auto_line, Bool &eol)C; // get index of character at 'x,y' position, returns "0 .. Length(text)"

   Int textIndexAlign(CChar *text, C StrData *data, Int datas, Flt x, Flt y, TEXT_INDEX_MODE index_mode, C Vec2 &size, Bool auto_line, Bool clamp, Bool &eol)C; // get index of character at 'x,y' position, returns "-1 .. Length(text)", 'clamp'=if position is outside of text then clamp to nearest character (if this is false then -1 is returned), this function uses 'align'

   Vec2 textPos(CChar *text,                             Int index, Flt width, Bool auto_line)C; // get position of character at 'index' location
   Vec2 textPos(CChar *text, C StrData *data, Int datas, Int index, Flt width, Bool auto_line)C; // get position of character at 'index' location

   Int textLines(CChar  *text,                             Flt width, Bool auto_line, Flt *actual_width=null)C; // get number of lines needed to draw 'text' in space as wide as 'width', 'actual_width'=actual width of the text (this is the Max of all line widths)
   Int textLines(CChar  *text, C StrData *data, Int datas, Flt width, Bool auto_line, Flt *actual_width=null)C; // get number of lines needed to draw 'text' in space as wide as 'width', 'actual_width'=actual width of the text (this is the Max of all line widths)
   Int textLines(CChar8 *text, C StrData *data, Int datas, Flt width, Bool auto_line, Flt *actual_width=null)C; // get number of lines needed to draw 'text' in space as wide as 'width', 'actual_width'=actual width of the text (this is the Max of all line widths)

   Flt textHeight(CChar  *text,                             Flt width, Bool auto_line, Flt *actual_width=null)C; // get height needed to draw 'text' in space as wide as 'width', 'actual_width'=actual width of the text (this is the Max of all line widths)
   Flt textHeight(CChar  *text, C StrData *data, Int datas, Flt width, Bool auto_line, Flt *actual_width=null)C; // get height needed to draw 'text' in space as wide as 'width', 'actual_width'=actual width of the text (this is the Max of all line widths)
   Flt textHeight(CChar8 *text, C StrData *data, Int datas, Flt width, Bool auto_line, Flt *actual_width=null)C; // get height needed to draw 'text' in space as wide as 'width', 'actual_width'=actual width of the text (this is the Max of all line widths)

   // operations
   TextStyleParams& reset          (Bool gui=false); // reset all   parameters to default settings, this copies settings from 'Gui.skin.text_style' when 'gui' is false, and 'Gui.skin.text.text_style' when 'gui' is true
   TextStyleParams& resetColors    (Bool gui=false); // reset color parameters to default settings, this copies settings from 'Gui.skin.text_style' when 'gui' is false, and 'Gui.skin.text.text_style' when 'gui' is true, parameters which are copied include: 'shadow', 'shade', 'color', 'selection'
   void             setPerPixelSize(              ); // set 1:1 pixel size "size = D.pixelToScreenSize(font.height())"

   // draw
   void drawSoft(Image &image, Flt x, Flt y, CChar  *text, C StrData *data, Int datas)C; // draw text in software mode to 'image', 'image' should be already locked for writing
   void drawSoft(Image &image, Flt x, Flt y, CChar8 *text, C StrData *data, Int datas)C; // draw text in software mode to 'image', 'image' should be already locked for writing

   void drawSoft(Image &image, C Rect &rect, CChar  *text, C StrData *data, Int datas, Bool auto_line)C; // draw text in software mode to 'image', 'image' should be already locked for writing
   void drawSoft(Image &image, C Rect &rect, CChar8 *text, C StrData *data, Int datas, Bool auto_line)C; // draw text in software mode to 'image', 'image' should be already locked for writing

   explicit TextStyleParams(                     Bool gui=false) {reset(gui);}
            TextStyleParams(TextStyleParams *ts, Bool gui      ) {if(ts)T=*ts;else reset(gui);}

private:
   Font *_font;
};
struct TextStyle : TextStyleParams // Text Style
{
 C FontPtr& font()C {return _font;}   TextStyle& font(C FontPtr &font); // get/set Font

   // operations
   TextStyle& reset      (Bool gui=false); // reset all   parameters to default settings, this copies settings from 'Gui.skin.text_style' when 'gui' is false, and 'Gui.skin.text.text_style' when 'gui' is true
   TextStyle& resetColors(Bool gui=false); // reset color parameters to default settings, this copies settings from 'Gui.skin.text_style' when 'gui' is false, and 'Gui.skin.text.text_style' when 'gui' is true, parameters which are copied include: 'shadow', 'shade', 'color', 'selection'

   // io
   void operator=(C Str &name) ; // load from file, Exit  on fail
   Bool save     (C Str &name)C; // save to   file, false on fail
   Bool load     (C Str &name) ; // load from file, false on fail

   Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   TextStyle();

private:
   FontPtr _font;
};
/******************************************************************************/
DECLARE_CACHE(TextStyle, TextStyles, TextStylePtr); // 'TextStyles' cache storing 'TextStyle' objects which can be accessed by 'TextStylePtr' pointer
/******************************************************************************/
