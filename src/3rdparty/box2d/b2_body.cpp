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

#include "b2_body.h"

namespace box2d
{
    Body::Body()
    {
        position.Set(0.0f, 0.0f);
        rotation = 0.0f;
        velocity.Set(0.0f, 0.0f);
        angularVelocity = 0.0f;
        force.Set(0.0f, 0.0f);
        torque = 0.0f;
        friction = 0.2f;
        restitution = 0.0f;
        width.Set(1.0f, 1.0f);
        mass = FLT_MAX;
        invMass = 0.0f;
        I = FLT_MAX;
        invI = 0.0f;
        linearDamping = 0.0f;
        angularDamping = 0.05f;
        gravityScale = 1.0f;
        freezeRotation = false;
        isTrigger = false;
        bodyType = BodyType::Dynamic;
        userData = nullptr;
    }

    void Body::Set(const Vec2 &w, float m)
    {
        position.Set(0.0f, 0.0f);
        rotation = 0.0f;
        velocity.Set(0.0f, 0.0f);
        angularVelocity = 0.0f;
        force.Set(0.0f, 0.0f);
        torque = 0.0f;
        friction = 0.2f;
        restitution = 0.0f;
        width = w;
        mass = m;

        if (mass < FLT_MAX && bodyType == BodyType::Dynamic)
        {
            invMass = 1.0f / mass;
            I = mass * (width.x * width.x + width.y * width.y) / 12.0f;
            invI = freezeRotation ? 0.0f : (1.0f / I);
        }
        else
        {
            invMass = 0.0f;
            I = FLT_MAX;
            invI = 0.0f;
        }
    }

    void Body::SetBodyType(BodyType type)
    {
        bodyType = type;
        if (bodyType == BodyType::Static)
        {
            invMass = 0.0f;
            invI = 0.0f;
            velocity.Set(0.0f, 0.0f);
            angularVelocity = 0.0f;
        }
        else if (bodyType == BodyType::Kinematic)
        {
            invMass = 0.0f;
            invI = 0.0f;
        }
        else // Dynamic
        {
            if (mass > 0.0f && mass < FLT_MAX)
            {
                invMass = 1.0f / mass;
                I = mass * (width.x * width.x + width.y * width.y) / 12.0f;
                invI = freezeRotation ? 0.0f : (1.0f / I);
            }
        }
    }
}
