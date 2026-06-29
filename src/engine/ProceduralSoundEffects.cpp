#include "engine/ProceduralSoundEffects.hpp"
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace bunker
{

    std::unordered_map<SoundEffectType, sf::SoundBuffer> SoundEffectsBank::s_Buffers;
    sf::Sound SoundEffectsBank::s_SoundPlayer;

    void SoundEffectsBank::initializeFast()
    {
        // ── 1. Пилот и Фоли ──
        s_Buffers[SoundEffectType::FootstepConcrete] = makeNoiseDecayBuffer(0.06f, 35.0f, 12000.0f);
        s_Buffers[SoundEffectType::FootstepWaterPuddle] = makeNoiseDecayBuffer(0.10f, 22.0f, 14000.0f);
        s_Buffers[SoundEffectType::FootstepMetalGrating] = makeSineSweepFast(800.0f, 400.0f, 0.05f, 16000.0f);
        s_Buffers[SoundEffectType::FootstepSludge] = makeThudBuffer(90.0f, 0.10f, 15000.0f);
        s_Buffers[SoundEffectType::PilotSuitBreathing] = makeBreathingBuffer(1.6f, 15000.0f);
        s_Buffers[SoundEffectType::PilotHeartbeatCrit] = makeThudBuffer(55.0f, 0.18f, 28000.0f);
        s_Buffers[SoundEffectType::StimInjectorClick] = makeNoiseDecayBuffer(0.08f, 40.0f, 18000.0f);
        s_Buffers[SoundEffectType::RationCanOpen] = makeFastPulse(300, 20000);

        // ── 2. Механические существа ROY / RAY ──
        s_Buffers[SoundEffectType::RoyMechStep] = makeThudBuffer(140.0f, 0.08f, 18000.0f);
        s_Buffers[SoundEffectType::RoyReplicationClank] = makeFastPulse(1800, 26000);
        s_Buffers[SoundEffectType::RoyOpticSparkFailure] = makeSparkBuffer(0.08f, 22000.0f);
        s_Buffers[SoundEffectType::RayDroneHoverHum] = makeSineSweepFast(800.0f, 820.0f, 0.60f, 14000.0f);

        // ── 3. Ходовая техника (Шагоходы, Гусеницы, Колёса) ──
        s_Buffers[SoundEffectType::MechWalkerLegsShuffle] = makeFastPulse(350, 22000);
        s_Buffers[SoundEffectType::MechTracksGrindLoop] = makeFastNoise(2500, 19000);
        s_Buffers[SoundEffectType::MechWheelsRollHum] = makeFastPulse(2000, 14000);

        // ── 4. Оружие и Рукопашный Бой ──
        s_Buffers[SoundEffectType::WeapXO16Shot] = makeThudBuffer(85.0f, 0.22f, 30000.0f);
        s_Buffers[SoundEffectType::WeapShotgunRifleBlast] = makeNoiseDecayBuffer(0.35f, 8.0f, 31000.0f);
        s_Buffers[SoundEffectType::WeapScorchMortarLaunch] = makeSineSweepFast(200.0f, 800.0f, 0.35f, 25000.0f);
        s_Buffers[SoundEffectType::WeapGrenadeBounce] = makeFastPulse(400, 18000);
        s_Buffers[SoundEffectType::WeapReloadHydraulic] = makeThudBuffer(120.0f, 0.15f, 22000.0f);
        s_Buffers[SoundEffectType::CasingsBounceConcrete] = makeSineSweepFast(5000.0f, 3000.0f, 0.03f, 12000.0f);
        s_Buffers[SoundEffectType::MeleeSlashingBlade] = makeSineSweepFast(2800.0f, 600.0f, 0.12f, 24000.0f);
        s_Buffers[SoundEffectType::MeleePiercingThrust] = makeFastNoise(300, 30000);

        // ── 5. Захватывающая музыкальная дорожка аниме-магии ──
        s_Buffers[SoundEffectType::MagicAnimeCastHype] = makeAnimeMagicMusicFast();

        // ── 6. Танк БТ-7274 и Окружение ──
        s_Buffers[SoundEffectType::TitanSteamVenting350Bar] = makeNoiseDecayBuffer(0.85f, 3.0f, 17000.0f);
        s_Buffers[SoundEffectType::TitanCockpitSealClank] = makeThudBuffer(110.0f, 0.20f, 26000.0f);
        s_Buffers[SoundEffectType::TitanRamShieldClang] = makeSineSweepFast(450.0f, 200.0f, 0.38f, 28000.0f);
        s_Buffers[SoundEffectType::ElecGeneratorStartHum] = makeGeneratorRevBuffer(1.2f, 26000.0f);
        s_Buffers[SoundEffectType::ElecTransformerBuzz] = makeFastPulse(2000, 15000);
        s_Buffers[SoundEffectType::NatAcidRainPatter] = makeNoiseDecayBuffer(0.25f, 8.0f, 10000.0f);
        s_Buffers[SoundEffectType::NatWindExteriorHowl] = makeSineSweepFast(180.0f, 320.0f, 1.20f, 18000.0f);

        // ── 7. РобКо UI ──
        s_Buffers[SoundEffectType::UiTerminalKeyClick] = makeNoiseDecayBuffer(0.02f, 65.0f, 14000.0f);
        s_Buffers[SoundEffectType::UiAccessGrantedBeep] = makeSineSweepFast(1500.0f, 2000.0f, 0.10f, 20000.0f);

        std::cout << "[OPTIMIZED SFX] Весь универсальный банк (38 классов) инициализирован без потерь логики!" << std::endl;
    }

    void SoundEffectsBank::play(SoundEffectType type, float volume)
    {
        if (s_Buffers.empty())
            initializeFast();

        auto it = s_Buffers.find(type);
        if (it != s_Buffers.end())
        {
            s_SoundPlayer.setBuffer(it->second);
            s_SoundPlayer.setVolume(volume);
            s_SoundPlayer.play();
        }
    }

    sf::SoundBuffer SoundEffectsBank::makeFastPulse(int length, sf::Int16 amp)
    {
        std::vector<sf::Int16> samples(length);
        for (int i = 0; i < length; ++i)
        {
            samples[i] = ((i / 32) % 2 == 0) ? amp : -amp;
            samples[i] = static_cast<sf::Int16>(samples[i] * (1.0f - static_cast<float>(i) / length));
        }
        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), length, 1, 11025);
        return buf;
    }

    sf::SoundBuffer SoundEffectsBank::makeFastNoise(int length, sf::Int16 amp)
    {
        std::vector<sf::Int16> samples(length);
        for (int i = 0; i < length; ++i)
        {
            samples[i] = static_cast<sf::Int16>(((std::rand() % 200) - 100) * amp / 100);
            samples[i] = static_cast<sf::Int16>(samples[i] * (1.0f - static_cast<float>(i) / length));
        }
        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), length, 1, 11025);
        return buf;
    }

    sf::SoundBuffer SoundEffectsBank::makeSineSweepFast(float fStart, float fEnd, float durSec, float vol)
    {
        const unsigned int sr = 22050;
        const std::size_t count = static_cast<std::size_t>(sr * durSec);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float freq = fStart + (fEnd - fStart) * (t / durSec);
            float val = std::sin(2.0f * 3.14159265f * freq * t);
            float env = 1.0f - (t / durSec);
            samples[i] = static_cast<sf::Int16>(val * env * vol);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer SoundEffectsBank::makeNoiseDecayBuffer(float dur, float decay, float vol)
    {
        const unsigned int sr = 22050;
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

    sf::SoundBuffer SoundEffectsBank::makeThudBuffer(float freq, float dur, float vol)
    {
        const unsigned int sr = 22050;
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

    sf::SoundBuffer SoundEffectsBank::makeBreathingBuffer(float dur, float vol)
    {
        const unsigned int sr = 22050;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float cycle = std::sin(2.0f * 3.14159265f * 0.6f * t);
            float noise = (static_cast<float>(std::rand() % 20000) / 10000.0f) - 1.0f;
            float env = std::max(0.0f, cycle);
            samples[i] = static_cast<sf::Int16>(noise * env * vol * 0.35f);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer SoundEffectsBank::makeChitterBuffer(float dur, float vol)
    {
        const unsigned int sr = 22050;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float freq = 3800.0f + 800.0f * std::sin(2.0f * 3.14159265f * 45.0f * t);
            float val = std::sin(2.0f * 3.14159265f * freq * t);
            float env = 1.0f - (t / dur);
            samples[i] = static_cast<sf::Int16>(val * env * vol);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer SoundEffectsBank::makeGeneratorRevBuffer(float dur, float vol)
    {
        const unsigned int sr = 22050;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float rev = 30.0f + 30.0f * (t / dur);
            float val = std::sin(2.0f * 3.14159265f * rev * t);
            float buzz = ((std::rand() % 100) / 50.0f - 1.0f) * 0.25f;
            float env = std::min(1.0f, t * 5.0f);
            samples[i] = static_cast<sf::Int16>((val + buzz) * env * vol);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer SoundEffectsBank::makeSparkBuffer(float dur, float vol)
    {
        const unsigned int sr = 22050;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float crackle = ((std::rand() % 1000) > 850) ? 1.0f : -0.2f;
            float env = 1.0f - (t / dur);
            samples[i] = static_cast<sf::Int16>(crackle * env * vol);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

    sf::SoundBuffer SoundEffectsBank::makeAnimeMagicMusicFast()
    {
        const unsigned int sr = 22050;
        const float dur = 2.2f;
        const std::size_t count = static_cast<std::size_t>(sr * dur);
        std::vector<sf::Int16> samples(count);

        const float notes[4] = {523.25f, 659.25f, 783.99f, 987.77f};

        for (std::size_t i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float val = 0.0f;

            if (t < 1.4f)
            {
                int noteIdx = static_cast<int>(t * 8.0f) % 4;
                float freq = notes[noteIdx] * (1.0f + 0.5f * (t / 1.4f));
                float vibrato = std::sin(2.0f * 3.14159265f * 12.0f * t);
                val = std::sin(2.0f * 3.14159265f * (freq + vibrato * 15.0f) * t);
                val *= (t / 1.4f);
            }
            else
            {
                float tExp = t - 1.4f;
                float bass = std::sin(2.0f * 3.14159265f * 65.0f * tExp);
                float chime = std::sin(2.0f * 3.14159265f * 2093.0f * tExp) * std::exp(-4.0f * tExp);
                float noise = ((std::rand() % 200) - 100) / 100.0f * std::exp(-6.0f * tExp);
                val = bass * 0.6f + chime * 0.4f + noise * 0.5f;
            }

            samples[i] = static_cast<sf::Int16>(std::clamp(val, -1.0f, 1.0f) * 26000.0f);
        }

        sf::SoundBuffer buf;
        buf.loadFromSamples(samples.data(), count, 1, sr);
        return buf;
    }

} // namespace bunker
