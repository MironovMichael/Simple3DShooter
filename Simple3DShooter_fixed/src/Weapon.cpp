#include "Weapon.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    constexpr float PI =
        3.14159265358979323846f;

    Vec3 cross(
        const Vec3& a,
        const Vec3& b
    )
    {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    float dot(
        const Vec3& a,
        const Vec3& b
    )
    {
        return
            a.x * b.x +
            a.y * b.y +
            a.z * b.z;
    }

    Vec3 safeNormalize(
        const Vec3& value
    )
    {
        const float len =
            length(value);

        if (len < 0.000001f)
            return Vec3(
                0.0f,
                0.0f,
                -1.0f
            );

        return value * (1.0f / len);
    }
}

Weapon::Weapon()
    : currentType(WeaponType::Pistol),
      weapons(),
      magazineAmmo(0),
      reserveAmmo(0),
      fireTimer(0.0f),
      reloadTimer(0.0f),
      reloadDuration(0.0f),
      recoil(0.0f),
      randomSeed(0x12345678u)
{
}

void Weapon::initialize()
{
    weapons.clear();

    WeaponStats pistol;

    pistol.type =
        WeaponType::Pistol;

    pistol.name =
        "PISTOL";

    pistol.magazineSize =
        12;

    pistol.ammoReserve =
        72;

    pistol.damage =
        28.0f;

    pistol.fireInterval =
        0.22f;

    pistol.reloadTime =
        1.25f;

    pistol.pellets =
        1;

    pistol.spread =
        0.012f;

    pistol.recoil =
        0.45f;

    pistol.range =
        100.0f;

    weapons.push_back(pistol);

    WeaponStats rifle;

    rifle.type =
        WeaponType::Rifle;

    rifle.name =
        "ASSAULT RIFLE";

    rifle.magazineSize =
        30;

    rifle.ammoReserve =
        120;

    rifle.damage =
        18.0f;

    rifle.fireInterval =
        0.085f;

    rifle.reloadTime =
        1.65f;

    rifle.pellets =
        1;

    rifle.spread =
        0.025f;

    rifle.recoil =
        0.24f;

    rifle.range =
        140.0f;

    weapons.push_back(rifle);

    WeaponStats shotgun;

    shotgun.type =
        WeaponType::Shotgun;

    shotgun.name =
        "SHOTGUN";

    shotgun.magazineSize =
        6;

    shotgun.ammoReserve =
        36;

    shotgun.damage =
        11.0f;

    shotgun.fireInterval =
        0.75f;

    shotgun.reloadTime =
        2.0f;

    shotgun.pellets =
        9;

    shotgun.spread =
        0.105f;

    shotgun.recoil =
        1.1f;

    shotgun.range =
        55.0f;

    weapons.push_back(shotgun);

    currentType =
        WeaponType::Pistol;

    fireTimer = 0.0f;
    reloadTimer = 0.0f;
    reloadDuration = 0.0f;
    recoil = 0.0f;

    loadCurrentWeapon();
}

void Weapon::update(
    float dt
)
{
    if (dt <= 0.0f)
        return;

    fireTimer -= dt;

    if (fireTimer < 0.0f)
        fireTimer = 0.0f;

    if (recoil > 0.0f)
    {
        /*
            Отдача плавно затухает.
        */

        recoil -=
            dt * 5.5f;

        if (recoil < 0.0f)
            recoil = 0.0f;
    }

    if (reloadTimer > 0.0f)
    {
        reloadTimer -= dt;

        if (reloadTimer <= 0.0f)
        {
            reloadTimer = 0.0f;

            finishReload();
        }
    }
}

bool Weapon::shoot(
    const Vec3& origin,
    const Vec3& direction,
    std::vector<Vec3>& shotDirections
)
{
    (void)origin;

    shotDirections.clear();

    if (isReloading())
        return false;

    if (!canShoot())
        return false;

    if (magazineAmmo <= 0)
        return false;

    if (fireTimer > 0.0f)
        return false;

    const WeaponStats& stats =
        currentStats();

    magazineAmmo--;

    fireTimer =
        stats.fireInterval;

    recoil +=
        stats.recoil;

    /*
        Не позволяем накопить бесконечную
        отдачу при зажатой кнопке.
    */

    if (recoil > 2.0f)
        recoil = 2.0f;

    const Vec3 baseDirection =
        safeNormalize(direction);

    for (int i = 0;
         i < stats.pellets;
         ++i)
    {
        shotDirections.push_back(
            makeSpreadDirection(
                baseDirection,
                stats.spread
            )
        );
    }

    return true;
}

bool Weapon::startReload()
{
    if (isReloading())
        return false;

    const WeaponStats& stats =
        currentStats();

    if (magazineAmmo >=
        stats.magazineSize)
    {
        return false;
    }

    if (reserveAmmo <= 0)
        return false;

    reloadDuration =
        stats.reloadTime;

    reloadTimer =
        reloadDuration;

    return true;
}

void Weapon::finishReload()
{
    const WeaponStats& stats =
        currentStats();

    const int missing =
        stats.magazineSize -
        magazineAmmo;

    if (missing <= 0)
        return;

    const int amount =
        std::min(
            missing,
            reserveAmmo
        );

    magazineAmmo += amount;
    reserveAmmo -= amount;
}

void Weapon::switchWeapon(
    WeaponType type
)
{
    if (type == currentType)
        return;

    bool exists = false;

    for (const WeaponStats& stats :
         weapons)
    {
        if (stats.type == type)
        {
            exists = true;
            break;
        }
    }

    if (!exists)
        return;

    /*
        Если переключаемся во время перезарядки,
        перезарядка отменяется.
    */

    reloadTimer = 0.0f;
    reloadDuration = 0.0f;

    currentType =
        type;

    loadCurrentWeapon();

    fireTimer =
        0.15f;
}

void Weapon::addAmmo(
    int amount
)
{
    if (amount <= 0)
        return;

    reserveAmmo += amount;

    /*
        Запас патронов ограничен.
    */

    const int maximum =
        currentStats().ammoReserve * 2;

    if (reserveAmmo > maximum)
        reserveAmmo = maximum;
}

bool Weapon::isReloading() const
{
    return reloadTimer > 0.0f;
}

bool Weapon::canShoot() const
{
    return
        !isReloading() &&
        magazineAmmo > 0 &&
        fireTimer <= 0.0f;
}

bool Weapon::hasAmmo() const
{
    return
        magazineAmmo > 0 ||
        reserveAmmo > 0;
}

WeaponType Weapon::getType() const
{
    return currentType;
}

const char* Weapon::getName() const
{
    return currentStats().name;
}

int Weapon::getMagazineAmmo() const
{
    return magazineAmmo;
}

int Weapon::getMagazineSize() const
{
    return currentStats().magazineSize;
}

int Weapon::getReserveAmmo() const
{
    return reserveAmmo;
}

float Weapon::getDamage() const
{
    return currentStats().damage;
}

float Weapon::getFireInterval() const
{
    return currentStats().fireInterval;
}

float Weapon::getReloadProgress() const
{
    if (!isReloading())
        return 0.0f;

    if (reloadDuration <= 0.0f)
        return 0.0f;

    const float progress =
        1.0f -
        reloadTimer /
        reloadDuration;

    return std::max(
        0.0f,
        std::min(
            1.0f,
            progress
        )
    );
}

float Weapon::getRecoil() const
{
    return recoil;
}

float Weapon::consumeRecoil()
{
    const float result =
        recoil;

    recoil = 0.0f;

    return result;
}

const WeaponStats&
Weapon::currentStats() const
{
    for (const WeaponStats& stats :
         weapons)
    {
        if (stats.type == currentType)
            return stats;
    }

    /*
        initialize() вызывается раньше
        использования оружия, поэтому
        это только аварийная защита.
    */

    return weapons.front();
}

WeaponStats&
Weapon::currentStats()
{
    for (WeaponStats& stats :
         weapons)
    {
        if (stats.type == currentType)
            return stats;
    }

    return weapons.front();
}

float Weapon::random01()
{
    /*
        Простой и быстрый генератор.
        Для игровой отдачи его более чем
        достаточно.
    */

    randomSeed =
        randomSeed * 1664525u +
        1013904223u;

    const unsigned int value =
        (randomSeed >> 8) &
        0x00FFFFFFu;

    return
        static_cast<float>(value) /
        static_cast<float>(0x00FFFFFFu);
}

float Weapon::randomSigned()
{
    return
        random01() * 2.0f -
        1.0f;
}

Vec3 Weapon::makeSpreadDirection(
    const Vec3& direction,
    float spread
)
{
    const Vec3 forward =
        safeNormalize(direction);

    /*
        Создаём устойчивый базис вокруг
        направления взгляда.
    */

    Vec3 reference(
        0.0f,
        1.0f,
        0.0f
    );

    if (std::fabs(
            dot(
                forward,
                reference
            )
        ) > 0.95f)
    {
        reference =
            Vec3(
                1.0f,
                0.0f,
                0.0f
            );
    }

    Vec3 right =
        safeNormalize(
            cross(
                forward,
                reference
            )
        );

    Vec3 up =
        safeNormalize(
            cross(
                right,
                forward
            )
        );

    /*
        Круговой разброс.
    */

    const float angle =
        random01() *
        PI * 2.0f;

    const float radius =
        std::sqrt(
            random01()
        ) * spread;

    const float horizontal =
        std::cos(angle) *
        radius;

    const float vertical =
        std::sin(angle) *
        radius;

    Vec3 result =
        forward +
        right * horizontal +
        up * vertical;

    return safeNormalize(
        result
    );
}

void Weapon::loadCurrentWeapon()
{
    const WeaponStats& stats =
        currentStats();

    magazineAmmo =
        stats.magazineSize;

    reserveAmmo =
        stats.ammoReserve;
}

// V25: ammo and recoil quality helpers.
int Weapon::getTotalAmmo() const { return magazineAmmo + reserveAmmo; }
void Weapon::recoverRecoil(float dt) { recoil = std::max(0.0f, recoil - std::max(0.0f,dt) * 5.5f); }
float Weapon::getAccuracyFactor() const
{ const WeaponStats& s=currentStats(); return std::clamp(1.0f - s.spread * 3.0f - recoil * 0.02f, 0.05f, 1.0f); }
