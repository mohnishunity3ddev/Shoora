#if !defined(DYNAMIC_ARRAY_H)

#include <defines.h>
#include <memory.h>

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
        this->Capacity = 1;
        arr = new T[this->Capacity];
        Size = 0;
    }
    shoora_dynamic_array(i32 ReserveCapacity) { reserve(ReserveCapacity); }
    ~shoora_dynamic_array()
    {
        LogFatalUnformatted("Dynamic array destructor called!\n");
        ASSERT(arr != nullptr);
        delete[] arr;
    }

    inline void
    reserve(i32 capacity)
    {
        arr = new T[capacity];
        Size = 0;
        this->Capacity = capacity;
    }

    inline i32 size() { return Size; }
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

        LogWarn("Array has been resized to capacity: %d!\n", Capacity);
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

    template <typename... Args>
    inline void
    emplace_back(Args &&...args)
    {
        if ((Size + 1) >= Capacity)
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
    operator[](i32 Index)
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
    
    inline T*
    data()
    {
        return arr;
    }

    inline void
    Clear()
    {
        memset(arr, 0, sizeof(T) * Size);
    }
};

#define DYNAMIC_ARRAY_H
#endif