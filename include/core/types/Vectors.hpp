#pragma once

#include <cmath>

namespace bunker
{
    // ══════════════════════════════════════════════════════════════════════
    // Математические примитивы
    // ══════════════════════════════════════════════════════════════════════

    struct Vector2D
    {
        float x = 0.0f;
        float y = 0.0f;

        Vector2D() = default;
        Vector2D(float ax, float ay) : x(ax), y(ay) {}

        Vector2D operator+(const Vector2D& o) const
        {
            return {x + o.x, y + o.y};
        }
        Vector2D operator-(const Vector2D& o) const
        {
            return {x - o.x, y - o.y};
        }
        Vector2D operator*(float s) const
        {
            return {x * s, y * s};
        }
        Vector2D& operator+=(const Vector2D& o)
        {
            x += o.x;
            y += o.y;
            return *this;
        }
        Vector2D& operator-=(const Vector2D& o)
        {
            x -= o.x;
            y -= o.y;
            return *this;
        }

        float length() const
        {
            return std::sqrt(x * x + y * y);
        }
        float lengthSq() const
        {
            return x * x + y * y;
        }

        Vector2D normalized() const
        {
            float len = length();
            if (len < 0.0001f)
            {
                return {0.0f, 0.0f};
            }
            return {x / len, y / len};
        }
    };

    struct Vector3D
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vector3D() = default;
        Vector3D(float ax, float ay, float az = 0.0f) : x(ax), y(ay), z(az) {}

        Vector3D operator+(const Vector3D& o) const
        {
            return {x + o.x, y + o.y, z + o.z};
        }
        Vector3D operator-(const Vector3D& o) const
        {
            return {x - o.x, y - o.y, z - o.z};
        }
        Vector3D operator*(float s) const
        {
            return {x * s, y * s, z * s};
        }
        Vector3D& operator+=(const Vector3D& o)
        {
            x += o.x;
            y += o.y;
            z += o.z;
            return *this;
        }

        float length() const
        {
            return std::sqrt(x * x + y * y + z * z);
        }
        float lengthSq() const
        {
            return x * x + y * y + z * z;
        }

        Vector2D xy() const
        {
            return {x, y};
        }

        Vector3D normalized() const
        {
            float len = length();
            if (len < 0.0001f)
            {
                return {};
            }
            return {x / len, y / len, z / len};
        }
    };


} // namespace bunker
