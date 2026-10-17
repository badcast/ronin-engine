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

#include "b2_arbiter.h"
#include "b2_body.h"

namespace box2d
{
    Arbiter::Arbiter(Body *b1, Body *b2)
    {
        if (b1 < b2)
        {
            body1 = b1;
            body2 = b2;
        }
        else
        {
            body1 = b2;
            body2 = b1;
        }

        numContacts = Collide(contacts, body1, body2);
        friction = std::sqrt(body1->friction * body2->friction);
        restitution = std::max(body1->restitution, body2->restitution);
    }

    void Arbiter::Update(Contact *newContacts, int numNewContacts)
    {
        Contact mergedContacts[2];

        for (int i = 0; i < numNewContacts; ++i)
        {
            Contact *cNew = newContacts + i;
            int k = -1;
            for (int j = 0; j < numContacts; ++j)
            {
                Contact *cOld = contacts + j;
                if (cNew->feature.value == cOld->feature.value)
                {
                    k = j;
                    break;
                }
            }

            if (k > -1)
            {
                Contact *cOld = contacts + k;
                mergedContacts[i] = *cNew;
                mergedContacts[i].Pn = cOld->Pn;
                mergedContacts[i].Pt = cOld->Pt;
                mergedContacts[i].Pnb = cOld->Pnb;
            }
            else
            {
                mergedContacts[i] = *cNew;
            }
        }

        for (int i = 0; i < numNewContacts; ++i)
            contacts[i] = mergedContacts[i];

        numContacts = numNewContacts;
    }

    void Arbiter::PreStep(float inv_dt)
    {
        const float k_allowedPenetration = 0.01f;
        float k_biasFactor = 0.2f;

        for (int i = 0; i < numContacts; ++i)
        {
            Contact *c = contacts + i;

            Vec2 r1 = c->position - body1->position;
            Vec2 r2 = c->position - body2->position;

            // Precompute normal mass, tangent mass, and bias
            float rn1 = Dot(r1, c->normal);
            float rn2 = Dot(r2, c->normal);
            float kNormal = body1->invMass + body2->invMass;
            kNormal += body1->invI * (Dot(r1, r1) - rn1 * rn1) + body2->invI * (Dot(r2, r2) - rn2 * rn2);
            c->massNormal = 1.0f / kNormal;

            Vec2 tangent = Cross(c->normal, 1.0f);
            float rt1 = Dot(r1, tangent);
            float rt2 = Dot(r2, tangent);
            float kTangent = body1->invMass + body2->invMass;
            kTangent += body1->invI * (Dot(r1, r1) - rt1 * rt1) + body2->invI * (Dot(r2, r2) - rt2 * rt2);
            c->massTangent = 1.0f / kTangent;

            c->bias = -k_biasFactor * inv_dt * std::min(0.0f, c->separation + k_allowedPenetration);

            // Apply accumulated impulse
            Vec2 P = c->Pn * c->normal + c->Pt * tangent;

            body1->velocity -= body1->invMass * P;
            body1->angularVelocity -= body1->invI * Cross(r1, P);

            body2->velocity += body2->invMass * P;
            body2->angularVelocity += body2->invI * Cross(r2, P);
        }
    }

    void Arbiter::ApplyImpulse()
    {
        Body *b1 = body1;
        Body *b2 = body2;

        for (int i = 0; i < numContacts; ++i)
        {
            Contact *c = contacts + i;
            c->r1 = c->position - b1->position;
            c->r2 = c->position - b2->position;

            // Relative velocity at contact
            Vec2 dv = b2->velocity + Cross(b2->angularVelocity, c->r2) - b1->velocity - Cross(b1->angularVelocity, c->r1);

            // Compute normal impulse with restitution (bounciness)
            float vn = Dot(dv, c->normal);

            float dPn = c->massNormal * (-vn + c->bias);

            // Clamp the accumulated impulse
            float Pn0 = c->Pn;
            c->Pn = std::max(Pn0 + dPn, 0.0f);
            dPn = c->Pn - Pn0;

            // Apply contact impulse
            Vec2 Pn = dPn * c->normal;

            b1->velocity -= b1->invMass * Pn;
            b1->angularVelocity -= b1->invI * Cross(c->r1, Pn);

            b2->velocity += b2->invMass * Pn;
            b2->angularVelocity += b2->invI * Cross(c->r2, Pn);

            // Relative velocity for friction
            dv = b2->velocity + Cross(b2->angularVelocity, c->r2) - b1->velocity - Cross(b1->angularVelocity, c->r1);

            Vec2 tangent = Cross(c->normal, 1.0f);
            float vt = Dot(dv, tangent);
            float dPt = c->massTangent * (-vt);

            // Compute friction impulse
            float maxPt = friction * c->Pn;

            // Clamp friction
            float Pt0 = c->Pt;
            c->Pt = Clamp(Pt0 + dPt, -maxPt, maxPt);
            dPt = c->Pt - Pt0;

            // Apply friction impulse
            Vec2 Pt = dPt * tangent;

            b1->velocity -= b1->invMass * Pt;
            b1->angularVelocity -= b1->invI * Cross(c->r1, Pt);

            b2->velocity += b2->invMass * Pt;
            b2->angularVelocity += b2->invI * Cross(c->r2, Pt);
        }
    }

    // Box vs Box SAT collision detection
    enum Axis
    {
        FACE_A_X,
        FACE_A_Y,
        FACE_B_X,
        FACE_B_Y
    };

    enum EdgeNumbers
    {
        NO_EDGE = 0,
        EDGE1,
        EDGE2,
        EDGE3,
        EDGE4
    };

    struct ClipVertex
    {
        ClipVertex() { v.Set(0.0f, 0.0f); fp.value = 0; }
        Vec2 v;
        FeaturePair fp;
    };

    static int ClipSegmentToLine(ClipVertex vOut[2], ClipVertex vIn[2], const Vec2 &normal, float offset, char clipEdge)
    {
        int numOut = 0;

        float distance0 = Dot(normal, vIn[0].v) - offset;
        float distance1 = Dot(normal, vIn[1].v) - offset;

        if (distance0 <= 0.0f) vOut[numOut++] = vIn[0];
        if (distance1 <= 0.0f) vOut[numOut++] = vIn[1];

        if (distance0 * distance1 < 0.0f)
        {
            float interp = distance0 / (distance0 - distance1);
            vOut[numOut].v = vIn[0].v + interp * (vIn[1].v - vIn[0].v);
            if (distance0 > 0.0f)
            {
                vOut[numOut].fp = vIn[0].fp;
                vOut[numOut].fp.e.inEdge1 = clipEdge;
                vOut[numOut].fp.e.inEdge2 = NO_EDGE;
            }
            else
            {
                vOut[numOut].fp = vIn[1].fp;
                vOut[numOut].fp.e.outEdge1 = clipEdge;
                vOut[numOut].fp.e.outEdge2 = NO_EDGE;
            }
            numOut++;
        }

        return numOut;
    }

    static void ComputeIncidentEdge(ClipVertex c[2], const Vec2 &h, const Vec2 &pos, const Mat22 &Rot, const Vec2 &normal)
    {
        Mat22 RotT = Rot.Transpose();
        Vec2 n = -(RotT * normal);
        Vec2 nAbs = Abs(n);

        if (nAbs.x > nAbs.y)
        {
            if (Sign(n.x) > 0.0f)
            {
                c[0].v.Set(h.x, -h.y);
                c[0].fp.e.inEdge2 = EDGE3;
                c[0].fp.e.outEdge2 = EDGE4;

                c[1].v.Set(h.x, h.y);
                c[1].fp.e.inEdge2 = EDGE4;
                c[1].fp.e.outEdge2 = EDGE1;
            }
            else
            {
                c[0].v.Set(-h.x, h.y);
                c[0].fp.e.inEdge2 = EDGE1;
                c[0].fp.e.outEdge2 = EDGE2;

                c[1].v.Set(-h.x, -h.y);
                c[1].fp.e.inEdge2 = EDGE2;
                c[1].fp.e.outEdge2 = EDGE3;
            }
        }
        else
        {
            if (Sign(n.y) > 0.0f)
            {
                c[0].v.Set(h.x, h.y);
                c[0].fp.e.inEdge2 = EDGE4;
                c[0].fp.e.outEdge2 = EDGE1;

                c[1].v.Set(-h.x, h.y);
                c[1].fp.e.inEdge2 = EDGE1;
                c[1].fp.e.outEdge2 = EDGE2;
            }
            else
            {
                c[0].v.Set(-h.x, -h.y);
                c[0].fp.e.inEdge2 = EDGE2;
                c[0].fp.e.outEdge2 = EDGE3;

                c[1].v.Set(h.x, -h.y);
                c[1].fp.e.inEdge2 = EDGE3;
                c[1].fp.e.outEdge2 = EDGE4;
            }
        }

        c[0].v = pos + Rot * c[0].v;
        c[1].v = pos + Rot * c[1].v;
    }

    int Collide(Contact *contacts, Body *bodyA, Body *bodyB)
    {
        Vec2 hA = 0.5f * bodyA->width;
        Vec2 hB = 0.5f * bodyB->width;

        Vec2 posA = bodyA->position;
        Vec2 posB = bodyB->position;

        Mat22 RotA(bodyA->rotation);
        Mat22 RotB(bodyB->rotation);

        Mat22 RotAT = RotA.Transpose();
        Mat22 RotBT = RotB.Transpose();

        Vec2 dp = posB - posA;
        Vec2 dA = RotAT * dp;
        Vec2 dB = RotBT * dp;

        Mat22 C = RotAT * RotB;
        Mat22 absC = Abs(C);
        Mat22 absCT = absC.Transpose();

        // Box A faces
        Vec2 faceA = Abs(dA) - hA - absC * hB;
        if (faceA.x > 0.0f || faceA.y > 0.0f)
            return 0;

        // Box B faces
        Vec2 faceB = Abs(dB) - absCT * hA - hB;
        if (faceB.x > 0.0f || faceB.y > 0.0f)
            return 0;

        // Find axis of minimum penetration
        Axis axis;
        float separation;
        Vec2 normal;

        // Box A faces
        axis = FACE_A_X;
        separation = faceA.x;
        normal = dA.x > 0.0f ? RotA.col1 : -RotA.col1;

        const float relativeTol = 0.95f;
        const float absoluteTol = 0.01f;

        if (faceA.y > separation + absoluteTol)
        {
            axis = FACE_A_Y;
            separation = faceA.y;
            normal = dA.y > 0.0f ? RotA.col2 : -RotA.col2;
        }

        // Box B faces
        if (faceB.x > separation + absoluteTol)
        {
            axis = FACE_B_X;
            separation = faceB.x;
            normal = dB.x > 0.0f ? RotB.col1 : -RotB.col1;
        }

        if (faceB.y > separation + absoluteTol)
        {
            axis = FACE_B_Y;
            separation = faceB.y;
            normal = dB.y > 0.0f ? RotB.col2 : -RotB.col2;
        }

        // Setup clipping planes
        Vec2 frontNormal, sideNormal;
        ClipVertex incidentEdge[2];
        float frontOffset, sideOffset1, sideOffset2;
        char edge1, edge2;

        if (axis == FACE_A_X)
        {
            frontNormal = normal;
            frontOffset = Dot(posA, frontNormal) + hA.x;
            sideNormal = RotA.col2;
            float sideOffset = Dot(posA, sideNormal);
            sideOffset1 = -sideOffset + hA.y;
            sideOffset2 = sideOffset + hA.y;
            edge1 = EDGE3;
            edge2 = EDGE1;
            ComputeIncidentEdge(incidentEdge, hB, posB, RotB, frontNormal);
        }
        else if (axis == FACE_A_Y)
        {
            frontNormal = normal;
            frontOffset = Dot(posA, frontNormal) + hA.y;
            sideNormal = RotA.col1;
            float sideOffset = Dot(posA, sideNormal);
            sideOffset1 = -sideOffset + hA.x;
            sideOffset2 = sideOffset + hA.x;
            edge1 = EDGE2;
            edge2 = EDGE4;
            ComputeIncidentEdge(incidentEdge, hB, posB, RotB, frontNormal);
        }
        else if (axis == FACE_B_X)
        {
            frontNormal = -normal;
            frontOffset = Dot(posB, frontNormal) + hB.x;
            sideNormal = RotB.col2;
            float sideOffset = Dot(posB, sideNormal);
            sideOffset1 = -sideOffset + hB.y;
            sideOffset2 = sideOffset + hB.y;
            edge1 = EDGE3;
            edge2 = EDGE1;
            ComputeIncidentEdge(incidentEdge, hA, posA, RotA, frontNormal);
        }
        else
        {
            frontNormal = -normal;
            frontOffset = Dot(posB, frontNormal) + hB.y;
            sideNormal = RotB.col1;
            float sideOffset = Dot(posB, sideNormal);
            sideOffset1 = -sideOffset + hB.x;
            sideOffset2 = sideOffset + hB.x;
            edge1 = EDGE2;
            edge2 = EDGE4;
            ComputeIncidentEdge(incidentEdge, hA, posA, RotA, frontNormal);
        }

        // Clip other face with 5 box planes
        ClipVertex clipPoints1[2];
        ClipVertex clipPoints2[2];
        int np;

        // Clip to box side 1
        np = ClipSegmentToLine(clipPoints1, incidentEdge, -sideNormal, sideOffset1, edge1);
        if (np < 2)
            return 0;

        // Clip to negative box side 1
        np = ClipSegmentToLine(clipPoints2, clipPoints1, sideNormal, sideOffset2, edge2);
        if (np < 2)
            return 0;

        // Now clipPoints2 contains the vertices behind front face
        int numContacts = 0;
        for (int i = 0; i < 2; ++i)
        {
            float sep = Dot(frontNormal, clipPoints2[i].v) - frontOffset;
            if (sep <= 0.0f)
            {
                contacts[numContacts].separation = sep;
                contacts[numContacts].normal = normal;
                contacts[numContacts].position = clipPoints2[i].v - sep * frontNormal;
                contacts[numContacts].feature = clipPoints2[i].fp;
                if (axis == FACE_B_X || axis == FACE_B_Y)
                    contacts[numContacts].normal = -normal;
                numContacts++;
            }
        }

        return numContacts;
    }
}
