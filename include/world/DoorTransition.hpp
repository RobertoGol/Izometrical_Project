#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"

#include <optional>
#include <string>
#include <vector>

namespace bunker
{
    enum class DoorTransitionPhase
    {
        Idle,
        FadingOut,
        Loading,
        FadingIn
    };

    struct DoorLink
    {
        int id = 0;
        Vector3D triggerPosition{};
        std::string linkTarget;
        Vector3D destinationSpawnPoint{};
        float activationRadius = 1.25f;
    };

    class DoorTransition
    {
      public:
        DoorTransition();

        void registerDoor(const DoorLink& link);
        void registerDefaultLinks();
        std::optional<DoorLink> findNearestDoor(const Vector3D& playerPosition) const;
        bool requestTransition(int doorId);
        void update(GameState& gameState, float dt);

        DoorTransitionPhase phase() const { return m_Phase; }
        float fadeAlpha() const { return m_FadeAlpha; }
        const std::string& activeWorldId() const { return m_ActiveWorldId; }
        const std::vector<DoorLink>& links() const { return m_Links; }

      private:
        const DoorLink* findDoorById(int doorId) const;
        void completeLoad(GameState& gameState);

        std::vector<DoorLink> m_Links;
        DoorTransitionPhase m_Phase = DoorTransitionPhase::Idle;
        int m_PendingDoorId = -1;
        float m_FadeAlpha = 0.0f;
        float m_LoadTimer = 0.0f;
        std::string m_ActiveWorldId = "Vault17_Main";
    };
} // namespace bunker
