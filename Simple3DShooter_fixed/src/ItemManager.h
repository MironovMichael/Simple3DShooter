#pragma once

#include "Item.h"

#include <vector>

class World;
class Player;
class Weapon;
class Story;

class ItemManager
{
public:
    ItemManager();

    void initialize(
        World& world
    );

    void update(
        float dt,
        World& world,
        Player& player,
        Weapon& weapon,
        Story& story
    );

    void spawnAmmo(
        const Vec3& position,
        int amount
    );

    void spawnHealth(
        const Vec3& position,
        int amount
    );

    void spawnArmor(
        const Vec3& position,
        int amount
    );

    void spawnMissionSupply(
        const Vec3& position
    );

    void spawnWeaponUpgrade(
        const Vec3& position
    );

    const std::vector<Item>&
    getItems() const;

    int getCollectedSupplies() const;

private:
    std::vector<Item> items;

    int collectedSupplies;

    void collectItem(
        Item& item,
        Player& player,
        Weapon& weapon,
        Story& story
    );

    bool canSpawnAt(
        const Vec3& position,
        World& world
    ) const;
    // V25 inventory helpers: active count, nearest pickup and stale-item cleanup.
    int getActiveCount() const;
    const Item* findNearest(const Vec3& position, float maxDistance) const;
    void removeInactive();

};