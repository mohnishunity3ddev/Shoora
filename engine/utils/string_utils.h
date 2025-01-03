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
        string(const char *d);

        string() = delete;
        string(const string &other) = delete;
        string &operator=(const char *d) = delete;
        string &operator=(string &&other) = delete;

        void Append(const char *str);
        void Modify(const char *str);

        size_t Length() const noexcept;
        b32 IsNullOrEmpty() const noexcept;
        const char *c_str() const noexcept;

        void Free();

        ~string();
        b32 IsSmallString() { return IsSSO; }

      private:
        union {
          char *MemPtr;
          char SmallStringArr[8];
        } data;

        b32 IsSSO = false;
        size_t Len;
        // TODO: If the size is less than SSO_SIZE, then we don't need to use heap memory.
        size_t Capacity;
    };

    struct string_table
    {
        public:
          u32 GetHash(const char *InputString, size_t Length, b32 IgnoreCase = false);
          void Initialize(u32 InitCount, u32 Seed);
          b32 AddString(const char *String);
          void Enumerate() const;
          void Free();
          u32
          GetSeed() { return Seed; }

        private:
          u32 Seed;
          b32 Initialized;
          size_t EntryCount;
          size_t Capacity;
          const char **Entries;
    };
    void StringTest();
    void StringTableTest();

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