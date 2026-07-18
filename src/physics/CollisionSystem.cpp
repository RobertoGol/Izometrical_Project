#include "physics/CollisionSystem.hpp"

#include "entities/Collisions.hpp"

#include <algorithm>
#include <cmath>

namespace bunker
{
    namespace
    {
        constexpr float GroundPlaneZ = 0.0f;
        constexpr float DefaultCeilingZ = 2.4f;

        Vector3D makeDelta(float x, float y, float z)
        {
            Vector3D delta{};
            delta.x = x;
            delta.y = y;
            delta.z = z;
            return delta;
        }
    } // namespace

    bool CollisionSystem::checkWorldCollision(const GameState& gameState, float x, float y, float radius)
    {
        PhysicsWorld world(gameState);
        return world.isCircleBlocked(x, y, radius);
    }

    CollisionMoveResult CollisionSystem::sweepAndSlide(const PhysicsWorld& world,
                                                       const CollisionProbe& probe,
                                                       const Vector3D& desiredDelta)
    {
        CollisionMoveResult result{};
        result.position = probe.position;
        result.attemptedDelta = desiredDelta;
        result.onGround = isOnGround(world, probe);

        Vector3D next = probe.position;
        const float targetX = probe.position.x + desiredDelta.x;
        const float targetY = probe.position.y + desiredDelta.y;
        const float targetZ = probe.position.z + desiredDelta.z;

        if (!world.isCircleBlocked(targetX, probe.position.y, probe.radius))
        {
            next.x = targetX;
        }
        else
        {
            result.hitWall = true;
        }

        if (!world.isCircleBlocked(next.x, targetY, probe.radius))
        {
            next.y = targetY;
        }
        else
        {
            result.hitWall = true;
        }

        next.z = std::max(GroundPlaneZ, targetZ);
        if (targetZ < GroundPlaneZ)
        {
            result.onGround = true;
        }

        CollisionProbe ceilingProbe = probe;
        ceilingProbe.position = next;
        if (hasCeilingContact(world, ceilingProbe))
        {
            next.z = std::min(next.z, DefaultCeilingZ - probe.height);
            result.hitCeiling = true;
        }

        result.position = next;
        result.appliedDelta = makeDelta(next.x - probe.position.x, next.y - probe.position.y, next.z - probe.position.z);
        return result;
    }

    bool CollisionSystem::isOnGround(const PhysicsWorld& /*world*/, const CollisionProbe& probe)
    {
        return probe.position.z <= GroundPlaneZ + 0.001f;
    }

    bool CollisionSystem::hasCeilingContact(const PhysicsWorld& /*world*/, const CollisionProbe& probe)
    {
        return probe.position.z + probe.height > DefaultCeilingZ;
    }

    void CollisionSystem::resolveDynamicCollisions(GameState& gameState)
    {
        Collisions::resolveAllCollisions(gameState);
    }
} // namespace bunker
