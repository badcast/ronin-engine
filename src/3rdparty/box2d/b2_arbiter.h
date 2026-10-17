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
#include <cstdint>

namespace box2d
{
    struct Body;

    union FeaturePair
    {
        struct Edges
        {
            char inEdge1;
            char outEdge1;
            char inEdge2;
            char outEdge2;
        } e;
        uint32_t value;
    };

    struct Contact
    {
        Contact() : Pn(0.0f), Pt(0.0f), Pnb(0.0f), bias(0.0f) {}

        Vec2 position;
        Vec2 normal;
        Vec2 r1, r2;
        float separation;
        float Pn;  // Accumulated normal impulse
        float Pt;  // Accumulated tangent impulse
        float Pnb; // Accumulated bias impulse
        float massNormal, massTangent;
        float bias;
        FeaturePair feature;
    };

    struct ArbiterKey
    {
        ArbiterKey(Body *b1, Body *b2)
        {
            if (b1 < b2)
            {
                body1 = b1; body2 = b2;
            }
            else
            {
                body1 = b2; body2 = b1;
            }
        }

        Body *body1;
        Body *body2;
    };

    inline bool operator<(const ArbiterKey &a1, const ArbiterKey &a2)
    {
        if (a1.body1 < a2.body1)
            return true;
        if (a1.body1 == a2.body1 && a1.body2 < a2.body2)
            return true;
        return false;
    }

    struct Arbiter
    {
        enum { MAX_POINTS = 2 };

        Arbiter(Body *b1, Body *b2);

        void Update(Contact *newContacts, int numNewContacts);

        void PreStep(float inv_dt);
        void ApplyImpulse();

        Contact contacts[MAX_POINTS];
        int numContacts;

        Body *body1;
        Body *body2;

        float friction;
        float restitution;
    };

    int Collide(Contact *contacts, Body *body1, Body *body2);
}
