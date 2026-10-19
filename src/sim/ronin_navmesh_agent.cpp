#include "ronin.h"
#include <cmath>
#include <algorithm>

namespace RoninEngine::Runtime
{
    NavMeshAgent::NavMeshAgent() : NavMeshAgent(DESCRIBE_AS_MAIN_OFF(NavMeshAgent))
    {
    }

    NavMeshAgent::NavMeshAgent(const std::string &name)
        : Behaviour(DESCRIBE_AS_ONLY_NAME(NavMeshAgent)),
          m_navMesh(nullptr),
          m_destination(Vec2::zero),
          m_velocity(Vec2::zero),
          m_currentWaypointIndex(0),
          m_hasPath(false),
          speed(3.5f),
          angularSpeed(120.0f),
          acceleration(8.0f),
          stoppingDistance(0.25f),
          radius(0.4f),
          updatePosition(true),
          updateRotation(false),
          autoRepath(true),
          isStopped(false)
    {
        DESCRIBE_AS_MAIN(NavMeshAgent);
    }

    void NavMeshAgent::OnAwake()
    {
        GetNavMesh();
    }

    void NavMeshAgent::OnStart()
    {
    }

    void NavMeshAgent::OnUpdate()
    {
        float dt = Time::deltaTime();
        if (dt <= 0.0f)
            dt = 1.0f / 60.0f;

        if (isStopped || !m_hasPath || m_waypoints.empty())
        {
            m_velocity = Vec2::MoveTowards(m_velocity, Vec2::zero, acceleration * dt);
            return;
        }

        Vec2 currentPos = transform()->position();

        // Advance waypoints
        while (m_currentWaypointIndex < m_waypoints.size())
        {
            float distToWp = Vec2::Distance(currentPos, m_waypoints[m_currentWaypointIndex]);
            if (distToWp <= std::max(0.08f, radius * 0.4f) && (m_currentWaypointIndex + 1 < m_waypoints.size()))
            {
                m_currentWaypointIndex++;
            }
            else
            {
                break;
            }
        }

        // Check if destination is reached
        float remDist = remainingDistance();
        if (remDist <= stoppingDistance)
        {
            m_hasPath = false;
            m_velocity = Vec2::zero;
            return;
        }

        if (m_currentWaypointIndex >= m_waypoints.size())
        {
            m_hasPath = false;
            m_velocity = Vec2::zero;
            return;
        }

        // Steer towards current waypoint
        Vec2 targetWp = m_waypoints[m_currentWaypointIndex];
        Vec2 toTarget = targetWp - currentPos;
        float distToTarget = toTarget.magnitude();

        Vec2 desiredDir = (distToTarget > 0.0001f) ? (toTarget / distToTarget) : Vec2::zero;
        Vec2 targetVel = desiredDir * speed;

        m_velocity = Vec2::MoveTowards(m_velocity, targetVel, acceleration * dt);

        if (updatePosition)
        {
            transform()->position(currentPos + m_velocity * dt);
        }

        if (updateRotation && m_velocity.sqrMagnitude() > 0.001f)
        {
            float targetAngle = std::atan2(m_velocity.y, m_velocity.x) * Math::rad2deg;
            float currentAngle = transform()->angle();
            float diff = std::remainder(targetAngle - currentAngle, 360.0f);
            float step = std::clamp(diff, -angularSpeed * dt, angularSpeed * dt);
            transform()->angle(currentAngle + step);
        }
    }

    void NavMeshAgent::OnGizmos()
    {
        RenderUtility::DrawCircle(transform()->position(), radius);

        if (m_hasPath)
        {
            RenderUtility::DrawCircle(m_destination, stoppingDistance);
            RenderUtility::DrawLine(transform()->position(), m_destination);
        }
    }

    bool NavMeshAgent::SetDestination(const Vec2 &target)
    {
        AI::NavMesh *nav = GetNavMesh();
        if (!nav)
            return false;

        Vec2 currentPos = transform()->position();
        AI::NavResultSite result;
        bool ok = nav->Find(result, currentPos, target);

        if (ok && result.status == AI::NavStatus::Opened)
        {
            m_waypoints.clear();
            for (const auto &pt : result.connections)
            {
                m_waypoints.push_back(nav->PointToWorldPoint(pt));
            }

            if (m_waypoints.empty() || Vec2::Distance(m_waypoints.back(), target) > 0.05f)
            {
                m_waypoints.push_back(target);
            }

            m_currentWaypointIndex = 0;
            m_destination = target;
            m_hasPath = true;
            return true;
        }

        return false;
    }

    void NavMeshAgent::ResetPath()
    {
        m_waypoints.clear();
        m_currentWaypointIndex = 0;
        m_hasPath = false;
        m_velocity = Vec2::zero;
    }

    void NavMeshAgent::Warp(const Vec2 &newPosition)
    {
        transform()->position(newPosition);
        ResetPath();
    }

    bool NavMeshAgent::CalculatePath(const Vec2 &target, std::vector<Vec2> &outPath)
    {
        outPath.clear();
        AI::NavMesh *nav = GetNavMesh();
        if (!nav)
            return false;

        Vec2 currentPos = transform()->position();
        AI::NavResultSite result;
        bool ok = nav->Find(result, currentPos, target);

        if (ok && result.status == AI::NavStatus::Opened)
        {
            for (const auto &pt : result.connections)
            {
                outPath.push_back(nav->PointToWorldPoint(pt));
            }
            if (outPath.empty() || Vec2::Distance(outPath.back(), target) > 0.05f)
            {
                outPath.push_back(target);
            }
            return true;
        }

        return false;
    }

    bool NavMeshAgent::hasPath() const
    {
        return m_hasPath && !m_waypoints.empty() && (m_currentWaypointIndex < m_waypoints.size());
    }

    float NavMeshAgent::remainingDistance() const
    {
        if (!m_hasPath || m_waypoints.empty() || m_currentWaypointIndex >= m_waypoints.size())
            return 0.0f;

        Vec2 currentPos = const_cast<NavMeshAgent *>(this)->transform()->position();
        float dist = Vec2::Distance(currentPos, m_waypoints[m_currentWaypointIndex]);

        for (size_t i = m_currentWaypointIndex; i + 1 < m_waypoints.size(); ++i)
        {
            dist += Vec2::Distance(m_waypoints[i], m_waypoints[i + 1]);
        }

        return dist;
    }

    Vec2 NavMeshAgent::destination() const
    {
        return m_destination;
    }

    Vec2 NavMeshAgent::velocity() const
    {
        return m_velocity;
    }

    bool NavMeshAgent::isOnNavMesh() const
    {
        AI::NavMesh *nav = const_cast<NavMeshAgent *>(this)->GetNavMesh();
        if (!nav)
            return false;

        AI::Neuron *neuron = nav->get(const_cast<NavMeshAgent *>(this)->transform()->position());
        return (neuron != nullptr && !nav->hasLocked(neuron));
    }

    void NavMeshAgent::SetNavMesh(AI::NavMesh *mesh)
    {
        m_navMesh = mesh;
    }

    AI::NavMesh *NavMeshAgent::GetNavMesh()
    {
        if (m_navMesh)
            return m_navMesh;

        if (gameObject())
        {
            Terrain2D *t = gameObject()->GetComponent<Terrain2D>();
            if (t && t->get_navmesh2D())
            {
                m_navMesh = t->get_navmesh2D();
                return m_navMesh;
            }
        }

        World *world = World::GetCurrentWorld();
        if (world)
        {
            auto terrains = world->FindObjectsWithType<Terrain2D>();
            for (auto &t : terrains)
            {
                if (t && t->get_navmesh2D())
                {
                    m_navMesh = t->get_navmesh2D();
                    return m_navMesh;
                }
            }
        }

        return m_navMesh;
    }

    const std::vector<Vec2> &NavMeshAgent::GetPathCorners() const
    {
        return m_waypoints;
    }

} // namespace RoninEngine::Runtime
