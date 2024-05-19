/******************************************************************************
   The MIT License (MIT)

   Copyright (c) <2014> <Michal Drobot>
   Copyright Epic Games, Inc. All Rights Reserved.
   Copyright (c) Grzegorz Slazinski. All Rights Reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
/******************************************************************************
  'RsqrtFast'  is unused because quality is not good enough
  'RsqrtFast1' is unused because it's slower than full precision 'Rsqrt'
/******************************************************************************/
Flt AcosFast(Flt cos) // 'cos'=-1..1, returns [0, PI]
{
    Flt x = Abs(cos);
    Flt res = (-0.156583 * x + PI_2) * Sqrt(1 - x);
    return (cos >= 0) ? res : PI - res;
}
/******************************************************************************/
Flt AsinFast(Flt sin) // 'sin'=-1..1, returns [-PI/2, PI/2]
{
    Flt x = Abs(sin);
    Flt res = (-0.156583 * x + PI_2) * Sqrt(1 - x);
    return (sin >= 0) ? PI_2 - res : res - PI_2;
}
/******************************************************************************/
Flt AtanFastPos(Flt tan) // 'tan'=0..Inf, returns [0, PI/2]
{
    Flt t0 = (tan <= 1) ? tan : 1 / tan;
    Flt t1 = t0 * t0;
    Flt poly = 0.0872929;
    poly = poly * t1 - 0.301895;
    poly = poly * t1 + 1;
    poly = poly * t0;
    return (tan <= 1) ? poly : PI_2 - poly;
}
Vec2 AtanFastPos(Vec2 tan) // 'tan'=0..Inf, returns [0, PI/2]
{
    Vec2 t0 = (tan <= 1) ? tan : 1 / tan;
    Vec2 t1 = t0 * t0;
    Vec2 poly = 0.0872929;
    poly = poly * t1 - 0.301895;
    poly = poly * t1 + 1;
    poly = poly * t0;
    return (tan <= 1) ? poly : PI_2 - poly;
}
Flt AtanFast(Flt tan) // 'tan'=-Inf..Inf, returns [-PI/2, PI/2]
{
    Flt a = AtanFastPos(Abs(tan));
    return (tan < 0) ? -a : a;
}
Vec2 AtanFast(Vec2 tan) // 'tan'=-Inf..Inf, returns [-PI/2, PI/2]
{
    Vec2 a = AtanFastPos(Abs(tan));
    return (tan < 0) ? -a : a;
}
/******************************************************************************/
Flt Atan2Fast(Flt y, Flt x) {
    Flt t0 = Max(Abs(x), Abs(y));
    Flt t1 = Min(Abs(x), Abs(y));
    Flt t3 = t1 / t0;
    Flt t4 = t3 * t3;

    t0 = +0.0872929;
    t0 = t0 * t4 - 0.301895;
    t0 = t0 * t4 + 1;
    t3 = t0 * t3;

    t3 = (Abs(y) > Abs(x)) ? PI_2 - t3 : t3;
    t3 = (x < 0) ? PI - t3 : t3;
    t3 = (y < 0) ? -t3 : t3;

    return t3;
}
/******************************************************************************/
