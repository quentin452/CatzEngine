/*
* The MIT License (MIT)
* Copyright (c) 2015 amigo(white luckers), tanakamura, DeadSix27, YukihoAA and contributors
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#if defined _WIN64 || defined __LP64__
   #define X64 1 // 64-bit
#else
   #define X64 0 // 32-bit
#endif

#ifdef _WIN32
   #define PLATFORM(windows, unix) windows
#else
   #define PLATFORM(windows, unix) unix
#endif

typedef   PLATFORM(  signed __int32,  int32_t)   I32,    Int;
typedef   PLATFORM(unsigned __int32, uint32_t)   U32,   UInt;
typedef   PLATFORM(  signed __int64,  int64_t)   I64,   Long;
typedef   PLATFORM(unsigned __int64, uint64_t)   U64,  ULong;
typedef                       void              *Ptr        ;

#if X64
   typedef I64 IntPtr;
#else
   typedef I32 IntPtr;
#endif

#define T1(a      )   template<typename a                        > // 1 type  template

namespace EE
{
            void WaifuMultiThreadedCall (Int elms, void func(IntPtr elm_index, Ptr   user, Int thread_index), Ptr user);
            void WaifuMultiThreadedCall1(Int elms, void func(IntPtr elm_index, Ptr   user, Int thread_index), Ptr user);
   T1(FUNC) void CallFunc               (                    IntPtr elm_index, FUNC &func, Int thread_index) {func(elm_index);}
   T1(FUNC) void MultiThreadedCall      (Int elms, FUNC &func) {WaifuMultiThreadedCall (elms, (void(*)(IntPtr elm_index, Ptr user, Int thread_index))CallFunc<FUNC>, &func);}
   T1(FUNC) void MultiThreadedCall1     (Int elms, FUNC &func) {WaifuMultiThreadedCall1(elms, (void(*)(IntPtr elm_index, Ptr user, Int thread_index))CallFunc<FUNC>, &func);}
}

#endif
