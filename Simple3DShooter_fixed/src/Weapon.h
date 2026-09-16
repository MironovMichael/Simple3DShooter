#pragma once

#include "Collision.h"

#include <string>
#include <vector>

enum class WeaponType
{
    Pistol,
    Rifle,
    Shotgun
};

struct WeaponStats
{
    WeaponType type;

    const char* name;

    int magazineSize;
    int ammoReserve;

    float damage;
    float fireInterval;

    float reloadTime;

    int pellets;

    float spread;

    float recoil;

    float range;
};

class Weapon
{
public:
    Weapon();

    void initialize();

    void update(float dt);

    bool shoot(
        const Vec3& origin,
        const Vec3& direction,
        std::vector<Vec3>& shotDirections
    );

    bool startReload();

    void finishReload();

    void switchWeapon(
        WeaponType type
    );

    void addAmmo(
        int amount
    );

    bool isReloading() const;

    bool canShoot() const;

    bool hasAmmo() const;

    WeaponType getType() const;

    const char* getName() const;

    int getMagazineAmmo() const;

    int getMagazineSize() const;

    int getReserveAmmo() const;

    float getDamage() const;

    float getFireInterval() const;

    float getReloadProgress() const;

    float getRecoil() const;

    float consumeRecoil();

private:
    WeaponType currentType;

    std::vector<WeaponStats> weapons;

    int magazineAmmo;

    int reserveAmmo;

    float fireTimer;

    float reloadTimer;

    float reloadDuration;

    float recoil;

    unsigned int randomSeed;

    const WeaponStats& currentStats() const;

    WeaponStats& currentStats();

    float random01();

    float randomSigned();

    Vec3 makeSpreadDirection(
        const Vec3& direction,
        float spread
    );

    void loadCurrentWeapon();
};