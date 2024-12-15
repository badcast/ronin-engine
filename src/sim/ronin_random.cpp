#include <immintrin.h>

#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

#ifdef __linux__
std::fstream _fileDevRand;
#endif

static std::uint32_t __seed__ = 1;
constexpr auto ronin_rand_max = std::numeric_limits<decltype(__seed__)>::max();

// get random value
std::uint32_t internal_rand_custom()
{
    __seed__ = (__seed__ * 73129u + 95121u) % ronin_rand_max;
    return __seed__;
}

std::uint32_t internal_rand_rdrnd()
{
    std::uint32_t random_val;

    if(!_rdrand32_step(&random_val))
    {
        // Have error, then generate custom random
        random_val = internal_rand_custom();
    }

    return random_val;
}

std::uint32_t internal_rand_devrand()
{
#ifdef __linux__
    std::uint32_t random_val;
    if(!_fileDevRand)
        _fileDevRand.open("/dev/urandom", std::ios_base::binary | std::ios_base::in);
    if(_fileDevRand)
    {
        _fileDevRand.read(reinterpret_cast<char *>(&random_val), sizeof(random_val));
        return random_val;
    }
#endif

    return internal_rand_custom();
}

std::function<std::uint32_t(void)> internalRandom = internal_rand_rdrnd;

void Random::SRand(std::uint32_t seed)
{
    if(seed == 0)
        seed = 1;
    __seed__ = seed;
}

int Random::Range(int min, int max)
{
    return min + internalRandom() % (max - min + 1);
}

float Random::Range(float min, float max)
{
    return Value() * (max - min) + min;
}

float Random::Value()
{
    // const for diaz. 0.0 - 1.0F
    return static_cast<float>(static_cast<double>(internalRandom()) / ronin_rand_max);
}

Vec2 Random::RandomVector()
{
    return {Random::Value() * 2 - 1, Random::Value() * 2 - 1};
}

float Random::AngleDegress()
{
    return Value() * 360;
}

float Random::AngleRadian()
{
    return Value() * Math::Pi2;
}

Color Random::color(){
    return Color(internalRandom(),255);
}
