/******************************************************************************/
extern const bool PUBLISH          ;
extern const bool EMBED_ENGINE_DATA;
extern cchar *C    ENGINE_DATA_PATH;
extern cchar *C   PROJECT_DATA_PATH;
extern cchar *C   PROJECT_NAME     ;
extern cchar *C   APP_NAME         ;
extern const UID  APP_GUI_SKIN     ;
extern Cipher *C  PROJECT_CIPHER   ;
/******************************************************************************/
void INIT(bool load_engine_data=true, bool load_project_data=true);
void INIT_OBJ_TYPE();
/******************************************************************************/
