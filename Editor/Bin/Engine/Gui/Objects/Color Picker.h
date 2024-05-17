/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
const_mem_addr struct ColorPicker : Window // !! must be stored in constant memory address !!
{
   MemberDesc md;

   // manage
   ColorPicker& create(C Str &name);

   // get / set
 C Vec4       & operator()(                                        )C {return _rgba;} // get current color in   RGBA format
   ColorPicker& set       (C Vec4 &color, SET_MODE mode=SET_DEFAULT);                 // set current color from RGBA values
   ColorPicker& setRGB    (C Vec  &rgb  , SET_MODE mode=SET_DEFAULT);                 // set current color from RGB  values
   ColorPicker& setHSB    (C Vec  &hsb  , SET_MODE mode=SET_DEFAULT);                 // set current color from HSB  values
   ColorPicker& setAlpha  (  Flt   alpha, SET_MODE mode=SET_DEFAULT);                 // set current alpha

            ColorPicker& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=true);                                                       // set function called when color has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) ColorPicker& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=true) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when color has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)
   T1(TYPE) ColorPicker& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=true) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when color has changed, with 'user' as its parameter, 'immediate'=if call the function immediately when a change occurs (this will happen inside object update function where you cannot delete any objects) if set to false then the function will get called after all objects finished updating (there you can delete objects)

            void       (*func         ()C) (Ptr user)  {return _func                              ;} // get                         function called when color has changed, this returns a pointer to "void func(Ptr user)" function
            Ptr          funcUser     ()C              {return _func_user                         ;} // get user      parameter for function called when color has changed
            Bool         funcImmediate()C              {return _func_immediate                    ;} // get immediate parameter for function called when color has changed
            ColorPicker& funcImmediate(Bool immediate) {       _func_immediate=immediate; return T;} // set immediate parameter for function called when color has changed

   // operations
   virtual ColorPicker& show(         )override;
           ColorPicker& mode(Bool real); // change value display mode ('real'=0..1 range, or byte=0..255 range)

   virtual void update(C GuiPC &gpc)override;

   ColorPicker();

#if !EE_PRIVATE
private:
#endif
   struct SatLum : GuiCustom
   {
      virtual void update(C GuiPC &gpc)override;
      virtual void draw  (C GuiPC &gpc)override;
   };
   struct Hue : GuiCustom
   {
      virtual void update(C GuiPC &gpc)override;
      virtual void draw  (C GuiPC &gpc)override;
   };
   struct Colors : GuiCustom
   {
      virtual void update(C GuiPC &gpc)override;
      virtual void draw  (C GuiPC &gpc)override;
   };
   Bool           _real, _func_immediate;
   Vec            _hsb;
   Vec4           _rgba, _old;
   Memx<Property> _props;
   SatLum         _sat_lum;
   Hue            _hue;
   Colors         _color;
   Text           _tnew, _told;
   Button         _mode;
   Ptr            _func_user;
   void         (*_func)(Ptr user);

#if EE_PRIVATE
   void  call    ();
   void  toGui   (Bool rgb=true, Bool alpha=true, Bool hsb=true, Bool rgba=true);
   void _set     (C Vec4 &color, SET_MODE mode=SET_DEFAULT); // set without 'toGui'
   void _setRGB  (C Vec  &rgb  , SET_MODE mode=SET_DEFAULT); // set without 'toGui'
   void _setHSB  (C Vec  &hsb  , SET_MODE mode=SET_DEFAULT); // set without 'toGui'
   void _setAlpha(  Flt   alpha, SET_MODE mode=SET_DEFAULT); // set without 'toGui'
   void  setOld  ();

   static void SetTextStyle();
#endif
};
/******************************************************************************/
