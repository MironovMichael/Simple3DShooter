#include <algorithm>
#include <cmath>
#include "Object.h"

namespace
{
    void setupTree(GameObject& object)
    {
        object.type = ObjectType::Tree;

        object.size =
            Vec3(1.6f, 5.0f, 1.6f);

        object.r = 0.25f;
        object.g = 0.55f;
        object.b = 0.18f;

        object.solid = true;
        object.destructible = false;
    }

    void setupCrate(GameObject& object)
    {
        object.type = ObjectType::Crate;

        object.size =
            Vec3(1.5f, 1.5f, 1.5f);

        object.r = 0.48f;
        object.g = 0.27f;
        object.b = 0.10f;

        object.solid = true;
        object.destructible = true;

        object.health = 50.0f;
        object.maxHealth = 50.0f;
    }

    void setupAmmoCrate(GameObject& object)
    {
        object.type = ObjectType::AmmoCrate;

        object.size =
            Vec3(1.4f, 1.0f, 1.4f);

        object.r = 0.20f;
        object.g = 0.35f;
        object.b = 0.16f;

        object.solid = true;
        object.destructible = false;
    }

    void setupMedkit(GameObject& object)
    {
        object.type = ObjectType::Medkit;

        object.size =
            Vec3(0.8f, 0.8f, 0.8f);

        object.r = 0.85f;
        object.g = 0.10f;
        object.b = 0.12f;

        object.solid = false;
        object.destructible = false;
    }

    void setupArmor(GameObject& object)
    {
        object.type = ObjectType::Armor;

        object.size =
            Vec3(0.9f, 1.0f, 0.9f);

        object.r = 0.15f;
        object.g = 0.35f;
        object.b = 0.75f;

        object.solid = false;
        object.destructible = false;
    }

    void setupBarrel(GameObject& object)
    {
        object.type = ObjectType::Barrel;

        object.size =
            Vec3(0.9f, 1.4f, 0.9f);

        object.r = 0.45f;
        object.g = 0.08f;
        object.b = 0.05f;

        object.solid = true;
        object.destructible = true;

        object.health = 30.0f;
        object.maxHealth = 30.0f;
    }
}

GameObject createObject(
    ObjectType type,
    const Vec3& position
)
{
    GameObject object;

    object.position = position;

    switch (type)
    {
        case ObjectType::Tree:
            setupTree(object);
            break;

        case ObjectType::Crate:
            setupCrate(object);
            break;

        case ObjectType::AmmoCrate:
            setupAmmoCrate(object);
            break;

        case ObjectType::Medkit:
            setupMedkit(object);
            break;

        case ObjectType::Armor:
            setupArmor(object);
            break;

        case ObjectType::Barrel:
            setupBarrel(object);
            break;

        case ObjectType::Solid:
        default:
            object.type = ObjectType::Solid;

            object.size =
                Vec3(2.0f, 2.0f, 2.0f);

            object.r = 0.45f;
            object.g = 0.45f;
            object.b = 0.48f;

            object.solid = true;
            object.destructible = false;
            break;
    }

    return object;
}

// V25: safer destructible-object API.
bool GameObject::applyDamage(float damage)
{
    if (!active || !destructible || damage <= 0.0f) return false;
    health = std::max(0.0f, health - damage);
    if (health <= 0.0f) { active = false; return true; }
    return false;
}
float GameObject::getTopY() const { return position.y + size.y * 0.5f; }
float GameObject::distanceXZ(const Vec3& point) const
{ const float dx=point.x-position.x,dz=point.z-position.z; return std::sqrt(dx*dx+dz*dz); }
