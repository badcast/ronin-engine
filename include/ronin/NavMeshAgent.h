#pragma once

#include "begin.h"
#include "Behaviour.h"
#include "NavMesh.h"
#include <vector>

namespace RoninEngine::Runtime
{
    /**
     * @brief Navigation agent component that navigates and moves across a NavMesh.
     */
    class RONIN_API NavMeshAgent : public Behaviour
    {
    private:
        AI::NavMesh *m_navMesh;
        Vec2 m_destination;
        Vec2 m_velocity;
        std::vector<Vec2> m_waypoints;
        size_t m_currentWaypointIndex;
        bool m_hasPath;

    public:
        float speed;
        float angularSpeed;
        float acceleration;
        float stoppingDistance;
        float radius;
        bool updatePosition;
        bool updateRotation;
        bool autoRepath;
        bool isStopped;

        NavMeshAgent();
        explicit NavMeshAgent(const std::string &name);
        virtual ~NavMeshAgent() = default;

        void OnAwake() override;
        void OnStart() override;
        void OnUpdate() override;
        void OnGizmos() override;

        bool SetDestination(const Vec2 &target);
        void ResetPath();
        void Warp(const Vec2 &newPosition);
        bool CalculatePath(const Vec2 &target, std::vector<Vec2> &outPath);

        bool hasPath() const;
        float remainingDistance() const;
        Vec2 destination() const;
        Vec2 velocity() const;
        bool isOnNavMesh() const;

        void SetNavMesh(AI::NavMesh *mesh);
        AI::NavMesh *GetNavMesh();
        const std::vector<Vec2> &GetPathCorners() const;
    };

} // namespace RoninEngine::Runtime
