#include "engine/ProceduralSoundEffects.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    // ── 4. ОРУЖИЕ И РУКОПАШНЫЙ БОЙ ──
    sf::SoundBuffer SoundEffectsBank::makeXO16()
    {
        int len = 1400;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            float thud = std::sin(2.0f * 3.14159f * 85.0f * t);
            float noise = ((std::rand() % 200) - 100) / 100.0f;
            samples[i] = static_cast<std::int16_t>((thud * 0.7f + noise * 0.3f) * 28000.0f * std::exp(-6.0f * t));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeShotgun()
    {
        int len = 2000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float env = std::exp(-5.0f * i / len);
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 310.0f * env);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeMortar()
    {
        int len = 2200;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            float freq = 150.0f + 650.0f * (static_cast<float>(i) / len);
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * freq * t) * 25000.0f *
                                                   (1.0f - static_cast<float>(i) / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeGrenade()
    {
        int len = 500;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] =
                static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 1200.0f * t) * 18000.0f * std::exp(-10.0f * t));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeReload()
    {
        int len = 1400;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            samples[i] = ((i / 60) % 2 == 0) ? 20000 : -20000;
            samples[i] = static_cast<std::int16_t>(samples[i] * (1.0f - static_cast<float>(i) / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeCasings()
    {
        int len = 350;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 5200.0f * t) * 14000.0f *
                                                   (1.0f - static_cast<float>(i) / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeSlashing()
    {
        int len = 1200;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            float freq = 3000.0f - 2400.0f * (static_cast<float>(i) / len); // Свист клинка катаны вниз!
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * freq * t) * 24000.0f *
                                                   std::sin(3.14159f * i / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makePiercing()
    {
        int len = 400;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float env = 1.0f - static_cast<float>(i) / len;
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 300.0f * env);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    // ── 5. ЗАХВАТЫВАЮЩАЯ МАГИЯ АНИМЕ ВЫСОКОГО УРОВНЯ! ──
    sf::SoundBuffer SoundEffectsBank::makeAnimeMagicMusic()
    {
        const unsigned int sr = 11025; // Чистая дискретизация без нагрузки ЦП
        const float dur = 2.2f;
        const int count = static_cast<int>(sr * dur);
        std::vector<std::int16_t> samples(count);

        const float notes[4] = {523.25f, 659.25f, 783.99f, 987.77f}; // До5, Ми5, Соль5, Си5

        for (int i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float val = 0.0f;

            if (t < 1.4f)
            {
                int nIdx = static_cast<int>(t * 8.0f) % 4;
                float freq = notes[nIdx] * (1.0f + 0.5f * (t / 1.4f));
                float vib = std::sin(2.0f * 3.14159f * 12.0f * t);
                val = std::sin(2.0f * 3.14159f * (freq + vib * 15.0f) * t) * (t / 1.4f);
            }
            else
            {
                float tExp = t - 1.4f;
                float bass = std::sin(2.0f * 3.14159f * 65.0f * tExp);
                float chime = std::sin(2.0f * 3.14159f * 2093.0f * tExp) * std::exp(-4.0f * tExp);
                val = bass * 0.6f + chime * 0.4f;
            }

            samples[i] = static_cast<std::int16_t>(std::clamp(val, -1.0f, 1.0f) * 28000.0f);
        }

        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), count, 1, sr, {sf::SoundChannel::Mono});
        return b;
    }


} // namespace bunker
