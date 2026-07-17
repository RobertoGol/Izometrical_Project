#include "engine/ProceduralSoundEffects.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    // ── 1. ПИЛОТ И ШАГИ ──
    sf::SoundBuffer SoundEffectsBank::makeConcreteStep()
    {
        int len = 500;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float env = 1.0f - static_cast<float>(i) / len;
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 120.0f * env);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeWaterStep()
    {
        int len = 1100;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float env = std::sin(3.14159f * i / len);
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 140.0f * env);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeGratingStep()
    {
        int len = 700;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            float env = 1.0f - static_cast<float>(i) / len;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 750.0f * t) * 15000.0f * env);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeSludgeStep()
    {
        int len = 900;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float env = std::exp(-4.0f * i / len);
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 110.0f * env);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeBreathing()
    {
        int len = 4000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            float cyc = std::sin(2.0f * 3.14159f * 0.7f * t);
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 60.0f * std::max(0.0f, cyc));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeHeartbeat()
    {
        int len = 1800;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            float env = std::exp(-8.0f * (static_cast<float>(i % 900) / 900));
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 52.0f * t) * 25000.0f * env);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeStimClick()
    {
        int len = 400;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float env = 1.0f - static_cast<float>(i) / len;
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 180.0f * env);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeRationClick()
    {
        int len = 300;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 3500.0f * t) * 20000.0f *
                                                   (1.0f - static_cast<float>(i) / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }


} // namespace bunker
