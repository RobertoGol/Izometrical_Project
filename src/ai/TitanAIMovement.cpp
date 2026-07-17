#include "ai/TitanAI.hpp"
#include "engine/Log.hpp"
#include "gameplay/BulletSystem.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    void TitanAI::updateAutonomousMode(GameState& gs, float dt)
    {
        float tdx = gs.playerPos.x - gs.titan.position.x;
        float tdy = gs.playerPos.y - gs.titan.position.y;
        float distSq = tdx * tdx + tdy * tdy;

        if (distSq < m_ExclusionRadiusSq)
        {
            float dist = std::sqrt(distSq);
            if (dist > 0.001f)
            {
                Vector3D push = {tdx / dist, tdy / dist, 0.0f};
                float nextX = gs.titan.position.x - push.x * 12.0f * dt;
                float nextY = gs.titan.position.y - push.y * 12.0f * dt;

                if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
                    gs.titan.position.x = nextX;
                if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
                    gs.titan.position.y = nextY;
            }
        }
        else
        {
            Vector3D anchor = calculateCombatAnchor(gs);
            float adx = anchor.x - gs.titan.position.x;
            float ady = anchor.y - gs.titan.position.y;
            float aLenSq = adx * adx + ady * ady;

            if (aLenSq > 1.0f)
            {
                float aLen = std::sqrt(aLenSq);
                float nextX = gs.titan.position.x + (adx / aLen) * gs.titan.speed * dt;
                float nextY = gs.titan.position.y + (ady / aLen) * gs.titan.speed * dt;

                if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
                    gs.titan.position.x = nextX;
                if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
                    gs.titan.position.y = nextY;
            }
        }
    }

    void TitanAI::updatePilotedMode(GameState& gs, const InputSnapshot& input, float dt)
    {
        float camDx = gs.mouseWorldPos.x - gs.titan.position.x;
        float camDy = gs.mouseWorldPos.y - gs.titan.position.y;
        float camLen = std::sqrt(camDx * camDx + camDy * camDy);

        Vector3D camForward = {0.707f, -0.707f, 0.0f};
        Vector3D camRight = {0.707f, 0.707f, 0.0f};

        if (camLen > 0.01f)
        {
            camForward.x = camDx / camLen;
            camForward.y = camDy / camLen;
            camRight.x = camForward.y;
            camRight.y = -camForward.x;
        }

        Vector3D moveInput = {0.0f, 0.0f, 0.0f};
        moveInput.x = camForward.x * input.moveForward + camRight.x * input.moveStrafe;
        moveInput.y = camForward.y * input.moveForward + camRight.y * input.moveStrafe;

        float inputLenSq = moveInput.x * moveInput.x + moveInput.y * moveInput.y;
        Vector3D targetVel = {0.0f, 0.0f, 0.0f};

        if (inputLenSq > 0.001f)
        {
            float len = std::sqrt(inputLenSq);
            float activeSpeed = m_VortexActive ? (gs.titan.speed * 0.4f) : gs.titan.speed;
            if (gs.titan.systems.tracksCondition < 40.0f)
                activeSpeed *= 0.3f;

            targetVel.x = (moveInput.x / len) * activeSpeed;
            targetVel.y = (moveInput.y / len) * activeSpeed;
        }

        float lerpAcc = (inputLenSq > 0.001f) ? m_TitanAcceleration : m_TitanDeceleration;
        m_Velocity.x += (targetVel.x - m_Velocity.x) * lerpAcc * dt;
        m_Velocity.y += (targetVel.y - m_Velocity.y) * lerpAcc * dt;

        float nextX = gs.titan.position.x + m_Velocity.x * dt;
        float nextY = gs.titan.position.y + m_Velocity.y * dt;

        if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
            gs.titan.position.x = nextX;
        if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
            gs.titan.position.y = nextY;

        gs.playerPos = gs.titan.position;
    }

    Vector3D TitanAI::calculateCombatAnchor(const GameState& gs)
    {
        if (gs.enemies.empty())
            return gs.playerPos;

        Vector3D center = {0.0f, 0.0f, 0.0f};
        int count = 0;

        for (const auto& e : gs.enemies)
        {
            if (!e.isAlive)
                continue;
            center.x += e.position.x;
            center.y += e.position.y;
            count++;
        }

        if (count == 0)
            return gs.playerPos;

        center.x /= count;
        center.y /= count;

        return {gs.playerPos.x + (center.x - gs.playerPos.x) * 0.40f,
                gs.playerPos.y + (center.y - gs.playerPos.y) * 0.40f, 0.0f};
    }


} // namespace bunker
