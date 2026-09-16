#pragma once

#include <cmath>
#include <algorithm>

struct Vec3
{
    float x;
    float y;
    float z;

    Vec3()
        : x(0.0f), y(0.0f), z(0.0f)
    {
    }

    Vec3(float X, float Y, float Z)
        : x(X), y(Y), z(Z)
    {
    }

    Vec3 operator+(const Vec3& other) const
    {
        return Vec3(
            x + other.x,
            y + other.y,
            z + other.z
        );
    }

    Vec3 operator-(const Vec3& other) const
    {
        return Vec3(
            x - other.x,
            y - other.y,
            z - other.z
        );
    }

    Vec3 operator*(float value) const
    {
        return Vec3(
            x * value,
            y * value,
            z * value
        );
    }

    Vec3& operator+=(const Vec3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }

    Vec3& operator-=(const Vec3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;

        return *this;
    }
};

inline float length(const Vec3& v)
{
    return std::sqrt(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    );
}

inline float lengthXZ(const Vec3& v)
{
    return std::sqrt(
        v.x * v.x +
        v.z * v.z
    );
}

inline Vec3 normalize(const Vec3& v)
{
    const float l = length(v);

    if (l <= 0.00001f)
        return Vec3();

    return Vec3(
        v.x / l,
        v.y / l,
        v.z / l
    );
}

inline Vec3 normalizeXZ(const Vec3& v)
{
    const float l = lengthXZ(v);

    if (l <= 0.00001f)
        return Vec3();

    return Vec3(
        v.x / l,
        0.0f,
        v.z / l
    );
}

struct AABB
{
    Vec3 min;
    Vec3 max;

    AABB()
        : min(), max()
    {
    }

    AABB(
        const Vec3& Min,
        const Vec3& Max
    )
        : min(Min), max(Max)
    {
    }
};

inline bool pointInsideAABB(
    const Vec3& p,
    const AABB& box
)
{
    return
        p.x >= box.min.x &&
        p.x <= box.max.x &&
        p.y >= box.min.y &&
        p.y <= box.max.y &&
        p.z >= box.min.z &&
        p.z <= box.max.z;
}

inline bool intersectsAABB(
    const AABB& a,
    const AABB& b
)
{
    return
        a.min.x <= b.max.x &&
        a.max.x >= b.min.x &&
        a.min.y <= b.max.y &&
        a.max.y >= b.min.y &&
        a.min.z <= b.max.z &&
        a.max.z >= b.min.z;
}

inline bool sphereIntersectsAABB(
    const Vec3& center,
    float radius,
    const AABB& box
)
{
    float x = center.x;

    if (x < box.min.x)
        x = box.min.x;
    else if (x > box.max.x)
        x = box.max.x;

    float y = center.y;

    if (y < box.min.y)
        y = box.min.y;
    else if (y > box.max.y)
        y = box.max.y;

    float z = center.z;

    if (z < box.min.z)
        z = box.min.z;
    else if (z > box.max.z)
        z = box.max.z;

    const float dx = center.x - x;
    const float dy = center.y - y;
    const float dz = center.z - z;

    return
        dx * dx +
        dy * dy +
        dz * dz <=
        radius * radius;
}

inline bool rayAABB(
    const Vec3& origin,
    const Vec3& direction,
    const AABB& box,
    float& hitDistance
)
{
    float tMin = -1000000000.0f;
    float tMax =  1000000000.0f;

    const float originValues[3] =
    {
        origin.x,
        origin.y,
        origin.z
    };

    const float directionValues[3] =
    {
        direction.x,
        direction.y,
        direction.z
    };

    const float minValues[3] =
    {
        box.min.x,
        box.min.y,
        box.min.z
    };

    const float maxValues[3] =
    {
        box.max.x,
        box.max.y,
        box.max.z
    };

    for (int i = 0; i < 3; ++i)
    {
        const float o = originValues[i];
        const float d = directionValues[i];

        if (std::fabs(d) < 0.000001f)
        {
            if (
                o < minValues[i] ||
                o > maxValues[i]
            )
            {
                return false;
            }

            continue;
        }

        const float invD = 1.0f / d;

        float t1 =
            (minValues[i] - o) * invD;

        float t2 =
            (maxValues[i] - o) * invD;

        if (t1 > t2)
        {
            const float temp = t1;

            t1 = t2;
            t2 = temp;
        }

        if (t1 > tMin)
            tMin = t1;

        if (t2 < tMax)
            tMax = t2;

        if (tMin > tMax)
            return false;
    }

    if (tMax < 0.0f)
        return false;

    hitDistance =
        tMin < 0.0f
            ? 0.0f
            : tMin;

    return true;
}
// V25 collision quality helpers: finite checks, expansion and safe clamping.
inline bool isFiniteVec3(const Vec3& v)
{ return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); }

inline float clampCollisionRadius(float r)
{ return std::max(0.01f, std::min(r, 1000.0f)); }

inline AABB expandAABB(const AABB& box, float margin)
{ const float m=std::max(0.0f,margin); return AABB(Vec3(box.min.x-m,box.min.y-m,box.min.z-m), Vec3(box.max.x+m,box.max.y+m,box.max.z+m)); }
