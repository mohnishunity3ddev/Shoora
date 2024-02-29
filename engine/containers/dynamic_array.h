#if !defined(DYNAMIC_ARRAY_H)

#include <defines.h>
#include <memory.h>
#include <platform/platform.h>
#include <utility>

template<typename T>
struct shoora_dynamic_array
{
  private:
    T *arr;
    i32 Size = 0;
    i32 Capacity = 0;

  public:
    shoora_dynamic_array()
    {
        this->Capacity = 0;
        Size = 0;
        this->arr = nullptr;
    }

    shoora_dynamic_array(const shoora_dynamic_array<T> &Rhs)
    {
        this->Capacity = Rhs.Capacity;
        Size = Rhs.Size;
        this->arr = new T[this->Capacity];
        for(i32 i = 0; i < Size; ++i)
        {
            this->arr[i] = Rhs[i];
        }
    }

    shoora_dynamic_array(shoora_dynamic_array<T> &&Rhs)
    {
        this->Capacity = Rhs.Capacity;
        this->Size = Rhs.Size;
        this->arr = Rhs.arr;

        Rhs.Capacity = 0;
        Rhs.Size = 0;
        Rhs.arr = nullptr;
    }

    shoora_dynamic_array &
    operator=(const shoora_dynamic_array<T> &Rhs)
    {
        if(this->arr) { delete[] this->arr; }

        this->Capacity = Rhs.Capacity;
        Size = Rhs.Size;
        this->arr = new T[this->Capacity];
        for(i32 i = 0; i < Size; ++i)
        {
            this->arr[i] = Rhs[i];
        }
        return *this;
    }

    shoora_dynamic_array &
    operator=(shoora_dynamic_array<T> &&Rhs)
    {
        this->Capacity = Rhs.Capacity;
        Size = Rhs.Size;
        this->arr = Rhs.arr;

        Rhs.Capacity = 0;
        Rhs.Size = 0;
        Rhs.arr = nullptr;

        return *this;
    }

    shoora_dynamic_array(i32 ReserveCapacity) { reserve(ReserveCapacity); }
    ~shoora_dynamic_array()
    {
        // LogFatalUnformatted("Dynamic array destructor called!\n");
        if(arr != nullptr)
        {
            delete[] arr;
        }
        this->Capacity = 0;
        this->Size = 0;
    }

    inline void
    reserve(i32 capacity)
    {
        ASSERT(this->Capacity == 0 && this->Size == 0);

        this->Capacity = capacity;
        arr = new T[this->Capacity];
        Size = 0;

        this->Clear();
    }

    inline i32 size() const { return Size; }
    inline i32 capacity() { return Capacity; }

    inline void
    Resize()
    {
        i32 NewCapacity = Capacity == 0 ? 1 : Capacity*2;
        T *NewArr = new T[NewCapacity];
        for (i32 i = 0; i < Size; ++i)
        {
            NewArr[i] = std::move(arr[i]);
        }
        delete[] arr;
        arr = NewArr;
        Capacity = NewCapacity;

        // LogWarn("Array has been resized to capacity: %d!\n", Capacity);
    }

    inline void
    push_back(T &&item)
    {
        if((Size + 1) >= Capacity)
        {
            Resize();
        }

        arr[Size++] = std::forward<T>(item);
    }

    inline void
    push_back(T &item)
    {
        if((Size + 1) >= Capacity)
        {
            Resize();
        }

        arr[Size++] = std::forward<T>(item);
    }

    template <typename... Args>
    inline void
    emplace_back(Args &&...args)
    {
        if ((Size + 1) > Capacity)
        {
            Resize();
        }

        // NOTE: does not create new memory. inserts whatever value into memory pointed by () operator.
        // below, std::forward calls the proper constructor(whether args was lvalue or rvalue)
        // the value passed is inserted into &arr[Size] memory. new here is used but it does not allocate
        // additional memory.
        new (&arr[Size++]) T(std::forward<Args>(args)...);
    }

    inline T&
    operator[](i32 Index) const
    {
        ASSERT(Index < Size);
        return arr[Index];
    }

    inline T *
    get(i32 Index)
    {
        ASSERT(Index < Size);
        T *Result = arr + Index;
        return Result;
    }

    inline void
    erase(i32 Index)
    {
        ASSERT(Index < Size);
        for(i32 i = Index; i < (Size-1); ++i)
        {
            SWAP(arr[i], arr[i + 1]);
        }
        --Size;
    }

    inline T*
    data() const
    {
        return arr;
    }

    inline void
    Clear()
    {
        ASSERT(Capacity >= Size);
        memset(arr, 0, sizeof(T) * Capacity);
    }
};

#define DYNAMIC_ARRAY_H
#endif