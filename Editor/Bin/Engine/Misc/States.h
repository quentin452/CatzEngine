/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'States' to handle different application states, for which you can specify custom functions for:
      -Initialize
      -Shut Down
      -Update
      -Draw

/******************************************************************************/
struct State // Application State
{
   void (*draw)();

   // set
   void set(                                        ); // set as active state (this sets current State to the StateNext, and in the next frame it will be set also as StateActive)
   void set(Flt fade_time, Bool fade_immediate=false); // set as active state and use screen fade effect between states, that lasts 'fade_time' number of seconds, 'fade_immediate'=if make an immediate copy of the screen when calling this function with current application state (this is slower because it re-draws entire screen, but uses current application state), false=use the next frame result (faster but uses next frame which may not have current application state)

   State(Bool (*update)(), void (*draw)(), Bool (*init)()=null, void (*shut)()=null); // 'init' and 'shut' may be set to null

#if EE_PRIVATE
   void shutDo() {     if(shut)   shut();}
   Bool initDo() {return !init || init();}
#endif

#if !EE_PRIVATE
private:
#endif
   Bool (*update)();
   Bool (*init  )();
   void (*shut  )();
}extern
   StateExit  , //        state which always exits application (  null, null, null, null)
   StateMain  , // main   state                                (Update, Draw, null, null)
  *StateActive, // active state
  *StateNext  ; // next   state that will be activated in the next frame, default=&StateMain
/******************************************************************************/
#if EE_PRIVATE
void   InitState();
void   ShutState();
Bool UpdateState();
Bool   DrawState();
#endif
/******************************************************************************/
