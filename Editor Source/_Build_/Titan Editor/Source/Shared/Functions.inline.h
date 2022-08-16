/******************************************************************************/
template<typename TYPE> bool Sync(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(Sync(time, src_time)){data=src_data; return true;} return false;
}
template<typename TYPE> bool SyncMem(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(Sync(time, src_time)){Copy(data, src_data); return true;} return false;
}
template<typename TYPE> bool UndoByTime(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(Undo(time, src_time)){data=src_data; return true;} return false;
}
template<typename TYPE> bool UndoByTimeMem(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(Undo(time, src_time)){Copy(data, src_data); return true;} return false;
}
template<typename TYPE> bool SyncByValue(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(data!=src_data){data=src_data; time=src_time; return true;} return false;
}
template<typename TYPE> bool SyncByValueEqual(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(!Equal(data, src_data)){data=src_data; time=src_time; return true;} return false;
}
template<typename TYPE> bool SyncByValueMem(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(!EqualMem(data, src_data)){Copy(data, src_data); time=src_time; return true;} return false;
}
template<typename TYPE> bool UndoByValue(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(data!=src_data){data=src_data; MAX1(time, src_time); return true;} return false;
}
template<typename TYPE> bool UndoByValueEqual(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(!Equal(data, src_data)){data=src_data; MAX1(time, src_time); return true;} return false;
}
template<typename TYPE> bool UndoByValueMem(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(!EqualMem(data, src_data)){Copy(data, src_data); MAX1(time, src_time); return true;} return false;
}
template<typename TYPE> bool Undo(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data) // ByTimeAndValue, make this the default function because there can be a lot of changes in the same second on the local computer
{
   return UndoByTime (time, src_time, data, src_data) // first check by time because it's faster
       || UndoByValue(time, src_time, data, src_data);
}
template<typename TYPE> bool UndoEqual(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data) // ByTimeAndValue
{
   return UndoByTime      (time, src_time, data, src_data) // first check by time because it's faster
       || UndoByValueEqual(time, src_time, data, src_data);
}
template<typename TYPE> bool UndoMem(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data) // ByTimeAndValue
{
   return UndoByTimeMem (time, src_time, data, src_data) // first check by time because it's faster
       || UndoByValueMem(time, src_time, data, src_data);
}
VecI2 TextVecI2Ex(cchar *t) {return TextVecI2(t);}
Vec2  TextVec2Ex(cchar *t) {return TextVec2 (t);}
Vec   TextVecEx(cchar *t) {return TextVec  (t);}
bool NegativeSB(flt  x) {return FlagOn    ((uint&)x, SIGN_BIT);}
void      CHSSB(flt &x) {       FlagToggle((uint&)x, SIGN_BIT);}
template<typename TYPE      > bool Save(File &f, C Memc<TYPE> &m              ) {  f.putInt(m.elms()); FREPA(m)if(!m[i].save(f      ))return false; return f.ok();}
template<typename TYPE, typename USER> bool Save(File &f, C Memc<TYPE> &m, C USER &user) {  f.putInt(m.elms()); FREPA(m)if(!m[i].save(f, user))return false; return f.ok();}
template<typename TYPE      > bool Save(File &f, C Memx<TYPE> &m              ) {  f.putInt(m.elms()); FREPA(m)if(!m[i].save(f      ))return false; return f.ok();}
template<typename TYPE      > bool Load(File &f,   Memc<TYPE> &m              ) {m.setNum(f.getInt()); FREPA(m)if(!m[i].load(f      ))goto   error; if(f.ok())return true; error: m.clear(); return false;}
template<typename TYPE, typename USER> bool Load(File &f,   Memc<TYPE> &m, C USER &user) {m.setNum(f.getInt()); FREPA(m)if(!m[i].load(f, user))goto   error; if(f.ok())return true; error: m.clear(); return false;}
template<typename TYPE      > bool Load(File &f,   Memx<TYPE> &m              ) {m.setNum(f.getInt()); FREPA(m)if(!m[i].load(f      ))goto   error; if(f.ok())return true; error: m.clear(); return false;}
/******************************************************************************/
