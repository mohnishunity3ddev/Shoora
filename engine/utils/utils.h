#if !defined(UTILS_H)

#include <defines.h>
#include "sort/sort.h"
#include <math/math.h>

struct colorU32
{
    static const u32 Red = 0xffff0000;
    static const u32 Green = 0xff00ff00;
    static const u32 Blue = 0xff0000ff;
    static const u32 Cyan = 0xff00ffff;
    static const u32 Magenta = 0xffff00ff;
    static const u32 Yellow = 0xffffff00;
    static const u32 White = 0xffffffff;
    static const u32 Gray = 0xff313131;

    static const u32 Proto_Orange = 0xffFB8621;
    static const u32 Proto_Yellow = 0xffFFB900;
    static const u32 Proto_Green = 0xff6DA174;
    static const u32 Proto_Red = 0xffBD4334;
    static const u32 Proto_Blue = 0xff697FC4;
};

u32 GetMaxValueIndex(u32 *NumsArray, u32 NumsCount);
u32 LogBase2(u32 Num);

u32 StringLength(char *A);
void StringConcat(const char *A, const char *B, char *Out);
void StringNCopy(const char *Src, char *Dst, i32 Num);
void StringCopy(const char *Src, char *Dst);

u32 StringFindLastOf(const char *String, char Separator);
void StringSubString(const char *SrcString, u32 StartIndex, u32 EndIndex, char *OutString);
b32 StringsEqual(const char *A, const char *B);

Shu::vec3f GetColor(u32 Col);
u32 GetColorU32(const Shu::vec3f &Color);

#define UTILS_H
#endif // UTILS_H