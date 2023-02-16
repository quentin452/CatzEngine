/******************************************************************************/
const_mem_addr struct TextLine : GuiObj // Gui TextLine !! must be stored in constant memory address !!
{
   Bool   kb_lit   , // if highlight when has keyboard focus           , default=true
          show_find; // if show find image when there's no text entered, default=false
   Str    hint     ; // hint displayed     when there's no text entered, default=""
   Button reset    ; // reset/clear button                             , default=created but hidden

   // manage
   virtual TextLine& del   (                               )override;                             // delete
           TextLine& create(                  C Str &text=S);                                     // create
           TextLine& create(C Rect     &rect, C Str &text=S) {create(text).rect(rect); return T;} // create
           TextLine& create(C TextLine &src                );                                     // create from 'src'

   // get / set
   Bool  password  ()C;                        TextLine& password (  Bool on                             );    // get/set password mode, in password mode text characters will be displayed as '*' (the character can be changed using 'Gui.passwordChar' method), password mode additionally prevents copying text to the clipboard
   Bool  number    ()C;                        TextLine& number   (  Bool on                             );    // get/set number   mode
   Bool  email     ()C;                        TextLine& email    (  Bool on                             );    // get/set email    mode
   Bool  url       ()C;                        TextLine& url      (  Bool on                             );    // get/set url      mode
   Int   maxLength ()C {return _max_length;}   TextLine& maxLength(  Int  max_length                     );    // get/set maximum allowed text length (-1=no limit), default=-1
   Int   cursor    ()C {return _edit.cur  ;}   TextLine& cursor   (  Int  position, Bool margin=true     );    // get/set cursor position
 C Str&  operator()()C {return _text      ;}   TextLine& set      (C Str &text, SET_MODE mode=SET_DEFAULT);    // get/set text
                                               TextLine& clear    (             SET_MODE mode=SET_DEFAULT);    // clear   text
   Flt   offset    ()C {return _offset    ;}                                                                   // get     horizontal offset currently used for displaying text
 C GuiSkinPtr& skin()C {return _skin      ;}   TextLine& skin     (C GuiSkinPtr &skin, Bool sub_objects=true); // get/set skin override, default=null (if set to null then current value of 'Gui.skin' is used), 'sub_objects'=if additionally change the skin of 'reset' button
   GuiSkin* getSkin()C {return _skin ? _skin() : Gui.skin();}                                                  // get     actual skin

            TextLine& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=false);                                                       // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) TextLine& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=false) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) TextLine& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=false) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when text has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)

            void    (*func         ()C) (Ptr user)  {return _func                              ;} // get                         function called when text has changed, this returns a pointer to "void func(Ptr user)" function
            Ptr       funcUser     ()C              {return _func_user                         ;} // get user      parameter for function called when text has changed
            Bool      funcImmediate()C              {return _func_immediate                    ;} // get immediate parameter for function called when text has changed
            TextLine& funcImmediate(Bool immediate) {       _func_immediate=immediate; return T;} // set immediate parameter for function called when text has changed

   virtual TextLine& rect(C Rect &rect )override;   C Rect& rect()C {return super::rect();} // set/get rectangle
   virtual TextLine& move(C Vec2 &delta)override;                                           // move by delta

   Vec2 overlayScreenPos ()C; // get absolute position  on the screen, if this TextLine is covered by Screen Keyboard, then it's drawn as overlay, this function will return position  of that overlay
   Rect overlayScreenRect()C; // get absolute rectangle on the screen, if this TextLine is covered by Screen Keyboard, then it's drawn as overlay, this function will return rectangle of that overlay

   // operations
   TextLine& selectNone  (); // select no  text
   TextLine& selectAll   (); // select all text
   TextLine& selectExtNot(); // select all but extension

   TextLine& cut  (); // perform operation "cut"
   TextLine& copy (); // perform operation "copy"
   TextLine& paste(); // perform operation "paste"

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override; // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc)override; // update object
   virtual void    draw  (C GuiPC &gpc)override; // draw   object

#if EE_PRIVATE
   Bool     showClear()C;
   Flt   clientWidth ()C;
   Flt   clientHeight()C {return rect().h();}
   Vec2  clientSize  ()C {return Vec2(clientWidth(), clientHeight());}
 C Str&   displayText()C; // returns "***" when in password mode, Warning: this is not thread-safe
   void          zero() ;
   void          call() ;
   void   createReset() ;
   Bool    setChanged(C Str &text, SET_MODE mode=SET_DEFAULT);
   void  adjustOffset(              Bool margin=true);
   Bool cursorChanged(Int position, Bool margin=true);
   void  setTextInput()C;

   Flt  localTextPosX(Int index             )C;
   Vec2 localTextPosX(Int index0, Int index1)C; // x=index0 PosX, y=index1 PosX
   Rect localTextRect(Int index             )C;
   Rect localTextRect(Int index0, Int index1)C;
   Rect localSelRect (                      )C;

   enum
   {
      NUMBER=1<<0,
      EMAIL =1<<1,
      URL   =1<<2,
   };
#endif

  ~TextLine() {del();}
   TextLine();

#if !EE_PRIVATE
private:
#endif
   Bool       _can_select, _func_immediate;
   Byte       _flag;
   Int        _max_length;
   Flt        _offset;
   Str        _text;
   TextEdit   _edit;
   GuiSkinPtr _skin;
   Ptr        _func_user;
   void     (*_func)(Ptr user);

protected:
   virtual Bool save(File &f, CChar *path=null)C override;
   virtual Bool load(File &f, CChar *path=null)  override;

   NO_COPY_CONSTRUCTOR(TextLine);
#if EE_PRIVATE
   friend struct ComboBox;
#endif
};
/******************************************************************************/
