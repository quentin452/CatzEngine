#define C       const
#define T       (*this)
#define null    nullptr
#define SIZE(x) UIntPtr(sizeof(x))

#undef  DEBUG
#define DEBUG   SET_DEBUG
#undef  WINDOWS
#define WINDOWS SET_WINDOWS
#undef  MAC
#define MAC     SET_MAC
#undef  LINUX
#define LINUX   SET_LINUX
#undef  ANDROID
#define ANDROID SET_ANDROID
#undef  IOS
#define IOS     SET_IOS
#undef  WEB
#define WEB     SET_WEB
#undef   LONG_MIN
#define  LONG_MIN (-0x7FFFFFFFFFFFFFFF-1) // Minimum possible value of 64-bit   signed int ( Long )
#undef   LONG_MAX
#define  LONG_MAX   0x7FFFFFFFFFFFFFFF    // Maximum possible value of 64-bit   signed int ( Long )
#undef  ULONG_MAX
#define ULONG_MAX   0xFFFFFFFFFFFFFFFFu   // Maximum possible value of 64-bit unsigned int (ULong )
