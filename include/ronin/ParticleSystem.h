/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023-2024
 * status: already coding
 */

#ifndef _PARTICLESYSTEM_H
#define _PARTICLESYSTEM_H

#include "Behaviour.h"

namespace RoninEngine::Runtime
{
    enum ParticleSourceInspect
    {
        InspectNext,
        InspectRandom
    };

    enum ParticleSystemState
    {
        Destroyable = 0,
        Executable = 1,
        Fabricatable = 2
    };


    /**
     * @brief A class representing a particle system.
     *
     * This class extends the functionality of the "Behaviour" class to create and manage a particle system.
     *
     * @see Behaviour
     */
    class RONIN_API ParticleSystem : public Behaviour
    {
    private:
        struct ParticleSystemImpl *impl;

        // Source for set
        std::vector<SpriteRef> m_sources;
        ParticleSourceInspect m_sourceInspect;

    public:
        // Main Control
        bool emit = true;
        bool loop = true;
        bool rotate = true;
        bool worldSpace = true;
        bool destroyAfter = true;
        bool randomDirection = false;
        bool reserving = true;

        // Set the interval for the next particle
        float interval = 1.0f;
        float delay = 0;

        // State
        int startWith = 1;

        // Live
        float speed = 1;
        float rotatePerFrame = 10;

        Vec2 direction = Vec2::up;

        // Methods

        ParticleSystem();

        ParticleSystem(const std::string &name);

        ~ParticleSystem();

        /**
         * @brief Get the duration of the particle object.
         *
         * @return The duration of the particle object in seconds.
         */
        float getDuration();

        /**
         * @brief Get the count of particles.
         *
         * @return An integer representing the count of items.
         */
        int getActiveCount();

        /**
         * @brief Get the current state of particles.
         *
         * @return  An result of the state.
         */
        ParticleSystemState getState();

        /**
         * @brief Set the maximum limit for particles to an infinite value.
         *
         * This function sets the maximum limit for particles to an unlimited or infinite value.
         * This is typically used to indicate that there is no fixed maximum limit for particles.
         * @see setLimit
         */
        void setLimitInfinitely();

        /**
         * @brief Set the maximum number of particles to a specified value.
         *
         * This function allows you to set a specific maximum limit for the number of particles.
         * Set the 0, for unlimitely (setLimitInfinitely)
         * @param max The maximum number of particles to set.
         * @see setLimitInfinitely
         */
        void setLimit(int max);

        /**
         * @brief Set the state of Reservable for particles.
         *
         * This method sets the state for reserving resources for particle objects,
         * helping to economize memory during reallocation.
         *
         * @param state The state to set.
         * @see Reset, ClearReserved
         */
        void setReservable(bool state);

        /**
         * @brief Set the source sprite for the particle.
         *
         * @param source The source sprite to set.
         * @param inspectType (Optional) The inspection type (default: ParticleSourceInspect::InspectNext).
         */
        void setSource(SpriteRef source);

        /**
         * @brief Set two source sprites for the particle.
         *
         * @param src1 The first source sprite to set.
         * @param src2 The second source sprite to set.
         * @param inspectType (Optional) The inspection type (default: ParticleSourceInspect::InspectNext).
         */
        void setSources(SpriteRef src1, SpriteRef src2, ParticleSourceInspect inspectType = ParticleSourceInspect::InspectNext);

        /**
         * @brief Set three source sprites for the particle.
         *
         * @param src1 The first source sprite to set.
         * @param src2 The second source sprite to set.
         * @param src3 The third source sprite to set.
         * @param inspectType (Optional) The inspection type (default: ParticleSourceInspect::InspectNext).
         */
        void setSources(SpriteRef src1, SpriteRef src2, SpriteRef src3, ParticleSourceInspect inspectType = ParticleSourceInspect::InspectNext);

        /**
         * @brief Set multiple source sprites from a container for the particle.
         *
         * @tparam Container The container type containing Sprite objects.
         * @param sources The container of source sprites to set.
         * @param inspectType (Optional) The inspection type (default: ParticleSourceInspect::InspectNext).
         */
        template <typename Container>
        void setSources(const Container &sources, ParticleSourceInspect inspectType = ParticleSourceInspect::InspectNext)
        {
            using TargetType = Sprite;
            static_assert(std::is_same_v<typename Container::value_type, Ref<TargetType>>, "Container is not contains Sprite* objects");
            m_sources.clear();
            m_sourceInspect = inspectType;
            for(const Ref<TargetType>& sprite : sources)
            {
                m_sources.emplace_back(sprite);
            }
        }

        /**
         * @brief Set the duration for interpolation, with default start and end ranges.
         *
         * This function sets the duration for interpolation with the default start and end ranges, which are both set to 0.1.
         *
         * @param duration The duration for interpolation.
         */
        void setInterpolates(float duration);

        /**
         * @brief Set the duration for interpolation with custom start and end ranges.
         *
         * This function sets the duration for interpolation and allows you to specify custom start and end ranges for the interpolation.
         *
         * @param duration    The duration for interpolation.
         * @param startRange  The start range for interpolation (0.0 to 1.0).
         * @param endRange    The end range for interpolation (0.0 to 1.0).
         */
        void setInterpolates(float duration, float startRange, float endRange);

        /**
         * @brief Set the color without interpolation.
         *
         * This function sets the color without any interpolation.
         *
         * @param color The color to be set.
         */
        void setColor(Color color);

        /**
         * @brief Set interpolation of colors from start to end.
         *
         * This function sets the interpolation of colors from a starting state to an ending state.
         *
         * @param startState The starting color.
         * @param endState   The ending color.
         */
        void setColors(Color startState, Color endState);

        /**
         * @brief Set interpolation of colors from specified ranges.
         *
         * This function sets the interpolation of colors from specified start, center, and end states.
         *
         * @param startState   The starting color.
         * @param centerState  The center color.
         * @param endState     The ending color.
         */
        void setColors(Color startState, Color centerState, Color endState);

        /**
         * @brief Set the size without interpolation.
         *
         * This function sets the size without any interpolation.
         *
         * @param size The size to be set.
         */
        void setSize(Vec2 size);

        /**
         * @brief Set interpolation of sizes from start to end.
         *
         * This function sets the interpolation of sizes from a starting state to an ending state.
         *
         * @param startState The starting size.
         * @param endState   The ending size.
         */
        void setSizes(Vec2 startState, Vec2 endState);

        /**
         * @brief Set interpolation of sizes from specified ranges.
         *
         * This function sets the interpolation of sizes from specified start, center, and end states.
         *
         * @param startState   The starting size.
         * @param centerState  The center size.
         * @param endState     The ending size.
         */
        void setSizes(Vec2 startState, Vec2 centerState, Vec2 endState);

        /**
         * @brief Reset the state of the particle object to its initial values.
         *
         * This function resets the particle object to its initial state, clearing any accumulated data or changes.
         */
        void Reset();

        /**
         * @brief Clear any reserved resources associated with the particle object.
         */
        void ClearReserved();

        void OnAwake();
        void OnStart();
        void OnUpdate();
        void OnDestroy();
    };

#endif
}
