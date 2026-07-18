#include "gameplay/InteractionManager.hpp"

#include "WorkstationManager.hpp"
#include "engine/Log.hpp"
#include "persistence/ModularEquipmentSystem.hpp"
#include "ui/TerminalUI.hpp"
#include "vehicles/VehicleManager.hpp"
#include "world/DoorTransition.hpp"
#include "world/WorldSession.hpp"

#include <algorithm>
#include <cmath>

namespace bunker
{
    namespace
    {
        constexpr float TerminalRangeSq = 1.8f * 1.8f;
        constexpr float VehicleRangeSq = 2.0f * 2.0f;
        constexpr float ContainerPlayerRangeSq = 1.5f * 1.5f;
        constexpr float ContainerAimRangeSq = 1.25f * 1.25f;
        constexpr float PickupRangeSq = 1.25f * 1.25f;
        constexpr float WorkstationRange = 1.8f;
        constexpr float NpcRangeSq = 1.6f * 1.6f;
        constexpr float LineOfSightStep = 0.25f;

        float distanceSq2D(const Vector3D& a, const Vector3D& b)
        {
            const float dx = a.x - b.x;
            const float dy = a.y - b.y;
            return dx * dx + dy * dy;
        }

        WorkstationType toEquipmentStationType(::WorkstationType stationType)
        {
            switch (stationType)
            {
            case ::WorkstationType::ArmorWorkbench:
                return WorkstationType::ArmorWorkbench;
            case ::WorkstationType::WeaponsWorkbench:
                return WorkstationType::WeaponsWorkbench;
            case ::WorkstationType::TinkerWorkbench:
                return WorkstationType::TinkerWorkbench;
            case ::WorkstationType::ChemStation:
                return WorkstationType::ChemStation;
            case ::WorkstationType::TankMaintenanceBay:
                return WorkstationType::TankMaintenanceBay;
            case ::WorkstationType::Terminal:
                return WorkstationType::Terminal;
            case ::WorkstationType::Storage:
                return WorkstationType::Storage;
            case ::WorkstationType::None:
            default:
                return WorkstationType::None;
            }
        }

        const char* craftResultName(CraftResult result)
        {
            switch (result)
            {
            case CraftResult::Success:
                return "crafted";
            case CraftResult::InvalidRecipeIndex:
                return "no valid recipe";
            case CraftResult::InsufficientScrap:
                return "missing scrap";
            case CraftResult::InsufficientCircuits:
                return "missing circuits";
            case CraftResult::InsufficientCoreEnergy:
                return "missing core energy";
            default:
                return "unknown result";
            }
        }
    } // namespace

    std::optional<InteractionTarget> InteractionManager::queryBestTarget(const GameState& gameState,
                                                                         const TerminalManager& terminals,
                                                                         const VehicleManager& vehicles,
                                                                         const DoorTransition& doors) const
    {
        PhysicsWorld world(gameState);
        std::optional<InteractionTarget> bestTarget;

        const auto& terminalList = terminals.getTerminals();
        for (int i = 0; i < static_cast<int>(terminalList.size()); ++i)
        {
            const float distSq = distanceSq2D(gameState.playerPos, terminalList[i].position);
            if (distSq <= TerminalRangeSq)
            {
                InteractionTarget target{};
                target.type = InteractionType::Terminal;
                target.position = terminalList[i].position;
                target.index = i;
                target.distanceSq = distSq;
                target.label = terminalList[i].title;
                considerTarget(world, gameState.playerPos, target, bestTarget);
            }
        }

        if (const std::optional<DoorLink> door = doors.findNearestDoor(gameState.playerPos))
        {
            InteractionTarget target{};
            target.type = InteractionType::Door;
            target.position = door->triggerPosition;
            target.index = door->id;
            target.distanceSq = distanceSq2D(gameState.playerPos, door->triggerPosition);
            target.label = door->linkTarget;
            considerTarget(world, gameState.playerPos, target, bestTarget);
        }

        const auto& spawnedVehicles = vehicles.getSpawned();
        for (int i = 0; i < static_cast<int>(spawnedVehicles.size()); ++i)
        {
            if (spawnedVehicles[i].isOccupied)
            {
                continue;
            }

            const float distSq = distanceSq2D(gameState.playerPos, spawnedVehicles[i].position);
            if (distSq <= VehicleRangeSq)
            {
                InteractionTarget target{};
                target.type = InteractionType::Vehicle;
                target.position = spawnedVehicles[i].position;
                target.index = i;
                target.distanceSq = distSq;
                target.label = spawnedVehicles[i].config ? spawnedVehicles[i].config->displayName : "Vehicle";
                considerTarget(world, gameState.playerPos, target, bestTarget);
            }
        }

        for (int i = 0; i < static_cast<int>(gameState.lootContainers.size()); ++i)
        {
            const LootContainer& container = gameState.lootContainers[i];
            if (container.isOpened)
            {
                continue;
            }

            const float playerDistSq = distanceSq2D(gameState.playerPos, container.position);
            const float aimDistSq = distanceSq2D(gameState.mouseWorldPos, container.position);
            if (playerDistSq <= ContainerPlayerRangeSq && aimDistSq <= ContainerAimRangeSq)
            {
                InteractionTarget target{};
                target.type = InteractionType::Container;
                target.position = container.position;
                target.index = i;
                target.distanceSq = playerDistSq;
                target.label = "Container";
                considerTarget(world, gameState.playerPos, target, bestTarget);
            }
        }

        for (int i = 0; i < static_cast<int>(gameState.loosePickups.size()); ++i)
        {
            const LoosePickup& pickup = gameState.loosePickups[i];
            if (pickup.collected)
            {
                continue;
            }

            const float distSq = distanceSq2D(gameState.playerPos, pickup.position);
            const float radiusSq = pickup.interactionRadius * pickup.interactionRadius;
            if (distSq <= std::max(PickupRangeSq, radiusSq))
            {
                InteractionTarget target{};
                target.type = InteractionType::Pickup;
                target.position = pickup.position;
                target.index = i;
                target.distanceSq = distSq;
                target.label = pickup.item.displayName;
                considerTarget(world, gameState.playerPos, target, bestTarget);
            }
        }

        if (const ::WorldWorkstation* station =
                ::WorkstationManager::Get().FindNearest(gameState.playerPos.x, gameState.playerPos.y, WorkstationRange))
        {
            if (station->CanUse())
            {
                InteractionTarget target{};
                target.type = InteractionType::CraftingStation;
                target.position = {station->x, station->y, 0.0f};
                target.index = station->objectID;
                target.distanceSq = distanceSq2D(gameState.playerPos, target.position);
                target.label = "Workbench";
                considerTarget(world, gameState.playerPos, target, bestTarget);
            }
        }

        for (int i = 0; i < static_cast<int>(gameState.neutralNpcs.size()); ++i)
        {
            const NeutralNpc& npc = gameState.neutralNpcs[i];
            const float distSq = distanceSq2D(gameState.playerPos, npc.position);
            const float radiusSq = npc.interactionRadius * npc.interactionRadius;
            if (distSq <= std::max(NpcRangeSq, radiusSq))
            {
                InteractionTarget target{};
                target.type = InteractionType::NPC;
                target.position = npc.position;
                target.index = i;
                target.distanceSq = distSq;
                target.label = npc.displayName;
                considerTarget(world, gameState.playerPos, target, bestTarget);
            }
        }

        return bestTarget;
    }

    bool InteractionManager::tryInteract(GameState& gameState,
                                         TerminalManager& terminals,
                                         VehicleManager& vehicles,
                                         DoorTransition& doors,
                                         WorldSession& worldSession,
                                         PlayerInventory& inventory,
                                         ModularEquipmentSystem& equipmentSystem)
    {
        m_HighlightedTarget = queryBestTarget(gameState, terminals, vehicles, doors);
        if (!m_HighlightedTarget)
        {
            return false;
        }

        switch (m_HighlightedTarget->type)
        {
        case InteractionType::Terminal:
            return terminals.tryInteractTerminal(gameState);
        case InteractionType::Vehicle:
            return vehicles.mountNearest(gameState);
        case InteractionType::Container:
            worldSession.interactWithContainers(gameState, inventory, m_HighlightedTarget->position);
            return true;
        case InteractionType::Door:
            return doors.requestTransition(m_HighlightedTarget->index);
        case InteractionType::Pickup:
        {
            const int index = m_HighlightedTarget->index;
            if (index < 0 || index >= static_cast<int>(gameState.loosePickups.size()))
            {
                return false;
            }

            LoosePickup& pickup = gameState.loosePickups[index];
            if (pickup.collected)
            {
                return false;
            }

            if (!inventory.addItem(pickup.item.itemID,
                                   pickup.item.type,
                                   pickup.item.quantity,
                                   pickup.item.weightPerUnit,
                                   pickup.item.displayName))
            {
                bunker::logInfo() << "[PICKUP] Inventory full, could not pick up " << pickup.item.displayName
                                  << "." << std::endl;
                return false;
            }

            pickup.collected = true;
            bunker::logInfo() << "[PICKUP] Picked up " << pickup.item.displayName << "." << std::endl;
            return true;
        }
        case InteractionType::CraftingStation:
        {
            const ::WorldWorkstation* station = ::WorkstationManager::Get().FindByObjectID(m_HighlightedTarget->index);
            if (station == nullptr || !station->CanUse())
            {
                return false;
            }

            const WorkstationType equipmentStationType = toEquipmentStationType(station->type);
            const auto recipes = equipmentSystem.getRecipesForStation(equipmentStationType);
            if (recipes.empty())
            {
                bunker::logInfo() << "[WORKSTATION] Opened station " << station->objectID
                                  << "; no direct craft recipe is registered yet." << std::endl;
                return true;
            }

            const auto& allRecipes = equipmentSystem.recipes();
            const auto found = std::find_if(allRecipes.begin(), allRecipes.end(), [&](const CraftingRecipe& recipe) {
                return recipe.resultItemID == recipes.front().resultItemID;
            });
            if (found == allRecipes.end())
            {
                return false;
            }

            const int recipeIndex = static_cast<int>(std::distance(allRecipes.begin(), found));
            const CraftResult result = equipmentSystem.craftItem(inventory, recipeIndex);
            bunker::logInfo() << "[WORKSTATION] " << craftResultName(result) << " at station " << station->objectID
                              << "." << std::endl;
            return result == CraftResult::Success;
        }
        case InteractionType::NPC:
        {
            const int index = m_HighlightedTarget->index;
            if (index < 0 || index >= static_cast<int>(gameState.neutralNpcs.size()))
            {
                return false;
            }

            NeutralNpc& npc = gameState.neutralNpcs[index];
            npc.hasTalked = true;
            bunker::logInfo() << "[NPC] " << npc.displayName << ": Stay sharp, pilot." << std::endl;
            return true;
        }
        case InteractionType::None:
        default:
            return false;
        }
    }

    void InteractionManager::refreshHighlightedTarget(const GameState& gameState,
                                                      const TerminalManager& terminals,
                                                      const VehicleManager& vehicles,
                                                      const DoorTransition& doors)
    {
        m_HighlightedTarget = queryBestTarget(gameState, terminals, vehicles, doors);
    }

    bool InteractionManager::hasLineOfSight(const PhysicsWorld& world, const Vector3D& from, const Vector3D& to) const
    {
        const float dx = to.x - from.x;
        const float dy = to.y - from.y;
        const float distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= 0.001f)
        {
            return true;
        }

        const int steps = static_cast<int>(distance / LineOfSightStep);
        for (int i = 1; i < steps; ++i)
        {
            const float t = static_cast<float>(i) / static_cast<float>(steps);
            const int tileX = static_cast<int>(from.x + dx * t);
            const int tileY = static_cast<int>(from.y + dy * t);
            if (world.isSolidTile(tileX, tileY))
            {
                return false;
            }
        }

        return true;
    }

    void InteractionManager::considerTarget(const PhysicsWorld& world,
                                            const Vector3D& playerPosition,
                                            InteractionTarget candidate,
                                            std::optional<InteractionTarget>& bestTarget) const
    {
        candidate.hasLineOfSight = hasLineOfSight(world, playerPosition, candidate.position);
        if (!candidate.hasLineOfSight)
        {
            return;
        }

        if (!bestTarget || candidate.distanceSq < bestTarget->distanceSq)
        {
            bestTarget = candidate;
        }
    }
} // namespace bunker
