/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
const_mem_addr struct Slider : GuiObj // Gui Slider !! must be stored in constant memory address !!
{
   GuiSkinPtr skin; // skin override, default=null (if set to null then current value of 'Gui.skin' is used)

   // manage
   virtual Slider& del   (                         )override;                                    // delete
           Slider& create(                         );                                            // create
           Slider& create(C Rect   &rect           ) {create()           .rect(rect); return T;} // create and set initial values
           Slider& create(C Rect   &rect, Flt value) {create().set(value).rect(rect); return T;} // create and set initial values
           Slider& create(C Slider &src            );                                            // create from 'src'

   // get / set
   Flt      operator()(                                    )C {return _value;} // get value (0..1)
   Slider & set       (Flt value, SET_MODE mode=SET_DEFAULT);                  // set value (0..1)
   Bool       vertical(                                    )C {return  _vertical;} // if slider is   vertical
   Bool     horizontal(                                    )C {return !_vertical;} // if slider is horizontal
   GuiSkin* getSkin   (                                    )C {return skin ? skin() : Gui.skin();} // get actual skin

            Slider& func(void (*func)(Ptr   user), Ptr   user=null, Bool immediate=true);                                                       // set function called when value has changed, with 'user' as its parameter
   T1(TYPE) Slider& func(void (*func)(TYPE *user), TYPE *user     , Bool immediate=true) {return T.func((void(*)(Ptr))func,  user, immediate);} // set function called when value has changed, with 'user' as its parameter
   T1(TYPE) Slider& func(void (*func)(TYPE &user), TYPE &user     , Bool immediate=true) {return T.func((void(*)(Ptr))func, &user, immediate);} // set function called when value has changed, with 'user' as its parameter

            void  (*func         ()C) (Ptr user)  {return _func                              ;} // get                         function called when value has changed, this returns a pointer to "void func(Ptr user)" function
            Ptr     funcUser     ()C              {return _func_user                         ;} // get user      parameter for function called when value has changed
            Bool    funcImmediate()C              {return _func_immediate                    ;} // get immediate parameter for function called when value has changed
            Slider& funcImmediate(Bool immediate) {       _func_immediate=immediate; return T;} // set immediate parameter for function called when value has changed

   Slider& focusable(Bool on);   Bool focusable()C {return _focusable;} // set/get if can catch keyboard focus, default=false

   // operations
   virtual Slider& rect(C Rect &rect)override;   C Rect& rect()C {return super::rect();} // set/get rectangle

   // main
   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override; // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
   virtual void    update(C GuiPC &gpc)override; // update object
   virtual void    draw  (C GuiPC &gpc)override; // draw   object

#if EE_PRIVATE
   void zero();
   void call();
#endif

  ~Slider() {del();}
   Slider();

#if !EE_PRIVATE
private:
#endif
   Bool   _vertical, _focusable, _func_immediate;
   Flt    _value, _lit;
   Ptr    _func_user;
   void (*_func)(Ptr user);

protected:
   virtual Bool save(File &f, CChar *path=null)C override;
   virtual Bool load(File &f, CChar *path=null)  override;
};
/******************************************************************************/
