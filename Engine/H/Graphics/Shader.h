/******************************************************************************/
#if EE_PRIVATE
struct ShaderSampler
{
#if DX11
   ID3D11SamplerState *state=null;

   Bool is()C {return state!=null;}

   Bool createTry(D3D11_SAMPLER_DESC &desc); // false on fail
   void create   (D3D11_SAMPLER_DESC &desc); // Exit  on fail
   void setVS    (Int index)C;
   void setHS    (Int index)C;
   void setDS    (Int index)C;
   void setPS    (Int index)C;
   void setCS    (Int index)C;
#elif GL
   UInt sampler=0;
#if GL_ES
   UInt sampler_no_filter=0; // GLES3 doesn't support filtering F32/Depth textures, so use a separate sampler with filtering disabled - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml   "depth textures are not filterable" - https://arm-software.github.io/opengl-es-sdk-for-android/occlusion_culling.html
#endif
   UInt filter_min, filter_mag;
   UInt address[3];

   Bool is()C {return sampler!=0;}

          void create();
   inline void setPS(Int index)C {set(index);}
#endif
   void             set(Int index)C;
   void             del();
  ~ShaderSampler() {del();}
};
#endif
struct ShaderImage // Shader Image
{
 C Image* get(                 )C {return   _image  ;}
   void   set(C Image    *image)  {T._image= image  ;}
   void   set(C Image    &image)  {T._image=&image  ;}
 //void   set(C ImagePtr &image)  {T._image= image();} this is not safe, as 'ShaderImage' does not store 'ImagePtr' for performance reasons

#if !EE_PRIVATE
private:
#endif
 C Image *_image=null;
#if EE_PRIVATE
   #if DX11
      INLINE ID3D11ShaderResourceView* getSRV()C {return _image ? _image->_srv  : null;}
   #endif
#endif
};
struct ShaderRWImage // Shader Read Write Image
{
 C ImageRT* get(                )C {return   _image;}
   void     set(C ImageRT *image)  {T._image= image;}
   void     set(C ImageRT &image)  {T._image=&image;}

#if !EE_PRIVATE
private:
#endif
 C ImageRT *_image=null;
#if EE_PRIVATE
   #if DX11
      INLINE ID3D11UnorderedAccessView* getUAV()C {return _image ? _image->_uav  : null;}
   #endif
#endif
};
/******************************************************************************/
struct ShaderParam // Shader Parameter
{
            void set(  Bool     b               ); // set boolean  value
            void set(  Int      i               ); // set integer  value
            void set(  Flt      f               ); // set float    value
            void set(  Dbl      d               ); // set double   value
            void set(C Vec2    &v               ); // set vector2D value
            void set(C VecD2   &v               ); // set vector2D value
            void set(C VecI2   &v               ); // set vector2D value
            void set(C Vec     &v               ); // set vector3D value
            void set(C VecD    &v               ); // set vector3D value
            void set(C VecI    &v               ); // set vector3D value
            void set(C Vec4    &v               ); // set vector4D value
            void set(C VecD4   &v               ); // set vector4D value
            void set(C VecI4   &v               ); // set vector4D value
            void set(C Color   &color           ); // set vector4D value
            void set(C Rect    &rect            ); // set vector4D value
            void set(C Matrix3 &matrix          ); // set matrix3  value
            void set(C Matrix  &matrix          ); // set matrix   value
            void set(C MatrixM &matrix          ); // set matrix   value
            void set(C Matrix4 &matrix          ); // set matrix4  value
            void set(C Vec     *v     , Int elms); // set vector3D array
            void set(C Vec4    *v     , Int elms); // set vector4D array
            void set(C Matrix  *matrix, Int elms); // set matrix   array
            void set(  CPtr     data  , Int size); // set memory
   T1(TYPE) void set(C TYPE    &data            ) {set((CPtr)&data, SIZE(data));}
         #if EE_PRIVATE
            void set    (C Vec       &v              , UInt elm ); // set vector3D   array element value
            void set    (C Vec       &a, C Vec &b               ); // set vector3Dx2               value
            void set    (C Vec       &a, C Vec &b    , UInt elm ); // set vector3Dx2 array element value
            void set    (C Vec4      &v              , UInt elm ); // set vector4D   array element value
            void set    (C Matrix    &matrix         , UInt elm ); // set matrix     array element value
            void fromMul(C Matrix    &a, C Matrix  &b           ); // set matrix                   value from "a*b"
            void fromMul(C Matrix    &a, C MatrixM &b           ); // set matrix                   value from "a*b"
            void fromMul(C MatrixM   &a, C MatrixM &b           ); // set matrix                   value from "a*b"
            void fromMul(C Matrix    &a, C Matrix  &b, UInt elm ); // set matrix     array element value from "a*b"
            void fromMul(C Matrix    &a, C MatrixM &b, UInt elm ); // set matrix     array element value from "a*b"
            void fromMul(C MatrixM   &a, C MatrixM &b, UInt elm ); // set matrix     array element value from "a*b"
            void set    (C GpuMatrix &matrix                    ); // set matrix                   value
            void set    (C GpuMatrix &matrix         , UInt elm ); // set matrix     array element value
            void set    (C GpuMatrix *matrix         ,  Int elms); // set matrix     array

            void setConditional(C Flt   &f                    ); // set float                    value only if it's different
            void setConditional(C Vec2  &v                    ); // set vector2D                 value only if it's different
            void setConditional(C Vec   &v                    ); // set vector3D                 value only if it's different
            void setConditional(C Vec4  &v                    ); // set vector4D                 value only if it's different
            void setConditional(C Color &color                ); // set vector4D                 value only if it's different
            void setConditional(C Vec   &v,           UInt elm); // set vector3D   array element value only if it's different
            void setConditional(C Vec   &a, C Vec &b          ); // set vector3Dx2               value only if it's different
            void setConditional(C Vec   &a, C Vec &b, UInt elm); // set vector3Dx2 array element value only if it's different
            void setConditional(C Rect  &r                    ); // set vector4D                 value only if it's different

            void setInRangeConditional(C Vec &a, C Vec &b, UInt elm); // set vector3Dx2 array element value only if it's different, values assumed to be always in range

            void setSafe(C Vec4 &v); // set from vector4D value, but limit the actual size copied based on 'ShaderParam' size
         #endif

#if EE_PRIVATE
   #define MIN_SHADER_PARAM_DATA_SIZE SIZEI(Vec4)

   ASSERT(MIN_SHADER_PARAM_DATA_SIZE>=SIZEI(Vec4));
 C Flt & getFlt ()C {return *(Flt *)_data;}
 C Vec2& getVec2()C {return *(Vec2*)_data;}
 C Vec & getVec ()C {return *(Vec *)_data;}
 C Vec4& getVec4()C {return *(Vec4*)_data;}

   struct Translation
   {
      Int cpu_offset, gpu_offset, elm_size; // 'gpu_offset'=during shader creation and saving it's set relative to start of cbuffer, but while loading it's adjusted to be relative to start of param. This is done because 'ShaderParam.data' is adjusted to point directly to param (so 'set' methods can work correctly), since 'data' is adjusted then we have to adjust 'gpu_offset' too.

      Bool operator!=(C Translation &trans)C {return T.cpu_offset!=trans.cpu_offset || T.gpu_offset!=trans.gpu_offset || T.elm_size!=trans.elm_size;}

      void set(Int cpu_offset, Int gpu_offset, Int elm_size) {T.cpu_offset=cpu_offset; T.gpu_offset=gpu_offset; T.elm_size=elm_size;}
   };

   static void OptimizeTranslation(C CMemPtr<Translation> &src, Mems<Translation> &dest);

   Byte *_data;
   Bool *_changed;
   UInt  _cpu_data_size, _gpu_data_size, _elements;
   Mems<Translation> _full_translation, _optimized_translation;

   Bool is()C {return _cpu_data_size>0;}
   Int  gpuArrayStride()C;

   INLINE void setChanged() {*_changed=true;}
constexpr Bool canFit   (UInt size)C {return MIN_SHADER_PARAM_DATA_SIZE>=size || _gpu_data_size>=size;} // use this if 'size' is known at compile-time
   INLINE Bool canFitVar(UInt size)C {return                                     _gpu_data_size>=size;} // use this if 'size' is known at     run-time
          void optimize  () {OptimizeTranslation(_full_translation, _optimized_translation);}
          void initAsElement(ShaderParam &parent, Int index);
          void zero();

   INLINE GpuMatrix  * asGpuMatrix  () {return (GpuMatrix  *)_data;}
   INLINE GpuVelocity* asGpuVelocity() {return (GpuVelocity*)_data;}

  ~ShaderParam() {zero();}
   ShaderParam() {zero();}

   NO_COPY_CONSTRUCTOR(ShaderParam);
#endif
};
struct ShaderParamBool : private ShaderParam // Shader Parameter
{
   void set(Bool b); // set boolean value
#if EE_PRIVATE
   void setConditional(Bool b); // set boolean value only if it's different

   ASSERT(MIN_SHADER_PARAM_DATA_SIZE>=SIZE(Bool));
 C Bool& getBool()C {return *(Bool*)_data;}
#endif
};
struct ShaderParamInt : private ShaderParam // Shader Parameter
{
   void set(  Int    i); // set integer  value
   void set(C VecI2 &v); // set vector2D value
   void set(C VecI  &v); // set vector3D value
   void set(C VecI4 &v); // set vector4D value
   void set(C RectI &v); // set vector4D value
#if EE_PRIVATE
   void setConditional(  Int    i); // set integer  value only if it's different
   void setConditional(C VecI2 &v); // set vector2D value only if it's different
   void setConditional(C VecI  &v); // set vector3D value only if it's different
   void setConditional(C VecI4 &v); // set vector4D value only if it's different
   void setConditional(C RectI &v); // set vector4D value only if it's different

   ASSERT(MIN_SHADER_PARAM_DATA_SIZE>=SIZE(VecI4));
 C Int  & getInt  ()C {return *(Int  *)_data;}
 C VecI2& getVecI2()C {return *(VecI2*)_data;}
 C VecI & getVecI ()C {return *(VecI *)_data;}
 C VecI4& getVecI4()C {return *(VecI4*)_data;}
 C RectI& getRectI()C {return *(RectI*)_data;}
#endif
};
/******************************************************************************/
struct ShaderParamChange // Shader Parameter Change
{
   ShaderParam *param  ; // parameter to change
   Vec4         value  ; // value     to change to
   Bool         restore; // if restore previous value when finished drawing

   ShaderParamChange& set(  Bool   b) {value.x  =b; return T;}
   ShaderParamChange& set(  Int    i) {value.x  =i; return T;}
   ShaderParamChange& set(  Flt    f) {value.x  =f; return T;}
   ShaderParamChange& set(C Vec2  &v) {value.xy =v; return T;}
   ShaderParamChange& set(C Vec   &v) {value.xyz=v; return T;}
   ShaderParamChange& set(C Vec4  &v) {value    =v; return T;}
   ShaderParamChange& set(C Color &v);

   ShaderParamChange& set(ShaderParam *param) {T.param=param; return T;}

   ShaderParamChange() {param=null; value.zero(); restore=true;}
};
/******************************************************************************/
#if EE_PRIVATE
#define GL_MULTIPLE_UBOS (MAC && GL) // FIXME because Mac has problems with UBO updates - https://feedbackassistant.apple.com/feedback/7117741
struct ShaderBuffer // Constant Buffer
{
   struct Buffer
   {
      GPU_API(ID3D11Buffer*, UInt) buffer; // keep this as first member because it's used most often
      Int                          size  ;

      void del   ();
      void create(Int size);

      Bool is()C {return size>0;}
      void zero() {buffer=GPU_API(null, 0); size=0;}

     ~Buffer() {del ();}
      Buffer() {zero();}
      // intentionally keep copy constructor as raw member copy, because we expect this behavior
   };

   Buffer buffer; // keep this as first member because it's used most often
   Byte  *data;
   Bool   changed;
   SByte  explicit_bind_slot; // -1=any available
   Int    full_size; // remember full size, because 'buffer.size' can be dynamically adjusted

#if DX11
   Mems<Buffer> parts;
   void      setPart (Int part );
   void   createParts(C Int *elms, Int elms_num);
#endif
#if GL_MULTIPLE_UBOS
   Mems<Buffer> parts;
   Int          part;
#endif

   Bool is       ()C {return full_size>0;} // check 'full_size' instead of 'buffer' which can be empty for APP_ALLOW_NO_GPU/APP_ALLOW_NO_XDISPLAY
   void del      ();
   void create   (Int size );
   void update   (         );
   void bind     (Int index);
   void bindCheck(Int index);
   void zero     ();

  ~ShaderBuffer() {del ();}
   ShaderBuffer() {zero();}

   NO_COPY_CONSTRUCTOR(ShaderBuffer);
};
#endif
/******************************************************************************/
struct ShaderData : Mems<Byte>
{
#if EE_PRIVATE
   void clean() {super::del();}

   Bool operator==(C ShaderData &sd)C {return elms()==sd.elms() && EqualMem(data(), sd.data(), elms());}

   Bool save(File &f)C {return super::saveRaw(f);}
   Bool load(File &f)  {return super::loadRaw(f);}
#endif
};
#if EE_PRIVATE
/******************************************************************************/
enum SHADER_TYPE : Byte
{
   ST_VS,
   ST_HS,
   ST_DS,
   ST_PS,
   ST_CS,
   ST_NUM,
   ST_BASE=ST_CS, // first 4 shaders
};
#if WINDOWS
struct ShaderVS11 : ShaderData
{
   ID3D11VertexShader *vs=null;

   ID3D11VertexShader* create();

  ~ShaderVS11();
   ShaderVS11() {}
   NO_COPY_CONSTRUCTOR(ShaderVS11);
};
struct ShaderHS11 : ShaderData
{
   ID3D11HullShader *hs=null;

   ID3D11HullShader* create();

  ~ShaderHS11();
   ShaderHS11() {}
   NO_COPY_CONSTRUCTOR(ShaderHS11);
};
struct ShaderDS11 : ShaderData
{
   ID3D11DomainShader *ds=null;

   ID3D11DomainShader* create();

  ~ShaderDS11();
   ShaderDS11() {}
   NO_COPY_CONSTRUCTOR(ShaderDS11);
};
struct ShaderPS11 : ShaderData
{
   ID3D11PixelShader *ps=null;

   ID3D11PixelShader* create();

  ~ShaderPS11();
   ShaderPS11() {}
   NO_COPY_CONSTRUCTOR(ShaderPS11);
};
struct ShaderCS11 : ShaderData
{
   ID3D11ComputeShader *cs=null;

   ID3D11ComputeShader* create();

  ~ShaderCS11();
   ShaderCS11() {}
   NO_COPY_CONSTRUCTOR(ShaderCS11);
};
#endif
/******************************************************************************/
struct ShaderSubGL : ShaderData
{
   UInt shader=0;

   UInt create(UInt gl_type, Str *messages);
   Str  source();

  ~ShaderSubGL();
   ShaderSubGL() {}
   NO_COPY_CONSTRUCTOR(ShaderSubGL);
};
/******************************************************************************/
struct BufferLink
{
   Int                          index;
   GPU_API(ShaderBuffer*, UInt) buffer;

   void set(Int index, GPU_API(ShaderBuffer*, UInt) buffer) {T.index=index; T.buffer=buffer;}
   Bool load(File &f, C MemtN<ShaderBuffer*, 256> &buffers);
};
struct ImageLink
{
   Int          index;
   ShaderImage *image;

   void set(Int index, ShaderImage &image) {T.index=index; T.image=&image;}
   Bool load(File &f, C MemtN<ShaderImage*, 256> &images);
};
struct RWImageLink
{
   Int            index;
   ShaderRWImage *image;

   void set(Int index, ShaderRWImage &image) {T.index=index; T.image=&image;}
   Bool load(File &f, C MemtN<ShaderRWImage*, 256> &images);
};

struct BufferLinkPtr
{
 C BufferLink *data=null;
   Int         elms=0;

 C BufferLink& operator[](Int i)C {DEBUG_RANGE_ASSERT(i, elms); return data[i];}
   void operator=(C Mems<BufferLink> &links) {data=links.data(); elms=links.elms();}
};
struct ImageLinkPtr
{
 C ImageLink *data=null;
   Int        elms=0;

 C ImageLink& operator[](Int i)C {DEBUG_RANGE_ASSERT(i, elms); return data[i];}
   void operator=(C Mems<ImageLink> &links) {data=links.data(); elms=links.elms();}
};
struct RWImageLinkPtr
{
 C RWImageLink *data=null;
   Int          elms=0;

 C RWImageLink& operator[](Int i)C {DEBUG_RANGE_ASSERT(i, elms); return data[i];}
   void operator=(C Mems<RWImageLink> &links) {data=links.data(); elms=links.elms();}
};

inline Int Elms(C  BufferLinkPtr &links) {return links.elms;}
inline Int Elms(C   ImageLinkPtr &links) {return links.elms;}
inline Int Elms(C RWImageLinkPtr &links) {return links.elms;}

#if WINDOWS
struct Shader11
{
   ID3D11VertexShader *vs=null;
   ID3D11HullShader   *hs=null;
   ID3D11DomainShader *ds=null;
   ID3D11PixelShader  *ps=null;
   Mems<ShaderBuffer*> all_buffers; // shader buffers used by all shader stages (VS HS DS PS) combined into one array
   BufferLinkPtr           buffers[ST_BASE];
    ImageLinkPtr            images[ST_BASE];
   Int                  data_index[ST_BASE]={-1, -1, -1, -1}; ASSERT(ST_BASE==4);
   Str8                       name;

   Bool validate (ShaderFile &shader, Str *messages=null);
   Bool load     (File &f, C ShaderFile &shader_file, C MemtN<ShaderBuffer*, 256> &buffers);

   void commit   ()C;
   void commitTex()C;
   void start    ()C;
   void startTex ()C;
   void begin    ()C;

   void  setVSBuffers()C;
   void  setHSBuffers()C;
   void  setDSBuffers()C;
   void  setPSBuffers()C;
   void updateBuffers()C;

   void setVSImages()C;
   void setHSImages()C;
   void setDSImages()C;
   void setPSImages()C;

//~Shader11(); no need to release 'vs,hs,ds,ps' or 'buffers,images' since they're just copies from 'Shader*11'
};
struct ComputeShader11
{
   ID3D11ComputeShader *cs=null;
   Mems<ShaderBuffer*> all_buffers; // shader buffers used by all shader stages (CS) combined into one array
   BufferLinkPtr           buffers;
    ImageLinkPtr            images;
  RWImageLinkPtr         rw_images;
   Int                  data_index=-1;
   Str8                       name;

   Bool validate(ShaderFile &shader, Str *messages=null);
   Bool load    (File &f, C ShaderFile &shader_file, C MemtN<ShaderBuffer*, 256> &buffers);

   void    setBuffers()C;
   void updateBuffers()C;
   void    setImages ()C;
   void  clearImages ()C;

   void begin()C;
   void end  ()C;

   void compute(C VecI2 &groups)C;
   void compute(C VecI  &groups)C;

//~ComputeShader11(); no need to release 'vs,hs,ds,ps' or 'buffers,images,rw_images' since they're just copies from 'Shader*11'
};
#endif
/******************************************************************************/
struct SamplerImageLink
{
   Int          index, sampler;
   ShaderImage *image;

   void set(Int index, Int sampler, ShaderImage &image) {T.index=index; T.sampler=sampler; T.image=&image;}
};
struct ShaderGL
{
   UInt                        prog=0, vs=0, ps=0;
   Int                     vs_index=-1, ps_index=-1;
   Mems<ShaderBuffer*   > all_buffers; // shader buffers used by all shader stages (VS HS DS PS) combined into one array
   Mems<      BufferLink>     buffers;
   Mems<SamplerImageLink>      images;
   Str8                          name;

   Str  source  ()C;
   UInt compile (MemPtr<ShaderSubGL> vs_array, MemPtr<ShaderSubGL> ps_array, ShaderFile *shader, Str *messages);
   Bool validate(ShaderFile &shader, Str *messages=null);
   Bool load    (File &f, C ShaderFile &shader_file, C MemtN<ShaderBuffer*, 256> &buffers);

   void   bindImages ()C;
   void   bindBuffers()C;
   void updateBuffers()C;

   void commit   ()C;
   void commitTex()C;
   void start    ()C;
   void startTex ()C;
   void begin    ()C;

  ~ShaderGL();
};
struct ComputeShaderGL
{
   UInt                        prog=0, cs=0;
   Int                     cs_index=-1;
   Mems<ShaderBuffer*   > all_buffers; // shader buffers used by all shader stages (CS) combined into one array
   Mems<      BufferLink>     buffers;
   Mems<SamplerImageLink>      images;
   Mems<     RWImageLink>   rw_images;
   Str8                          name;

   Str  source  ()C;
   UInt compile (MemPtr<ShaderSubGL> cs_array, ShaderFile *shader, Str *messages);
   Bool validate(ShaderFile &shader, Str *messages=null);
   Bool load    (File &f, C ShaderFile &shader_file, C MemtN<ShaderBuffer*, 256> &buffers);

   void   bindImages ()C;
   void   bindBuffers()C;
   void updateBuffers()C;

   void begin()C;
   void end  ()C;

   void compute(C VecI2 &groups)C;
   void compute(C VecI  &groups)C;

  ~ComputeShaderGL();
};
/******************************************************************************/
struct ShaderBase
{
   Int shader_draw; // index of this shader in 'ShaderDraws' container

   INLINE Shader& asShader       () {return   (Shader& )T;}
   INLINE Shader& asForwardShader() {return **(Shader**)(((Byte*)this)+Renderer._frst_light_offset);}
   INLINE Shader& asBlendShader  () {return **(Shader**)(((Byte*)this)+Renderer._blst_light_offset);}

   INLINE Shader& getShader(Bool forward)
   {
      if(forward)return asForwardShader();
      else       return asShader       ();
   }
   INLINE Shader& getBlendShader(Bool blst)
   {
      if(blst)return asBlendShader();
      else    return asShader     ();
   }

   void unlink() {shader_draw=-1;}

   ShaderBase() {unlink();}
};
struct Shader : ShaderBase, GPU_API(Shader11, ShaderGL)
{
#else
struct Shader
{
#endif
   void draw(                C Rect *rect=null        )C;                             // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Img' shader image, 'rect'=screen rectangle for the effect (set null for full viewport)
   void draw(C Image *image, C Rect *rect=null        )C;                             // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Img' shader image, 'rect'=screen rectangle for the effect (set null for full viewport)
   void draw(C Image &image, C Rect *rect=null        )C {draw(&image  , rect     );} // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Img' shader image, 'rect'=screen rectangle for the effect (set null for full viewport)
   void draw(                C Rect *rect, C Rect &tex)C;                             // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Img' shader image, 'rect'=screen rectangle for the effect (set null for full viewport), 'tex'=source image texture coordinates
   void draw(C Image *image, C Rect *rect, C Rect &tex)C;                             // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Img' shader image, 'rect'=screen rectangle for the effect (set null for full viewport), 'tex'=source image texture coordinates
   void draw(C Image &image, C Rect *rect, C Rect &tex)C {draw(&image  , rect, tex);} // apply custom 2D effect on the screen, 'image'=image to automatically set as 'Img' shader image, 'rect'=screen rectangle for the effect (set null for full viewport), 'tex'=source image texture coordinates*/
};
#if EE_PRIVATE
struct ComputeShader : GPU_API(ComputeShader11, ComputeShaderGL)
{
#else
struct ComputeShader
{
#endif
};
/******************************************************************************/
struct ShaderFile // Shader File
{
   // get
          Int            shaders(            )C {return         _shaders.elms();} // get number of         shaders in this file
          Int     computeShaders(            )C {return _compute_shaders.elms();} // get number of compute shaders in this file
          Shader*        shader (Int i       ); // get i-th         shader, null on fail
   ComputeShader* computeShader (Int i       ); // get i-th compute shader, null on fail
          Shader*        find   (C Str8 &name); // find             shader, null on fail
   ComputeShader* computeFind   (C Str8 &name); // find     compute shader, null on fail
          Shader*        get    (C Str8 &name); // get              shader, Exit on fail
   ComputeShader* computeGet    (C Str8 &name); // get      compute shader, Exit on fail
#if EE_PRIVATE
          Shader*        find   (C Str8 &name, Str *messages); // find         shader, put error messages into 'messages', null on fail
   ComputeShader* computeFind   (C Str8 &name, Str *messages); // find compute shader, put error messages into 'messages', null on fail
#endif

   // manage
   void del();

   // io
   Bool load(C Str &name); // load, false on fail

   ShaderFile();
  ~ShaderFile() {del();}

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   #if DX11
      Mems<ShaderVS11> _vs;
      Mems<ShaderHS11> _hs;
      Mems<ShaderDS11> _ds;
      Mems<ShaderPS11> _ps;
      Mems<ShaderCS11> _cs;
   #elif GL
      Mems<ShaderSubGL> _vs, _hs, _ds, _ps, _cs;
   #endif
   Mems<Mems< BufferLink>>   _buffer_links;
   Mems<Mems<  ImageLink>>    _image_links;
   Mems<Mems<RWImageLink>> _rw_image_links;
#else
   Mems<ShaderData   > _vs, _hs, _ds, _ps, _cs, _buffer_links, _image_links, _rw_image_links;
#endif
   Mems<       Shader>         _shaders;
   Mems<ComputeShader> _compute_shaders;
   NO_COPY_CONSTRUCTOR(ShaderFile);
};
/******************************************************************************/
#if EE_PRIVATE
struct FRSTKey // Forward Rendering Shader Techniques Key
{
   Byte skin, materials, layout, bump_mode, alpha_test, reflect, emissive_map, detail, color, mtrl_blend, uv_scale, heightmap, fx, per_pixel, tesselate;

   FRSTKey() {Zero(T);}
};
struct FRST : ShaderBase // Forward Rendering Shader Techniques
{
   Bool all_passes;
   Shader
      *none  ,                 // no          light
      *dir   , *   dir_shd[6], // directional light, [MapNum]
      *point , * point_shd   , // point       light
      *linear, *linear_shd   , // square      light
      *cone  , *  cone_shd   ; // cone        light
};
/******************************************************************************/
struct BLSTKey // Blend Light Shader Techniques
{
   Byte skin, color, layout, bump_mode, alpha_test, alpha, reflect, emissive_map, uv_scale, fx, per_pixel;

   BLSTKey() {Zero(T);}
};
struct BLST // Blend Light Shader Techniques
{
   Shader *dir[7];
};
/******************************************************************************/
extern ThreadSafeMap<FRSTKey, FRST         > Frsts         ; // Forward Rendering Shader Techniques
extern ThreadSafeMap<BLSTKey, BLST         > Blsts         ; // Blend   Light     Shader Techniques
extern ThreadSafeMap<Str8   , ShaderImage  > ShaderImages  ; // Shader            Images
extern ThreadSafeMap<Str8   , ShaderRWImage> ShaderRWImages; // Shader Read Write Images
extern ThreadSafeMap<Str8   , ShaderParam  > ShaderParams  ; // Shader Parameters
extern ThreadSafeMap<Str8   , ShaderBuffer > ShaderBuffers ; // Shader Constant Buffers
#endif
extern Cache<ShaderFile> ShaderFiles; // Shader File Cache
/******************************************************************************/
struct ShaderMacro // macro used for shader compilation
{
   Str8 name      ,
        definition;

   void set(C Str8 &name, C Str8 &definition) {T.name=name; T.definition=definition;}
};
#if EE_PRIVATE
struct ShaderGLSL
{
   Str8              group_name, tech_name;
   Memc<ShaderMacro> params;

   ShaderGLSL& set(C Str8 &group_name, C Str8 &tech_name ) {T.group_name=group_name; T.tech_name=tech_name; return T;}
   ShaderGLSL& par(C Str8 &name      , C Str8 &definition) {params.New().set(name, definition);             return T;}
};
#endif
/******************************************************************************/
// shader image
ShaderImage* FindShaderImage(CChar8 *name); // find shader image, null on fail (shader image can be returned only after loading a shader which contains the image)
ShaderImage*  GetShaderImage(CChar8 *name); // find shader image, Exit on fail (shader image can be returned only after loading a shader which contains the image)

// shader read write image
ShaderRWImage* FindShaderRWImage(CChar8 *name); // find shader read write image, null on fail (shader image can be returned only after loading a shader which contains the image)
ShaderRWImage*  GetShaderRWImage(CChar8 *name); // find shader read write image, Exit on fail (shader image can be returned only after loading a shader which contains the image)

// shader parameter
ShaderParam* FindShaderParam(CChar8 *name); // find shader parameter, null on fail (shader parameter can be returned only after loading a shader which contains the parameter)
ShaderParam*  GetShaderParam(CChar8 *name); // find shader parameter, Exit on fail (shader parameter can be returned only after loading a shader which contains the parameter)

inline ShaderParamInt * FindShaderParamInt (CChar8 *name) {return (ShaderParamInt *)FindShaderParam(name);}
inline ShaderParamBool*  GetShaderParamBool(CChar8 *name) {return (ShaderParamBool*) GetShaderParam(name);}
inline ShaderParamInt *  GetShaderParamInt (CChar8 *name) {return (ShaderParamInt *) GetShaderParam(name);}

         inline void SPSet(CChar8 *name,   Bool     b               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(b           );} // set boolean  value
         inline void SPSet(CChar8 *name,   Int      i               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(i           );} // set integer  value
         inline void SPSet(CChar8 *name,   Flt      f               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(f           );} // set float    value
         inline void SPSet(CChar8 *name,   Dbl      d               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(d           );} // set double   value
         inline void SPSet(CChar8 *name, C Vec2    &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector2D value
         inline void SPSet(CChar8 *name, C VecD2   &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector2D value
         inline void SPSet(CChar8 *name, C VecI2   &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector2D value
         inline void SPSet(CChar8 *name, C Vec     &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector3D value
         inline void SPSet(CChar8 *name, C VecD    &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector3D value
         inline void SPSet(CChar8 *name, C VecI    &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector3D value
         inline void SPSet(CChar8 *name, C Vec4    &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector4D value
         inline void SPSet(CChar8 *name, C VecD4   &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector4D value
         inline void SPSet(CChar8 *name, C VecI4   &v               ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v           );} // set vector4D value
         inline void SPSet(CChar8 *name, C Color   &color           ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(color       );} // set vector4D value
         inline void SPSet(CChar8 *name, C Rect    &rect            ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(rect        );} // set vector4D value
         inline void SPSet(CChar8 *name, C Matrix3 &matrix          ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(matrix      );} // set matrix   value
         inline void SPSet(CChar8 *name, C Matrix  &matrix          ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(matrix      );} // set matrix   value
         inline void SPSet(CChar8 *name, C Matrix4 &matrix          ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(matrix      );} // set matrix4  value
         inline void SPSet(CChar8 *name, C Vec     *v     , Int elms) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v     , elms);} // set vector3D array
         inline void SPSet(CChar8 *name, C Vec4    *v     , Int elms) {if(ShaderParam *sp=FindShaderParam(name))sp->set(v     , elms);} // set vector4D array
         inline void SPSet(CChar8 *name, C Matrix  *matrix, Int elms) {if(ShaderParam *sp=FindShaderParam(name))sp->set(matrix, elms);} // set matrix   array
         inline void SPSet(CChar8 *name,   CPtr     data  , Int size) {if(ShaderParam *sp=FindShaderParam(name))sp->set(data  , size);} // set memory
T1(TYPE) inline void SPSet(CChar8 *name, C TYPE    &data            ) {if(ShaderParam *sp=FindShaderParam(name))sp->set(data        );} // set memory

         inline void SPISet(CChar8 *name,   Int    i) {if(ShaderParamInt *sp=FindShaderParamInt(name))sp->set(i);} // set integer  value
         inline void SPISet(CChar8 *name, C VecI2 &v) {if(ShaderParamInt *sp=FindShaderParamInt(name))sp->set(v);} // set vector2D value
         inline void SPISet(CChar8 *name, C VecI  &v) {if(ShaderParamInt *sp=FindShaderParamInt(name))sp->set(v);} // set vector3D value
         inline void SPISet(CChar8 *name, C VecI4 &v) {if(ShaderParamInt *sp=FindShaderParamInt(name))sp->set(v);} // set vector4D value

#if EE_PRIVATE
ShaderBuffer* FindShaderBuffer(CChar8 *name);
ShaderBuffer*  GetShaderBuffer(CChar8 *name);

#if GL_ES
UInt GLNoFilter(UInt filter);
#endif

Bool VerifyPrecompiledShaderCache(C Str &name);
#endif
/******************************************************************************/
