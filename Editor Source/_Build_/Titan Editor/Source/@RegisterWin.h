﻿/******************************************************************************/
/******************************************************************************/
class RegisterWin : ClosableWindow
{
   Text   t_name;
   TextLine name;
   Button     ok;

   static void OK(RegisterWin &rw);

   void create();
   virtual void update(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
extern RegisterWin Register;
/******************************************************************************/
