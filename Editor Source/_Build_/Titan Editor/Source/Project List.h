/******************************************************************************/
extern Str ProjectsPath;
extern Str MainProjectPath;
extern Str CurrentlyOpenedFilePath;
extern State StateProjectList;
/******************************************************************************/
bool InitProjectList();
void ShutProjectList();
bool UpdateProjectList();
void DrawProjectList();
/******************************************************************************/
