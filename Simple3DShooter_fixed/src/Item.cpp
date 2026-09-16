#include "Item.h"

#include <cmath>
#include <algorithm>

namespace
{
    constexpr float PI =
        3.14159265358979323846f;
}

Item::Item()
    : type(ItemType::Ammo),
      position(),
      active(false),
      radius(0.5f),
      height(0.7f),
      rotation(0.0f),
      bobTime(0.0f),
      amount(0)
{
}

void Item::initialize(
    ItemType itemType,
    const Vec3& itemPosition
)
{
    type =
        itemType;

    position =
        itemPosition;

    active =
        true;

    rotation =
        0.0f;

    bobTime =
        0.0f;

    /*
        Настраиваем размер предметов.
    */

    switch (type)
    {
        case ItemType::Ammo:
        {
            radius = 0.55f;
            height = 0.65f;
            amount = 30;
            break;
        }

        case ItemType::Health:
        {
            radius = 0.45f;
            height = 0.55f;
            amount = 30;
            break;
        }

        case ItemType::Armor:
        {
            radius = 0.5f;
            height = 0.75f;
            amount = 25;
            break;
        }

        case ItemType::WeaponUpgrade:
        {
            radius = 0.45f;
            height = 0.8f;
            amount = 1;
            break;
        }

        case ItemType::MissionSupply:
        {
            radius = 0.7f;
            height = 0.8f;
            amount = 1;
            break;
        }
    }
}

void Item::update(
    float dt
)
{
    if (!active)
        return;

    if (dt <= 0.0f)
        return;

    rotation +=
        dt * 1.5f;

    if (rotation >
        PI * 2.0f)
    {
        rotation -=
            PI * 2.0f;
    }

    bobTime +=
        dt * 2.5f;

    if (bobTime >
        PI * 2.0f)
    {
        bobTime -=
            PI * 2.0f;
    }
}

bool Item::isActive() const
{
    return active;
}

void Item::collect()
{
    active =
        false;
}

ItemType Item::getType() const
{
    return type;
}

Vec3 Item::getPosition() const
{
    return position;
}

float Item::getRadius() const
{
    return radius;
}

float Item::getHeight() const
{
    return height;
}

float Item::getRotation() const
{
    return rotation;
}

float Item::getBobOffset() const
{
    if (!active)
        return 0.0f;

    return
        std::sin(bobTime) *
        0.12f;
}

int Item::getAmount() const
{
    return amount;
}

void Item::setAmount(
    int value
)
{
    if (value < 0)
        value = 0;

    amount =
        value;
}

// V25: safer item interaction and quantity handling.
bool Item::canCollect(const Vec3& collector, float extraRadius) const
{
    if (!active) return false;
    const float r = std::max(0.01f, radius + extraRadius);
    const float dx = collector.x - position.x, dz = collector.z - position.z;
    const float dy = collector.y - position.y;
    return dx*dx + dz*dz <= r*r && std::fabs(dy) <= std::max(1.5f, height + 0.75f);
}
float Item::getInteractionRadius() const { return std::max(0.5f, radius + 0.85f); }
void Item::clampAmount(int minimum, int maximum)
{
    if (minimum > maximum) std::swap(minimum, maximum);
    amount = std::max(minimum, std::min(maximum, amount));
}
