#include "ronin.h"
#include "MultiCollider2D.h"
#include "BoxCollider2D.h"
#include "Collision.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace RoninEngine::Runtime
{
    namespace
    {
        // 2D Cross product of (b - a) and (c - a)
        inline float CrossProduct(const Vec2 &a, const Vec2 &b, const Vec2 &c)
        {
            return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        }

        // Test if two 2D line segments intersect
        inline bool SegmentsIntersect(const Vec2 &p1, const Vec2 &p2, const Vec2 &q1, const Vec2 &q2)
        {
            float cp1 = CrossProduct(p1, p2, q1);
            float cp2 = CrossProduct(p1, p2, q2);
            float cp3 = CrossProduct(q1, q2, p1);
            float cp4 = CrossProduct(q1, q2, p2);

            if (((cp1 > 0.0f && cp2 < 0.0f) || (cp1 < 0.0f && cp2 > 0.0f)) &&
                ((cp3 > 0.0f && cp4 < 0.0f) || (cp3 < 0.0f && cp4 > 0.0f)))
            {
                return true;
            }

            return false;
        }

        // Project vertices onto an axis and get min/max interval
        inline void ProjectPolygon(const std::vector<Vec2> &pts, const Vec2 &axis, float &outMin, float &outMax)
        {
            outMin = outMax = Vec2::Dot(pts[0], axis);
            for (size_t i = 1; i < pts.size(); ++i)
            {
                float d = Vec2::Dot(pts[i], axis);
                if (d < outMin) outMin = d;
                if (d > outMax) outMax = d;
            }
        }

        // Test if SAT separating axis exists between two polygons
        inline bool HasSeparatingAxis(const std::vector<Vec2> &ptsA, const std::vector<Vec2> &ptsB, const Vec2 &axis)
        {
            float minA, maxA, minB, maxB;
            ProjectPolygon(ptsA, axis, minA, maxA);
            ProjectPolygon(ptsB, axis, minB, maxB);
            return (maxA < minB || maxB < minA);
        }

        // Check SAT for all edges of polygon
        inline bool TestSAT(const std::vector<Vec2> &ptsA, const std::vector<Vec2> &ptsB)
        {
            size_t nA = ptsA.size();
            for (size_t i = 0; i < nA; ++i)
            {
                size_t next = (i + 1) % nA;
                Vec2 edge = ptsA[next] - ptsA[i];
                Vec2 normal(-edge.y, edge.x);
                if (normal.sqrMagnitude() > 1e-8f)
                {
                    normal.normalize();
                    if (HasSeparatingAxis(ptsA, ptsB, normal))
                        return false;
                }
            }
            return true;
        }
    } // namespace

    MultiCollider2D::MultiCollider2D() : MultiCollider2D("MultiCollider2D")
    {
    }

    MultiCollider2D::MultiCollider2D(const std::string &name) : Behaviour(name)
    {
        Reset();
    }

    void MultiCollider2D::OnAwake()
    {
    }

    void MultiCollider2D::Reset()
    {
        offset = Vec2::zero;
        isTrigger = false;
        density = 1.0f;
        friction = 0.4f;
        bounciness = 0.1f;
        isClosed = true;

        // Default: 1x1 quad
        points = {
            Vec2(-0.5f, -0.5f),
            Vec2( 0.5f, -0.5f),
            Vec2( 0.5f,  0.5f),
            Vec2(-0.5f,  0.5f)
        };
    }

    int MultiCollider2D::GetPointCount() const
    {
        return static_cast<int>(points.size());
    }

    const std::vector<Vec2> &MultiCollider2D::GetPoints() const
    {
        return points;
    }

    void MultiCollider2D::SetPoints(const std::vector<Vec2> &newPoints)
    {
        points = newPoints;
    }

    Vec2 MultiCollider2D::GetPoint(int index) const
    {
        if (index >= 0 && index < static_cast<int>(points.size()))
            return points[index];
        return Vec2::zero;
    }

    void MultiCollider2D::SetPoint(int index, const Vec2 &point)
    {
        if (index >= 0 && index < static_cast<int>(points.size()))
            points[index] = point;
    }

    void MultiCollider2D::AddPoint(const Vec2 &point)
    {
        points.push_back(point);
    }

    void MultiCollider2D::InsertPoint(int index, const Vec2 &point)
    {
        if (index < 0)
            points.insert(points.begin(), point);
        else if (index >= static_cast<int>(points.size()))
            points.push_back(point);
        else
            points.insert(points.begin() + index, point);
    }

    void MultiCollider2D::RemovePoint(int index)
    {
        if (index >= 0 && index < static_cast<int>(points.size()))
            points.erase(points.begin() + index);
    }

    void MultiCollider2D::ClearPoints()
    {
        points.clear();
    }

    void MultiCollider2D::SetBox(const Vec2 &size)
    {
        float hx = std::abs(size.x) * 0.5f;
        float hy = std::abs(size.y) * 0.5f;
        points = {
            Vec2(-hx, -hy),
            Vec2( hx, -hy),
            Vec2( hx,  hy),
            Vec2(-hx,  hy)
        };
        isClosed = true;
    }

    void MultiCollider2D::SetTriangle(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2)
    {
        points = {p0, p1, p2};
        isClosed = true;
    }

    void MultiCollider2D::SetRegularPolygon(int sides, float radius)
    {
        if (sides < 3) sides = 3;
        if (radius <= 0.0f) radius = 1.0f;

        points.clear();
        points.reserve(sides);
        float step = (2.0f * Math::Pi) / static_cast<float>(sides);
        // Start from top vertex (pi / 2)
        for (int i = 0; i < sides; ++i)
        {
            float ang = (Math::Pi * 0.5f) + static_cast<float>(i) * step;
            points.push_back(Vec2(std::cos(ang) * radius, std::sin(ang) * radius));
        }
        isClosed = true;
    }

    void MultiCollider2D::SetCapsule(float height, float radius, int capSegments)
    {
        if (capSegments < 2) capSegments = 2;
        if (radius <= 0.0f) radius = 0.5f;
        if (height < radius * 2.0f) height = radius * 2.0f;

        float halfShaft = (height * 0.5f) - radius;
        points.clear();

        // Top semi-circle
        for (int i = 0; i <= capSegments; ++i)
        {
            float ang = static_cast<float>(i) * Math::Pi / static_cast<float>(capSegments);
            points.push_back(Vec2(std::cos(ang) * radius, halfShaft + std::sin(ang) * radius));
        }

        // Bottom semi-circle
        for (int i = 0; i <= capSegments; ++i)
        {
            float ang = Math::Pi + static_cast<float>(i) * Math::Pi / static_cast<float>(capSegments);
            points.push_back(Vec2(std::cos(ang) * radius, -halfShaft + std::sin(ang) * radius));
        }

        isClosed = true;
    }

    std::vector<Vec2> MultiCollider2D::GetWorldPoints(Transform *trans) const
    {
        std::vector<Vec2> worldPts;
        worldPts.reserve(points.size());
        if (!trans)
        {
            for (const auto &p : points)
                worldPts.push_back(p + offset);
            return worldPts;
        }

        float angRad = trans->angle() * Math::deg2rad;
        float cosA = std::cos(angRad);
        float sinA = std::sin(angRad);
        Vec2 pos = trans->position();

        for (const auto &p : points)
        {
            Vec2 local = p + offset;
            Vec2 rotated(local.x * cosA - local.y * sinA, local.x * sinA + local.y * cosA);
            worldPts.push_back(pos + rotated);
        }
        return worldPts;
    }

    Rectf MultiCollider2D::GetBounds(Transform *trans) const
    {
        auto wpts = GetWorldPoints(trans);
        if (wpts.empty())
            return Rectf(0.0f, 0.0f, 0.0f, 0.0f);

        float minX = wpts[0].x, maxX = wpts[0].x;
        float minY = wpts[0].y, maxY = wpts[0].y;
        for (size_t i = 1; i < wpts.size(); ++i)
        {
            minX = std::min(minX, wpts[i].x);
            maxX = std::max(maxX, wpts[i].x);
            minY = std::min(minY, wpts[i].y);
            maxY = std::max(maxY, wpts[i].y);
        }
        return Rectf(minX, minY, maxX - minX, maxY - minY);
    }

    float MultiCollider2D::GetArea() const
    {
        if (points.size() < 3) return 0.0f;
        float area = 0.0f;
        size_t n = points.size();
        for (size_t i = 0; i < n; ++i)
        {
            size_t j = (i + 1) % n;
            area += points[i].x * points[j].y - points[j].x * points[i].y;
        }
        return std::abs(area) * 0.5f;
    }

    Vec2 MultiCollider2D::GetCentroid() const
    {
        if (points.empty()) return offset;
        if (points.size() == 1) return points[0] + offset;
        if (points.size() == 2) return (points[0] + points[1]) * 0.5f + offset;

        float signedArea = 0.0f;
        float cx = 0.0f, cy = 0.0f;
        size_t n = points.size();
        for (size_t i = 0; i < n; ++i)
        {
            size_t j = (i + 1) % n;
            float a = points[i].x * points[j].y - points[j].x * points[i].y;
            signedArea += a;
            cx += (points[i].x + points[j].x) * a;
            cy += (points[i].y + points[j].y) * a;
        }
        signedArea *= 0.5f;
        if (std::abs(signedArea) < 1e-6f)
        {
            Vec2 sum = Vec2::zero;
            for (const auto &p : points) sum += p;
            return (sum / static_cast<float>(n)) + offset;
        }
        cx /= (6.0f * signedArea);
        cy /= (6.0f * signedArea);
        return Vec2(cx, cy) + offset;
    }

    bool MultiCollider2D::IsConvex() const
    {
        if (points.size() < 3) return false;
        size_t n = points.size();
        bool gotNegative = false;
        bool gotPositive = false;
        for (size_t i = 0; i < n; ++i)
        {
            const Vec2 &a = points[i];
            const Vec2 &b = points[(i + 1) % n];
            const Vec2 &c = points[(i + 2) % n];
            float cross = (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
            if (cross < -1e-5f) gotNegative = true;
            if (cross > 1e-5f) gotPositive = true;
            if (gotNegative && gotPositive) return false;
        }
        return true;
    }

    bool MultiCollider2D::ContainsPoint(Transform *trans, const Vec2 &worldPoint) const
    {
        auto wpts = GetWorldPoints(trans);
        if (wpts.size() < 3) return false;

        bool inside = false;
        size_t n = wpts.size();
        for (size_t i = 0, j = n - 1; i < n; j = i++)
        {
            const Vec2 &pi = wpts[i];
            const Vec2 &pj = wpts[j];
            if (((pi.y > worldPoint.y) != (pj.y > worldPoint.y)) &&
                (worldPoint.x < (pj.x - pi.x) * (worldPoint.y - pi.y) / (pj.y - pi.y) + pi.x))
            {
                inside = !inside;
            }
        }
        return inside;
    }

    bool MultiCollider2D::CheckCollision(
        const MultiCollider2D &a,
        Transform *transA,
        const MultiCollider2D &b,
        Transform *transB)
    {
        auto ptsA = a.GetWorldPoints(transA);
        auto ptsB = b.GetWorldPoints(transB);

        if (ptsA.size() < 2 || ptsB.size() < 2)
            return false;

        // 1. Broadphase AABB rejection
        Rectf bA = a.GetBounds(transA);
        Rectf bB = b.GetBounds(transB);
        if (bA.x > bB.x + bB.w || bA.x + bA.w < bB.x ||
            bA.y > bB.y + bB.h || bA.y + bA.h < bB.y)
        {
            return false;
        }

        // 2. SAT on edges of A and B
        bool satA = TestSAT(ptsA, ptsB);
        if (!satA) return false;

        bool satB = TestSAT(ptsB, ptsA);
        if (!satB) return false;

        // If both are convex and passed SAT, they collide
        if (a.IsConvex() && b.IsConvex())
            return true;

        // 3. For concave or general cases, check edge-edge intersections
        size_t nA = ptsA.size();
        size_t nB = ptsB.size();
        for (size_t i = 0; i < nA; ++i)
        {
            size_t nextA = (i + 1) % nA;
            for (size_t j = 0; j < nB; ++j)
            {
                size_t nextB = (j + 1) % nB;
                if (SegmentsIntersect(ptsA[i], ptsA[nextA], ptsB[j], ptsB[nextB]))
                    return true;
            }
        }

        // 4. Check if one is fully inside the other
        if (a.ContainsPoint(transA, ptsB[0]) || b.ContainsPoint(transB, ptsA[0]))
            return true;

        return false;
    }

    bool MultiCollider2D::CheckCollision(
        const MultiCollider2D &poly,
        Transform *transPoly,
        const BoxCollider2D &box,
        Transform *transBox)
    {
        auto ptsPoly = poly.GetWorldPoints(transPoly);
        if (ptsPoly.size() < 2) return false;

        // Calculate 4 world vertices of box
        Vec2 boxPos = transBox ? transBox->position() : Vec2::zero;
        float boxAngRad = transBox ? transBox->angle() * Math::deg2rad : 0.0f;
        float cosA = std::cos(boxAngRad);
        float sinA = std::sin(boxAngRad);

        float hx = box.size.x * 0.5f;
        float hy = box.size.y * 0.5f;
        Vec2 localCorners[4] = {
            box.offset + Vec2(-hx, -hy),
            box.offset + Vec2( hx, -hy),
            box.offset + Vec2( hx,  hy),
            box.offset + Vec2(-hx,  hy)
        };

        std::vector<Vec2> ptsBox(4);
        for (int i = 0; i < 4; ++i)
        {
            Vec2 rotated(localCorners[i].x * cosA - localCorners[i].y * sinA,
                         localCorners[i].x * sinA + localCorners[i].y * cosA);
            ptsBox[i] = boxPos + rotated;
        }

        // SAT checks
        if (!TestSAT(ptsPoly, ptsBox)) return false;
        if (!TestSAT(ptsBox, ptsPoly)) return false;

        // Edge intersections for concave
        size_t nPoly = ptsPoly.size();
        for (size_t i = 0; i < nPoly; ++i)
        {
            size_t nextPoly = (i + 1) % nPoly;
            for (size_t j = 0; j < 4; ++j)
            {
                size_t nextBox = (j + 1) % 4;
                if (SegmentsIntersect(ptsPoly[i], ptsPoly[nextPoly], ptsBox[j], ptsBox[nextBox]))
                    return true;
            }
        }

        if (poly.ContainsPoint(transPoly, ptsBox[0]))
            return true;

        // Check if poly point is inside box
        Vec2 polyPtLocal = ptsPoly[0] - boxPos;
        Vec2 unrot(polyPtLocal.x * cosA + polyPtLocal.y * sinA, -polyPtLocal.x * sinA + polyPtLocal.y * cosA);
        unrot -= box.offset;
        if (std::abs(unrot.x) <= hx && std::abs(unrot.y) <= hy)
            return true;

        return false;
    }

    bool MultiCollider2D::CheckCollision(
        const MultiCollider2D &poly,
        Transform *transPoly,
        const Vec2 &circleCenter,
        float circleRadius)
    {
        if (circleRadius <= 0.0f) return false;

        // 1. Point in polygon
        if (poly.ContainsPoint(transPoly, circleCenter))
            return true;

        // 2. Distance from circle center to each polygon edge segment
        auto pts = poly.GetWorldPoints(transPoly);
        if (pts.empty()) return false;

        float rSq = circleRadius * circleRadius;
        size_t n = pts.size();
        for (size_t i = 0; i < n; ++i)
        {
            size_t next = (i + 1) % n;
            Vec2 seg = pts[next] - pts[i];
            float segLenSq = seg.sqrMagnitude();
            float t = (segLenSq > 1e-8f) ? std::clamp(Vec2::Dot(circleCenter - pts[i], seg) / segLenSq, 0.0f, 1.0f) : 0.0f;
            Vec2 closest = pts[i] + seg * t;
            if (Vec2::DistanceSqr(closest, circleCenter) <= rSq)
                return true;
        }

        return false;
    }

    bool MultiCollider2D::Raycast(
        const MultiCollider2D &poly,
        Transform *transPoly,
        const Vec2 &rayOrigin,
        const Vec2 &rayDirection,
        float maxDist,
        Vec2 &outHitPoint,
        Vec2 &outHitNormal,
        float &outDist)
    {
        Vec2 rayDir = rayDirection.normalized();
        if (rayDir.sqrMagnitude() < 1e-6f) return false;

        auto pts = poly.GetWorldPoints(transPoly);
        if (pts.size() < 2) return false;

        float nearestT = std::numeric_limits<float>::max();
        Vec2 hitPt = Vec2::zero;
        Vec2 hitNorm = Vec2::zero;
        bool hasHit = false;

        size_t n = pts.size();
        for (size_t i = 0; i < n; ++i)
        {
            size_t next = (i + 1) % n;
            const Vec2 &p1 = pts[i];
            const Vec2 &p2 = pts[next];

            Vec2 v1 = rayOrigin - p1;
            Vec2 v2 = p2 - p1;
            Vec2 v3(-rayDir.y, rayDir.x);

            float dot = Vec2::Dot(v2, v3);
            if (std::abs(dot) < 1e-7f)
                continue;

            float t1 = (v2.x * v1.y - v2.y * v1.x) / dot;
            float t2 = Vec2::Dot(v1, v3) / dot;

            if (t1 >= 0.0f && t1 <= maxDist && t2 >= 0.0f && t2 <= 1.0f)
            {
                if (t1 < nearestT)
                {
                    nearestT = t1;
                    hitPt = rayOrigin + rayDir * t1;
                    Vec2 edgeNormal(-v2.y, v2.x);
                    hitNorm = edgeNormal.normalized();
                    // Ensure normal opposes ray direction
                    if (Vec2::Dot(hitNorm, rayDir) > 0.0f)
                        hitNorm = -hitNorm;
                    hasHit = true;
                }
            }
        }

        if (hasHit)
        {
            outHitPoint = hitPt;
            outHitNormal = hitNorm;
            outDist = nearestT;
            return true;
        }

        return false;
    }

    void MultiCollider2D::SetBox(float width, float height)
    {
        SetBox(Vec2{width, height});
    }

    void MultiCollider2D::SetTriangle(float width, float height)
    {
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        SetTriangle({0.0f, hh}, {hw, -hh}, {-hw, -hh});
    }

    void MultiCollider2D::SetStar(int numPoints, float outerRadius, float innerRadius)
    {
        points.clear();
        int total = numPoints * 2;
        for (int i = 0; i < total; ++i)
        {
            float a = i * Math::Pi / numPoints - Math::Pi * 0.5f;
            float r = (i % 2 == 0) ? outerRadius : innerRadius;
            points.push_back({r * std::cos(a), r * std::sin(a)});
        }
    }

    bool MultiCollider2D::ContainsPoint(const Vec2 &worldPoint) const
    {
        return ContainsPoint(const_cast<MultiCollider2D *>(this)->transform().get(), worldPoint);
    }

    bool MultiCollider2D::Intersects(const Ref<MultiCollider2D> &other) const
    {
        if (!other) return false;
        return CheckCollision(*this, const_cast<MultiCollider2D *>(this)->transform().get(), *other, other->transform().get());
    }

    bool MultiCollider2D::Intersects(const Ref<BoxCollider2D> &box) const
    {
        if (!box) return false;
        return CheckCollision(*this, const_cast<MultiCollider2D *>(this)->transform().get(), *box, box->transform().get());
    }

    bool MultiCollider2D::IntersectsCircle(const Vec2 &circleCenter, float circleRadius) const
    {
        return CheckCollision(*this, const_cast<MultiCollider2D *>(this)->transform().get(), circleCenter, circleRadius);
    }

    bool MultiCollider2D::Raycast(
        const Vec2 &rayOrigin,
        const Vec2 &rayDirection,
        float maxDist,
        Vec2 &outHitPoint,
        Vec2 &outHitNormal,
        float &outDist) const
    {
        return Raycast(*this, const_cast<MultiCollider2D *>(this)->transform().get(), rayOrigin, rayDirection, maxDist, outHitPoint, outHitNormal, outDist);
    }

    bool MultiCollider2D::Raycast(
        const Vec2 &rayOrigin,
        const Vec2 &rayDirection,
        Vec2 &outHitPoint,
        float &outDist) const
    {
        Vec2 norm;
        return Raycast(*this, const_cast<MultiCollider2D *>(this)->transform().get(), rayOrigin, rayDirection, 1000.0f, outHitPoint, norm, outDist);
    }
} // namespace RoninEngine::Runtime
