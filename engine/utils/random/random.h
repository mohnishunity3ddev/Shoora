#if !defined(RANDOM_H)

#include <defines.h>

namespace shu
{
    struct rand
    {
      public:
        rand();
        rand(u32 SeedValue);

        void Seed(u32 SeedValue);

        f32 Range01();
        f32 RangeMinus1To1();

        f32 RangeBetweenF32(f32 Low, f32 High);
        i32 RangeBetweenInt32(i32 Low, i32 High);
        u32 RangeBetweenUInt32(u32 Low, u32 High);

        u32 NextUInt32();
        i32 NextInt32();

      private:
        u32 Index;

        u32 GetNext();
    };

    // void getInfo();
}

#define RANDOM_H
#endif