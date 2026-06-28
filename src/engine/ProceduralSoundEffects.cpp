#include "engine/ProceduralSoundEffects.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace bunker
{

    std::unordered_map<SoundEffectType, sf::SoundBuffer> ProceduralSoundEffects::s_Buffers;
    sf::Sound ProceduralSoundEffects::s_SoundPlayer;

    void ProceduralSoundEffects::initializeDefaults()
    {
        // ── 1. Природа и Окружение Убежища 17 ──
        s_Buffers[SoundEffectType::AmbBunkerVentilationLoop] = makeNoiseDecayBuffer(1.5f, 1.0f, 8000.0f);
        s_Buffers[SoundEffectType::AmbEtherGasHiss] = makeSineSweepBuffer(400.0f, 150.0f, 0.45f, 12000.0f);
        s_Buffers[SoundEffectType::AmbCaveWaterDrip] = makeSineSweepBuffer(2400.0f, 1800.0f, 0.05f, 16000.0f);
        s_Buffers[SoundEffectType::AmbAcidRainMetalRoof] = makeNoiseDecayBuffer(0.25f, 8.0f, 10000.0f);
        s_Buffers[SoundEffectType::AmbAshStormHowl] = makeSineSweepBuffer(180.0f, 320.0f, 1.20f, 18000.0f);
        s_Buffers[SoundEffectType::AmbDeepThunderRumble] = makeNoiseDecayBuffer(2.20f, 2.5f, 30000.0f);
        s_Buffers[SoundEffectType::AmbPipesCreakingMetal] = makeSineSweepBuffer(3200.0f, 2800.0f, 0.15f, 14000.0f);

        // ── 2. Движение Пилота и Фоли ──
        s_Buffers[SoundEffectType::PilotSuitBreathingMask] = makeBreathingBuffer(1.6f, 15000.0f);
        s_Buffers[SoundEffectType::PilotHeartbeatSecondWind] = makeThudBuffer(55.0f, 0.18f, 28000.0f);
        s_Buffers[SoundEffectType::FootstepConcrete1] = makeNoiseDecayBuffer(0.06f, 35.0f, 12000.0f);
        s_Buffers[SoundEffectType::FootstepConcrete2] = makeNoiseDecayBuffer(0.05f, 38.0f, 11500.0f);
        s_Buffers[SoundEffectType::FootstepWaterPuddle1] = makeNoiseDecayBuffer(0.10f, 22.0f, 14000.0f);
        s_Buffers[SoundEffectType::FootstepWaterPuddle2] = makeNoiseDecayBuffer(0.12f, 20.0f, 13500.0f);
        s_Buffers[SoundEffectType::FootstepMetalGrating1] = makeSineSweepBuffer(800.0f, 400.0f, 0.06f, 16000.0f);
        s_Buffers[SoundEffectType::FootstepMetalGrating2] = makeSineSweepBuffer(750.0f, 380.0f, 0.06f, 15500.0f);
        s_Buffers[SoundEffectType::FootstepSludgeErosion1] = makeThudBuffer(90.0f, 0.10f, 15000.0f);
        s_Buffers[SoundEffectType::FoleyStimInjectorHiss] = makeNoiseDecayBuffer(0.08f, 40.0f, 18000.0f);
        s_Buffers[SoundEffectType::FoleyRationCanOpenClick] = makeSineSweepBuffer(4000.0f, 2000.0f, 0.02f, 20000.0f);

        // ── 3. Оружие и Разрушения ──
        s_Buffers[SoundEffectType::WeapXO16ShotSingle] = makeThudBuffer(85.0f, 0.22f, 30000.0f);
        s_Buffers[SoundEffectType::WeapXO16ShotBurst] = makeNoiseDecayBuffer(0.30f, 10.0f, 29000.0f);
        s_Buffers[SoundEffectType::WeapScorchMortarLaunch] = makeSineSweepBuffer(200.0f, 800.0f, 0.35f, 25000.0f);
        s_Buffers[SoundEffectType::WeapThermiteBurnCrackle] = makeNoiseDecayBuffer(0.60f, 5.0f, 18000.0f);
        s_Buffers[SoundEffectType::WeapIonLaserHum] = makeSineSweepBuffer(400.0f, 400.0f, 0.50f, 16000.0f);
        s_Buffers[SoundEffectType::WeapCasingsClinkConcrete] = makeSineSweepBuffer(5000.0f, 3000.0f, 0.03f, 12000.0f);
        s_Buffers[SoundEffectType::WeapReloadHydraulicLatch] = makeThudBuffer(120.0f, 0.15f, 22000.0f);
        s_Buffers[SoundEffectType::HitRicochetMetalWhine] = makeSineSweepBuffer(2500.0f, 6000.0f, 0.12f, 16000.0f);
        s_Buffers[SoundEffectType::GlassShatterSpike] = makeNoiseDecayBuffer(0.18f, 25.0f, 22000.0f);
        s_Buffers[SoundEffectType::CrateBreakThud] = makeNoiseDecayBuffer(0.22f, 15.0f, 24000.0f);

        // ── 4. Танк Титан БТ-7274 ──
        s_Buffers[SoundEffectType::TitanSteamHiss350Bar] = makeNoiseDecayBuffer(0.85f, 3.0f, 17000.0f);
        s_Buffers[SoundEffectType::CockpitSealClank] = makeThudBuffer(110.0f, 0.20f, 26000.0f);
        s_Buffers[SoundEffectType::RamShieldHeavyClang] = makeSineSweepBuffer(450.0f, 200.0f, 0.38f, 28000.0f);
        s_Buffers[SoundEffectType::BucketRigScoopScrape] = makeNoiseDecayBuffer(0.40f, 6.0f, 19000.0f);
        s_Buffers[SoundEffectType::TowCouplerLatchClick] = makeSineSweepBuffer(1800.0f, 1200.0f, 0.05f, 21000.0f);

        // ── 5. Рой RAY и Противники ──
        s_Buffers[SoundEffectType::SwarmVerminChitterGroup] = makeChitterBuffer(0.35f, 18000.0f);
        s_Buffers[SoundEffectType::SwarmEggHatchSquelch] = makeNoiseDecayBuffer(0.14f, 18.0f, 20000.0f);
        s_Buffers[SoundEffectType::GhoulScreechAggro] = makeSineSweepBuffer(900.0f, 2200.0f, 0.40f, 25000.0f);
        s_Buffers[SoundEffectType::RayDroneHoverHum] = makeSineSweepBuffer(800.0f, 820.0f, 0.60f, 14000.0f);
        s_Buffers[SoundEffectType::RayOpticSparkBlind] = makeNoiseDecayBuffer(0.08f, 50.0f, 22000.0f);

        // ── 6. Терминалы и РобКо ──
        s_Buffers[SoundEffectType::UiRobCoKeyClickAmber] = makeNoiseDecayBuffer(0.02f, 65.0f, 14000.0f);
        s_Buffers[SoundEffectType::UiAccessGrantedBeep] = makeSineSweepBuffer(1500.0f, 2000.0f, 0.10f, 20000.0f);

        std::cout << "[PROCEDURAL AUDIO] Синтезированы все 40 акустических PCM-классов в памяти!" << std::endl;
    }

    void ProceduralSoundEffects::play(SoundEffectType type, float volume)
    {
        if (s_Buffers.empty())
        {
            initializeDefaults();
        }

        auto it = s_Buffers.find(type);
        if (it != s_Buffers.end())
        {
            s_SoundPlayer.setBuffer(it->second);
            s_SoundPlayer.setVolume(volume);
            s_SoundPlayer.play();
        }
    }

    sf::SoundBuffer ProceduralSoundEffects::makeSineSweepBuffer(float fStart, float fEnd, float dur, float vol)
    {
        const unsigned int sr = 44100;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float freq = fStart + (fEnd - fStart) * (t / dur);
            float val = std::sin(2.0f * 3.14159265f * freq * t);
            float env = 1.0f - (t / dur);
            samples[i] = static_cast<sf::Int16>(val * env * vol);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer ProceduralSoundEffects::makeNoiseDecayBuffer(float dur, float decay, float vol)
    {
        const unsigned int sr = 44100;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float noise = (static_cast<float>(std::rand() % 20000) / 10000.0f) - 1.0f;
            float env = std::exp(-decay * t);
            samples[i] = static_cast<sf::Int16>(noise * env * vol);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer ProceduralSoundEffects::makeThudBuffer(float freq, float dur, float vol)
    {
        const unsigned int sr = 44100;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float val = std::sin(2.0f * 3.14159265f * freq * t);
            float env = 1.0f - (t / dur);
            samples[i] = static_cast<sf::Int16>(val * env * vol);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer ProceduralSoundEffects::makeBreathingBuffer(float dur, float vol)
    {
        const unsigned int sr = 44100;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float cycle = std::sin(2.0f * 3.14159265f * 0.6f * t); // Ритм вдоха/выдоха маски
            float noise = (static_cast<float>(std::rand() % 20000) / 10000.0f) - 1.0f;
            float env = std::max(0.0f, cycle);
            samples[i] = static_cast<sf::Int16>(noise * env * vol * 0.35f);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer ProceduralSoundEffects::makeChitterBuffer(float dur, float vol)
    {
        const unsigned int sr = 44100;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float freq = 3800.0f + 800.0f * std::sin(2.0f * 3.14159265f * 45.0f * t); // Стрекотание насекомых
            float val = std::sin(2.0f * 3.14159265f * freq * t);
            float env = 1.0f - (t / dur);
            samples[i] = static_cast<sf::Int16>(val * env * vol);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

} // namespace bunker
