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

#pragma once

#include "b2_math.h"

namespace box2d
{
    enum class BodyType
    {
        Dynamic = 0,
        Kinematic = 1,
        Static = 2
    };

    struct Body
    {
        Body();

        void Set(const Vec2 &w, float m);
        void SetBodyType(BodyType type);

        void AddForce(const Vec2 &f)
        {
            if (bodyType == BodyType::Dynamic)
                force += f;
        }

        void AddTorque(float t)
        {
            if (bodyType == BodyType::Dynamic && !freezeRotation)
                torque += t;
        }

        void AddImpulse(const Vec2 &impulse)
        {
            if (bodyType == BodyType::Dynamic)
                velocity += impulse * invMass;
        }

        Vec2 position;
        float rotation; // In radians

        Vec2 velocity;
        float angularVelocity;

        Vec2 force;
        float torque;

        Vec2 width;

        float friction;
        float restitution; // Bounciness (0.0 to 1.0)
        float mass, invMass;
        float I, invI;

        float linearDamping;
        float angularDamping;
        float gravityScale;
        bool freezeRotation;
        bool isTrigger;

        BodyType bodyType;
        void *userData;
    };
}
