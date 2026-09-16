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
    // V25 item quality helpers: safe amount changes and interaction distance.
    bool canCollect(const Vec3& collector, float extraRadius = 0.0f) const;
    float getInteractionRadius() const;
    void clampAmount(int minimum = 0, int maximum = 999);

};