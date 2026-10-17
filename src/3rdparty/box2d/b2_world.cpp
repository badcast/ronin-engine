/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "b2_world.h"

namespace box2d
{
    World::World(Vec2 gravity_, int iterations_)
        : gravity(gravity_), iterations(iterations_)
    {
    }

    World::~World()
    {
        Clear();
    }

    void World::Add(Body *body)
    {
        bodies.push_back(body);
    }

    void World::Remove(Body *body)
    {
        auto it = std::find(bodies.begin(), bodies.end(), body);
        if (it != bodies.end())
            bodies.erase(it);

        for (auto arbIt = arbiters.begin(); arbIt != arbiters.end();)
        {
            if (arbIt->first.body1 == body || arbIt->first.body2 == body)
            {
                arbIt = arbiters.erase(arbIt);
            }
            else
            {
                ++arbIt;
            }
        }
    }

    void World::Clear()
    {
        bodies.clear();
        arbiters.clear();
    }

    void World::BroadPhase()
    {
        // O(N^2) pairwise broadphase suitable for 2D game scenes
        for (size_t i = 0; i < bodies.size(); ++i)
        {
            Body *bi = bodies[i];
            for (size_t j = i + 1; j < bodies.size(); ++j)
            {
                Body *bj = bodies[j];

                // Skip static-static, static-kinematic, or kinematic-kinematic
                if (bi->invMass == 0.0f && bj->invMass == 0.0f)
                    continue;

                // Skip triggers
                if (bi->isTrigger || bj->isTrigger)
                    continue;

                Arbiter newArb(bi, bj);
                ArbiterKey key(bi, bj);

                if (newArb.numContacts > 0)
                {
                    auto it = arbiters.find(key);
                    if (it == arbiters.end())
                    {
                        arbiters.insert(std::make_pair(key, newArb));
                    }
                    else
                    {
                        it->second.Update(newArb.contacts, newArb.numContacts);
                    }
                }
                else
                {
                    arbiters.erase(key);
                }
            }
        }
    }

    void World::Step(float dt)
    {
        if (dt <= 0.0f)
            return;

        float inv_dt = 1.0f / dt;

        // 1. Broadphase
        BroadPhase();

        // 2. Integrate forces and apply damping for Dynamic bodies
        for (Body *b : bodies)
        {
            if (b->bodyType != BodyType::Dynamic)
                continue;

            b->velocity += dt * (b->gravityScale * gravity + b->invMass * b->force);

            if (!b->freezeRotation)
            {
                b->angularVelocity += dt * (b->invI * b->torque);
            }
            else
            {
                b->angularVelocity = 0.0f;
            }

            // Damping (linearDrag and angularDrag)
            if (b->linearDamping > 0.0f)
            {
                b->velocity *= 1.0f / (1.0f + b->linearDamping * dt);
            }
            if (b->angularDamping > 0.0f)
            {
                b->angularVelocity *= 1.0f / (1.0f + b->angularDamping * dt);
            }
        }

        // 3. Pre-step arbiters
        for (auto &pair : arbiters)
        {
            pair.second.PreStep(inv_dt);
        }

        // 4. Impulse iterations
        for (int i = 0; i < iterations; ++i)
        {
            for (auto &pair : arbiters)
            {
                pair.second.ApplyImpulse();
            }
        }

        // 5. Integrate velocities
        for (Body *b : bodies)
        {
            if (b->bodyType == BodyType::Static)
                continue;

            b->position += dt * b->velocity;
            if (!b->freezeRotation)
            {
                b->rotation += dt * b->angularVelocity;
            }

            b->force.Set(0.0f, 0.0f);
            b->torque = 0.0f;
        }
    }
}
