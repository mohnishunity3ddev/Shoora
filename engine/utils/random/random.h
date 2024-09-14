#ifndef SHOORA_RANDOM_H
#define SHOORA_RANDOM_H

#include <defines.h>
#if IMPLEMENT_STD_RANDOM
    #include <random>
    #include <chrono>

    struct shoora_random_std
    {
      private:
        std::mt19937 MT;
        // uniform distribution for integers
        std::uniform_int_distribution<u32> UniformDistribution;
        // uniform distribution for floats
        std::uniform_real_distribution<f32> NormalDistribution;

      public:
        shoora_random_std();
        shoora_random_std(u32 RandomSeed);

        static u32 GetRandomSeed();
        u32 NextU32();
        f32 R01();
        f32 Bilateral();
        u32 Between(u32 Min, u32 Max);
        f32 Between(f32 Min, f32 Max);
    };

    void RandomTestSTD();
#endif

#define MaxRandomNumber 0x05f5c21f
#define MinRandomNumber 0x000025a0

    struct shoora_random
    {
        u32 Index;

        shoora_random();
        shoora_random(u32 RandomSeed);

        u32 GetSeed();
        u32 NextU32();
        f32 R01();
        f32 Bilateral();
        u32 Between(u32 Min, u32 Max);
        f32 Between(f32 Min, f32 Max);
    };

    // void RandomTest();

#endif
