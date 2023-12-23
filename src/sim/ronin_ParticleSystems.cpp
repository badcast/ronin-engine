#include "ParticleSystem.hpp"

enum ParticleState
{
    BEGIN_STATE = 0,
    SIMULATE_STATE = 1,
    END_STATE = 2
};

bool operator<(const ParticleDrain &lhs, const ParticleDrain &rhs)
{
    return lhs.render < rhs.render;
}

// using LowerParticleDrain = std::integral_constant<decltype(&IsLowerParticleDrain), &IsLowerParticleDrain>;

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

void ParticleSystem::link_particles(int n)
{
    std::string particle_name;

    // lambda method
    auto fabricate = [&](SpriteRenderer *spriteRender)
    {
        Transform *t = spriteRender->transform();
        t->position(transform()->position());
        if(worldSpace)
        {
            t->setParent(nullptr);
        }
        else
        {
            t->setParent(transform(), false);
        }

        if(rotate)
            t->angle(Random::AngleDegress());
        else if(t->angle())
            t->angle(0);

        // Set Layer
        t->layer(Layers::ParticleClass);

        // Set Sprite Source
        Sprite *targetSprte = inspect_at(m_sources, m_lastInspected, m_sourceInspect);
        if(spriteRender->getSprite() != targetSprte)
            spriteRender->setSprite(targetSprte);

        // Set Start Size
        spriteRender->setSize(startSize);

        // Set Start Color
        spriteRender->setColor(startColor);

        activeParticles.insert({spriteRender, TimeEngine::time(), randomDirection ? Random::RandomVector() : direction});
    };

    // use existences
    if(reserving)
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
        ClearReserved();
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

void ParticleSystem::unlink_particle(const ParticleDrain *drain)
{
    bool destroyToo = reserving == false;

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

ParticleSystem::ParticleSystem()
    : Behaviour("Particle System"), m_timing(0), m_maked(0), m_sourceInspect(ParticleSourceInspect::InspectNext)
{
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
    setInterpolates(m_duration, m_durationStartRange, m_durationEndRange);
    m_maked = 0;
    m_lastInspected = 0;
}

void ParticleSystem::OnStart()
{
    // init
    transform()->layer(Layers::ParticleClass);

    if(delay > 0)
        m_timing = delay + TimeEngine::time();
    else
        m_timing = 0;
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

    int makeFlag = (loop || !activeParticles.empty()) ? M_Execute : 0;

    if(loop && m_limit == 0 || m_maked < m_limit || m_maked < startWith)
    {
        makeFlag |= M_Fabricate;
    }

    if(makeFlag)
    {
        float t = TimeEngine::time();
        // Make new particle (interval)
        if(emit && m_timing <= t && (makeFlag & M_Fabricate))
        {
            int make_n = m_limit == 0 ? startWith : Math::Min(startWith, static_cast<int>(m_limit - activeParticles.size()));
            link_particles(make_n);
            m_maked += make_n;

            // update interval
            m_timing = t + interval;
        }

        // Update existing particles (drains)
        for(std::set<ParticleDrain>::iterator drain = std::begin(activeParticles); drain != std::end(activeParticles); ++drain)
        {
            float drain_lifetime = t - drain->initTime;
            if(drain_lifetime >= m_duration)
            {
                unlink_particle(&(*drain));
                continue;
            }

            float drain_speed = TimeEngine::deltaTime() * speed;

            float state_percentage;
            int interpolateBegin, interpolateEnd;

            if(drain_lifetime <= m_durationStartRange)
            {
                interpolateBegin = BEGIN_STATE;
                state_percentage = drain_lifetime / m_durationStartRange;
            }
            else if(drain_lifetime >= m_durationEndRange)
            {
                interpolateBegin = END_STATE;
                state_percentage = drain_lifetime / m_durationEndRange;
            }
            else
            {
                interpolateBegin = SIMULATE_STATE;
                state_percentage = drain_lifetime / m_duration;
            }

            interpolateEnd = Math::Min<int>(Math::Max<int>(interpolateBegin, interpolateBegin + 1), END_STATE);

            // Colorize
            if(colorable)
            {
                Color interpolate_from = InterpolationValue(interpolateBegin, startColor, centerColor, endColor);
                Color interpolate_to = InterpolationValue(interpolateEnd, startColor, centerColor, endColor);
                drain->render->setColor(Color::Lerp(interpolate_from, interpolate_to, state_percentage));
            }

            // Scaling
            if(scalable)
            {
                Vec2 interpolate_from = InterpolationValue(interpolateBegin, startSize, centerSize, endSize);
                Vec2 interpolate_to = InterpolationValue(interpolateEnd, startSize, centerSize, endSize);
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
        for(SpriteRenderer *reserved : cachedParticles)
        {
            drained.render = reserved;
            activeParticles.erase(drained);
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
    this->m_duration = Math::Max(duration, 0.f);
    this->m_durationStartRange = this->m_duration * Math::Clamp01(startRange);
    this->m_durationEndRange = this->m_duration - this->m_duration * Math::Clamp01(endRange);
}

void ParticleSystem::setColor(Color color)
{
    colorable = false;
    startColor = color;
}

void ParticleSystem::setColors(Color startState, Color endState)
{
    setColors(startState, Color::Lerp(startState, endState, 1.f / 2), endState);
}

void ParticleSystem::setColors(Color startState, Color centerState, Color endState)
{
    colorable = true;
    startColor = startState;
    centerColor = centerState;
    endColor = endState;
}

void ParticleSystem::setSize(Vec2 size)
{
    scalable = false;
    startSize = size;
}

void ParticleSystem::setSizes(Vec2 startState, Vec2 endState)
{
    setSizes(startState, Vec2::Lerp(startState, endState, .5f), endState);
}

void ParticleSystem::setSizes(Vec2 startState, Vec2 centerState, Vec2 endState)
{
    scalable = true;
    startSize = startState;
    centerSize = centerState;
    endSize = endState;
}

float ParticleSystem::getDuration()
{
    return m_duration;
}

int ParticleSystem::getActiveCount()
{
    return static_cast<int>(activeParticles.size());
}

ParticleSystemState ParticleSystem::getState()
{
    return ParticleSystemState(
        ((loop || !activeParticles.empty()) ? ParticleSystemState::Executable : 0) |
        ((m_maked < m_limit || m_maked < startWith) ? ParticleSystemState::Fabricatable : 0));
}

void ParticleSystem::setLimitInfinitely()
{
    setLimit(0);
}

void ParticleSystem::setLimit(int max)
{
    if(max < 0)
        return;

    m_limit = max;
}

void ParticleSystem::Reset()
{
    ClearReserved();

    for(const ParticleDrain &drain : activeParticles)
    {
        drain.render->ClearOnDestroy();
        drain.render->gameObject()->Destroy(); // destroy now
    }

    activeParticles.clear();
    m_maked = 0;
    m_timing = 0;
    m_lastInspected = 0;
}

void ParticleSystem::ClearReserved()
{
    for(SpriteRenderer *drain : cachedParticles)
    {
        drain->ClearOnDestroy();
        drain->gameObject()->Destroy(); // destroy now
    }
    cachedParticles.clear();
    cachedParticles.shrink_to_fit();
}
