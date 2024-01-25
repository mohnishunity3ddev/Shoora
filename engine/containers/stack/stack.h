#if !defined(STACK_H)

#include <defines.h>
#include <platform/platform.h>
#include <memory.h>


template <typename T>
struct stack
{
  private:
    T *items;
    i32 top = -1;
    i32 capacity = 0;
    i32 size = 0;
    T invalidValue;

  public:
    stack(i32 reserveCapacity, T invalidVal)
    {
        items = new T[reserveCapacity];
        this->invalidValue = invalidVal;
        top = -1;
        size = 0;
        capacity = reserveCapacity;
    }

    stack(T invalidVal) : stack(1, invalidVal) {}

    void
    push(T item)
    {
        if ((size + 1) > capacity)
        {
            T *memory = new T[capacity * 2];
            memcpy(memory, items, sizeof(T) * size);
            delete[] items;
            items = memory;
            capacity *= 2;
        }
        items[++top] = item;
        size += 1;
    }

    T
    peek(int i = 0)
    {
        ASSERT(i <= 0);
        if(isEmpty())
        {
            return invalidValue;
        }
        if(top + i <= -1) {
            ASSERT(!"Stack does not contain that many elements!");
            return invalidValue;
        }

        T result = items[top + i];
        return result;
    }

    bool
    isEmpty()
    {
        bool result = top == -1;
        return result;
    }

    T
    pop(bool displayMsg = true)
    {
        if (isEmpty())
        {
            if (displayMsg)
            {
                LogInfoUnformatted("The Stack is empty!\n");
            }
            return invalidValue;
        }
        T result = items[top--];
        size--;
        return result;
    }

    void
    display()
    {
        // TODO: Circumvent this.
        LogInfoUnformatted("Sorry, Cannot display stack right now since this is a template!\n");
        // LogInfoUnformatted("The stack is: \n");
        T *curr = items + top;
        for (i32 i = 0; i < size; ++i)
        {
            // LogInfo("%")
            // std::cout << *(curr--) << "\n";
        }
    }

    i32
    getSize()
    {
        return size;
    }

    ~stack()
    {
        delete[] items;
        items = nullptr;
        size = 0;
        capacity = 0;
        top = -1;
        LogInfoUnformatted("Stack Destructor called!");
    }
};

#define STACK_H
#endif // STACK_H