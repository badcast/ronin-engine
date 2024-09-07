#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Spotlight final : public Light
    {
    public:
        // Range of the light (world space)
        float range;
        // Intensity light
        float intensity;

        Spotlight();
        Spotlight(const std::string &name);
        ~Spotlight();

    protected:
        virtual void get_light();
    };
} // namespace RoninEngine::Runtime
