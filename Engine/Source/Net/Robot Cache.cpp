/******************************************************************************/
#include "stdafx.h"
#define SUPPORT_ROBOT_CACHE WINDOWS_OLD
#if     SUPPORT_ROBOT_CACHE
   #include "../../../ThirdPartyLibs/begin.h"
   #include "../../../ThirdPartyLibs/RobotCache/circuitry_api.h"
   #include "../../../ThirdPartyLibs/end.h"
#endif
namespace EE{
/******************************************************************************/
RobotCacheClass RobotCache;
RobotCacheClass::~RobotCacheClass()
{
#if SUPPORT_ROBOT_CACHE
   CircuitryAPI_Shutdown();
#endif
}
Bool RobotCacheClass::init()
{
#if SUPPORT_ROBOT_CACHE
   if(CircuitryAPI_Init())
   {
     _user_name=FromUTF8(CircuitryFriends()->GetPersonaName());
     _user_id  =         CircuitryUser   ()->GetCircuitryID().ConvertToUint64();
      return true;
   }
#endif
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
