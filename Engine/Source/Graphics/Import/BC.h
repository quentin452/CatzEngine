/******************************************************************************/
void DecompressBlockBC1 (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockBC2 (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockBC3 (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockBC4 (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockBC4S(C Byte *b, SByte  (&block)[4][4]);
void DecompressBlockBC5 (C Byte *b, Color  (&block)[4][4]);
void DecompressBlockBC5S(C Byte *b, VecSB2 (&block)[4][4]);
void DecompressBlockBC6 (C Byte *b, VecH   (&block)[4][4]);
void DecompressBlockBC7 (C Byte *b, Color  (&block)[4][4]);

void DecompressBlockBC1 (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockBC2 (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockBC3 (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockBC4 (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockBC4S(C Byte *b, SByte  *dest, Int pitch);
void DecompressBlockBC5 (C Byte *b, Color  *dest, Int pitch);
void DecompressBlockBC5S(C Byte *b, VecSB2 *dest, Int pitch);
void DecompressBlockBC6 (C Byte *b, VecH   *dest, Int pitch);
void DecompressBlockBC7 (C Byte *b, Color  *dest, Int pitch);

Color  DecompressPixelBC1 (C Byte *b, Int x, Int y);
Color  DecompressPixelBC2 (C Byte *b, Int x, Int y);
Color  DecompressPixelBC3 (C Byte *b, Int x, Int y);
Color  DecompressPixelBC4 (C Byte *b, Int x, Int y);
SByte  DecompressPixelBC4S(C Byte *b, Int x, Int y);
Color  DecompressPixelBC5 (C Byte *b, Int x, Int y);
VecSB2 DecompressPixelBC5S(C Byte *b, Int x, Int y);
VecH   DecompressPixelBC6 (C Byte *b, Int x, Int y);
Color  DecompressPixelBC7 (C Byte *b, Int x, Int y);

Bool CompressBC(C Image &src, Image &dest);
/******************************************************************************/
