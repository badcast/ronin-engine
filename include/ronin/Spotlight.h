#pragma once
#include "Light.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Spotlight final : public Light
    {
    public:
        // Cone angle in degrees (default 60)
        float angle;

        Spotlight();
        Spotlight(const std::string &name);
        Spotlight(const Spotlight &other);
        ~Spotlight();

    protected:
        void get_light() override;
        void get_light(Rendering *render_info) override;
    };
} // namespace RoninEngine::Runtime
