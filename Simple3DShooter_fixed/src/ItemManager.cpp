#include "ItemManager.h"

#include "Player.h"
#include "Story.h"
#include "Weapon.h"
#include "World.h"

#include <cmath>

namespace
{
    float distanceXZ(
        const Vec3& a,
        const Vec3& b
    )
    {
        const float dx =
            a.x - b.x;

        const float dz =
            a.z - b.z;

        return std::sqrt(
            dx * dx +
            dz * dz
        );
    }
}

ItemManager::ItemManager()
    : items(),
      collectedSupplies(0)
{
}

void ItemManager::initialize(
    World& world
)
{
    items.clear();

    collectedSupplies =
        0;

    /*
        Несколько заранее подготовленных
        точек с предметами.

        World::isPositionFree() защищает
        нас от появления предметов внутри
        стен и колонн.
    */

    const Vec3 ammoPositions[] =
    {
        Vec3(-10.0f, 0.35f, -5.0f),
        Vec3( 10.0f, 0.35f, -5.0f),
        Vec3(-10.0f, 0.35f,  5.0f),
        Vec3( 10.0f, 0.35f,  5.0f)
    };

    for (const Vec3& position :
         ammoPositions)
    {
        if (canSpawnAt(
                position,
                world
            ))
        {
            spawnAmmo(
                position,
                30
            );
        }
    }

    const Vec3 healthPositions[] =
    {
        Vec3(-15.0f, 0.35f,  0.0f),
        Vec3( 15.0f, 0.35f,  0.0f)
    };

    for (const Vec3& position :
         healthPositions)
    {
        if (canSpawnAt(
                position,
                world
            ))
        {
            spawnHealth(
                position,
                30
            );
        }
    }

    const Vec3 armorPositions[] =
    {
        Vec3(0.0f, 0.4f, -14.0f),
        Vec3(0.0f, 0.4f,  14.0f)
    };

    for (const Vec3& position :
         armorPositions)
    {
        if (canSpawnAt(
                position,
                world
            ))
        {
            spawnArmor(
                position,
                25
            );
        }
    }

    /*
        Три сюжетных ящика.

        Именно они используются миссией
        CollectSupplies.
    */

    const Vec3 supplyPositions[] =
    {
        Vec3(-15.0f, 0.45f, -14.0f),
        Vec3( 15.0f, 0.45f, -14.0f),
        Vec3( 15.0f, 0.45f,  14.0f)
    };

    for (const Vec3& position :
         supplyPositions)
    {
        if (canSpawnAt(
                position,
                world
            ))
        {
            spawnMissionSupply(
                position
            );
        }
    }

    /*
        Апгрейд оружия находится в центре
        одной из безопасных зон.
    */

    const Vec3 upgradePosition(
        -14.0f,
        0.5f,
        14.0f
    );

    if (canSpawnAt(
            upgradePosition,
            world
        ))
    {
        spawnWeaponUpgrade(
            upgradePosition
        );
    }
}

void ItemManager::update(
    float dt,
    World& world,
    Player& player,
    Weapon& weapon,
    Story& story
)
{
    (void)world;

    for (Item& item :
         items)
    {
        if (!item.isActive())
            continue;

        item.update(dt);

        const Vec3 playerPosition =
            player.getPosition();

        const Vec3 itemPosition =
            item.getPosition();

        const float distance =
            distanceXZ(
                playerPosition,
                itemPosition
            );

        /*
            Радиус подбора немного больше
            физического радиуса предмета.
        */

        const float pickupDistance =
            item.getRadius() +
            0.65f;

        if (distance <=
            pickupDistance)
        {
            collectItem(
                item,
                player,
                weapon,
                story
            );
        }
    }
}

void ItemManager::spawnAmmo(
    const Vec3& position,
    int amount
)
{
    Item item;

    item.initialize(
        ItemType::Ammo,
        position
    );

    item.setAmount(
        amount
    );

    items.push_back(
        item
    );
}

void ItemManager::spawnHealth(
    const Vec3& position,
    int amount
)
{
    Item item;

    item.initialize(
        ItemType::Health,
        position
    );

    item.setAmount(
        amount
    );

    items.push_back(
        item
    );
}

void ItemManager::spawnArmor(
    const Vec3& position,
    int amount
)
{
    Item item;

    item.initialize(
        ItemType::Armor,
        position
    );

    item.setAmount(
        amount
    );

    items.push_back(
        item
    );
}

void ItemManager::spawnMissionSupply(
    const Vec3& position
)
{
    Item item;

    item.initialize(
        ItemType::MissionSupply,
        position
    );

    items.push_back(
        item
    );
}

void ItemManager::spawnWeaponUpgrade(
    const Vec3& position
)
{
    Item item;

    item.initialize(
        ItemType::WeaponUpgrade,
        position
    );

    items.push_back(
        item
    );
}

const std::vector<Item>&
ItemManager::getItems() const
{
    return items;
}

int ItemManager::getCollectedSupplies() const
{
    return collectedSupplies;
}

void ItemManager::collectItem(
    Item& item,
    Player& player,
    Weapon& weapon,
    Story& story
)
{
    if (!item.isActive())
        return;

    switch (item.getType())
    {
        case ItemType::Ammo:
        {
            weapon.addAmmo(
                item.getAmount()
            );

            item.collect();

            break;
        }

        case ItemType::Health:
        {
            /*
                Player::heal() должен ограничивать
                здоровье значением maxHealth.
            */

            player.heal(
                static_cast<float>(
                    item.getAmount()
                )
            );

            item.collect();

            break;
        }

        case ItemType::Armor:
        {
            player.addArmor(
                static_cast<float>(
                    item.getAmount()
                )
            );

            item.collect();

            break;
        }

        case ItemType::WeaponUpgrade:
        {
            /*
                Пока апгрейд просто выдаёт
                дополнительный боезапас.
                Позже его можно заменить
                на систему модулей оружия.
            */

            weapon.addAmmo(60);

            item.collect();

            break;
        }

        case ItemType::MissionSupply:
        {
            collectedSupplies++;

            story.onSupplyCollected();

            item.collect();

            break;
        }
    }
}

bool ItemManager::canSpawnAt(
    const Vec3& position,
    World& world
) const
{
    /*
        Предмет не должен появляться:
        - внутри препятствия;
        - слишком близко к границе;
        - на позиции, где физически
          невозможно стоять.
    */

    if (!world.isPositionFree(
            position,
            0.6f
        ))
    {
        return false;
    }

    if (position.x < -20.0f ||
        position.x >  20.0f ||
        position.z < -20.0f ||
        position.z >  20.0f)
    {
        return false;
    }

    /*
        Не допускаем две вещи почти
        в одной точке.
    */

    for (const Item& item :
         items)
    {
        if (!item.isActive())
            continue;

        const float distance =
            distanceXZ(
                item.getPosition(),
                position
            );

        if (distance < 1.2f)
            return false;
    }

    return true;
}