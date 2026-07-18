#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "persistence/Inventory.hpp"
#include "physics/PhysicsWorld.hpp"

#include <optional>
#include <string>

namespace bunker
{
    class TerminalManager;
    class VehicleManager;
    class WorldSession;
    class DoorTransition;

    enum class InteractionType
    {
        None,
        Container,
        Door,
        Terminal,
        Pickup,
        CraftingStation,
        Vehicle
    };

    struct InteractionTarget
    {
        InteractionType type = InteractionType::None;
        Vector3D position{};
        int index = -1;
        float distanceSq = 0.0f;
        bool hasLineOfSight = false;
        std::string label;
    };

    class InteractionManager
    {
      public:
        std::optional<InteractionTarget> queryBestTarget(const GameState& gameState,
                                                         const TerminalManager& terminals,
                                                         const VehicleManager& vehicles,
                                                         const DoorTransition& doors) const;

        bool tryInteract(GameState& gameState,
                         TerminalManager& terminals,
                         VehicleManager& vehicles,
                         DoorTransition& doors,
                         WorldSession& worldSession,
                         PlayerInventory& inventory);

        const std::optional<InteractionTarget>& highlightedTarget() const { return m_HighlightedTarget; }

      private:
        bool hasLineOfSight(const PhysicsWorld& world, const Vector3D& from, const Vector3D& to) const;
        void considerTarget(const PhysicsWorld& world,
                            const Vector3D& playerPosition,
                            InteractionTarget candidate,
                            std::optional<InteractionTarget>& bestTarget) const;

        std::optional<InteractionTarget> m_HighlightedTarget;
    };
} // namespace bunker
