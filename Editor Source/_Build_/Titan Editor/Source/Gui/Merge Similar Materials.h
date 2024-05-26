/******************************************************************************/
extern Memc<IDReplace> ReplaceIDs;
extern State IDReplaceState;
/******************************************************************************/
int Compare(C IDReplace &a, C IDReplace &b);
int Compare(C IDReplace &a, C UID &b);
C IDReplace *ReplaceID(C UID &src, Memc<IDReplace> &replace = ReplaceIDs);
bool ThreadIDReplace(Thread &thread);
bool InitIDReplace();
void ShutIDReplace();
bool UpdateIDReplace();
void DrawIDReplace();
/******************************************************************************/
