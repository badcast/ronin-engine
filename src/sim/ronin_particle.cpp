#include "ronin.h"
#include "basic_ronin.h"

namespace RoninEngine::Runtime
{

    enum ParticleState
    {
        BEGIN_STATE = 0,
        SIMULATE_STATE = 1,
        END_STATE = 2
    };

    struct ParticleDrain
    {
        SpriteRendererRef render{};
        float initTime;
        Vec2 direction;

        ParticleDrain() = default;

        ParticleDrain(SpriteRendererRef _render, float _initTime, Vec2 _direction) : render(_render), initTime(_initTime), direction(_direction) {}
    };

    bool operator<(const ParticleDrain &lhs, const ParticleDrain &rhs)
    {
        return lhs.render.ptr_ < rhs.render.ptr_;
    }

    struct ParticleSystemImpl
    {
        // TODO: Optimize here for Particle member (Contain in to ronin_particles*.cpp
        std::set<ParticleDrain> activeParticleDrains;
        std::vector<SpriteRendererRef> cachedParticles;
        float m_timing;
        int m_maked;
        int m_lastInspected = 0;
        int m_limit = 0;

               // bool:colorable - if true simulate it
        bool colorable = true;
        Color startColor = Color::transparent;
        Color centerColor = Color::white;
        Color endColor = Color::transparent;

               // bool:scalable - if true simulate it
        bool scalable = true;
        Vec2 startSize = Vec2::zero;
        Vec2 centerSize = Vec2::one;
        Vec2 endSize = Vec2::zero;

               // Lifetime for particle
        float m_duration = 10;
        float m_durationStartRange = 0.1f; // Range [0.0,1.0]
        float m_durationEndRange = 0.1f;   // Range [0.0,1.0]

        void link_particles(ParticleSystem* from, int n);
        void unlink_particle(ParticleSystem* from, const ParticleDrain *drain);
    };

    inline SpriteRef inspect_at(const std::vector<SpriteRef> &sources, int &last_inspected, ParticleSourceInspect inspection)
    {
        SpriteRef inspected;
        if(!sources.empty())
        {
            switch(inspection)
            {
                case ParticleSourceInspect::InspectNext:
                    last_inspected = static_cast<int>((last_inspected + 1) % sources.size());
                    break;
                case ParticleSourceInspect::InspectRandom:
                    last_inspected = Random::Range(0, sources.size());
                    break;
            }

            inspected = sources[last_inspected];
        }
        return inspected;
    }

    void ParticleSystemImpl::link_particles(ParticleSystem* from, int n)
    {
        std::string particle_name {};

               // lambda method
        auto fabricate = [&](SpriteRendererRef spriteRender)
        {
            TransformRef linked = spriteRender->transform();
            linked->layer(from->transform()->layer());
            linked->_owner->m_zOrder = from->_owner->m_zOrder;
            linked->position(from->transform()->position());
            if(from->worldSpace)
            {
                linked->setParent(nullptr);
            }
            else
            {
                linked->setParent(from->transform(), false);
                linked->position(from->transform()->position());
            }

            if(from->rotate)
                linked->angle(Random::AngleDegress());
            else if(linked->angle())
                linked->angle(0);

                   // Set Sprite Source
            SpriteRef targetSprte = inspect_at(from->m_sources, m_lastInspected, from->m_sourceInspect);
            if(spriteRender->getSprite() != targetSprte)
                spriteRender->setSprite(targetSprte);

                   // Set Start Size
            spriteRender->setSize(startSize);

                   // Set Start Color
            spriteRender->setColor(startColor);

            activeParticleDrains.emplace(spriteRender, Time::time(), (from->randomDirection ? Random::RandomVector() : from->direction));
        };

               // use existences
        if(from->reserving)
        {
            while(!cachedParticles.empty() && n-- > 0)
            {
                SpriteRendererRef exist = cachedParticles.back();
                cachedParticles.pop_back();
                // showing
                exist->gameObject()->SetActive(true);
                fabricate(exist);
            }
        }
        else if(!cachedParticles.empty())
        {
            from->ClearReserved();
        }

               // make new
        for(; n-- > 0;)
        {
            SpriteRendererRef spriteRender = Primitive::CreateEmptyGameObject()->AddComponent<SpriteRenderer>();
            particle_name = spriteRender->gameObject()->name();
            particle_name += " # ";
            particle_name += std::to_string(n);
            spriteRender->gameObject()->name(particle_name);
            spriteRender->AddOnDestroy(
                [this](ComponentRef comp)
                {
                    // Remove childs

                    ParticleDrain drain {};
                    drain.render = comp->GetComponent<SpriteRenderer>();
                    auto iter = this->activeParticleDrains.find(drain);
                    if(iter != std::end(this->activeParticleDrains))
                    {
                        this->activeParticleDrains.erase(iter);
                    }
                });
            fabricate(spriteRender);
        }
    }

    void ParticleSystemImpl::unlink_particle(ParticleSystem* from, const ParticleDrain *drain)
    {
        bool destroyToo = from->reserving == false;

        if(destroyToo)
        {
            drain->render->gameObject()->Destroy();
        }
        else
        {
            drain->render->gameObject()->SetActive(false);
            // State clear render cache
            drain->render->setSprite(nullptr);
            cachedParticles.emplace_back(drain->render);
        }
    }

    ParticleSystem::ParticleSystem() : ParticleSystem("Particle System")
    {
    }

    ParticleSystem::ParticleSystem(const std::string &name) : Behaviour(name), m_sourceInspect(ParticleSourceInspect::InspectNext)
    {
        RoninMemory::alloc_self(impl);
    }

    ParticleSystem::~ParticleSystem()
    {
        RoninMemory::free(impl);
    }

    void ParticleSystem::setSource(SpriteRef source)
    {
        m_sources.clear();
        m_sources.push_back(source);
    }

    void ParticleSystem::setSources(SpriteRef src1, SpriteRef src2, ParticleSourceInspect inspectType)
    {
        m_sources.clear();
        m_sourceInspect = inspectType;
        m_sources.push_back(src1);
        m_sources.push_back(src2);
    }

    void ParticleSystem::setSources(SpriteRef src1, SpriteRef src2, SpriteRef src3, ParticleSourceInspect inspectType)
    {
        m_sources.clear();
        m_sourceInspect = inspectType;
        m_sources.push_back(src1);
        m_sources.push_back(src2);
        m_sources.push_back(src3);
    }

    void ParticleSystem::OnDestroy()
    {
        Reset();
    }

    void ParticleSystem::OnAwake()
    {
        setInterpolates(impl->m_duration, impl->m_durationStartRange, impl->m_durationEndRange);
        impl->m_maked = 0;
        impl->m_lastInspected = 0;
    }

    void ParticleSystem::OnStart()
    {
        if(delay > 0)
            impl->m_timing = delay + Time::time();
        else
            impl->m_timing = 0;
    }

    template <typename Param>
    constexpr Param &InterpolationValue(int state, Param &begin, Param &any, Param &end)
    {
        switch(static_cast<ParticleState>(state))
        {
            case BEGIN_STATE:
                return begin;
            case SIMULATE_STATE:
                return any;
            case END_STATE:
            default:
                return end;
        }
    }

    void ParticleSystem::OnUpdate()
    {
        enum
        {
            M_DestroyNow = 0,
            M_Execute = 1,
            M_Fabricate = 2,
        };

        int makeFlag = (loop || !impl->activeParticleDrains.empty()) ? M_Execute : 0;
        if(loop && impl->m_limit == 0 || impl->m_maked < impl->m_limit || impl->m_maked < startWith)
        {
            makeFlag |= M_Fabricate;
        }
        if(makeFlag)
        {
            float t = Time::time();
            if(emit && impl->m_timing <= t && (makeFlag & M_Fabricate))
            {
                int make_n = impl->m_limit == 0 ? startWith : Math::Min(startWith, static_cast<int>(impl->m_limit - impl->activeParticleDrains.size()));
                impl->link_particles(this, make_n);
                impl->m_maked += make_n;

                       // update interval
                impl->m_timing = t + interval;
            }

                   // Update existing particles (drains)
            for(std::set<ParticleDrain>::iterator drain = std::begin(impl->activeParticleDrains); drain != std::end(impl->activeParticleDrains); ++drain)
            {
                if(drain->render->isNull())
                    continue;
                float drain_lifetime = t - drain->initTime;
                if(drain_lifetime >= impl->m_duration)
                {
                    impl->unlink_particle(this, &(*drain));
                    continue;
                }

                float drain_speed = Time::deltaTime() * speed;

                float state_percentage;
                int interpolateBegin, interpolateEnd;

                if(drain_lifetime <= impl->m_durationStartRange)
                {
                    interpolateBegin = BEGIN_STATE;
                    state_percentage = drain_lifetime / impl->m_durationStartRange;
                }
                else if(drain_lifetime >= impl->m_durationEndRange)
                {
                    interpolateBegin = END_STATE;
                    state_percentage = drain_lifetime / impl->m_durationEndRange;
                }
                else
                {
                    interpolateBegin = SIMULATE_STATE;
                    state_percentage = drain_lifetime / impl->m_duration;
                }

                interpolateEnd = Math::Min<int>(Math::Max<int>(interpolateBegin, interpolateBegin + 1), END_STATE);

                       // Colorize
                if(impl->colorable)
                {
                    Color interpolate_from = InterpolationValue(interpolateBegin, impl->startColor, impl->centerColor, impl->endColor);
                    Color interpolate_to = InterpolationValue(interpolateEnd, impl->startColor, impl->centerColor, impl->endColor);
                    drain->render->setColor(Color::Lerp(interpolate_from, interpolate_to, state_percentage));
                }

                       // Scaling
                if(impl->scalable)
                {
                    Vec2 interpolate_from = InterpolationValue(interpolateBegin, impl->startSize, impl->centerSize, impl->endSize);
                    Vec2 interpolate_to = InterpolationValue(interpolateEnd, impl->startSize, impl->centerSize, impl->endSize);
                    drain->render->setSize(Vec2::Lerp(interpolate_from, interpolate_to, state_percentage));
                }

                       // Move, rotate
                TransformRef particleTransform = drain->render->transform();
                particleTransform->Translate(drain->direction * drain_speed);

                if(rotate)
                {
                    particleTransform->angle(particleTransform->angle() + rotatePerFrame * drain_speed);
                }
            }

            ParticleDrain drained;
            for(SpriteRendererRef &reserved : impl->cachedParticles)
            {
                drained.render = reserved;
                impl->activeParticleDrains.erase(drained);
            }
        }
        else if(destroyAfter)
        {
            gameObject()->Destroy();
        }
    }

    void ParticleSystem::setInterpolates(float duration)
    {
        setInterpolates(duration, 0.1f, 0.1f);
    }

    void ParticleSystem::setInterpolates(float duration, float startRange, float endRange)
    {
        impl->m_duration = Math::Max(duration, 0.f);
        impl->m_durationStartRange = impl->m_duration * Math::Clamp01(startRange);
        impl->m_durationEndRange = impl->m_duration - impl->m_duration * Math::Clamp01(endRange);
    }

    void ParticleSystem::setColor(Color color)
    {
        impl->colorable = false;
        impl->startColor = color;
    }

    void ParticleSystem::setColors(Color startState, Color endState)
    {
        setColors(startState, Color::Lerp(startState, endState, 1.f / 2), endState);
    }

    void ParticleSystem::setColors(Color startState, Color centerState, Color endState)
    {
        impl->colorable = true;
        impl->startColor = startState;
        impl->centerColor = centerState;
        impl->endColor = endState;
    }

    void ParticleSystem::setSize(Vec2 size)
    {
        impl->scalable = false;
        impl->startSize = size;
    }

    void ParticleSystem::setSizes(Vec2 startState, Vec2 endState)
    {
        setSizes(startState, Vec2::Lerp(startState, endState, .5f), endState);
    }

    void ParticleSystem::setSizes(Vec2 startState, Vec2 centerState, Vec2 endState)
    {
        impl->scalable = true;
        impl->startSize = startState;
        impl->centerSize = centerState;
        impl->endSize = endState;
    }

    float ParticleSystem::getDuration()
    {
        return impl->m_duration;
    }

    int ParticleSystem::getActiveCount()
    {
        return static_cast<int>(impl->activeParticleDrains.size());
    }

    ParticleSystemState ParticleSystem::getState()
    {
        return ParticleSystemState(((loop || !impl->activeParticleDrains.empty()) ? ParticleSystemState::Executable : 0) | ((impl->m_maked < impl->m_limit || impl->m_maked < startWith) ? ParticleSystemState::Fabricatable : 0));
    }

    void ParticleSystem::setLimitInfinitely()
    {
        setLimit(0);
    }

    void ParticleSystem::setLimit(int max)
    {
        if(max < 0)
            return;

        impl->m_limit = max;
    }

    void ParticleSystem::Reset()
    {
        ClearReserved();

        for(const ParticleDrain &drain : impl->activeParticleDrains)
        {
            drain.render->ClearOnDestroy();
            drain.render->gameObject()->Destroy(); // destroy now
        }

        impl->activeParticleDrains.clear();
        impl->m_maked = 0;
        impl->m_timing = 0;
        impl->m_lastInspected = 0;
    }

    void ParticleSystem::ClearReserved()
    {
        for(SpriteRendererRef &drain : impl->cachedParticles)
        {
            if(drain.isNull())
                continue;
            drain->ClearOnDestroy();
            drain->gameObject()->Destroy(); // destroy now
        }
        impl->cachedParticles.clear();
        impl->cachedParticles.shrink_to_fit();
    }
}
