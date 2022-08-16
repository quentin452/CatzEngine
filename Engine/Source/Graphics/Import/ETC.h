/******************************************************************************/
void DecompressBlockETC1     (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockETC2R    (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockETC2RS   (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockETC2RS   (C Byte *b, SByte  (&block)[4][4]);
void DecompressBlockETC2RG   (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockETC2RGS  (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockETC2RGS  (C Byte *b, VecSB2 (&block)[4][4]);
void DecompressBlockETC2RGB  (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockETC2RGBA1(C Byte *b, Color  (&block)[4][4]);
void DecompressBlockETC2RGBA (C Byte *b, Color  (&block)[4][4]);

void DecompressBlockETC1     (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockETC2R    (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockETC2RS   (C Byte *b, SByte  *dest, Int pitch);
void DecompressBlockETC2RG   (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockETC2RGS  (C Byte *b, VecSB2 *dest, Int pitch);
void DecompressBlockETC2RGB  (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockETC2RGBA1(C Byte *b, Color  *dest, Int pitch);
void DecompressBlockETC2RGBA (C Byte *b, Color  *dest, Int pitch);

Color  DecompressPixelETC1     (C Byte *b, Int x, Int y);
Color  DecompressPixelETC2R    (C Byte *b, Int x, Int y);
SByte  DecompressPixelETC2RS   (C Byte *b, Int x, Int y);
Color  DecompressPixelETC2RG   (C Byte *b, Int x, Int y);
VecSB2 DecompressPixelETC2RGS  (C Byte *b, Int x, Int y);
Color  DecompressPixelETC2RGB  (C Byte *b, Int x, Int y);
Color  DecompressPixelETC2RGBA1(C Byte *b, Int x, Int y);
Color  DecompressPixelETC2RGBA (C Byte *b, Int x, Int y);
/******************************************************************************/
