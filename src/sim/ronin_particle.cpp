#include "ronin.h"

using namespace RoninEngine::Runtime;

enum ParticleState
{
    BEGIN_STATE = 0,
    SIMULATE_STATE = 1,
    END_STATE = 2
};

namespace RoninEngine::Runtime
{
    bool operator<(const ParticleDrain &lhs, const ParticleDrain &rhs)
    {
        return lhs.render < rhs.render;
    }
} // namespace RoninEngine::Runtime

inline Sprite *inspect_at(const std::vector<Sprite *> &sources, int &last_inspected, ParticleSourceInspect inspection)
{
    Sprite *inspected;

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
    else
        inspected = nullptr;

    return inspected;
}

void ParticleSystemRef::link_particles(ParticleSystem *from, int n)
{
    std::string particle_name {};

    // lambda method
    auto fabricate = [&](SpriteRenderer *spriteRender)
    {
        Transform *linked = spriteRender->transform();
        linked->layer(from->transform()->layer());
        linked->position(from->transform()->position());
        if(from->worldSpace)
        {
            linked->setParent(nullptr);
        }
        else
        {
            linked->setParent(from->transform(), false);
        }

        if(from->rotate)
            linked->angle(Random::AngleDegress());
        else if(linked->angle())
            linked->angle(0);

        // Set Sprite Source
        Sprite *targetSprte = inspect_at(from->m_sources, m_lastInspected, from->m_sourceInspect);
        if(spriteRender->getSprite() != targetSprte)
            spriteRender->setSprite(targetSprte);

        // Set Start Size
        spriteRender->setSize(startSize);

        // Set Start Color
        spriteRender->setColor(startColor);

        activeParticles.insert({spriteRender, TimeEngine::time(), from->randomDirection ? Random::RandomVector() : from->direction});
    };

    // use existences
    if(from->reserving)
    {
        while(!cachedParticles.empty() && n-- > 0)
        {
            SpriteRenderer *exist = cachedParticles.back();
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
        SpriteRenderer *spriteRender = Primitive::CreateEmptyGameObject()->AddComponent<SpriteRenderer>();
        particle_name = spriteRender->gameObject()->name();
        particle_name += " # ";
        particle_name += std::to_string(n);
        spriteRender->gameObject()->name(particle_name);
        spriteRender->AddOnDestroy(
            [this](Component *comp)
            {
                // Remove old's

                ParticleDrain drain;
                drain.render = comp->GetComponent<SpriteRenderer>();
                auto iter = this->activeParticles.find(drain);
                if(iter != std::end(this->activeParticles))
                {
                    this->activeParticles.erase(iter);
                }
            });
        fabricate(spriteRender);
    }
}

void ParticleSystemRef::unlink_particle(ParticleSystem *from, const ParticleDrain *drain)
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
    RoninMemory::alloc_self(ref);
}

ParticleSystem::~ParticleSystem()
{
    RoninMemory::free(ref);
}

void ParticleSystem::setSource(Sprite *source)
{
    m_sources.clear();
    m_sources.push_back(source);
}

void ParticleSystem::setSources(Sprite *src1, Sprite *src2, ParticleSourceInspect inspectType)
{
    m_sources.clear();
    m_sourceInspect = inspectType;
    m_sources.push_back(src1);
    m_sources.push_back(src2);
}

void ParticleSystem::setSources(Sprite *src1, Sprite *src2, Sprite *src3, ParticleSourceInspect inspectType)
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
    setInterpolates(ref->m_duration, ref->m_durationStartRange, ref->m_durationEndRange);
    ref->m_maked = 0;
    ref->m_lastInspected = 0;
}

void ParticleSystem::OnStart()
{
    if(delay > 0)
        ref->m_timing = delay + TimeEngine::time();
    else
        ref->m_timing = 0;
}

template <typename Param>
constexpr const Param &InterpolationValue(int state, const Param &begin, const Param &any, const Param &end)
{
    switch(static_cast<ParticleState>(state))
    {
        case SIMULATE_STATE:
            return any;
        case BEGIN_STATE:
            return begin;
        case END_STATE:
        default:
            return end;
    }
}

void ParticleSystem::OnUpdate()
{
    // Create new particles if the "timing" time has come, or its creation has reached the end (maxParticles)
    // Update existing particles and perform interpolation for them

    enum
    {
        M_DestroyNow = 0, // Уничтожить эту систему частиц
        M_Execute = 1, // Выполнять в любом случаем, например частицы уже созданы и их нужно отрендерить и интерполировать
        M_Fabricate = 2, // Создать еще частицы
    };

    int makeFlag = (loop || !ref->activeParticles.empty()) ? M_Execute : 0;

    if(loop && ref->m_limit == 0 || ref->m_maked < ref->m_limit || ref->m_maked < startWith)
    {
        makeFlag |= M_Fabricate;
    }

    if(makeFlag)
    {
        float t = TimeEngine::time();
        // Make new particle (interval)
        if(emit && ref->m_timing <= t && (makeFlag & M_Fabricate))
        {
            int make_n = ref->m_limit == 0 ? startWith : Math::Min(startWith, static_cast<int>(ref->m_limit - ref->activeParticles.size()));
            ref->link_particles(this, make_n);
            ref->m_maked += make_n;

            // update interval
            ref->m_timing = t + interval;
        }

        // Update existing particles (drains)
        for(std::set<ParticleDrain>::iterator drain = std::begin(ref->activeParticles); drain != std::end(ref->activeParticles); ++drain)
        {
            float drain_lifetime = t - drain->initTime;
            if(drain_lifetime >= ref->m_duration)
            {
                ref->unlink_particle(this, &(*drain));
                continue;
            }

            float drain_speed = TimeEngine::deltaTime() * speed;

            float state_percentage;
            int interpolateBegin, interpolateEnd;

            if(drain_lifetime <= ref->m_durationStartRange)
            {
                interpolateBegin = BEGIN_STATE;
                state_percentage = drain_lifetime / ref->m_durationStartRange;
            }
            else if(drain_lifetime >= ref->m_durationEndRange)
            {
                interpolateBegin = END_STATE;
                state_percentage = drain_lifetime / ref->m_durationEndRange;
            }
            else
            {
                interpolateBegin = SIMULATE_STATE;
                state_percentage = drain_lifetime / ref->m_duration;
            }

            interpolateEnd = Math::Min<int>(Math::Max<int>(interpolateBegin, interpolateBegin + 1), END_STATE);

            // Colorize
            if(ref->colorable)
            {
                Color interpolate_from = InterpolationValue(interpolateBegin, ref->startColor, ref->centerColor, ref->endColor);
                Color interpolate_to = InterpolationValue(interpolateEnd, ref->startColor, ref->centerColor, ref->endColor);
                drain->render->setColor(Color::Lerp(interpolate_from, interpolate_to, state_percentage));
            }

            // Scaling
            if(ref->scalable)
            {
                Vec2 interpolate_from = InterpolationValue(interpolateBegin, ref->startSize, ref->centerSize, ref->endSize);
                Vec2 interpolate_to = InterpolationValue(interpolateEnd, ref->startSize, ref->centerSize, ref->endSize);
                drain->render->setSize(Vec2::Lerp(interpolate_from, interpolate_to, state_percentage));
            }

            // Move, rotate
            Transform *particleTransform = drain->render->transform();
            particleTransform->Translate(drain->direction * drain_speed);

            if(rotate)
            {
                particleTransform->angle(particleTransform->angle() + rotatePerFrame * drain_speed);
            }
        }

        ParticleDrain drained;
        for(SpriteRenderer *reserved : ref->cachedParticles)
        {
            drained.render = reserved;
            ref->activeParticles.erase(drained);
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
    ref->m_duration = Math::Max(duration, 0.f);
    ref->m_durationStartRange = ref->m_duration * Math::Clamp01(startRange);
    ref->m_durationEndRange = ref->m_duration - ref->m_duration * Math::Clamp01(endRange);
}

void ParticleSystem::setColor(Color color)
{
    ref->colorable = false;
    ref->startColor = color;
}

void ParticleSystem::setColors(Color startState, Color endState)
{
    setColors(startState, Color::Lerp(startState, endState, 1.f / 2), endState);
}

void ParticleSystem::setColors(Color startState, Color centerState, Color endState)
{
    ref->colorable = true;
    ref->startColor = startState;
    ref->centerColor = centerState;
    ref->endColor = endState;
}

void ParticleSystem::setSize(Vec2 size)
{
    ref->scalable = false;
    ref->startSize = size;
}

void ParticleSystem::setSizes(Vec2 startState, Vec2 endState)
{
    setSizes(startState, Vec2::Lerp(startState, endState, .5f), endState);
}

void ParticleSystem::setSizes(Vec2 startState, Vec2 centerState, Vec2 endState)
{
    ref->scalable = true;
    ref->startSize = startState;
    ref->centerSize = centerState;
    ref->endSize = endState;
}

float ParticleSystem::getDuration()
{
    return ref->m_duration;
}

int ParticleSystem::getActiveCount()
{
    return static_cast<int>(ref->activeParticles.size());
}

ParticleSystemState ParticleSystem::getState()
{
    return ParticleSystemState(
        ((loop || !ref->activeParticles.empty()) ? ParticleSystemState::Executable : 0) |
        ((ref->m_maked < ref->m_limit || ref->m_maked < startWith) ? ParticleSystemState::Fabricatable : 0));
}

void ParticleSystem::setLimitInfinitely()
{
    setLimit(0);
}

void ParticleSystem::setLimit(int max)
{
    if(max < 0)
        return;

    ref->m_limit = max;
}

void ParticleSystem::Reset()
{
    ClearReserved();

    for(const ParticleDrain &drain : ref->activeParticles)
    {
        drain.render->ClearOnDestroy();
        drain.render->gameObject()->Destroy(); // destroy now
    }

    ref->activeParticles.clear();
    ref->m_maked = 0;
    ref->m_timing = 0;
    ref->m_lastInspected = 0;
}

void ParticleSystem::ClearReserved()
{
    for(SpriteRenderer *drain : ref->cachedParticles)
    {
        drain->ClearOnDestroy();
        drain->gameObject()->Destroy(); // destroy now
    }
    ref->cachedParticles.clear();
    ref->cachedParticles.shrink_to_fit();
}
