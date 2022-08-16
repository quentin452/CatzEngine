/******************************************************************************/
/******************************************************************************/
// MATERIAL
/******************************************************************************/
class XMaterialEx : XMaterial
{
   Image    base_0, base_1, base_2, detail, macro, emissive_img;
   UID      base_0_id, base_1_id, base_2_id, detail_id, macro_id, emissive_id;
   bool     adjust_params;
   byte     tex_downsize[TSP_NUM];
   TEX_FLAG has_textures, known_textures;

   XMaterialEx(C Project *proj=null);

   void create(C Material &src);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
