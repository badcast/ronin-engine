#pragma once

#include "begin.h"
#include <vector>

namespace RoninEngine::Runtime
{
    class BoxCollider2D;

    class RONIN_API MultiCollider2D : public Behaviour
    {
    public:
        // Vertex points in local space
        std::vector<Vec2> points;
        Vec2 offset{0.0f, 0.0f};
        bool isTrigger{false};
        float density{1.0f};
        float friction{0.4f};
        float bounciness{0.1f};
        bool isClosed{true};

        MultiCollider2D();
        MultiCollider2D(const std::string &name);
        MultiCollider2D(const MultiCollider2D &) = delete;

        void OnAwake();
        void Reset();

        // Vertex manipulation API
        int GetPointCount() const;
        const std::vector<Vec2> &GetPoints() const;
        void SetPoints(const std::vector<Vec2> &newPoints);
        Vec2 GetPoint(int index) const;
        void SetPoint(int index, const Vec2 &point);
        void AddPoint(const Vec2 &point);
        void InsertPoint(int index, const Vec2 &point);
        void RemovePoint(int index);
        void ClearPoints();

        // Preset generators
        void SetBox(const Vec2 &size);
        void SetBox(float width, float height);
        void SetTriangle(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2);
        void SetTriangle(float width, float height);
        void SetRegularPolygon(int sides, float radius);
        void SetStar(int points, float outerRadius, float innerRadius);
        void SetCapsule(float height, float radius, int capSegments = 8);

        // Geometric calculations
        std::vector<Vec2> GetWorldPoints(Transform *trans) const;
        Rectf GetBounds(Transform *trans) const;
        float GetArea() const;
        Vec2 GetCentroid() const;
        bool IsConvex() const;
        bool ContainsPoint(Transform *trans, const Vec2 &worldPoint) const;
        bool ContainsPoint(const Vec2 &worldPoint) const;

        // Instance collision checks (using component's transform)
        bool Intersects(const Ref<MultiCollider2D> &other) const;
        bool Intersects(const Ref<BoxCollider2D> &box) const;
        bool IntersectsCircle(const Vec2 &circleCenter, float circleRadius) const;
        bool Raycast(const Vec2 &rayOrigin, const Vec2 &rayDirection, float maxDist, Vec2 &outHitPoint, Vec2 &outHitNormal, float &outDist) const;
        bool Raycast(const Vec2 &rayOrigin, const Vec2 &rayDirection, Vec2 &outHitPoint, float &outDist) const;

        // Static collision Checks
        static bool CheckCollision(
            const MultiCollider2D &a,
            Transform *transA,
            const MultiCollider2D &b,
            Transform *transB);

        static bool CheckCollision(
            const MultiCollider2D &poly,
            Transform *transPoly,
            const BoxCollider2D &box,
            Transform *transBox);

        static bool CheckCollision(
            const MultiCollider2D &poly,
            Transform *transPoly,
            const Vec2 &circleCenter,
            float circleRadius);

        static bool Raycast(
            const MultiCollider2D &poly,
            Transform *transPoly,
            const Vec2 &rayOrigin,
            const Vec2 &rayDirection,
            float maxDist,
            Vec2 &outHitPoint,
            Vec2 &outHitNormal,
            float &outDist);

        // Convenience lowercase aliases
        inline int getPointCount() const { return GetPointCount(); }
        inline const std::vector<Vec2> &getPoints() const { return GetPoints(); }
        inline void setPoints(const std::vector<Vec2> &p) { SetPoints(p); }
        inline Vec2 getPoint(int index) const { return GetPoint(index); }
        inline void setPoint(int index, const Vec2 &p) { SetPoint(index, p); }
        inline void addPoint(const Vec2 &p) { AddPoint(p); }
        inline void insertPoint(int index, const Vec2 &p) { InsertPoint(index, p); }
        inline void removePoint(int index) { RemovePoint(index); }
        inline void clear() { ClearPoints(); }
        inline void setBox(float w, float h) { SetBox(w, h); }
        inline void setBox(const Vec2 &s) { SetBox(s); }
        inline void setTriangle(float w, float h) { SetTriangle(w, h); }
        inline void setRegularPolygon(int s, float r) { SetRegularPolygon(s, r); }
        inline void setStar(int p, float o, float i) { SetStar(p, o, i); }
        inline void setCapsule(float h, float r, int segs = 8) { SetCapsule(h, r, segs); }
        inline float computeArea() const { return GetArea(); }
        inline Vec2 computeCentroid() const { return GetCentroid(); }
        inline bool isConvex() const { return IsConvex(); }
        inline bool containsPoint(const Vec2 &p) const { return ContainsPoint(p); }
        inline bool intersects(const Ref<MultiCollider2D> &o) const { return Intersects(o); }
        inline bool intersects(const Ref<BoxCollider2D> &b) const { return Intersects(b); }
        inline bool intersectsCircle(const Vec2 &c, float r) const { return IntersectsCircle(c, r); }
        inline bool raycast(const Vec2 &o, const Vec2 &d, Vec2 &hp, float &hd) const { return Raycast(o, d, hp, hd); }
    };
} // namespace RoninEngine::Runtime
