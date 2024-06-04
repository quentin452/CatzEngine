#pragma once
#undef Half
#undef VecH2
#undef VecH
#undef VecH4
#undef MatrixH3
#undef MatrixH
#undef MatrixH4

#if 0 // this is treated as "32-bit float" on SM_4
#define Half half
#define VecH2 half2
#define VecH half3
#define VecH4 half4
#define MatrixH3 half3x3
#define MatrixH half4x3
#define MatrixH4 half4x4
#else // this is treated as "16-bit or 32-bit float" depending on GPU
#define Half min16float
#define VecH2 min16float2
#define VecH min16float3
#define VecH4 min16float4
#define MatrixH3 min16float3x3
#define MatrixH min16float4x3
#define MatrixH4 min16float4x4
#endif
