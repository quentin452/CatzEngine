/******************************************************************************/
/******************************************************************************/
class ProjectSettings : ClosableWindow
{
   class CompressType
   {
      COMPRESS_TYPE type;
      cchar        *desc;
      cchar8       *name;
   };

   Button    randomize_key;
   TextLine  cipher_key;
   Text      compress, compress_mobile, mtrl_simplify_t, text_data_t, tex_size_mobile_t, tex_size_switch_t;
   ComboBox  cipher_type, compress_type, compress_level, compress_type_mobile, compress_level_mobile, mtrl_simplify, text_data, tex_size_mobile, tex_size_switch;
   Button    ok;
   TextBlack ts;

   static CompressType cmpr_type[]
;
   static cchar8 *cmpr_lvl[]
;

   static NameDesc mtrl_simplify_nd[]
; ASSERT(MS_NEVER==0 && MS_MOBILE==1 && MS_ALWAYS==2);

   static cchar8 *text_data_modes[]
;

   static void CipherChanged             (ProjectSettings &ps);
   static void EncryptionKeyChanged      (ProjectSettings &ps);
   static void RandomizeEncryptionKey    (ProjectSettings &ps);
   static void CompressTypeChanged       (ProjectSettings &ps);
   static void CompressLevelChanged      (ProjectSettings &ps);
   static void CompressTypeMobileChanged (ProjectSettings &ps);
   static void CompressLevelMobileChanged(ProjectSettings &ps);
   static void MaterialSimplifyChanged   (ProjectSettings &ps);
   static void TextDataChanged           (ProjectSettings &ps);
   static void TexSizeMobileChanged      (ProjectSettings &ps);
   static void TexSizeSwitchChanged      (ProjectSettings &ps);

   COMPRESS_TYPE     compressType      ()C;
   COMPRESS_TYPE     compressTypeMobile()C;
   MATERIAL_SIMPLIFY       mtrlSimplify()C;

   void toGui();
   void create();
   void display();
};
/******************************************************************************/
/******************************************************************************/
extern ProjectSettings ProjSettings;
/******************************************************************************/
