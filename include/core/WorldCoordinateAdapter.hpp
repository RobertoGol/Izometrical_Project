#pragma once

#include "core/Types.hpp"
#include "entities/Camera.hpp"

#include <SFML/System/Vector2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <cmath>

namespace bunker
{
    class WorldCoordinateAdapter
    {
      public:
        static Vector3D screenToGameplayGround(const sf::Vector2i& pixel,
                                               const sf::Vector2u& viewportSize,
                                               const Camera& camera,
                                               const Vector3D& fallback)
        {
            if (viewportSize.x == 0 || viewportSize.y == 0)
            {
                return fallback;
            }

            const float ndcX = (2.0f * static_cast<float>(pixel.x)) / static_cast<float>(viewportSize.x) - 1.0f;
            const float ndcY = 1.0f - (2.0f * static_cast<float>(pixel.y)) / static_cast<float>(viewportSize.y);

            const float aspectRatio = static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y);
            const glm::mat4 inverseViewProjection =
                glm::inverse(camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix());

            const glm::vec4 nearWorld4 = inverseViewProjection * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
            const glm::vec4 farWorld4 = inverseViewProjection * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
            if (std::abs(nearWorld4.w) < 0.000001f || std::abs(farWorld4.w) < 0.000001f)
            {
                return fallback;
            }

            const glm::vec3 nearWorld = glm::vec3(nearWorld4) / nearWorld4.w;
            const glm::vec3 farWorld = glm::vec3(farWorld4) / farWorld4.w;
            const glm::vec3 rayDirection = farWorld - nearWorld;
            if (std::abs(rayDirection.y) < 0.000001f)
            {
                return fallback;
            }

            const float t = -nearWorld.y / rayDirection.y;
            if (t < 0.0f)
            {
                return fallback;
            }

            const glm::vec3 hit = nearWorld + rayDirection * t;
            return {hit.x, hit.z, 0.0f};
        }
    };

} // namespace bunker
