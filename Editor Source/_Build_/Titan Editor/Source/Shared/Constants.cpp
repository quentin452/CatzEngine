/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
// !! client/server version (client will fail if tries to connect to server compiled with different version), increase this by one if any of engine resource formats have changed or if the network protocol has changed or if editor classes formats have changed !!
const Str ClientServerString = ENGINE_NAME " Editor";
/******************************************************************************/
const cchar8 *WorldVerSuffix = "\\Data",
             *ProjectsBuildPath = "_Build_\\",         // !! this will     get deleted in 'CleanAll' !!
                 *ProjectsPublishPath = "_Publish_\\", // !! this will     get deleted in 'CleanAll' !!
                     *ProjectsCodePath = "_Code_\\",   // !! this will NOT get deleted in 'CleanAll' !!
                         *ProjectPackageExt = "EsenthelProject",
             *CodeExt = ".cpp", // cpp was selected, because unlike other ideas: "code", "txt" only "cpp" gets correct coloring when opened in Visual Studio
                 *CodeSyncExt = CodeExt;
const Str NullName = "<None>",
          UnknownName = "<Unknown>",
          MultipleName = "<Multiple Values>",
          EsenthelStoreURL = "https://esenthel.com/store.php";
const flt StateFadeTime = 0.2f,
          SkelSlotSize = 0.2f,
          WaypointRadius = 0.5f,
          FlushWaypointsDelay = 4,
          FlushWaterDelay = 4,
          BuyFullVersionTime = 10 * 60, // 10 min
    AutoSaveTime = 5 * 60,              // 5 min
    MaxGameViewRange = 2000,
          PropElmNameWidth = 0.325f, // value width of property having element name
    SendAreasDelay = 1.0f,
          VtxDupPosEps = 0.0002f,
          DefaultFOV = DegToRad(70),
          PreviewFOV = DegToRad(50);
// See also: FIND_ANIM_BY_NAME_ONLY in the Engine
// min size of mesh box (in meters) to split it
const MESH_FLAG MeshJoinAllTestVtxFlag = VTX_HLP | VTX_SIZE; // this is because of "Leaf" shader which works differently depending on existence of these components, testing for color/skin is not needed because parts that don't have them will have default values set (WHITE color, 0-bone/matrix)

const Edit::Material::TEX_QUALITY MinMtrlTexQualityBase0 = Edit::Material::LOW, // minimum texture compression quality for Material Base0  Texture (RGBA              ) #MaterialTextureLayout      , set to LOW    because can be maximized based on 'ElmMaterial.tex_quality/EditMaterial.tex_quality'
    MinMtrlTexQualityBase1 = Edit::Material::HIGH,                              // minimum texture compression quality for Material Base1  Texture (NxNy              ) #MaterialTextureLayout      , set to HIGH   because normals need this (without this, they get very blocky due to low quality)
    MinMtrlTexQualityBase2 = Edit::Material::MEDIUM,                            // minimum texture compression quality for Material Base2  Texture (MetalRoughBumpGlow) #MaterialTextureLayout      , set to MEDIUM because can't be changed otherwise
    MinMtrlTexQualityDetail = Edit::Material::HIGH,                             // minimum texture compression quality for Material Detail Texture (NxNyRoughCol      ) #MaterialTextureLayoutDetail, set to HIGH   because normals need this (without this, they get very blocky due to low quality)
    MinMtrlTexQualityMacro = Edit::Material::LOW,                               // minimum texture compression quality for Material Macro  Texture (RGB               ) #MaterialTextureLayout      , set to LOW    because can be maximized based on 'ElmMaterial.tex_quality/EditMaterial.tex_quality'
    MinMtrlTexQualityLight = Edit::Material::MEDIUM;                            // minimum texture compression quality for Material Light  Texture (RGB               ) #MaterialTextureLayout      , set to MEDIUM because can't be changed otherwise

const COMPRESS_TYPE ServerNetworkCompression = COMPRESS_LZ4, ClientNetworkCompression = COMPRESS_LZMA, ProjectPackageCompression = COMPRESS_LZMA;
// use 1 instead of 0, so when downloading file, we have no 'param_value' and 'param_value_time' at 0, meaning we need to download the param
const cchar8 *AppName = ENGINE_NAME " Editor",
             *ServerAppName = ENGINE_NAME " Server",
             *InstallerName = ENGINE_NAME " Installer";
const cchar8 *ObjAccessNames[] =
    {
        "Custom",  // 0
        "Terrain", // 1
        "Grass",   // 2
        "Overlay", // 3
};
int ObjAccessNamesElms = Elms(ObjAccessNames);

const cchar8 *ElmNameMesh = "mesh",
             *ElmNameSkel = "skel",
             *ElmNamePhys = "phys";

const MESH_FLAG EditMeshFlagAnd = ~(VTX_DUP | EDGE_ALL | FACE_NRM | ADJ_ALL | VTX_TAN_BIN), // TanBin are not needed in Edit because they're always re-created if needed
    GameMeshFlagAnd = ~(VTX_DUP | EDGE_ALL | FACE_NRM | ADJ_ALL);

const MaterialPtr MaterialNull;

bool IsServer = false;
ReadWriteSync WorldAreaSync;
TimeStamp CurTime; // it's set to the current time at the beginning of each frame, it is used to increase chances of 'AreaVer.old' at the moment of area send attempt
/******************************************************************************/
Str SDKPath() { return GetPath(App.exe()); }
Str BinPath() { return SDKPath().tailSlash(true) + "Bin\\"; } // must include tail slash
/******************************************************************************/
bool LoadOK(LOAD_RESULT result) {
    return result == LOAD_OK || result == LOAD_EMPTY;
}
/******************************************************************************/
ClassNameDesc CipherText[] =
    {
        {null, "No Encryption", u"Project data is not going to be encrypted (not recommended).\nAnyone will be able to access your project files.\nHowever this mode has the best performance x6"}, // CIPHER_NONE
        {"Cipher1", "Cipher 1", u"Project data is going to be encrypted using 'Cipher1' class (recommended).\nIt offers good security and good performance x2"},                                    // CIPHER_1
        {"Cipher2", "Cipher 2", u"Project data is going to be encrypted using 'Cipher2' class.\nIt offers great security and medium performance x1.5"},                                             // CIPHER_2
        {"Cipher3", "Cipher 3", u"Project data is going to be encrypted using 'Cipher3' class.\nIt offers best security and low performance x1"},                                                   // CIPHER_3
};
/******************************************************************************/
NameDesc UserAccessText[] =
    {
        {u"No Access", u"User has no access at all (he's either awaiting activation or got blocked)"},                        // UA_NO_ACCESS
        {u"Read Only", u"User can only read data from the server (he cannot upload anything)"},                               // UA_READ_ONLY
        {u"Artist", u"User can read and upload data (with the exception of changing Codes and some Application properties)"}, // UA_ARTIST
        {u"Normal", u"User can read and upload data"},                                                                        // UA_NORMAL
        {u"Admin", u"User can read and upload data (extra permissions reserved for future use)"},                             // UA_ADMIN
};
bool CanRead(USER_ACCESS access) { return access >= UA_READ_ONLY; }
bool CanWrite(USER_ACCESS access) { return access >= UA_ARTIST; }
bool CanWriteCode(USER_ACCESS access) { return access >= UA_NORMAL; }
/******************************************************************************/
int Compare(C TimeStamp &a, C TimeStamp &b) { return Compare(a.u, b.u); }

TimeStamp Min(C TimeStamp &a, C TimeStamp &b) { return a <= b ? a : b; }
TimeStamp Max(C TimeStamp &a, C TimeStamp &b) { return a >= b ? a : b; }
/******************************************************************************/
const Pose PoseIdentity;
/******************************************************************************/
/*class Matrix3_8 // Matrix3 stored using 8 bytes
{
   ushort orn    , // orientation                  (2 bytes total)
          size[3]; // size encoded to 16-bit float (6 bytes total)

   Vec  scale(               )C {return Vec(F16To32(size[0]), F16To32(size[1]), F16To32(size[2]));}
   void set  (Matrix3 &matrix)C {File f; f.readMem(&orn, SIZE(orn)).decOrient2(matrix); matrix.scaleL(scale());}
   void zero (               )  {Zero(T);}

   bool operator==(C Matrix3_8 &matrix)C {return orn==matrix.orn && size[0]==matrix.size[0] && size[1]==matrix.size[1] && size[2]==matrix.size[2];}
   bool operator!=(C Matrix3_8 &matrix)C {return orn!=matrix.orn || size[0]!=matrix.size[0] || size[1]!=matrix.size[1] || size[2]!=matrix.size[2];}

   Matrix3_8() {}
   Matrix3_8(C Matrix3 &matrix)
   {
      size[0]=F32To16(Mid(matrix.x.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      size[1]=F32To16(Mid(matrix.y.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      size[2]=F32To16(Mid(matrix.z.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      File f; f.writeMem().cmpOrient2(matrix).pos(0); f>>orn;
   }
}*/
/******************************************************************************/
ListColumn NameDescListColumn[1] = // !! need to define array size because this will be in headers and later we need 'Elms' !!
    {
        ListColumn(MEMBER(NameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
};
/******************************************************************************/

/******************************************************************************/
const long TimeStamp::Start = 63524217600, // 63524217600 is the number of seconds at 1st Jan 2013 (approximate time of the first application version)
    TimeStamp::Unix = 62167219200;
/******************************************************************************/
Cipher *ProjectCipher::operator()() { return cipher_ptr; }
ProjectCipher &ProjectCipher::clear() {
    cipher_ptr = null;
    return T;
}
ProjectCipher &ProjectCipher::set(Project &proj) {
    switch (proj.cipher) {
    default:
        cipher_ptr = null;
        break; // CIPHER_NONE
    case CIPHER_1:
        cipher_ptr = &cipher1.setKey(proj.cipher_key, SIZE(proj.cipher_key));
        break;
    case CIPHER_2:
        cipher_ptr = &cipher2.setKey(proj.cipher_key, SIZE(proj.cipher_key));
        break;
    case CIPHER_3:
        cipher_ptr = &cipher3.setKey(proj.cipher_key, SIZE(proj.cipher_key));
        break;
    }
    return T;
}
bool Version::operator==(C Version &v) C { return ver == v.ver; }
bool Version::operator!=(C Version &v) C { return ver != v.ver; }
bool Version::operator!() C { return !ver; }
Version::operator bool() C { return ver != 0; }
void Version::randomize() {
    uint old = ver;
    do
        ver = Random();
    while (!ver || ver == old);
}
TimeStamp TimeStamp::UTC() { return DateTime().getUTC(); }
void TimeStamp::text(C Str &t) { u = TextUInt(t); }
void TimeStamp::text(C TextNode &n) { text(n.value); }
TimeStamp &TimeStamp::getUTC() {
    T = DateTime().getUTC();
    return T;
}
TimeStamp &TimeStamp::now() {
    uint prev = u;
    getUTC();
    if (u <= prev)
        u = prev + 1;
    return T;
}
TimeStamp &TimeStamp::fromUnix(long u) {
    T = u + (Unix - Start);
    return T;
}
bool TimeStamp::old(C TimeStamp &now) C { return T < now; }
TimeStamp &TimeStamp::operator+=(int i) {
    T = long(T.u) + long(i);
    return T;
}
TimeStamp &TimeStamp::operator-=(int i) {
    T = long(T.u) - long(i);
    return T;
}
TimeStamp TimeStamp::operator+(int i) { return long(T.u) + long(i); }
TimeStamp TimeStamp::operator-(int i) { return long(T.u) - long(i); }
long TimeStamp::operator-(C TimeStamp &t) C { return long(u) - long(t.u); }
DateTime TimeStamp::asDateTime() C { return DateTime().fromSeconds(u + Start); }
TimeStamp::operator DateTime() C { return asDateTime(); }
long TimeStamp::age() C { return UTC() - T; }
TimeStamp::TimeStamp(int i) : u(Max(i, 0)) {}
TimeStamp::TimeStamp(uint u) : u(u) {}
TimeStamp::TimeStamp(long l) : u(Mid(l, (long)0, (long)UINT_MAX)) {}
TimeStamp::TimeStamp(C DateTime &dt) : u(Mid(dt.seconds() - Start, (long)0, (long)UINT_MAX)) {}
Matrix Pose::operator()() C { return MatrixD().setPosScale(pos, scale).rotateZ(rot.z).rotateXY(rot.x, rot.y); }
Str Pose::asText() C { return S + "Scale:" + scale + ", Pos:" + pos + ", Rot:" + rot; }
Pose &Pose::reset() {
    scale = 1;
    pos.zero();
    rot.zero();
    return T;
}
Pose &Pose::operator+=(C Vec &offset) {
    pos += offset;
    return T;
}
Pose &Pose::operator*=(C Pose &transform) {
    T = T() * transform();
    return T;
}
Pose &Pose::operator*=(C Matrix &matrix) {
    T = T() * matrix;
    return T;
}
Pose::Pose(C Matrix &matrix) : scale(1), pos(0), rot(0) {
    MatrixD matrix_d = matrix;
    scale = matrix.x.length();
    rot = matrix_d.angles();
    pos = matrix_d.rotateY(-rot.y).rotateX(-rot.x).rotateZ(-rot.z).scale(1 / scale).pos;
}
bool Pose::operator==(C Pose &t) C { return scale == t.scale && pos == t.pos && rot == t.rot; }
bool Pose::operator!=(C Pose &t) C { return !(T == t); }
bool SmallMatrix3::operator==(C SmallMatrix3 &m) C { return angle == m.angle && scale == m.scale; }
bool SmallMatrix3::operator!=(C SmallMatrix3 &m) C { return angle != m.angle || scale != m.scale; }
void SmallMatrix3::set(Matrix3 &matrix) C { matrix.setRotateZ(angle.z).rotateXY(angle.x, angle.y).scaleL(scale); }
SmallMatrix3::SmallMatrix3() {}
SmallMatrix3::SmallMatrix3(C Matrix3 &matrix) {
    angle = matrix.angles();
    scale = matrix.scale();
}
SmallMatrix3 &SmallMatrix::orn() { return T; }
C SmallMatrix3 &SmallMatrix::orn() C { return T; }
bool SmallMatrix::operator==(C SmallMatrix &m) C { return orn() == m.orn() && pos == m.pos; }
bool SmallMatrix::operator!=(C SmallMatrix &m) C { return orn() != m.orn() || pos != m.pos; }
void SmallMatrix::set(Matrix &matrix) C {
    super::set(matrix);
    matrix.pos = pos;
}
Matrix SmallMatrix::operator()() C {
    Matrix matrix;
    set(matrix);
    return matrix;
}
SmallMatrix::SmallMatrix() {}
SmallMatrix::SmallMatrix(C Matrix &matrix) : SmallMatrix3(matrix) { pos = matrix.pos; }
Chunk &Chunk::create(C Str &name, uint ver, File &file) {
    T.name = name;
    T.ver = ver;
    file.get(setNum(file.left()).data(), file.left());
    return T;
}
Chunk &Chunk::create(ChunkReader &cr, File &file) { return create(cr.name(), cr.ver(), file); }
int Chunks::Compare(C Chunk &a, C Chunk &b) { return ComparePath(a.name, b.name); }
Chunk *Chunks::findChunk(C Str &name) {
    REPA(chunks)
    if (EqualPath(chunks[i].name, name))
        return &chunks[i];
    return null;
}
Chunks &Chunks::delChunk(C Str &name) {
    REPA(chunks)
    if (EqualPath(chunks[i].name, name))
        chunks.remove(i, true);
    return T;
}
Chunks &Chunks::setChunk(C Str &name, uint ver, File &file) {
    Chunk *chunk = findChunk(name);
    if (!chunk)
        chunk = &chunks.New();
    chunk->create(name, ver, file);
    return T;
}
void Chunks::del() { chunks.del(); }
bool Chunks::load(File &f) {
    del();
    ChunkReader cr;
    if (cr.read(f)) {
        for (; File *f = cr();)
            chunks.New().create(cr, *f);
        return true;
    }
    return false;
}
bool Chunks::save(File &f) // warning: this sorts 'chunks' and changes memory addresses for each element
{
    ChunkWriter cw(f);
    chunks.sort(Compare);
    FREPA(chunks) {
        Chunk &chunk = chunks[i];
        if (File *f = cw.beginChunk(chunk.name, chunk.ver))
            f->put(chunk.data(), chunk.elms());
    }
    return f.ok();
}
bool Chunks::load(C Str &name, ReadWriteSync &rws) {
    ReadLock rl(rws);
    File f;
    if (f.read(name))
        return load(f);
    del();
    return false;
}
bool Chunks::save(C Str &name, ReadWriteSync &rws) // warning: this sorts 'chunks' and changes memory addresses for each element
{
    File f;
    if (save(f.writeMem())) {
        f.pos(0);
        return SafeOverwriteChunk(f, name, rws);
    }
    return false;
}
::MtrlImages::ImageResize &MtrlImages::ImageResize::clearParams() {
    size.zero();
    filter = -1;
    clamp = alpha_weight = keep_edges = false;
    return T;
}
::MtrlImages::ImageResize &MtrlImages::ImageResize::del() {
    super::del();
    return clearParams();
}
bool MtrlImages::ImageResize::create(C VecI2 &size, IMAGE_TYPE type) {
    clearParams();
    return super::createSoft(size.x, size.y, 1, type);
}
::MtrlImages::ImageResize &MtrlImages::ImageResize::resize(C VecI2 &size) {
    if (size.x > 0)
        T.size.x = size.x;
    if (size.y > 0)
        T.size.y = size.y;
    return T;
}
::MtrlImages::ImageResize &MtrlImages::ImageResize::setFrom(C TextParam &param) {
    clearParams();
    if (param.name.is() && ResizeTransformAny(param.name)) {
        size = GetSize(param.name, param.value, size3());
        filter = GetFilterI(param.name);
        clamp = GetClampWrap(param.name, false);
        alpha_weight = GetAlphaWeight(param.name);
        keep_edges = GetKeepEdges(param.name);
    }
    return T;
}
void MtrlImages::ImageResize::apply() {
    copy(T, (size.x > 0) ? size.x : -1, (size.y > 0) ? size.y : -1, -1, -1, -1, -1, InRange(filter, FILTER_NUM) ? FILTER_TYPE(filter) : FILTER_BEST, (clamp ? IC_CLAMP : IC_WRAP) | (alpha_weight ? IC_ALPHA_WEIGHT : 0) | (keep_edges ? IC_KEEP_EDGES : 0));
}
MtrlImages::ImageResize::operator ImageSource() C { return ImageSource(T, size, filter, clamp); }
MtrlImages &MtrlImages::del() {
    flip_normal_y = smooth_is_rough = false;
    tex = 0;
    color.del();
    alpha.del();
    bump.del();
    normal.del();
    smooth.del();
    metal.del();
    glow.del();
    return T;
}
void MtrlImages::fromMaterial(C EditMaterial &material, C Project &proj, bool changed_flip_normal_y, bool changed_smooth_is_rough) {
    del();

    // here when loading images, load them without resize, in case for example bump is original 256x256, resized to 128x128, and normal created from bump resized to 256x256, normally normal would be created from bump that was already resized from 256x256 to 128x128 and then resized again to 256x256
    TextParam color_resize, alpha_resize, smooth_resize, metal_resize, bump_resize, normal_resize, glow_resize;

    // !! here order of loading images is important, because we pass pointers to those images in subsequent loads !! #MaterialTextureLayout
    bool color_ok = proj.loadImages(color, &color_resize, material.color_map, true, false),
         alpha_ok = proj.loadImages(alpha, &alpha_resize, material.alpha_map, false, false, WHITE, &color, &color_resize),
         smooth_ok = proj.loadImages(smooth, &smooth_resize, material.smooth_map, false, false, BLACK, &color, &color_resize),
         metal_ok = proj.loadImages(metal, &metal_resize, material.metal_map, false, false, BLACK, &color, &color_resize, &smooth, &smooth_resize),
         bump_ok = proj.loadImages(bump, &bump_resize, material.bump_map, false, false, GREY, &color, &color_resize, &smooth, &smooth_resize),
         normal_ok = proj.loadImages(normal, &normal_resize, material.normal_map, false, false, Color(128, 128, 255), &color, &color_resize, &smooth, &smooth_resize, &bump, &bump_resize),
         glow_ok = proj.loadImages(glow, &glow_resize, material.glow_map, false, false);

    // process resize
    if (color_ok)
        color.setFrom(color_resize);
    if (alpha_ok)
        alpha.setFrom(alpha_resize);
    if (smooth_ok)
        smooth.setFrom(smooth_resize);
    if (metal_ok)
        metal.setFrom(metal_resize);
    if (bump_ok)
        bump.setFrom(bump_resize);
    if (normal_ok)
        normal.setFrom(normal_resize);
    if (glow_ok)
        glow.setFrom(glow_resize);

    if (!color_ok && !material.alpha_map.is())
        alpha_ok = false; // if color map failed to load, and there is no dedicated alpha  map, and since it's possible that alpha  was created from the color, which is not available, so alpha  needs to be marked as failed
    if (!bump_ok && !material.normal_map.is())
        normal_ok = false; // if bump  map failed to load, and there is no dedicated normal map, and since it's possible that normal was created from the bump , which is not available, so normal needs to be marked as failed

    ExtractBaseTextures(proj, material.base_0_tex, material.base_1_tex, material.base_2_tex,
                        color_ok ? null : &color, alpha_ok ? null : &alpha, bump_ok ? null : &bump, normal_ok ? null : &normal, smooth_ok ? null : &smooth, metal_ok ? null : &metal, glow_ok ? null : &glow);

    T.flip_normal_y = (normal_ok ? material.flip_normal_y : changed_flip_normal_y); // if we failed to load the original image, and instead we're using extracted normal map, then we need to flip Y only if we're changing flipping at this moment
    T.smooth_is_rough = (smooth_ok ? material.smooth_is_rough : changed_smooth_is_rough);
}
void MtrlImages::fromMaterial(C EditWaterMtrl &material, C Project &proj, bool changed_flip_normal_y, bool changed_smooth_is_rough) {
    del();

    // here when loading images, load them without resize, in case for example bump is original 256x256, resized to 128x128, and normal created from bump resized to 256x256, normally normal would be created from bump that was already resized from 256x256 to 128x128 and then resized again to 256x256
    TextParam color_resize, alpha_resize, smooth_resize, metal_resize, bump_resize, normal_resize, glow_resize;

    // !! here order of loading images is important, because we pass pointers to those images in subsequent loads !! #MaterialTextureLayoutWater
    bool color_ok = proj.loadImages(color, &color_resize, material.color_map, true, false),
         alpha_ok = proj.loadImages(alpha, &alpha_resize, material.alpha_map, false, false, WHITE, &color, &color_resize),
         smooth_ok = proj.loadImages(smooth, &smooth_resize, material.smooth_map, false, false, WHITE, &color, &color_resize),
         metal_ok = proj.loadImages(metal, &metal_resize, material.metal_map, false, false, BLACK, &color, &color_resize, &smooth, &smooth_resize),
         bump_ok = proj.loadImages(bump, &bump_resize, material.bump_map, false, false, GREY, &color, &color_resize, &smooth, &smooth_resize),
         normal_ok = proj.loadImages(normal, &normal_resize, material.normal_map, false, false, Color(128, 128, 255), &color, &color_resize, &smooth, &smooth_resize, &bump, &bump_resize),
         glow_ok = proj.loadImages(glow, &glow_resize, material.glow_map, false, false);

    // process resize
    if (color_ok)
        color.setFrom(color_resize);
    if (alpha_ok)
        alpha.setFrom(alpha_resize);
    if (smooth_ok)
        smooth.setFrom(smooth_resize);
    if (metal_ok)
        metal.setFrom(metal_resize);
    if (bump_ok)
        bump.setFrom(bump_resize);
    if (normal_ok)
        normal.setFrom(normal_resize);
    if (glow_ok)
        glow.setFrom(glow_resize);

    if (!color_ok && !material.alpha_map.is())
        alpha_ok = false; // if color map failed to load, and there is no dedicated alpha  map, and since it's possible that alpha  was created from the color, which is not available, so alpha  needs to be marked as failed
    if (!bump_ok && !material.normal_map.is())
        normal_ok = false; // if bump  map failed to load, and there is no dedicated normal map, and since it's possible that normal was created from the bump , which is not available, so normal needs to be marked as failed

    ExtractWaterBaseTextures(proj, material.base_0_tex, material.base_1_tex, material.base_2_tex,
                             color_ok ? null : &color, alpha_ok ? null : &alpha, bump_ok ? null : &bump, normal_ok ? null : &normal, smooth_ok ? null : &smooth, metal_ok ? null : &metal, glow_ok ? null : &glow);

    T.flip_normal_y = (normal_ok ? material.flip_normal_y : changed_flip_normal_y); // if we failed to load the original image, and instead we're using extracted normal map, then we need to flip Y only if we're changing flipping at this moment
    T.smooth_is_rough = (smooth_ok ? material.smooth_is_rough : changed_smooth_is_rough);
}
TEX_FLAG MtrlImages::createBaseTextures(Image &base_0, Image &base_1, Image &base_2) C {
    return CreateBaseTextures(base_0, base_1, base_2, color, alpha, bump, normal, smooth, metal, glow, true, flip_normal_y, smooth_is_rough);
}
TEX_FLAG MtrlImages::createWaterBaseTextures(Image &base_0, Image &base_1, Image &base_2) C {
    return CreateWaterBaseTextures(base_0, base_1, base_2, color, alpha, bump, normal, smooth, metal, glow, true, flip_normal_y, smooth_is_rough);
}
void MtrlImages::baseTextureSizes(VecI2 *size0, VecI2 *size1, VecI2 *size2) { // TODO: this could be optimized by calculating Max of image sizes, however there are some special cases (normal made from bump, etc.)
    Image base[3];
    createBaseTextures(base[0], base[1], base[2]);
    if (size0)
        *size0 = base[0].size();
    if (size1)
        *size1 = base[1].size();
    if (size2)
        *size2 = base[2].size();
}
void MtrlImages::waterBaseTextureSizes(VecI2 *size0, VecI2 *size1, VecI2 *size2) { // TODO: this could be optimized by calculating Max of image sizes, however there are some special cases (normal made from bump, etc.)
    Image base[3];
    createWaterBaseTextures(base[0], base[1], base[2]);
    if (size0)
        *size0 = base[0].size();
    if (size1)
        *size1 = base[1].size();
    if (size2)
        *size2 = base[2].size();
}
ProjectCipher::ProjectCipher() : cipher_ptr(null) {}

Version::Version() : ver(0) {}

TimeStamp::TimeStamp() : u(0) {}

Pose::Pose() : scale(1), pos(0), rot(0) {}

Chunk::Chunk() : ver(0) {}

MtrlImages::MtrlImages() : flip_normal_y(false), smooth_is_rough(false), tex(0) {}

MtrlImages::ImageResize::ImageResize() : size(0), filter(-1), clamp(false), alpha_weight(false), keep_edges(false) {}

/******************************************************************************/
