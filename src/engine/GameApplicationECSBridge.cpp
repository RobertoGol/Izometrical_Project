#include "engine/GameApplication.hpp"
#include "content/MeshBuilder.hpp"

namespace bunker
{
    namespace
    {
        glm::vec3 gameplayToRenderPosition(const Vector3D& position, float heightOffset = 0.0f)
        {
            return {position.x, position.z + heightOffset, position.y};
        }
    }

    void GameApplication::createGameplayAnchors()
    {
        m_PlayerAnchorEntity = m_Registry.createEntity();
        TransformComponent playerTransform;
        playerTransform.scale = glm::vec3(0.45f, 0.9f, 0.45f);
        m_Registry.transforms.insert(m_PlayerAnchorEntity, playerTransform);
        m_Registry.meshes.insert(m_PlayerAnchorEntity, MeshBuilder::createTestCube());

        m_TitanAnchorEntity = m_Registry.createEntity();
        TransformComponent titanTransform;
        titanTransform.scale = glm::vec3(1.4f, 1.0f, 1.4f);
        m_Registry.transforms.insert(m_TitanAnchorEntity, titanTransform);
        m_Registry.meshes.insert(m_TitanAnchorEntity, MeshBuilder::createTestCube());

        syncGameplayAnchorsToECS();
    }

    void GameApplication::syncGameplayAnchorsToECS()
    {
        if (auto* playerTransform = m_Registry.transforms.get(m_PlayerAnchorEntity))
        {
            playerTransform->position = gameplayToRenderPosition(m_GameState.playerPos, 0.35f);
        }

        if (auto* titanTransform = m_Registry.transforms.get(m_TitanAnchorEntity))
        {
            titanTransform->position = gameplayToRenderPosition(m_GameState.titan.position, 0.55f);
        }
    }

} // namespace bunker
