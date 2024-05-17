// This is the default config, used when "Esenthel Builder" wasn't started yet
#define SUPPORT_AAC 0
#if !IOS_SIMULATOR && !WEB
   #define PHYSX 1
#else
   #define PHYSX 0
#endif
