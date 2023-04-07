/******************************************************************************/
#include "!Header.h"
// MULTI_SAMPLE, FLAT, DITHER, GL_ES
/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(Atmosphere)
   Vec AtmosphereViewPos;
   Vec AtmosphereLightPos;
   Flt AtmospherePlanetRadius; // 0..Inf
 //Flt AtmosphereHeight; // 0..Inf
   Flt AtmosphereRadius; // AtmospherePlanetRadius+AtmosphereHeight
   Flt AtmosphereAltScaleRay, AtmosphereAltScaleMie; // 0..Inf
   Flt AtmosphereLightScale; // 0..Inf
   Flt AtmosphereFogReduce, // 0..1 = 1-MaxFog
       AtmosphereFogReduceDist; // 0..Inf
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************
Dbl ParticlePolarisabilityConstant()
{
   Dbl p_n =1.000293; // air refractive index
   Dbl p_ns=2.55e25; // particle density of air at sea level
   Dbl l_n2_minus_1=Sqr(p_n)-1;
   Dbl l_k=2*Sqr(PID*l_n2_minus_1)/(3*p_ns);
   return l_k;
}
VecD RayleighScatteringCoefficient(Dbl p_k, C VecD &p_wavelength)
{
   VecD l_wavelength4(Quart(p_wavelength.x), Quart(p_wavelength.y), Quart(p_wavelength.z));
   Dbl  l_numerator=4*PID*p_k;
   return l_numerator/l_wavelength4;
}
Dbl  kc=ParticlePolarisabilityConstant();
VecD wave=VecD(680, 550, 440)*1e-9;
VecD coef=RayleighScatteringCoefficient(kc, wave);
/******************************************************************************/
  static const Vec RayleighScattering=Vec(5.2091275314786692e-06, 1.2171661977460255e-05, 2.9715971624658822e-05); // Vec(5.802, 13.558, 33.1)*0.000001;
//static const Flt RayleighAbsorption=*0.000001;

  static const Flt MieScattering=3.996*0.000001;
//static const Flt MieAbsorption=4.4*0.000001; it's unclear if this is absorption or extinction (extinction=scattering+absorption)

//static const Vec OzoneAbsorption=Vec(0.650, 1.881, 0.085)*0.000001;
/******************************************************************************/
Flt RayleighPhase(Flt angle_cos)
{
   return 3/(16*PI)*(1+angle_cos*angle_cos);
}
Flt MiePhase(Flt angle_cos)
{
   const Flt g=0.8;

   Flt   num=3/(8*PI)*(1-g*g)*(1+angle_cos*angle_cos);
   Flt denom=(2+g*g)*Pow(1 + g*g - 2*g*angle_cos, 1.5);

   return num/denom;
}
/******************************************************************************/
void Scattering(Flt height,
            out Vec rayleigh_scattering,
            out Flt      mie_scattering,
            out Vec extinction)
{
   Flt altitude        =Max(0, height-AtmospherePlanetRadius);
   Flt rayleigh_density=Exp(altitude*AtmosphereAltScaleRay);
   Flt      mie_density=Exp(altitude*AtmosphereAltScaleMie);

       rayleigh_scattering=RayleighScattering*rayleigh_density;
 //Flt rayleigh_absorption=RayleighAbsorption*rayleigh_density;

       mie_scattering=MieScattering*mie_density;
 //Flt mie_absorption=MieAbsorption*mie_density;

 //extinction=rayleigh_scattering+(rayleigh_absorption+mie_scattering+mie_absorption);
   extinction=rayleigh_scattering+mie_scattering; // looked better when using just one mie_scattering or mie_absorption, since mie_scattering has to be calculated anyway, then just use that one

 //Vec ozone_absorption=OzoneAbsorption*Max(0, 1-Abs(altitude_km-25)/15); extinction+=ozone_absorption;
}
/******************************************************************************/
VecH4 RayMarchScattering(Vec pos,
                         Flt pixel_dist,
                         Vec ray,
                         Vec sun)
{
   Flt start, end;
   Flt fog_factor; // fake factor that removes fog and mie highlights on objects (this is faster than shadow testing per sample)
   {
      Flt b=Dot(pos, ray);
      Flt c=Length2(pos)-Sqr(AtmosphereRadius);
      Flt d=b*b-c;
      Flt atmos_start=-b-Sqrt(d),
          atmos_end  =-b+Sqrt(d),
          obstacle   =pixel_dist; // *AtmosphereScale
      start=Max(       0, atmos_start);
      end  =Min(obstacle, atmos_end  );
      if(d<=0 || end<=start)return VecH4(0, 0, 0, 0); // no atmosphere intersection
      Flt factor=1 ? (end-start)/(atmos_end-start) : end/atmos_end; // proportion of pixel_pos_cam_dist to atmos_end
      fog_factor=1-AtmosphereFogReduce*(1-factor)*Sat(1-end/AtmosphereFogReduceDist);
   }

   Flt      angle_cos=Dot(ray, sun);
   Flt rayleigh_phase=RayleighPhase(angle_cos);
   Flt      mie_phase=     MiePhase(angle_cos);

   const Int steps=16; // 16 is smallest value that looks as good as 256
   Vec lum=0, transmittance=1;
#if 0 // backwards, operate on alpha
   if(E)
   {
   Flt t1=end, mul=end-start;
   Vec alpha=0;
   for(Int i=steps-1; i>=0; i--)
   {
      Flt t0=Flt(i)/steps;
      if(1)t0*=t0; // focus on samples closer to camera, this increases precision
      t0=t0*mul+start;
      Flt t=Avg(t0, t1), dt=t1-t0; t1=t0;

      Vec sample_pos=pos+t*ray;
      Flt height=Length(sample_pos);

      Vec rayleigh_scattering, extinction;
      Flt      mie_scattering;
      Scattering(height, rayleigh_scattering, mie_scattering, extinction);

      Flt sun_zenith_angle_cos=Dot(sun, sample_pos)/height; // Dot(sun, sample_pos/height)
      Flt sun_transmittance   =Sqr(sun_zenith_angle_cos*0.5+0.5);
      Vec scattering          =(rayleigh_scattering*rayleigh_phase + mie_scattering*mie_phase)*sun_transmittance;

      Vec sample_transmittance=Exp(-dt*extinction);

      Vec l=scattering*dt;
      Vec a=1-sample_transmittance;

      lum  =MergeBlendColor(lum, alpha, l, a);
      alpha=     BlendAlpha(alpha, a);
   }
   lum*=fog_factor*AtmosphereLightScale;
   lum*=alpha; // premultiply
   return Vec4(lum, Max(alpha));
   }
#endif
   Flt t0=start, mul=end-start;
   for(Int i=1; i<=steps; i++)
   {
      Flt t1=Flt(i)/steps;
      if(1)t1*=t1; // focus on samples closer to camera, this increases precision
      t1=t1*mul+start;
      Flt t=Avg(t0, t1), dt=t1-t0; t0=t1;

      Vec sample_pos=pos+t*ray;
      Flt height=Length(sample_pos);

      Vec rayleigh_scattering, extinction;
      Flt      mie_scattering;
      Scattering(height, rayleigh_scattering, mie_scattering, extinction);

   #if 1
      Flt sun_zenith_angle_cos=Dot(sun, sample_pos)/height; // Dot(sun, sample_pos/height)
      Flt sun_transmittance   =Sqr(sun_zenith_angle_cos*0.5+0.5);
      Vec scattering          =(rayleigh_scattering*rayleigh_phase + mie_scattering*mie_phase)*sun_transmittance;
   #else
      Vec  up=sample_pos/height;
      Flt  sun_zenith_angle_cos=Dot(sun, up);
      Vec2 uv=Vec2(sun_zenith_angle_cos*0.5+0.5, (height-AtmospherePlanetRadius)/AtmosphereHeight);

      Vec sun_transmittance=TexLod(SkyA, uv).rgb; // Lod needed for clamp
      Vec psiMS            =TexLod(SkyB, uv).rgb; // Lod needed for clamp

      Vec rayleighInScattering=  rayleigh_scattering*(rayleigh_phase*sun_transmittance+psiMS);
      Vec      mieInScattering=       mie_scattering*(     mie_phase*sun_transmittance+psiMS);
      Vec           scattering=rayleighInScattering+mieInScattering;
   #endif

      Vec sample_transmittance=Exp(-dt*extinction);
   #if 1 // faster but may have issues in certain cases with low 'steps', have to test if results are satisfactory
      lum+=transmittance*scattering*dt;
   #else
      Vec scattering_integral=(scattering-scattering*sample_transmittance)/extinction;
      lum+=transmittance*scattering_integral;
   #endif
      transmittance*=sample_transmittance;
   }
   lum*=fog_factor*AtmosphereLightScale;
   Flt alpha=1-Min(transmittance);
   return VecH4(lum, alpha);
 //return VecH4(alpha.rrr, 1); // show alpha
}
/******************************************************************************/
void VS(VtxInput vtx,
#if FLAT || !GL_ES // GL_ES doesn't support NOPERSP
   NOPERSP out Vec2 posXY:POS_XY,
#endif

#if FLAT
   NOPERSP out Vec4 pixel:POSITION
#else
           out Vec4 pixel:POSITION
#endif
)
{
#if FLAT
   posXY=UVToPosXY(vtx.uv());
   pixel=Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
#else // GEOM
   pixel=Project(TransformPos(vtx.pos()));

#if !GL_ES
   Vec2 uv   =ProjectedPosToUV   (pixel);
        posXY=          UVToPosXY(uv);
#endif
#endif
}
/******************************************************************************/
VecH4 PS
(
#if FLAT || !GL_ES // GL_ES doesn't support NOPERSP
   NOPERSP Vec2 posXY:POS_XY,
#endif

#if FLAT
   NOPERSP PIXEL
#else
           PIXEL
#endif

#if MULTI_SAMPLE==2
 , UInt index:SV_SampleIndex
#endif
):TARGET
{
   VecI2 pix=pixel.xy;
#if !FLAT && GL_ES // GL_ES doesn't support NOPERSP
   Vec2 uv   =PixelToUV   (pixel);
   Vec2 posXY=   UVToPosXY(uv);
#endif

   VecH4 col;
   const Int samples=((MULTI_SAMPLE==1) ? MS_SAMPLES : 1);
   UNROLL for(Int i=0; i<samples; i++)
   {
      // distance
   #if   MULTI_SAMPLE==0
      Vec pos=GetPosPix(pix, posXY);
   #elif MULTI_SAMPLE==1
      Vec pos=GetPosMS(pix, i, posXY);
   #else
      Vec pos=GetPosMS(pix, index, posXY);
   #endif

      Flt   len=Length(pos);
      Vec   dir=Transform3(pos/len, CamMatrix); // convert to ball space
      VecH4 c=RayMarchScattering(AtmosphereViewPos, Min(len, Viewport.range), dir, AtmosphereLightPos);

      if(MULTI_SAMPLE!=1)col =c              ;else
      if(i==0           )col =c/(Half)samples;else
                         col+=c/(Half)samples;
   }

   if(DITHER && MULTI_SAMPLE!=2)ApplyDither(col.rgb, pixel.xy); // skip dither for MS because it won't be noticeable

   return col;
}
/******************************************************************************/
