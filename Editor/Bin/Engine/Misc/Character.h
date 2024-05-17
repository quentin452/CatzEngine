﻿/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
enum CHAR_FLAG // Character Flag
{
   CHARF_DIG2 =1<<0, // if binary  digit 01
   CHARF_DIG10=1<<1, // if decimal digit 0123456789
   CHARF_DIG16=1<<2, // if hex     digit 0123456789ABCDEFabcdef
   CHARF_SIGN =1<<3, // if sign          `~!@#$%^&_-+*=()[]{}<>;:'",./|\?
   CHARF_UNDER=1<<4, // if underline
   CHARF_ALPHA=1<<5, // if alphabetic
   CHARF_SPACE=1<<6, // if space, nbsp, FullWidthSpace, Tab
   CHARF_NBSP =1<<7, // if nbsp
   CHARF_UP   =1<<8, // if upper case
#if EE_PRIVATE
   CHARF_SIGN2     =1<< 9, // if sign2
   CHARF_COMBINING =1<<10, // if combining character
   CHARF_STACK     =1<<11, // if stack on top of each other
   CHARF_FONT_SPACE=1<<12, // if adjust font spacing
   CHARF_MULTI0    =1<<13, // high surrogate W1
   CHARF_MULTI1    =1<<14, // low  surrogate W2
   CHARF_SKIP      =CHARF_COMBINING|CHARF_MULTI1,
#endif

   CHARF_DIG=CHARF_DIG10,
};

UInt CharFlag(Char  c); // get CHAR_FLAG
UInt CharFlag(Char8 c); // get CHAR_FLAG
/******************************************************************************/
enum CHAR_TYPE : Byte // Character Type
{
   CHART_NONE , // none/unknown
   CHART_CHAR , // alphabetic, digit, underline, nbsp
   CHART_SPACE, // space, FullWidthSpace, Tab
   CHART_SIGN , // symbol
};

CHAR_TYPE CharType(Char  c); // get character type
CHAR_TYPE CharType(Char8 c); // get character type
/******************************************************************************/
constexpr UInt CC4(Byte a, Byte b, Byte c, Byte d) {return a | (b<<8) | (c<<16) | (d<<24);}

Bool WhiteChar(Char c); // if char is a white char - ' ', '\t', '\n', '\r'

constexpr Byte   Unsigned(Char8 x) {return x;}
constexpr UShort Unsigned(Char  x) {return x;}

constexpr Char8 Char16To8(Char  c) {return          c&0xFF;} // convert 16-bit to  8-bit character
constexpr Char  Char8To16(Char8 c) {return Unsigned(c)    ;} // convert  8-bit to 16-bit character

Char  CaseDown(Char  c); // return lower case 'c'
Char8 CaseDown(Char8 c); // return lower case 'c'
Char  CaseUp  (Char  c); // return upper case 'c'
Char8 CaseUp  (Char8 c); // return upper case 'c'

Int Compare(Char  a, Char  b, Bool case_sensitive=false); // compare characters, returns -1, 0, +1
Int Compare(Char  a, Char8 b, Bool case_sensitive=false); // compare characters, returns -1, 0, +1
Int Compare(Char8 a, Char  b, Bool case_sensitive=false); // compare characters, returns -1, 0, +1
Int Compare(Char8 a, Char8 b, Bool case_sensitive=false); // compare characters, returns -1, 0, +1

Bool Equal(Char  a, Char  b, Bool case_sensitive=false); // if characters are the same
Bool Equal(Char  a, Char8 b, Bool case_sensitive=false); // if characters are the same
Bool Equal(Char8 a, Char  b, Bool case_sensitive=false); // if characters are the same
Bool Equal(Char8 a, Char8 b, Bool case_sensitive=false); // if characters are the same

constexpr Bool EqualCS(Char8 a, Char8 b) {return           a ==          b ;} // if characters are the same, case sensitive
constexpr Bool EqualCS(Char8 a, Char  b) {return Char8To16(a)==          b ;} // if characters are the same, case sensitive
constexpr Bool EqualCS(Char  a, Char8 b) {return           a ==Char8To16(b);} // if characters are the same, case sensitive
constexpr Bool EqualCS(Char  a, Char  b) {return           a ==          b ;} // if characters are the same, case sensitive

Char RemoveAccent(Char c); // convert accented character to one without an accent, for example RemoveAccent('ą') -> 'a', if character is not accented then it will be returned without any modifications, RemoveAccent('a') -> 'a'

inline Bool HasUnicode(Char  c) {return Unsigned(c)>127;} // if character is a unicode character
inline Bool HasUnicode(Char8 c) {return Unsigned(c)>127;} // if character is a unicode character
#if EE_PRIVATE
inline Bool HasWide(Char  c) {return Unsigned(c)>255;} // if character is a wide character
inline Bool HasWide(Char8 c) {return false          ;} // if character is a wide character
#endif
/******************************************************************************/
const Char8 CharNull      ='\0',
            CharTab       ='\t',
            CharLine      ='\n';
const Char  CharBullet    =u'•',
            CharDegree    =u'°',
            CharSection   =u'§',
            CharPlusMinus =u'±',
            CharMul       =u'×',
            CharDiv       =u'÷',
            CharStar      =u'★',

            CharLeft      =u'←',
            CharRight     =u'→',
            CharDown      =u'↓',
            CharUp        =u'↑',
            CharUpLeft    =u'↖',
            CharUpRight   =u'↗',
            CharDownLeft  =u'↙',
            CharDownRight =u'↘',
            CharLeftRight =u'↔',
            CharDownUp    =u'↕',

            CharTriLeft   =u'⯇',
            CharTriRight  =u'⯈',
            CharTriDown   =u'⯆',
            CharTriUp     =u'⯅',

            CharTriangleUp=u'△',
            CharSquare    =u'□', // med=◻, large=⬜
            CharCircle    =u'○', // med=⚪, large=◯
            CharCross     =u'✕', // ╳❌✖⨉🗙🞪

            CharCopyright =u'©',
            CharRegTM     =u'®',
            CharTrademark =u'™',
            CharReturn    =u'⏎',
            CharEnter     =u'⎆',
            Nbsp          =u' ', // non-breaking space
            FullWidthSpace=u'　',
            Ellipsis      =u'…',
            CharAlpha     =u'α',
            CharBeta      =u'β',
            CharSuper2    =u'²',
            CharSuper3    =u'³',
            CharPermil    =u'‰'; // 1/1000
/******************************************************************************/
#if EE_PRIVATE
extern U16 _CharFlag[];

extern const Char8 Digits16[]; // '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'

INLINE UInt CharFlagFast(Char  c) {return _CharFlag    [Unsigned (c)];}
INLINE UInt CharFlagFast(Char8 c) {return  CharFlagFast(Char8To16(c));}

Bool Safe(Char8 c); // if character is     safe
Bool Safe(Char  c); // if character is     safe
Bool Skip(Char  c); // if character can be skipped

Int CharInt(Char c); // get character as integer, '0'->0, '1'->1, .., 'a/A'->10, 'b/B'->11, .., ?->-1

CChar8* CharName(Char c); // get character name, sample usage: CharName(' ') -> "Space"

UInt CharFlagFast(Char8 a, Char8 b);
UInt CharFlagFast(Char  a, Char  b);
#endif
/******************************************************************************/
