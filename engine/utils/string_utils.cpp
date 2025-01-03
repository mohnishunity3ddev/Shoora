#include "string_utils.h"
#include <platform/platform.h>

namespace shu
{
    // 138478156
    u32
    string_table::GetHash(const char *InputString, size_t Length, b32 IgnoreCase)
    {
        u32 Result = this->Seed;

        u32 Character;
        size_t Index;

        for (Index = 0; Index < Length; ++Index)
        {
            Character = InputString[Index];
            if (IgnoreCase)
            {
                Character = shu::ToLower(Character);
            }

            Result ^= Character;
            Result *= 16777619; // FNV Prime.
        }

        return Result;
    }

    void
    string_table::Initialize(u32 InitCapacity, u32 Seed)
    {
        this->Seed = Seed;
        this->Capacity = InitCapacity;
        this->Entries = (const char **)stringCalloc(this->Capacity, sizeof(const char **));
        this->Initialized = true;
        this->EntryCount = 0;
    }

    b32
    string_table::AddString(const char *String)
    {

        u32 Index = this->GetHash(String, StringLength(String)-1);
        Index %= this->Capacity;
        ASSERT(Index < this->Capacity);
        if (Entries[Index] != nullptr)
            return false;
        Entries[Index] = StringDuplicateMalloc(String);
        EntryCount++;
        ASSERT(EntryCount <= Capacity);
        return true;
    }

    void
    string_table::Enumerate() const
    {
        LogDebugUnformatted("HashTable Entries: \n");
        for (int i = 0; i < this->Capacity; ++i)
        {
            LogDebug("Entry %zu: %s.\n", i, this->Entries[i]);
        }
    }

    void
    string_table::Free()
    {
        for (int i = 0; i < this->EntryCount; ++i)
        {
            if (this->Entries[i] != nullptr) {
                free((void *) this->Entries[i]);
                this->Entries[i] = nullptr;
            }
        }
        this->EntryCount = 0;
        this->Capacity = 0;
        this->Seed = 0;
        this->Initialized = false;

        if (this->Entries != nullptr) {
            free(this->Entries);
            this->Entries = nullptr;
        }
    }

#if 0
    constexpr u32 operator"" _sid(const char *str, size_t s) {
        return GetHash(str, s, false);
    }
#endif

    void
    StringTableTest()
    {
        string_table t;
#define TRY_ADD(Str) if (!t.AddString(#Str)) { t.Free(); continue; }

        b32 success = false;
        i32 attemptCount = 0;
        while(true)
        {
            ++attemptCount;
            if (attemptCount == INT32_MAX)
                break;
            t.Initialize(11, Platform_GetRandomSeed());
            TRY_ADD("Mani");
            TRY_ADD("Sharma");
            TRY_ADD("Is");
            TRY_ADD("A");
            TRY_ADD("Nice");
            TRY_ADD("Guy");
            TRY_ADD("Bloody");
            TRY_ADD("Hell");
            TRY_ADD("Brother!");
            TRY_ADD("Mohnish Sharma is my name. And dont call me Nice \"BOY\"!");
            break;
        }

        LogInfo("Attempts: %d, Seed = %u.\n", attemptCount, t.GetSeed());
        t.Enumerate();
        t.Free();
        int x = 0;
#undef TRY_ADD
    }


    string::~string()
    {
        // if (!this->IsSSO && this->data.MemPtr) stringFree(this->data.MemPtr);
        // this->data.MemPtr = nullptr;
        // this->Capacity = 0;
        // this->IsSSO = false;
        // this->Len = 0;

        LogWarn("Attempting to free %s.\n", this->c_str());
    }

    string::string(const char *d)
    {
        ASSERT(!this->data.MemPtr);

        size_t len = StringLength(d);
        if (len > 8) {
            size_t alignedLength = ALIGN32(len);
            this->data.MemPtr = (char *)stringAlloc(alignedLength);
            this->Capacity = alignedLength;
            this->IsSSO = false;
            SHU_MEMCOPY(d, this->data.MemPtr, len);
        } else {
            SHU_MEMCOPY(d, this->data.SmallStringArr, 8);
            this->IsSSO = true;
            this->Capacity = 8;
        }

        this->Len = len;
    }

    void
    string::Append(const char *AppendStr)
    {
        i32 lenA = this->Len - 1;
        lenA = MAX(lenA, 0);
        ASSERT(lenA > 0);

        i32 lenB = StringLength(AppendStr) - 1;
        ASSERT(lenB > 0);

        i32 totalLength = lenA + lenB + 1;
        if (totalLength <= 8 && this->IsSSO)
        {
            ASSERT(this->Len < 8);
            for (i32 i = lenA, j = 0; i < totalLength; ++i, ++j) {
                this->data.SmallStringArr[i] = AppendStr[j];
            }
            this->Len = totalLength;
            this->data.SmallStringArr[lenA + lenB] = '\0';
        }
        else
        {
            size_t totalAligned = ALIGN32(totalLength);
            ASSERT(totalAligned > 8);
            char *stringMem = this->data.MemPtr;

            if (this->IsSSO) {
                stringMem = (char *)stringAlloc(totalAligned);
                SHU_MEMCOPY(this->data.SmallStringArr, stringMem, lenA);
                this->IsSSO = false;
            }
            else if (this->Capacity < totalAligned)
            {
                stringMem = (char *)stringAlloc(totalAligned);
                SHU_MEMCOPY(this->data.MemPtr, stringMem, lenA);
                stringFree(this->data.MemPtr);
            }

            this->data.MemPtr = stringMem;
            this->Capacity = totalAligned;
            this->Len = totalLength;

            SHU_MEMCOPY(AppendStr, this->data.MemPtr + lenA, lenB);
            this->data.MemPtr[lenA + lenB] = '\0';
        }
    }
    
    void
    string::Modify(const char *str)
    {
        size_t len = StringLength(str);
        if (len <= 8 && this->IsSSO)
        {
            SHU_MEMCOPY(str, data.SmallStringArr, len);
        } else {
            size_t lenAligned = ALIGN32(len);
            char *stringMem = this->data.MemPtr;

            if (this->Capacity < lenAligned)
            {
                stringMem = (char *)stringAlloc(lenAligned);
                if (!this->IsSSO)
                    stringFree(this->data.MemPtr);
            }

            this->data.MemPtr = stringMem;
            this->Capacity = lenAligned;
            this->Len = len;
            this->IsSSO = false;
            SHU_MEMCOPY(str, this->data.MemPtr, len);
        }
    }

    void StringTest()
    {
        string s = "a";
        ASSERT(StringsEqual(s.c_str(), "a") && s.IsSmallString() && s.Length() == 1);
        s.Modify("b");
        ASSERT(StringsEqual(s.c_str(), "b") && s.IsSmallString() && s.Length() == 1);
        s.Modify("abcdefghi");
        ASSERT(StringsEqual(s.c_str(), "abcdefghi") && !s.IsSmallString() && s.Length() == 9);
        s.Modify("a");
        s.Append("b");
        ASSERT(StringsEqual(s.c_str(), "ab") && !s.IsSmallString() && s.Length() == 2);
        s.Append("c");
        ASSERT(StringsEqual(s.c_str(), "abc") && !s.IsSmallString() && s.Length() == 3);
        s.Append("d");
        ASSERT(StringsEqual(s.c_str(), "abcd") && !s.IsSmallString() && s.Length() == 4);
        s.Append("e");
        ASSERT(StringsEqual(s.c_str(), "abcde") && !s.IsSmallString() && s.Length() == 5);
        s.Append("f");
        ASSERT(StringsEqual(s.c_str(), "abcdef") && !s.IsSmallString() && s.Length() == 6);
        s.Append("g");
        ASSERT(StringsEqual(s.c_str(), "abcdefg") && !s.IsSmallString() && s.Length() == 7);
        // should be heap allocated here.
        s.Append("h");
        const char *heapStr = s.c_str();
        ASSERT(StringsEqual(heapStr, "abcdefgh") && !s.IsSmallString() && s.Length() == 8);
        s.Append("i");
        // the memory ptr does not change since s should have enough memory to append i.
        ASSERT(heapStr == s.c_str());
        ASSERT(StringsEqual(s.c_str(), "abcdefghi") && !s.IsSmallString() && s.Length() == 9);

        s.Modify("Mani");
        ASSERT(StringsEqual(s.c_str(), "Mani") && !s.IsSmallString() && s.Length() == 4);
        s.Append(" Is A Good Boy!");
        ASSERT(StringsEqual(s.c_str(), "Mani Is A Good Boy!") && !s.IsSmallString());

        int x = 0;
    }

    size_t
    string::Length() const noexcept
    {
        if (!this->data.MemPtr) return 0;
        size_t Result = this->Len - 1;
        return Result;
    }

    b32
    string::IsNullOrEmpty() const noexcept
    {
        if (!this->data.MemPtr)
            return true;

        b32 Result = (*this->data.MemPtr == '\0');
        return Result;
    }

    const char *
    string::c_str() const noexcept
    {
        return this->IsSSO ? this->data.SmallStringArr : this->data.MemPtr;
    }

    void
    string::Free()
    {
        if (this->data.MemPtr != nullptr)
        {
            stringFree(this->data.MemPtr);
        }

        this->data.MemPtr = nullptr;
        this->Capacity = 0;
    }

    size_t
    StringLength(const char *A)
    {
        if (A == nullptr)
            return 0;

        size_t Count = 0;
        const char *Ptr = A;
        while (*Ptr++ != '\0')
        {
            ++Count;
        }
        return (Count + 1);
    }

    void
    StringConcat(const char *A, const char *B, char *Out)
    {
        u32 Index = 0;
        while (A[Index] != '\0')
        {
            Out[Index] = A[Index];
            ++Index;
        }

        u32 sIndex = 0;
        while (B[sIndex] != '\0')
        {
            Out[Index++] = B[sIndex++];
        }

        Out[Index] = '\0';
    }

    void
    StringConcat(char *A, const char *B)
    {
        u32 Index = 0;

        u32 sIndex = 0;
        while (B[sIndex] != '\0')
        {
            A[Index++] = B[sIndex++];
        }
    }

    void
    StringNCopy(const char *Src, char *Dst, i32 Num)
    {
        i32 Index = 0;
        for (; Index < Num; ++Index)
        {
            Dst[Index] = Src[Index];
        }

        Dst[Index] = '\0';
    }

    void
    StringCopy(const char *Src, char *Dst)
    {
        u32 Index = 0;
        while (Src[Index] != '\0')
        {
            Dst[Index] = Src[Index];
            Index++;
        }
        Dst[Index] = '\0';
    }

    i32
    StringFindLastOf(const char *String, char Separator)
    {
        size_t StrLen = StringLength(String) - 1;

        while (String[StrLen] != Separator && StrLen >= 0)
        {
            --StrLen;
        }

        return StrLen;
    }

    void
    StringSubString(const char *SrcString, size_t StartIndex, size_t EndIndex, char *OutString)
    {
        ASSERT(EndIndex > StartIndex);
        for (size_t Index = StartIndex; Index < EndIndex; ++Index)
        {
            OutString[Index - StartIndex] = SrcString[Index];
        }

        OutString[EndIndex - StartIndex] = '\0';
    }

    b32
    StringLowerCaseEqual(const char *a, const char *b)
    {
        if (!a || !b)
            return false;

        i32 m = 0;
        const char *c = a;
        while (*a != '\0')
        {
            char ca = ToLower(*a);
            char cb = ToLower(*b);

            m |= ca ^ cb;

            if (*b != '\0')
                b++;

            // NOTE: To keep the function constant time.
            if (*b == '\0')
                c++;

            a++;
        }

        b32 Result = (m == 0);
        return Result;
    }

    b32
    StringsEqual(const char *a, const char *b)
    {
        if (!a || !b)
            return false;

        i32 m = 0;
        const char *c = a;
        while (*a != '\0')
        {
            m |= *a ^ *b;

            if (*b != '\0')
                b++;

            // NOTE: To keep the function constant time.
            if (*b == '\0')
                c++;

            a++;
        }

        b32 Result = (m == 0);
        return Result;
    }

    b32
    StringToBoolean(const char *str)
    {
        ASSERT(str != nullptr);

        b32 result = true;

        if (StringsEqual(str, "false"))
            result = false;

        return result;
    }

    i32
    StringToInteger(const char *str)
    {
        i32 sign = 1;
        i32 ans = 0;
        i32 i = 0;

        while (str[i] == ' ')
        {
            i++;
        }

        if ((str[i] == '-' || str[i] == '+'))
        {
            sign = (str[i] == '-') ? -1 : 1;
            i++;
        }

        i32 result;
        while (str[i] != '\0')
        {
            if (IsDigit(str[i]))
            {
                ans = ans * 10 + (str[i] - '0');
                if (ans * sign <= SHU_INT_MIN)
                    return SHU_INT_MIN;
                if (ans * sign >= SHU_INT_MAX)
                    return SHU_INT_MAX;
                i++;
            }
            else
            {
                result = 0;
            }
        }

        result = ans * sign;

    #if TEST_STR_FUNCTIONS
        i32 atoi_test = atoi(str);
        ASSERT(atoi_test == result);
    #endif

        return result;
    }

    f64
    StringToDouble(const char *str)
    {
    #if 0
        double result = 0.0;
        double fraction = 1.0;
        b32 negative = false;
        b32 in_fraction = false;
        b32 in_exponent = false;
        i32 exponent_sign = 1;
        i32 exponent_value = 0;

        i32 i = 0;

        // Skip leading whitespaces
        while (str[i] == ' ') { i++; }

        // Handle optional sign
        if (str[i] == '-') {
            negative = true;
            i++;
        }
        else if (str[i] == '+') {
            i++;
        }

        // Process the digits and decimal part
        while (str[i] != '\0')
        {
            if (IsDigit(str[i])) {
                if (in_exponent) {
                    exponent_value = exponent_value * 10 + (str[i] - '0');
                }
                else if (in_fraction) {
                    fraction /= 10.0;
                    result += (str[i] - '0') * fraction;
                }
                else {
                    result = result * 10.0 + (str[i] - '0');
                }
            }
            else if (str[i] == '.' && !in_fraction && !in_exponent) {
                in_fraction = true;
            }
            else if ((str[i] == 'e' || str[i] == 'E') && !in_exponent) {
                in_exponent = true;
                i++;
                if (str[i] == '-')
                {
                    exponent_sign = -1;
                    i++;
                }
                else if (str[i] == '+')
                {
                    i++;
                }
                continue;
            }
            else {
                // Invalid character found
                break;
            }

            i++;
        }

        // Apply exponent if necessary
        if (in_exponent) {
            double exponent_multiplier = 1.0;

            while (exponent_value > 0) {
                exponent_multiplier *= 10.0;
                exponent_value--;
            }

            if (exponent_sign == -1) {
                result /= exponent_multiplier;
            }
            else {
                result *= exponent_multiplier;
            }
        }

        if (negative) { result = -result; }

    #if TEST_STR_FUNCTIONS
        char *end;
        double test = strtod(str, &end);
        ASSERT(test == result);
    #endif
    #else
        // TODO: Have to check for rounding errors. 1.213 gets parsed to 1.21299999999
        // TODO: strtod returns 1.21300001 which is better
        char *end;
        f64 result = strtod(str, &end);
    #endif

        return result;
    }

    b32
    StringIsEmpty(const char *str)
    {
        b32 result = false;

        if (StringLength(str) == 1 && str[0] == '\0')
            result = true;

        return result;
    }

    char
    ToLower(char c)
    {
        char Result = c;
        if (Result >= 'A' && Result <= 'Z')
            Result += 32;
        return Result;
    }

    char *
    StringDuplicateMalloc(const char *In)
    {
        i32 len = StringLength((const char *)In);
        char *Duplicate = (char *)malloc(len);
        SHU_MEMCOPY(In, Duplicate, len);
        return Duplicate;
    }

    b32
    IsDigit(char c)
    {
        b32 result = false;

        if (c >= '0' && c <= '9')
            result = true;

        return result;
    }

    b32
    IsAlpha(char c)
    {
        b32 result = ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));
        return result;
    }

    b32
    IsAlphaNumeric(char c)
    {
        b32 result = IsAlpha(c) || IsDigit(c);
        return result;
    }

    b32
    StringIsBoolean(const char *str)
    {
        ASSERT(str != nullptr);
        if (StringIsEmpty(str))
            return false;

        b32 result = false;
        if (StringsEqual(str, "false") || StringsEqual(str, "true"))
            result = true;

        return result;
    }

    b32
    StringIsInt(const char *str)
    {
        ASSERT(str != nullptr);
        if (StringIsEmpty(str))
            return false;

        const char *p = str;
        if (*p == '-' || *p == '+')
            ++p;

        while (*p != '\0')
        {
            if (!IsDigit(*p++))
                return false;
        }

        return true;
    }

    b32
    StringIsDouble(const char *str)
    {
        ASSERT(str != nullptr);
        if (StringIsEmpty(str))
            return false;

        b32 in_exponent = false;
        b32 in_fraction = false;

        b32 result = true;
        const char *p = str;
        if (*p == '-' || *p == '+')
            ++p;

        while (*p != '\0')
        {
            char c = *p++;

            if (!IsDigit(c))
            {
                if ((c == 'e' || c == 'E'))
                {
                    // two e's isnt allowed.
                    if (in_exponent)
                    {
                        result = false;
                        break;
                    }

                    in_exponent = true;
                    continue;
                }

                if (c == '.')
                {
                    // Two decimal dots are not allowed.
                    if (in_fraction)
                    {
                        result = false;
                        break;
                    }

                    in_fraction = true;
                    continue;
                }

                result = false;
                break;
            }
        }

        return result;
    }

    char
    CharAt(const char *str, i32 at)
    {
        char result = str[at];
        return result;
    }
} // namespace shu