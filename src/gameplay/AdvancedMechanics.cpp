#include "gameplay/AdvancedMechanics.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 15) FACADE
    // ═══════════════════════════════════════════════════════════════════════════════

    void AdvancedMechanics::initialize(GameState& gs, PlayerInventory& inv)
    {
        (void)inv;
        reactive.seedDefault();
        loot.normalizeWorldLoot(gs);
        profile.startSession(playerProfile);
        lanline.createLocalLobby(playerProfile.playerName);

        ObjModel testModel = ObjModelLoader::load("assets/models/bastion.obj");
        if (!testModel.empty())
        {
            bunker::logInfo() << "[OBJ LOADER] Модель bastion.obj успешно загружена: " << testModel.vertices.size()
                              << " вершин, " << testModel.faces.size() << " полигонов." << std::endl;
        }
    }

    void AdvancedMechanics::update(GameState& gs, PlayerInventory& inv, const InputSnapshot& input, float dt)
    {
        profile.update(playerProfile, dt);
        weather.update(gs, dt);
        radio.update(dt);
        story.update(gs, &radio);
        survival.update(gs, inv, dt);
        tankUtility.update(gs, dt);
        reactive.update(gs, dt);
        camp.updatePreview(gs, gs.mouseWorldPos);
        camp.updateTurrets(gs, dt);
        lanline.update(gs, inv, dt);

        if (input.toggleCamp)
        {
            camp.toggle();
        }
        if (input.switchWeapon && gs.playerMode == UnitMode::Titan)
        {
            tankUtility.nextUtility();
        }
        if (input.dismountVehicle && gs.playerMode == UnitMode::Titan)
        {
            tankUtility.swapSeat();
        }
        if (input.interact)
        {
            tankUtility.repairInHangar(gs, inv);
        }
    }

    void AdvancedMechanics::onExplosion(GameState& gs, Vector3D pos, float radius, float damage)
    {
        reactive.damageAt(gs, pos, radius, damage, radius * 1.8f);
    }

} // namespace bunker
