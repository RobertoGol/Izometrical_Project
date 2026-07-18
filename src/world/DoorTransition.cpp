#include "world/DoorTransition.hpp"

#include <algorithm>
#include <filesystem>
#include <utility>

namespace bunker
{
    namespace
    {
        constexpr float FadeSpeed = 3.0f;
        constexpr float SimulatedLoadSeconds = 0.15f;

        float distanceSq2D(const Vector3D& a, const Vector3D& b)
        {
            const float dx = a.x - b.x;
            const float dy = a.y - b.y;
            return dx * dx + dy * dy;
        }
    } // namespace

    DoorTransition::DoorTransition()
    {
        registerDefaultLinks();
    }

    void DoorTransition::registerDoor(const DoorLink& link)
    {
        const auto existing = std::find_if(m_Links.begin(), m_Links.end(), [&](const DoorLink& door) {
            return door.id == link.id;
        });

        if (existing != m_Links.end())
        {
            *existing = link;
            return;
        }

        m_Links.push_back(link);
    }

    void DoorTransition::registerDefaultLinks()
    {
        if (!m_Links.empty())
        {
            return;
        }

        registerDoor({1, {8.0f, 5.0f, 0.0f}, "Vault17_ServiceHall", {9.0f, 6.0f, 0.0f}, 1.35f});
        registerDoor({2, {12.0f, 14.0f, 0.0f}, "Vault17_Hangar", {11.0f, 13.0f, 0.0f}, 1.35f});
    }

    void DoorTransition::setWorldLoader(WorldLoader loader)
    {
        m_WorldLoader = std::move(loader);
    }

    std::optional<DoorLink> DoorTransition::findNearestDoor(const Vector3D& playerPosition) const
    {
        const DoorLink* bestDoor = nullptr;
        float bestDistSq = 0.0f;

        for (const DoorLink& door : m_Links)
        {
            const float maxDistSq = door.activationRadius * door.activationRadius;
            const float distSq = distanceSq2D(playerPosition, door.triggerPosition);
            if (distSq <= maxDistSq && (bestDoor == nullptr || distSq < bestDistSq))
            {
                bestDoor = &door;
                bestDistSq = distSq;
            }
        }

        if (bestDoor == nullptr)
        {
            return std::nullopt;
        }

        return *bestDoor;
    }

    bool DoorTransition::requestTransition(int doorId)
    {
        if (m_Phase != DoorTransitionPhase::Idle || findDoorById(doorId) == nullptr)
        {
            return false;
        }

        m_PendingDoorId = doorId;
        m_Phase = DoorTransitionPhase::FadingOut;
        return true;
    }

    void DoorTransition::restorePersistence(std::string activeWorldId, std::vector<int> openedDoorIds)
    {
        if (!activeWorldId.empty())
        {
            m_ActiveWorldId = std::move(activeWorldId);
        }
        m_OpenedDoorIds = std::move(openedDoorIds);
    }

    void DoorTransition::update(GameState& gameState, float dt)
    {
        switch (m_Phase)
        {
        case DoorTransitionPhase::Idle:
            m_FadeAlpha = 0.0f;
            break;
        case DoorTransitionPhase::FadingOut:
            m_FadeAlpha = std::min(1.0f, m_FadeAlpha + FadeSpeed * dt);
            if (m_FadeAlpha >= 1.0f)
            {
                m_LoadTimer = SimulatedLoadSeconds;
                m_Phase = DoorTransitionPhase::Loading;
            }
            break;
        case DoorTransitionPhase::Loading:
            m_LoadTimer -= dt;
            if (m_LoadTimer <= 0.0f)
            {
                completeLoad(gameState);
                m_Phase = DoorTransitionPhase::FadingIn;
            }
            break;
        case DoorTransitionPhase::FadingIn:
            m_FadeAlpha = std::max(0.0f, m_FadeAlpha - FadeSpeed * dt);
            if (m_FadeAlpha <= 0.0f)
            {
                m_Phase = DoorTransitionPhase::Idle;
                m_PendingDoorId = -1;
            }
            break;
        default:
            m_Phase = DoorTransitionPhase::Idle;
            m_PendingDoorId = -1;
            m_FadeAlpha = 0.0f;
            break;
        }
    }

    const DoorLink* DoorTransition::findDoorById(int doorId) const
    {
        const auto found = std::find_if(m_Links.begin(), m_Links.end(), [&](const DoorLink& door) {
            return door.id == doorId;
        });
        return found == m_Links.end() ? nullptr : &(*found);
    }

    void DoorTransition::completeLoad(GameState& gameState)
    {
        const DoorLink* door = findDoorById(m_PendingDoorId);
        if (door == nullptr)
        {
            return;
        }

        bool loaded = false;
        if (m_WorldLoader)
        {
            loaded = m_WorldLoader(*door, gameState);
        }

        if (!loaded)
        {
            const std::filesystem::path mapPath = std::filesystem::path("assets/worlds") / (door->linkTarget + ".bwld");
            loaded = std::filesystem::exists(mapPath);
        }

        m_ActiveWorldId = door->linkTarget;
        gameState.mapMeta.currentMapName = door->linkTarget;
        gameState.playerPos = door->destinationSpawnPoint;
        gameState.cameraTarget = door->destinationSpawnPoint;
        gameState.mouseWorldPos = door->destinationSpawnPoint;
        markDoorOpened(door->id);
        (void)loaded;
    }

    void DoorTransition::markDoorOpened(int doorId)
    {
        if (std::find(m_OpenedDoorIds.begin(), m_OpenedDoorIds.end(), doorId) == m_OpenedDoorIds.end())
        {
            m_OpenedDoorIds.push_back(doorId);
        }
    }
} // namespace bunker
