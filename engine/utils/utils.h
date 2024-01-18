#if !defined(UTILS_H)

#include <defines.h>
#include "sort/sort.h"
#include <math/math.h>

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

#define UTILS_H
#endif // UTILS_H