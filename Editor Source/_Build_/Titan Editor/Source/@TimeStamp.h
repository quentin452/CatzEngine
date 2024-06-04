#pragma once
/******************************************************************************/
/******************************************************************************/
class TimeStamp // TODO: Warning: this is a 32-bit value and will overflow at some point
{
    static const long Start, // 63524217600 is the number of seconds at 1st Jan 2013 (approximate time of the first application version)
        Unix;                // 62167219200 is the number of seconds at 1st Jan 1970

    static TimeStamp UTC();

    uint u;

    inline bool is() C; // if was set

    inline uint text() C; // this method is used when  saving to   text, it can be replaced in the future to something like "Str text()C {return ..;}"
    void text(C Str &t);  // this method is used when loading from text, it can be replaced in the future
    void text(C TextNode &n);

    inline TimeStamp &operator--();
    inline TimeStamp &operator--(int);
    inline TimeStamp &operator++();
    inline TimeStamp &operator++(int);

    inline TimeStamp &zero();
    TimeStamp &getUTC(); // set to current time
    TimeStamp &now();    // set to current time and make sure that it's newer than the previous time

    TimeStamp &fromUnix(long u);

    bool old(C TimeStamp &now = TimeStamp().getUTC()) C; // if this timestamp is older than 'now'

    inline bool operator==(C TimeStamp &t) C; // if equal
    inline bool operator!=(C TimeStamp &t) C; // if not equal
    inline bool operator>=(C TimeStamp &t) C; // if greater or equal
    inline bool operator<=(C TimeStamp &t) C; // if smaller or equal
    inline bool operator>(C TimeStamp &t) C;  // if greater
    inline bool operator<(C TimeStamp &t) C;  // if smaller

    TimeStamp &operator+=(int i);
    TimeStamp &operator-=(int i);

    TimeStamp operator+(int i);
    TimeStamp operator-(int i);

    long operator-(C TimeStamp &t) C;

    DateTime asDateTime() C;
    operator DateTime() C;

    long age() C;

    TimeStamp(int i);
    TimeStamp(uint u);
    TimeStamp(long l);
    TimeStamp(C DateTime &dt);

  public:
    TimeStamp();
};
/******************************************************************************/
/******************************************************************************/
extern TimeStamp CurTime;
/******************************************************************************/
