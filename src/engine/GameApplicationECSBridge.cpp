#include "engine/GameApplication.hpp"
#include "WorkstationManager.hpp"
#include "content/MeshBuilder.hpp"

#include <algorithm>

namespace bunker
{
    namespace
    {
        constexpr glm::vec3 HiddenAnchorScale{0.0f, 0.0f, 0.0f};
        constexpr std::uint32_t PlayerAnchorMaterial = 102;      // steel hull
        constexpr std::uint32_t TitanAnchorMaterial = 102;       // steel hull
        constexpr std::uint32_t EnemyAnchorMaterial = 104;       // caution/accent
        constexpr std::uint32_t VehicleAnchorMaterial = 102;     // steel hull
        constexpr std::uint32_t WorkstationAnchorMaterial = 103; // bunker concrete

        glm::vec3 gameplayToRenderPosition(const Vector3D& position, float heightOffset = 0.0f)
        {
            return {position.x, position.z + heightOffset, position.y};
        }

        glm::vec3 gameplayToRenderPosition(float x, float y, float heightOffset = 0.0f)
        {
            return {x, heightOffset, y};
        }

        EntityID createCubeAnchor(Registry& registry, glm::vec3 scale, std::uint32_t materialID)
        {
            const EntityID entity = registry.createEntity();

            TransformComponent transform;
            transform.scale = scale;
            registry.transforms.insert(entity, transform);

            MeshComponent mesh = MeshBuilder::createTestCube();
            mesh.materialID = materialID;
            registry.meshes.insert(entity, mesh);

            return entity;
        }

        void hideAnchor(Registry& registry, EntityID entity)
        {
            if (auto* transform = registry.transforms.get(entity))
            {
                transform->scale = HiddenAnchorScale;
            }
        }

        void ensureAnchorPoolSize(Registry& registry,
                                  std::vector<EntityID>& pool,
                                  std::size_t targetSize,
                                  glm::vec3 hiddenScale,
                                  std::uint32_t materialID)
        {
            while (pool.size() < targetSize)
            {
                const EntityID entity = createCubeAnchor(registry, hiddenScale, materialID);
                hideAnchor(registry, entity);
                pool.push_back(entity);
            }
        }
    }

    void GameApplication::createGameplayAnchors()
    {
        m_PlayerAnchorEntity = createCubeAnchor(m_Registry, {0.45f, 0.9f, 0.45f}, PlayerAnchorMaterial);
        m_TitanAnchorEntity = createCubeAnchor(m_Registry, {1.4f, 1.0f, 1.4f}, TitanAnchorMaterial);
        ensureAnchorPoolSize(m_Registry,
                             m_EnemyAnchorEntities,
                             Config::ENEMY_MAX_COUNT,
                             HiddenAnchorScale,
                             EnemyAnchorMaterial);

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

        ensureAnchorPoolSize(m_Registry,
                             m_EnemyAnchorEntities,
                             m_GameState.enemies.size(),
                             HiddenAnchorScale,
                             EnemyAnchorMaterial);
        for (std::size_t i = 0; i < m_EnemyAnchorEntities.size(); ++i)
        {
            auto* transform = m_Registry.transforms.get(m_EnemyAnchorEntities[i]);
            if (!transform)
            {
                continue;
            }

            if (i >= m_GameState.enemies.size() || !m_GameState.enemies[i].isAlive)
            {
                transform->scale = HiddenAnchorScale;
                continue;
            }

            const Enemy& enemy = m_GameState.enemies[i];
            const float radius = std::max(enemy.radius, 0.1f);
            transform->position = gameplayToRenderPosition(enemy.position, radius);
            transform->scale = {radius * 2.0f, radius * 1.6f, radius * 2.0f};
        }

        const auto& vehicles = m_VehicleManager.getSpawned();
        ensureAnchorPoolSize(m_Registry,
                             m_VehicleAnchorEntities,
                             vehicles.size(),
                             HiddenAnchorScale,
                             VehicleAnchorMaterial);
        for (std::size_t i = 0; i < m_VehicleAnchorEntities.size(); ++i)
        {
            auto* transform = m_Registry.transforms.get(m_VehicleAnchorEntities[i]);
            if (!transform)
            {
                continue;
            }

            if (i >= vehicles.size())
            {
                transform->scale = HiddenAnchorScale;
                continue;
            }

            const VehicleInstance& vehicle = vehicles[i];
            const float radius = vehicle.config ? std::max(vehicle.config->collisionRadius, 0.25f) : 0.45f;
            transform->position = gameplayToRenderPosition(vehicle.position, 0.35f);
            transform->scale = {radius * 2.2f, 0.7f, radius * 1.6f};
        }

        const auto& workstations = ::WorkstationManager::Get().GetAllStations();
        ensureAnchorPoolSize(m_Registry,
                             m_WorkstationAnchorEntities,
                             workstations.size(),
                             HiddenAnchorScale,
                             WorkstationAnchorMaterial);
        for (std::size_t i = 0; i < m_WorkstationAnchorEntities.size(); ++i)
        {
            auto* transform = m_Registry.transforms.get(m_WorkstationAnchorEntities[i]);
            if (!transform)
            {
                continue;
            }

            if (i >= workstations.size() || !workstations[i] || workstations[i]->isDestroyed)
            {
                transform->scale = HiddenAnchorScale;
                continue;
            }

            const ::WorldWorkstation& station = *workstations[i];
            transform->position = gameplayToRenderPosition(station.x, station.y, 0.45f);
            transform->scale = station.CanUse() ? glm::vec3{0.9f, 0.9f, 0.9f} : glm::vec3{0.75f, 0.35f, 0.75f};
        }
    }

} // namespace bunker
