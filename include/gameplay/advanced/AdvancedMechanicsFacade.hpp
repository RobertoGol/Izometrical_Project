#pragma once

#include "gameplay/advanced/AdvancedMechanicsSurvivalSystems.hpp"
#include "gameplay/advanced/AdvancedMechanicsWorldSystems.hpp"
#include "gameplay/advanced/AdvancedMechanicsProgressionSystems.hpp"
#include "gameplay/advanced/AdvancedMechanicsCampSystems.hpp"
#include "gameplay/advanced/AdvancedMechanicsToolGunSystems.hpp"
#include "gameplay/advanced/AdvancedMechanicsServices.hpp"
namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 15) FACADE: одна точка входа для main.cpp
    // ═══════════════════════════════════════════════════════════════════════════════

    class AdvancedMechanics
    {
      public:
        void initialize(GameState& gs, PlayerInventory& inv);
        void update(GameState& gs,
                    PlayerInventory& inv,
                    const InputSnapshot& input,
                    float dt,
                    const CampPlacementValidator& campValidator);
        void onExplosion(GameState& gs, Vector3D pos, float radius, float damage);

        WeatherSystem weather;
        RadioTapeSystem radio;
        SurvivalSystem survival;
        TankUtilitySystem tankUtility;
        ReactiveWorldSystem reactive;
        StoryRouteSystem story;
        SkillSystem skills;
        LootGenerator loot;
        CampSystem camp;
        PrefabLibrary prefabs;
        ToolGunSystem toolgun;
        LanlineServices lanline;
        ProfileSessionSystem profile;
        PlayerProfile playerProfile;
    };


} // namespace bunker
