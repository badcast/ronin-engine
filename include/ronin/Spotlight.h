#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Spotlight final : public Light
    {
    public:
        // Диапозон
        float range;
        // Интенсивность света
        float intensity;

        Spotlight();
        Spotlight(const std::string& name);
        ~Spotlight();

    protected:
        virtual void get_light();
    };
} // namespace RoninEngine::Runtime
