#include "vehicles/VehicleManager.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    void VehicleInstance::updatePhysics(GameState& gs, const InputSnapshot& input, float dt)
    {
        if (!config)
        {
            return;
        }

        if (config->driveType == "pressure")
        {
            updatePressureDrive(gs, input, dt);
        }
        else if (config->driveType == "hover")
        {
            updateHoverDrive(gs, input, dt);
        }
        else
        {
            updateThrottleDrive(gs, input, dt);
        }
    }

    void VehicleInstance::updatePressureDrive(GameState& gs, const InputSnapshot& input, float dt)
    {
        if (input.moveForward > 0.0f)
        {
            currentPressure = std::min(config->maxPressure, currentPressure + 40.0f * dt);
        }
        else
        {
            currentPressure = std::max(0.0f, currentPressure - 60.0f * dt);
        }

        if (input.moveStrafe < 0.0f)
        {
            hullAngle -= config->turnSpeed * dt;
        }
        if (input.moveStrafe > 0.0f)
        {
            hullAngle += config->turnSpeed * dt;
        }

        const float reverse = (input.moveForward < 0.0f) ? -3.0f : 0.0f;
        const float targetSpd = (currentPressure / config->maxPressure) * config->maxSpeed + reverse;

        const Vector3D targetVel = {std::cos(hullAngle) * targetSpd, std::sin(hullAngle) * targetSpd, 0.0f};

        velocity.x += (targetVel.x - velocity.x) * config->acceleration * dt;
        velocity.y += (targetVel.y - velocity.y) * config->acceleration * dt;

        applyMovement(gs, dt);
    }

    void VehicleInstance::updateThrottleDrive(GameState& gs, const InputSnapshot& input, float dt)
    {
        if (input.moveForward > 0.0f)
        {
            currentSpeed = std::min(config->maxSpeed, currentSpeed + config->acceleration * dt);
        }
        else if (input.moveForward < 0.0f)
        {
            currentSpeed = std::max(-config->maxSpeed * 0.4f, currentSpeed - config->deceleration * dt);
        }
        else
        {
            currentSpeed *= std::pow(0.05f, dt);
        }

        if (std::abs(currentSpeed) > 0.1f)
        {
            const float turnDir = (currentSpeed > 0.0f) ? 1.0f : -1.0f;
            if (input.moveStrafe < 0.0f)
            {
                hullAngle -= config->turnSpeed * turnDir * dt;
            }
            if (input.moveStrafe > 0.0f)
            {
                hullAngle += config->turnSpeed * turnDir * dt;
            }
        }

        velocity.x = std::cos(hullAngle) * currentSpeed;
        velocity.y = std::sin(hullAngle) * currentSpeed;

        applyMovement(gs, dt);
    }

    void VehicleInstance::updateHoverDrive(GameState& gs, const InputSnapshot& input, float dt)
    {
        Vector3D inputVector = {std::cos(hullAngle) * input.moveForward - std::sin(hullAngle) * input.moveStrafe,
                                std::sin(hullAngle) * input.moveForward + std::cos(hullAngle) * input.moveStrafe, 0.0f};

        if (input.moveForward == 0.0f && input.moveStrafe == 0.0f)
        {
            velocity.x *= std::pow(0.1f, dt);
            velocity.y *= std::pow(0.1f, dt);
            position.z = 0.5f + std::sin(static_cast<float>(std::rand() % 100)) * 0.05f;
            applyMovement(gs, dt);
            return;
        }

        const float len = std::sqrt(inputVector.x * inputVector.x + inputVector.y * inputVector.y);
        if (len > 0.0001f)
        {
            inputVector.x /= len;
            inputVector.y /= len;
        }

        velocity.x += inputVector.x * config->acceleration * dt;
        velocity.y += inputVector.y * config->acceleration * dt;

        const float spd = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (spd > config->maxSpeed)
        {
            velocity.x = (velocity.x / spd) * config->maxSpeed;
            velocity.y = (velocity.y / spd) * config->maxSpeed;
        }

        if (input.moveStrafe != 0.0f)
        {
            hullAngle += input.moveStrafe * config->turnSpeed * 0.5f * dt;
        }

        position.z = 0.5f + std::sin(static_cast<float>(std::rand() % 100)) * 0.05f;
        applyMovement(gs, dt);
    }

    void VehicleInstance::applyMovement(GameState& gs, float dt)
    {
        const float nextX = position.x + velocity.x * dt;
        const float nextY = position.y + velocity.y * dt;

        const float r = config ? config->collisionRadius : 0.5f;

        if (!Collisions::checkWorldCollision(gs, nextX, position.y, r))
        {
            position.x = nextX;
        }
        else
        {
            velocity.x *= -0.3f;
        }

        if (!Collisions::checkWorldCollision(gs, position.x, nextY, r))
        {
            position.y = nextY;
        }
        else
        {
            velocity.y *= -0.3f;
        }
    }

} // namespace bunker
