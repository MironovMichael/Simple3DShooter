#pragma once

#include "Collision.h"

enum class ObjectType
{
    Solid,
    Tree,
    Crate,
    AmmoCrate,
    Medkit,
    Armor,
    Barrel
};

struct GameObject
{
    ObjectType type;

    Vec3 position;
    Vec3 size;

    float rotationY;

    float r;
    float g;
    float b;

    bool active;
    bool solid;
    bool destructible;

    float health;
    float maxHealth;

    GameObject()
        : type(ObjectType::Solid),
          position(),
          size(1.0f, 1.0f, 1.0f),
          rotationY(0.0f),
          r(0.5f),
          g(0.5f),
          b(0.5f),
          active(true),
          solid(true),
          destructible(false),
          health(100.0f),
          maxHealth(100.0f)
    {
    }

    AABB getBounds() const
    {
        const Vec3 half =
            size * 0.5f;

        return AABB(
            Vec3(
                position.x - half.x,
                position.y - half.y,
                position.z - half.z
            ),
            Vec3(
                position.x + half.x,
                position.y + half.y,
                position.z + half.z
            )
        );
    }

    bool isAlive() const
    {
        return active && health > 0.0f;
    }

    void damage(float amount)
    {
        if (!destructible || !active)
            return;

        health -= amount;

        if (health <= 0.0f)
        {
            health = 0.0f;
            active = false;
        }
    }
    // V25 object helpers: safe damage, top surface and distance query.
    bool applyDamage(float damage);
    float getTopY() const;
    float distanceXZ(const Vec3& point) const;

};
GameObject createObject(
    ObjectType type,
    const Vec3& position
);