﻿/******************************************************************************/
/******************************************************************************/
class EE1EditLake : EE1ObjGlobal
{
   UID             id;
   flt             depth;
   Memc<Memc<Vec>> polys;
   Str             material;

   bool load(File &f, C Str &name);

public:
   EE1EditLake();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
