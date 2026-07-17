#pragma once

#include "Constants.hpp"
#include "GameState.hpp"
#include "InputManager.hpp"
#include "Inventory.hpp"
#include "Types.hpp"
#include "gameplay/DamageSystem.hpp"
#include "world/WeatherSystem.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bunker
{
    inline float advClamp(float v, float lo, float hi)
    {
        return std::max(lo, std::min(hi, v));
    }

    inline float advDistSq(const Vector3D& a, const Vector3D& b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    inline float advDist2D(const Vector3D& a, const Vector3D& b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    inline Vector3D advNormalize2D(Vector3D v)
    {
        const float len = std::sqrt(v.x * v.x + v.y * v.y);
        if (len < 0.0001f)
        {
            return {0.0f, 0.0f, 0.0f};
        }
        return {v.x / len, v.y / len, 0.0f};
    }


} // namespace bunker
