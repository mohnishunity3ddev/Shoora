#include "string_utils.h"

namespace shu
{
    string::~string()
    {
        if (this->Data) stringFree(this->Data);
        this->Data = nullptr;
        this->Capacity = 0;
    }

    string::string()
    {
        ASSERT(!this->Data);
        this->Data = nullptr;
        this->Capacity = 0;
    }

    string::string(size_t InitialCapacity)
    {
        ASSERT(!this->Data && InitialCapacity > 0);
        size_t len = ALIGN32(InitialCapacity);
        
        this->Data = (char *)stringAlloc(len);
        this->Capacity = len;
        *this->Data = '\0';
    }

    string::string(const char *d, size_t InitialCapacity)
    {
        ASSERT(!this->Data);

        size_t len = MAX(InitialCapacity, StringLength(d));
        len = ALIGN32(len);
        this->Data = (char *)stringAlloc(len);
        this->Capacity = len;

        size_t index = 0;
        while (*d != '\0')
        {
            this->Data[index++] = *d++;
        }
        this->Data[index] = '\0';
    }

    string::string(const string &other)
    {
        size_t len = StringLength(other.Data);
        len = ALIGN32(len);

        if (!this->Data)
        {
            this->Data = (char *)stringAlloc(len);
            this->Capacity = len;
        }

        if (this->Capacity < len)
        {
            if (this->Data)
                stringFree(this->Data);

            this->Data = (char *)stringAlloc(len);
            this->Capacity = len;
        }

        SHU_MEMCOPY(other.Data, this->Data, len);
    }

    string::string(const char *d)
    {
        ASSERT(!this->Data);

        size_t len = StringLength(d);
        len = ALIGN32(32);

        this->Data = (char *)stringAlloc(len);
        this->Capacity = len;

        size_t index = 0;
        while (*d != '\0')
        {
            this->Data[index++] = *d++;
        }
        this->Data[index] = '\0';
    }

    string &
    string::operator=(const char *cString)
    {
        size_t len = StringLength(cString);
        len = ALIGN32(len);

        if (!this->Data)
        {
            this->Data = (char *)stringAlloc(len);
            this->Capacity = len;
        }

        if (this->Capacity < len)
        {
            if (this->Data)
                stringFree(this->Data);

            this->Data = (char *)stringAlloc(len);
            this->Capacity = len;
        }

        SHU_MEMCOPY(cString, this->Data, len);
        return *this;
    }

    string &
    string::operator=(const string &other)
    {
        if (&other != this)
        {
            if (other.Data && other.Capacity > 0)
            {
                size_t strlen = StringLength(other.Data);
                strlen = ALIGN32(strlen);
                if (!this->Data)
                {
                    this->Data = (char *)stringAlloc(strlen);
                    this->Capacity = strlen;
                }

                if (this->Capacity < strlen)
                {
                    if (this->Data)
                        stringFree(this->Data);

                    this->Data = (char *)stringAlloc(strlen);
                    this->Capacity = strlen;
                }

                SHU_MEMCOPY(other.Data, this->Data, strlen);
            }
        }

        return *this;
    }

    string &
    string::operator=(string &&other) noexcept
    {
        if (other.Data && other.Capacity > 0)
        {
            size_t strlen = StringLength(other.Data);
            strlen = ALIGN32(strlen);
            if (!this->Data)
            {
                this->Data = (char *)stringAlloc(strlen);
                this->Capacity = strlen;
            }

            if (this->Capacity < strlen)
            {
                if (this->Data)
                    stringFree(this->Data);

                this->Data = (char *)stringAlloc(strlen);
                this->Capacity = strlen;
            }

            SHU_MEMCOPY(other.Data, this->Data, strlen);

            stringFree(other.Data);
            other.Data = nullptr;
            other.Capacity = 0;
        }

        return *this;
    }

    string &
    string::operator+=(const char *str)
    {
        this->Append(str);
        return *this;
    }

    string &
    string::Append(const char *String)
    {
        i32 lenA = StringLength(this->Data) - 1;
        lenA = MAX(lenA, 0);
        i32 lenB = StringLength(String) - 1;
        i32 totalLength = lenA + lenB + 1;

        if (this->Capacity < totalLength)
        {
            char *newLoc = (char *)stringAlloc(totalLength);
            if (lenA > 0)
                SHU_MEMCOPY(this->Data, newLoc, lenA);

            if (this->Data)
            {
                stringFree(this->Data);
            }
            this->Data = newLoc;
            this->Capacity = totalLength;
        }

        SHU_MEMCOPY(String, this->Data + lenA, lenB);
        this->Data[lenA + lenB] = '\0';

        return *this;
    }

    string &
    string::Append(const char *String, size_t Length)
    {
        i32 lenA = StringLength(this->Data) - 1;
        lenA = MAX(lenA, 0);

        i32 lenB = StringLength(String) - 1;
        i32 totalLength = lenA + lenB + 1;
        ASSERT(totalLength <= Length);

        if (this->Capacity < Length)
        {
            char *newLoc = (char *)stringAlloc(Length);
            SHU_MEMCOPY(this->Data, newLoc, lenA);
            if (this->Data)
            {
                stringFree(this->Data);
            }
            this->Data = newLoc;
            this->Capacity = Length;
        }

        SHU_MEMCOPY(String, this->Data + lenA, lenB);
        this->Data[lenA + lenB] = '\0';

        return *this;
    }

    size_t
    string::Length() const noexcept
    {
        if (!this->Data)
            return 0;

        size_t Length = StringLength(this->Data);
        return Length;
    }

    b32
    string::IsNullOrEmpty() const noexcept
    {
        if (!this->Data)
            return true;

        b32 Result = (*this->Data == '\0');
        return Result;
    }

    const char *
    string::c_str() const noexcept
    {
        return this->Data;
    }

    void
    string::Free()
    {
        if (this->Data != nullptr)
        {
            stringFree(this->Data);
        }

        this->Data = nullptr;
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