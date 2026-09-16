#pragma once

#include "Collision.h"

enum class ItemType
{
    Ammo,
    Health,
    Armor,
    WeaponUpgrade,
    MissionSupply
};

class Item
{
public:
    Item();

    void initialize(
        ItemType type,
        const Vec3& position
    );

    void update(
        float dt
    );

    bool isActive() const;

    void collect();

    ItemType getType() const;

    Vec3 getPosition() const;

    float getRadius() const;

    float getHeight() const;

    float getRotation() const;

    float getBobOffset() const;

    int getAmount() const;

    void setAmount(
        int amount
    );

private:
    ItemType type;

    Vec3 position;

    bool active;

    float radius;
    float height;

    float rotation;
    float bobTime;

    int amount;
};