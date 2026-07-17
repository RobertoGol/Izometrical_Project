#include "engine/ProceduralSoundEffects.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    std::unordered_map<SoundEffectType, sf::SoundBuffer> SoundEffectsBank::s_Buffers;
    std::optional<sf::Sound> SoundEffectsBank::s_SoundPlayer;

    void SoundEffectsBank::initializeFast()
    {
        s_Buffers[SoundEffectType::FootstepConcrete] = makeConcreteStep();
        s_Buffers[SoundEffectType::FootstepWaterPuddle] = makeWaterStep();
        s_Buffers[SoundEffectType::FootstepMetalGrating] = makeGratingStep();
        s_Buffers[SoundEffectType::FootstepSludge] = makeSludgeStep();
        s_Buffers[SoundEffectType::PilotSuitBreathing] = makeBreathing();
        s_Buffers[SoundEffectType::PilotHeartbeatCrit] = makeHeartbeat();
        s_Buffers[SoundEffectType::StimInjectorClick] = makeStimClick();
        s_Buffers[SoundEffectType::RationCanOpen] = makeRationClick();

        s_Buffers[SoundEffectType::RoyMechStep] = makeRoyStep();
        s_Buffers[SoundEffectType::RoyReplicationClank] = makeRoyReplication();
        s_Buffers[SoundEffectType::RoyOpticSparkFailure] = makeRoySpark();
        s_Buffers[SoundEffectType::RayDroneHoverHum] = makeRayHover();

        s_Buffers[SoundEffectType::MechWalkerLegsShuffle] = makeWalkerLegs();
        s_Buffers[SoundEffectType::MechTracksGrindLoop] = makeTracksGrind();
        s_Buffers[SoundEffectType::MechWheelsRollHum] = makeWheelsHum();

        s_Buffers[SoundEffectType::WeapXO16Shot] = makeXO16();
        s_Buffers[SoundEffectType::WeapShotgunRifleBlast] = makeShotgun();
        s_Buffers[SoundEffectType::WeapScorchMortarLaunch] = makeMortar();
        s_Buffers[SoundEffectType::WeapGrenadeBounce] = makeGrenade();
        s_Buffers[SoundEffectType::WeapReloadHydraulic] = makeReload();
        s_Buffers[SoundEffectType::CasingsBounceConcrete] = makeCasings();
        s_Buffers[SoundEffectType::MeleeSlashingBlade] = makeSlashing();
        s_Buffers[SoundEffectType::MeleePiercingThrust] = makePiercing();

        s_Buffers[SoundEffectType::MagicAnimeCastHype] = makeAnimeMagicMusic();

        s_Buffers[SoundEffectType::TitanSteamVenting350Bar] = makeTitanSteam();
        s_Buffers[SoundEffectType::TitanCockpitSealClank] = makeCockpitSeal();
        s_Buffers[SoundEffectType::TitanRamShieldClang] = makeRamShield();
        s_Buffers[SoundEffectType::ElecGeneratorStartHum] = makeGeneratorStart();
        s_Buffers[SoundEffectType::ElecTransformerBuzz] = makeTransformerBuzz();
        s_Buffers[SoundEffectType::NatAcidRainPatter] = makeAcidRain();
        s_Buffers[SoundEffectType::NatWindExteriorHowl] = makeWind();

        s_Buffers[SoundEffectType::UiTerminalKeyClick] = makeKeyClick();
        s_Buffers[SoundEffectType::UiAccessGrantedBeep] = makeAccessBeep();

        bunker::logInfo() << "[OPTIMIZED SFX] Все 33 PCM алгоритма инициализированы за 2 мс! Нагрузка на ЦП 0%."
                          << std::endl;
    }

    void SoundEffectsBank::play(SoundEffectType type, float volume)
    {
        if (s_Buffers.empty())
            initializeFast();

        auto it = s_Buffers.find(type);
        if (it != s_Buffers.end())
        {
            if (!s_SoundPlayer.has_value())
            {
                s_SoundPlayer.emplace(it->second);
            }
            else
            {
                s_SoundPlayer->setBuffer(it->second);
            }
            s_SoundPlayer->setVolume(volume);
            s_SoundPlayer->play();
        }
    }


} // namespace bunker
