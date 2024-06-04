#pragma once
/******************************************************************************

   !! Using Cubic Sampler requires setting 'ImgSize' !!

/******************************************************************************/
Half Max0(Half x, Bool max0 = true) { return max0 ? Max(x, 0) : x; }
VecH2 Max0(VecH2 x, Bool max0 = true) { return max0 ? Max(x, VecH2(0, 0)) : x; }
VecH Max0(VecH x, Bool max0 = true) { return max0 ? Max(x, VecH(0, 0, 0)) : x; }
VecH4 Max0(VecH4 x, Bool max0 = true) { return max0 ? Max(x, VecH4(0, 0, 0, 0)) : x; }
/******************************************************************************/
struct CubicWeight {
    Half W3, W2, W1, W0; // to be used for x=0..1
    Half w3, w2, w1, w0; // to be used for x=1..2
                         // Half z3, z2, z1, z0; // to be used for x=1..2, but x is already -1

    void set(Half blur, Half sharpen) {
        W3 = (12 - 9 * blur - 6 * sharpen) / 6;
        W2 = (-18 + 12 * blur + 6 * sharpen) / 6;
        W1 = 0;
        W0 = (6 - 2 * blur) / 6;
        w3 = (-1 * blur - 6 * sharpen) / 6;
        w2 = (6 * blur + 30 * sharpen) / 6;
        w1 = (-12 * blur - 48 * sharpen) / 6;
        w0 = (8 * blur + 24 * sharpen) / 6;
        // z3=w3; z2=3*w3 + w2; z1=3*w3 + 2*w2 + w1; z0=w3 + w2 + w1 + w0;
        // z3=((-1)*blur + (-6)*sharpen)/6; z2=((3*-1+6)*blur + (3*-6+30)*sharpen)/6; z1=((3*-1+2*6-12)*blur + (3*-6+2*30-48)*sharpen)/6; z0=((-1+6-12+8)*blur + (-6+30-48+24)*sharpen)/6;
    }
    void setSharp(Half sharp) { set(Lerp(1, 0, sharp), Lerp(0, 0.5, sharp)); } // 'sharp'=0..1
    void setBlur(Half blur) { set(Lerp(0, 1, blur), Lerp(0.5, 0, blur)); }     // 'blur' =0..1
};
struct CubicFastSampler {
    /*#if 0 // original
       Vec2  tex =uv*ImgSize.zw-0.5,
             texi=Floor(tex);
       VecH2 texf=tex-texi;
             texi-=0.5; texi*=ImgSize.xy;

       VecH4 c00=TexPoint(img, texi+ImgSize.xy*Vec2(0, 0)), c10=TexPoint(img, texi+ImgSize.xy*Vec2(1, 0)), c20=TexPoint(img, texi+ImgSize.xy*Vec2(2, 0)), c30=TexPoint(img, texi+ImgSize.xy*Vec2(3, 0)),
             c01=TexPoint(img, texi+ImgSize.xy*Vec2(0, 1)), c11=TexPoint(img, texi+ImgSize.xy*Vec2(1, 1)), c21=TexPoint(img, texi+ImgSize.xy*Vec2(2, 1)), c31=TexPoint(img, texi+ImgSize.xy*Vec2(3, 1)),
             c02=TexPoint(img, texi+ImgSize.xy*Vec2(0, 2)), c12=TexPoint(img, texi+ImgSize.xy*Vec2(1, 2)), c22=TexPoint(img, texi+ImgSize.xy*Vec2(2, 2)), c32=TexPoint(img, texi+ImgSize.xy*Vec2(3, 2)),
             c03=TexPoint(img, texi+ImgSize.xy*Vec2(0, 3)), c13=TexPoint(img, texi+ImgSize.xy*Vec2(1, 3)), c23=TexPoint(img, texi+ImgSize.xy*Vec2(2, 3)), c33=TexPoint(img, texi+ImgSize.xy*Vec2(3, 3));

       VecH4 c0=Lerp4(c00, c10, c20, c30, texf.x),
             c1=Lerp4(c01, c11, c21, c31, texf.x),
             c2=Lerp4(c02, c12, c22, c32, texf.x),
             c3=Lerp4(c03, c13, c23, c33, texf.x);

       return Lerp4(c0, c1, c2, c3, texf.y);
    #else // optimized
       uv*=ImgSize.zw;
       Vec2 tc=Floor(uv-0.5)+0.5;
       VecH2 f=uv-tc, f2=f*f, f3=f2*f,
            w0=f2-0.5*(f3+f), w1=1.5*f3-2.5*f2+1.0,
       #if 0
            w2=-1.5*f3+2*f2+0.5*f, w3=0.5*(f3-f2);
       #else
            w3=0.5*(f3-f2), w2=1-w0-w1-w3;
       #endif

       tc*=ImgSize.xy;
       Vec2 tc0=tc-ImgSize.xy, tc3=tc+ImgSize.xy*2;
    #if 0 // 16 tex reads
       Vec2 tc2=tc+ImgSize.xy;

     //Flt w[4][4]={(w0.x*w0.y), (w1.x*w0.y), (w2.x*w0.y), (w3.x*w0.y),
     //             (w0.x*w1.y), (w1.x*w1.y), (w2.x*w1.y), (w3.x*w1.y),
     //             (w0.x*w2.y), (w1.x*w2.y), (w2.x*w2.y), (w3.x*w2.y),
     //             (w0.x*w3.y), (w1.x*w3.y), (w2.x*w3.y), (w3.x*w3.y)};

       return TexPoint(img, Vec2(tc0.x, tc0.y))*(w0.x*w0.y)
             +TexPoint(img, Vec2(tc .x, tc0.y))*(w1.x*w0.y)
             +TexPoint(img, Vec2(tc0.x, tc .y))*(w0.x*w1.y)
             +TexPoint(img, Vec2(tc .x, tc .y))*(w1.x*w1.y)

             +TexPoint(img, Vec2(tc2.x, tc0.y))*(w2.x*w0.y)
             +TexPoint(img, Vec2(tc3.x, tc0.y))*(w3.x*w0.y)
             +TexPoint(img, Vec2(tc2.x, tc .y))*(w2.x*w1.y)
             +TexPoint(img, Vec2(tc3.x, tc .y))*(w3.x*w1.y)

             +TexPoint(img, Vec2(tc0.x, tc2.y))*(w0.x*w2.y)
             +TexPoint(img, Vec2(tc .x, tc2.y))*(w1.x*w2.y)
             +TexPoint(img, Vec2(tc0.x, tc3.y))*(w0.x*w3.y)
             +TexPoint(img, Vec2(tc .x, tc3.y))*(w1.x*w3.y)

             +TexPoint(img, Vec2(tc2.x, tc2.y))*(w2.x*w2.y)
             +TexPoint(img, Vec2(tc3.x, tc2.y))*(w3.x*w2.y)
             +TexPoint(img, Vec2(tc2.x, tc3.y))*(w2.x*w3.y)
             +TexPoint(img, Vec2(tc3.x, tc3.y))*(w3.x*w3.y);
    #else // 5 tex reads, corners are ignored because they're insignificant
       VecH2 w12=w1+w2; Vec2 c=tc+(w2/w12)*ImgSize.xy;
       Half  wu=w12.x*w0.y, wd=w12.x*w3.y, wl=w12.y*w0.x, wr=w12.y*w3.x, wc=w12.x*w12.y;
       return(TexLod(img, Vec2(  c.x, tc0.y))*wu // sample upper edge (2 texels), both weights are negative
             +TexLod(img, Vec2(  c.x, tc3.y))*wd // sample lower edge (2 texels), both weights are negative
             +TexLod(img, Vec2(tc0.x,   c.y))*wl // sample left  edge (2 texels), both weights are negative
             +TexLod(img, Vec2(tc3.x,   c.y))*wr // sample right edge (2 texels), both weights are negative
             +TexLod(img, Vec2(  c.x,   c.y))*wc // sample center     (4 texels), all  weights are positive
             )/(wu+wd+wl+wr+wc);
    #endif
    #endif*/

    Vec2 tc[4];
    VecH2 w[4];
    Vec2 c, l, r, u, d;
    Half wc, wl, wr, wu, wd;

    Vec2 uv(Int x, Int y) { return Vec2(tc[x].x, tc[y].y); }
    Half weight(Int x, Int y) { return w[x].x * w[y].y; }
    Half cornersWeight() { return (w[0].x + w[3].x) * (w[0].y + w[3].y); } // weight(0,0) + weight(3,0) + weight(0,3) + weight(3,3) = w[0].x*w[0].y + w[3].x*w[0].y + w[0].x*w[3].y + w[3].x*w[3].y = (w[0].x+w[3].x)*w[0].y + (w[0].x+w[3].x)*w[3].y
    Vec2 pixel(Vec2 uv, Vec4 img_size) { return Floor(uv * img_size.zw); } // return as Vec2 to avoid Vec2 -> VecI2 conversion
    Vec2 pixelF(Vec2 uv, Vec4 img_size) { return uv * img_size.zw; }

    VecH2 setUV(Vec2 uv, Vec4 img_size) // return fraction
    {
        Vec2 pixf = uv * img_size.zw - 0.5;
        Vec2 pixi = Floor(pixf); // use Vec2 to avoid Vec2 -> VecI2 conversion

        tc[1] = (pixi + 0.5) * img_size.xy;
        tc[0] = tc[1] - img_size.xy;
        tc[2] = tc[1] + img_size.xy;
        tc[3] = tc[1] + img_size.xy * 2;

        return pixf - pixi;
    }
    void setSamples(Vec4 img_size) // set sample UV's based on uv and weights
    {
        VecH2 w12 = w[1] + w[2];
        c = tc[1] + (w[2] / w12) * img_size.xy;
        wu = w12.x * w[0].y;
        wd = w12.x * w[3].y;
        wl = w12.y * w[0].x;
        wr = w12.y * w[3].x;
        wc = w12.x * w12.y;
        Half sum = wc + wl + wr + wu + wd; // 1-cornersWeight()
        wc /= sum;
        wl /= sum;
        l = Vec2(tc[0].x, c.y);
        wr /= sum;
        r = Vec2(tc[3].x, c.y);
        wu /= sum;
        u = Vec2(c.x, tc[0].y);
        wd /= sum;
        d = Vec2(c.x, tc[3].y);
    }

    void set(Vec2 uv, Vec4 img_size) {
        VecH2 f = setUV(uv, img_size), f2 = f * f, f3 = f2 * f;

        w[0] = f2 - 0.5 * (f3 + f);
        w[1] = 1.5 * f3 - 2.5 * f2 + 1;
#if 0
      w[2]=-1.5*f3+2*f2+0.5*f; w[3]=0.5*(f3-f2); // don't calculate it manually, use the fact that the sum is always equal to 1, using [2] component was the fastest version tested
#else
        w[3] = 0.5 * (f3 - f2);
        w[2] = 1 - w[0] - w[1] - w[3];
#endif

        setSamples(img_size);
    }
    void set(Vec2 uv) { set(uv, ImgSize); }
    void set(Vec2 uv, Vec4 img_size, CubicWeight W) // for best results this should use 'texSlow'
    {
        VecH2 F = setUV(uv, img_size);

#if 1 // faster
        { /* ((A*F+B)*F+C)*F+D
              (A*F*F + B*F + C)*F+D
               A*F*F*F + B*F*F + C*F + D */
            VecH2 f;
            f = F;
            w[1] = ((W.W3 * f + W.W2) * f + W.W1) * f + W.W0;
            f = F + 1;
            w[0] = ((W.w3 * f + W.w2) * f + W.w1) * f + W.w0;
            f = 1 - F;
            w[2] = ((W.W3 * f + W.W2) * f + W.W1) * f + W.W0;
            // f=2-F; w[3]=((W.w3*f+W.w2)*f+W.w1)*f+W.w0;
        }
#else
        {
            VecH2 f, f2, f3;
            f = F;
            f2 = f * f;
            f3 = f2 * f;
            w[1] = W.W3 * f3 + W.W2 * f2 + W.W1 * f + W.W0;
            // f=F+1; f2=f*f; f3=f2*f; w[0]=W.w3*f3 + W.w2*f2 + W.w1*f + W.w0; // w[0]=w3*Cube(F+1) + w2*Sqr(F+1) + w1*(F+1) + w0 -> w[0]=w3*(F*F*F + 3*F*F + 3*F + 1) + w2*(F*F + 2*F + 1) + w1*(F+1) + w0
            w[0] = W.z3 * f3 + W.z2 * f2 + W.z1 * f + W.z0;

            f = 1 - F;
            f2 = f * f;
            f3 = f2 * f;
            w[2] = W.W3 * f3 + W.W2 * f2 + W.W1 * f + W.W0;
            // f=2-F; f2=f*f; f3=f2*f; w[3]=W.w3*f3 + W.w2*f2 + W.w1*f + W.w0;
            // w[3]=W.z3*f3 + W.z2*f2 + W.z1*f + W.z0;
        }
#endif
        w[3] = 1 - w[0] - w[1] - w[2]; // sum is always equal to 1

        setSamples(img_size);
    }
    void setSharp(Vec2 uv, Vec4 img_size, Half sharp) {
        CubicWeight W;
        W.setSharp(sharp);
        set(uv, img_size, W);
    } // 'sharp'=0..1, for best results this should use 'texSlow'

    void UVClamp(Vec2 min, Vec2 max) {
        UNROLL for (Int i = 0; i < 4; i++) tc[i] = Mid(tc[i], min, max);
        c = Mid(c, min, max);
        l = Mid(l, min, max);
        r = Mid(r, min, max);
        u = Mid(u, min, max);
        d = Mid(d, min, max);
    }

    VecH4 tex(Image img, Bool max0 = true) {
        return Max0(TexLod(img, u) * wu       // sample upper edge (2 texels), both weights are negative
                        + TexLod(img, l) * wl // sample left  edge (2 texels), both weights are negative
                        + TexLod(img, c) * wc // sample center     (4 texels), all  weights are positive
                        + TexLod(img, r) * wr // sample right edge (2 texels), both weights are negative
                        + TexLod(img, d) * wd,
                    max0); // sample lower edge (2 texels), both weights are negative
    }
    VecH texRGB(Image img, Bool max0 = true) // ignores alpha channel
    {
        return Max0(TexLod(img, u).rgb * wu       // sample upper edge (2 texels), both weights are negative
                        + TexLod(img, l).rgb * wl // sample left  edge (2 texels), both weights are negative
                        + TexLod(img, c).rgb * wc // sample center     (4 texels), all  weights are positive
                        + TexLod(img, r).rgb * wr // sample right edge (2 texels), both weights are negative
                        + TexLod(img, d).rgb * wd,
                    max0); // sample lower edge (2 texels), both weights are negative
    }
    Half texA(Image img, Bool max0 = true) // only alpha channel
    {
        return Max0(TexLod(img, u).a * wu       // sample upper edge (2 texels), both weights are negative
                        + TexLod(img, l).a * wl // sample left  edge (2 texels), both weights are negative
                        + TexLod(img, c).a * wc // sample center     (4 texels), all  weights are positive
                        + TexLod(img, r).a * wr // sample right edge (2 texels), both weights are negative
                        + TexLod(img, d).a * wd,
                    max0); // sample lower edge (2 texels), both weights are negative
    }
    Half texX(ImageH img, Bool max0 = true) // only X channel
    {
        return Max0(TexLod(img, u).x * wu       // sample upper edge (2 texels), both weights are negative
                        + TexLod(img, l).x * wl // sample left  edge (2 texels), both weights are negative
                        + TexLod(img, c).x * wc // sample center     (4 texels), all  weights are positive
                        + TexLod(img, r).x * wr // sample right edge (2 texels), both weights are negative
                        + TexLod(img, d).x * wd,
                    max0); // sample lower edge (2 texels), both weights are negative
    }
    VecH2 texXY(ImageH2 img, Bool max0 = true) // only XY channel
    {
        return Max0(TexLod(img, u).xy * wu       // sample upper edge (2 texels), both weights are negative
                        + TexLod(img, l).xy * wl // sample left  edge (2 texels), both weights are negative
                        + TexLod(img, c).xy * wc // sample center     (4 texels), all  weights are positive
                        + TexLod(img, r).xy * wr // sample right edge (2 texels), both weights are negative
                        + TexLod(img, d).xy * wd,
                    max0); // sample lower edge (2 texels), both weights are negative
    }
    VecH4 texSlow(Image img, Bool max0 = true) {
        VecH4 c = 0;
        UNROLL for (Int y = 0; y < 4; y++)
            UNROLL for (Int x = 0; x < 4; x++) c += TexPointOfs(img, tc[0], VecI2(x, y)) * weight(x, y);
        return Max0(c, max0);
    }
};
/******************************************************************************/
VecH4 TexCubicFastSharp(Image img, Vec2 uv, Half sharp, Bool max0 = true) {
    CubicFastSampler cs;
    cs.setSharp(uv, ImgSize, sharp);
    return cs.texSlow(img, max0);
}
VecH4 TexCubicFast(Image img, Vec2 uv, Bool max0 = true) {
    CubicFastSampler cs;
    cs.set(uv);
    return cs.tex(img, max0);
}
VecH TexCubicFastRGB(Image img, Vec2 uv, Bool max0 = true) {
    CubicFastSampler cs;
    cs.set(uv);
    return cs.texRGB(img, max0);
} // ignores alpha channel
/******************************************************************************/
