#ifndef STRING_UTILS_H
#define STRING_UTILS_H
#include <cstdlib>
#include <defines.h>


namespace shu
{
    #define stringAlloc malloc
    #define stringFree free
    #define stringCalloc calloc
    struct string
    {
        // #define SSO_SIZE 32

        explicit string();
        explicit string(size_t InitLength);
        explicit string(const char *d, size_t InitLength);
        string(const char *d);
        string(const string &other);

        string &operator=(const char *cString);
        string &operator=(const string &other);
        string &operator=(string &&other) noexcept;
        string &operator+=(const char *str);

        size_t Length() const noexcept;
        b32 IsNullOrEmpty() const noexcept;

        const char *c_str() const noexcept;
        string &Append(const char *str);
        string &Append(const char *str, size_t Length);
        void Free();

        ~string();

      private:
        char *Data;
        // TODO: If the size is less than SSO_SIZE, then we don't need to use heap memory.
        // char ssoData[SSO_SIZE];
        size_t Capacity;
    };

    size_t StringLength(const char *A);
    void StringConcat(const char *A, const char *B, char *Out);
    void StringConcat(char *A, const char *B);
    void StringNCopy(const char *Src, char *Dst, i32 Num);
    void StringCopy(const char *Src, char *Dst);

    b32 StringLowerCaseEqual(const char *a, const char *b);

    i32 StringFindLastOf(const char *String, char Separator);
    void StringSubString(const char *SrcString, size_t StartIndex, size_t EndIndex, char *OutString);
    b32 StringsEqual(const char *A, const char *B);
    b32 StringLowerCaseEqual(const char *a, const char *b);
    char *StringDuplicateMalloc(const char *In);
    f64 StringToDouble(const char *str);

    b32 IsDigit(char c);
    b32 IsAlphaNumeric(char c);
    b32 IsAlpha(char c);
    char ToLower(char c);
    char CharAt(const char *str, i32 at);
}

#endif