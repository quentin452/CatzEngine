/******************************************************************************/
struct UID // 128-bit Unique Identifier
{
   union
   {
      struct{Byte  b[16];};
      struct{UInt  i[ 4];};
      struct{ULong l[ 2];};
   };

   Bool valid         ()C {return l[0] || l[1]    ;          } // if        the identifier is valid, not a zero
   UID& zero          ()  {       l[0] =  l[1] = 0; return T;} // zero      the identifier
   UID& randomize     ();                                      // randomize the identifier
   UID& randomizeValid();                                      // randomize the identifier and make sure that it's valid, not a zero

   UID& set(UInt  a, UInt  b, UInt c, UInt d) {i[0]=a; i[1]=b; i[2]=c; i[3]=d; return T;} // set identifier from custom values
   UID& set(ULong a, ULong b                ) {l[0]=a; l[1]=b;                 return T;} // set identifier from custom values

   Str    asHex(           )C; // return in hexadecimal format of 32 character length, for example "F2FD9C88E4E754AA4632C6D884D67DE8"
   Bool fromHex(C Str &text) ; // set  from hexadecimal format of 32 character length, for example "F2FD9C88E4E754AA4632C6D884D67DE8", sets to zero and returns false on fail (if 'text' is of invalid format), 'text' must contain exactly 32 hexadecimal characters (which can be a digit "0123456789", lower case character "abcdef" or upper case character "ABCDEF")

   Str    asCanonical(           )C; // return in canonical string format, for example "00000000-0000-0000-0000-000000000000"
   Bool fromCanonical(C Str &text) ; // set  from canonical string format, for example "00000000-0000-0000-0000-000000000000", sets to zero and returns false on fail (if 'text' is of invalid format)

   Str    asCString(           )C; // return in C string format, for example "UID(534740238, 76182471, 691827931, 239847293)"
   Bool fromCString(C Str &text) ; // set  from C string format, for example "UID(534740238, 76182471, 691827931, 239847293)", sets to zero and returns false on fail (if 'text' is of invalid format)

   Str    asFileName(           )C; // return in file name format of 24 character length, for example "h3kv^1fvcwe4ri0#ll#761m7"
   Bool fromFileName(C Str &text) ; // set  from file name format of 24 character length, for example "h3kv^1fvcwe4ri0#ll#761m7", sets to zero and returns false on fail (if 'text' is of invalid format)

   Bool fromText(C Str &text); // set from text, sets to zero and returns false on fail, this function will try to decode the ID from different kinds of formats (hex, C string, DecodeFileName)

   UID& operator++() {T+=1u; return T;}   void operator++(int) {T+=1u;} // use unsigned because those methods are faster
   UID& operator--() {T-=1u; return T;}   void operator--(int) {T-=1u;} // use unsigned because those methods are faster

   UID& operator+=(Int i);   UID& operator+=(UInt i);
   UID& operator-=(Int i);   UID& operator-=(UInt i);

   friend UID operator+(C UID &uid, Int i) {return UID(uid)+=i;}
   friend UID operator-(C UID &uid, Int i) {return UID(uid)-=i;}

   Bool operator==(C UID &uid)C {return l[0]==uid.l[0] && l[1]==uid.l[1];} // if identifiers are equal
   Bool operator!=(C UID &uid)C {return l[0]!=uid.l[0] || l[1]!=uid.l[1];} // if identifiers are different

#if EE_PRIVATE
#if WINDOWS
   GUID& guid()  {ASSERT(SIZE(T)==SIZE(GUID)); return (GUID&)T;}
 C GUID& guid()C {ASSERT(SIZE(T)==SIZE(GUID)); return (GUID&)T;}
#endif
#endif

   UID() {}
   UID(UInt  a, UInt  b, UInt c, UInt d) {set(a, b, c, d);}
   UID(ULong a, ULong b                ) {set(a, b      );}
};extern
   const UID UIDZero; // constant UID with zero values
/******************************************************************************/
struct IDGenerator // this class manages generating unique UInt ID's (starting from 0 and increasing by 1), an ID can be returned to the generator when it is no longer used, allowing the generator to reuse it in the future when requesting a new ID
{
   UInt New   (       ); // create a  new    ID
   void Return(UInt id); // return an unused ID so it can be re-used later (returning an invalid ID will result in incorrect behavior of this class, an invalid ID is an ID that was not generated by this class or an ID that is already returned)

  ~IDGenerator();

private:
   UInt       _created=0;
   Memc<UInt> _returned;
};
/******************************************************************************/
inline Int Compare(C UID &a, C UID &b)
{
   if(a.l[1]<b.l[1])return -1; if(a.l[1]>b.l[1])return +1;
   if(a.l[0]<b.l[0])return -1; if(a.l[0]>b.l[0])return +1;
   return 0;
}
/******************************************************************************/
const Byte UIDFileNameLen=24;
/******************************************************************************/
