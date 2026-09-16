#include "Player.h"

#include "World.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float DEFAULT_MAX_HEALTH = 100.0f;
    constexpr float DEFAULT_MAX_ARMOR  = 100.0f;

    constexpr float PLAYER_RADIUS =
        0.42f;

    constexpr float PLAYER_HEIGHT =
        1.80f;

    constexpr float PLAYER_EYE =
        1.60f;

    constexpr float GRAVITY =
        20.0f;

    constexpr float JUMP_FORCE =
        7.5f;

    constexpr float WALK_SPEED =
        6.0f;

    constexpr float SPRINT_SPEED =
        9.0f;

    constexpr float MAX_STAMINA =
        100.0f;

    constexpr float STAMINA_DRAIN =
        28.0f;

    constexpr float STAMINA_RECOVERY =
        22.0f;
}

Player::Player()
    : position(0.0f, 0.0f, 0.0f),
      velocity(0.0f, 0.0f, 0.0f),
      health(DEFAULT_MAX_HEALTH),
      maxHealth(DEFAULT_MAX_HEALTH),
      armor(0.0f),
      maxArmor(DEFAULT_MAX_ARMOR),
      radius(PLAYER_RADIUS),
      height(PLAYER_HEIGHT),
      eyeHeight(PLAYER_EYE),
      yaw(0.0f),
      pitch(0.0f),
      gravity(GRAVITY),
      jumpForce(JUMP_FORCE),
      walkSpeed(WALK_SPEED),
      sprintSpeed(SPRINT_SPEED),
      stamina(MAX_STAMINA),
      maxStamina(MAX_STAMINA),
      staminaDrain(STAMINA_DRAIN),
      staminaRecovery(STAMINA_RECOVERY),
      damageFlash(0.0f),
      onGround(false),
      sprinting(false)
{
}

void Player::initialize(
    const Vec3& spawnPosition
)
{
    position =
        spawnPosition;

    velocity =
        Vec3(
            0.0f,
            0.0f,
            0.0f
        );

    health =
        maxHealth;

    armor =
        0.0f;

    stamina =
        maxStamina;

    yaw =
        0.0f;

    pitch =
        0.0f;

    damageFlash =
        0.0f;

    onGround =
        false;

    sprinting =
        false;
}

void Player::reset(
    const Vec3& spawnPosition
)
{
    initialize(
        spawnPosition
    );
}

void Player::update(
    float dt,
    World& world
)
{
    if (dt <= 0.0f)
        return;

    if (dt > 0.1f)
        dt = 0.1f;

    /*
        Таймер визуальной вспышки
        получения урона.
    */

    if (damageFlash > 0.0f)
    {
        damageFlash -=
            dt * 5.0f;

        if (damageFlash < 0.0f)
            damageFlash = 0.0f;
    }

    /*
        Спринт не может продолжаться
        после полного истощения энергии.
    */

    if (sprinting)
    {
        stamina -=
            staminaDrain * dt;

        if (stamina <= 0.0f)
        {
            stamina = 0.0f;
            sprinting = false;
        }
    }
    else
    {
        stamina +=
            staminaRecovery * dt;

        if (stamina > maxStamina)
            stamina = maxStamina;
    }

    applyGravity(dt);

    /*
        Движение по вертикали.
    */

    Vec3 nextPosition =
        position;

    nextPosition.y +=
        velocity.y * dt;

    const float groundY =
        world.getGroundHeight(
            nextPosition.x,
            nextPosition.z
        );

    /*
        Не даём игроку провалиться
        под землю.
    */

    if (nextPosition.y <= groundY)
    {
        nextPosition.y =
            groundY;

        velocity.y =
            0.0f;

        onGround =
            true;
    }
    else
    {
        onGround =
            false;
    }

    position.y =
        nextPosition.y;

    /*
        Если игрок каким-либо образом
        оказался внутри объекта после
        изменения карты — World должен
        вытолкнуть его в безопасное место.
    */

    if (!world.isPositionFree(
            position,
            radius
        ))
    {
        Vec3 safePosition =
            world.findNearestFreePosition(
                position,
                radius
            );

        position =
            safePosition;

        velocity.x =
            0.0f;

        velocity.z =
            0.0f;
    }

    /*
        Проверка смертельного падения.
    */

    if (position.y < -10.0f)
    {
        health =
            0.0f;
    }
}

void Player::move(
    float forward,
    float right,
    float dt,
    World& world
)
{
    if (dt <= 0.0f)
        return;

    /*
        Нормализуем ввод.

        Это предотвращает ситуацию,
        когда движение по диагонали
        быстрее движения прямо.
    */

    const float length =
        std::sqrt(
            forward * forward +
            right * right
        );

    if (length > 1.0f)
    {
        forward /= length;
        right   /= length;
    }

    if (std::fabs(forward) < 0.0001f &&
        std::fabs(right) < 0.0001f)
    {
        return;
    }

    const float cosYaw =
        std::cos(yaw);

    const float sinYaw =
        std::sin(yaw);

    /*
        Направление вперёд.
    */

    const float forwardX =
        sinYaw;

    const float forwardZ =
        -cosYaw;

    /*
        Направление вправо.
    */

    const float rightX =
        cosYaw;

    const float rightZ =
        sinYaw;

    float moveX =
        forward * forwardX +
        right   * rightX;

    float moveZ =
        forward * forwardZ +
        right   * rightZ;

    const float moveLength =
        std::sqrt(
            moveX * moveX +
            moveZ * moveZ
        );

    if (moveLength > 0.0001f)
    {
        moveX /=
            moveLength;

        moveZ /=
            moveLength;
    }

    const float speed =
        sprinting
            ? sprintSpeed
            : walkSpeed;

    const float distance =
        speed * dt;

    Vec3 desiredPosition =
        position;

    desiredPosition.x +=
        moveX * distance;

    desiredPosition.z +=
        moveZ * distance;

    resolveHorizontalCollision(
        desiredPosition,
        world
    );
}

void Player::jump()
{
    if (!onGround)
        return;

    if (!isAlive())
        return;

    velocity.y =
        jumpForce;

    onGround =
        false;
}

void Player::addDamage(
    float damage
)
{
    if (damage <= 0.0f)
        return;

    if (!isAlive())
        return;

    /*
        Сначала поглощаем урон бронёй.

        Например:

        30 damage
        20 armor

        => armor = 0
        => health получает 10
    */

    float armorDamage =
        std::min(
            armor,
            damage
        );

    armor -=
        armorDamage;

    damage -=
        armorDamage;

    /*
        Остаток идёт в здоровье.
    */

    if (damage > 0.0f)
    {
        health -=
            damage;
    }

    if (health < 0.0f)
        health = 0.0f;

    damageFlash =
        1.0f;

    /*
        Получение урона прерывает
        спринт.
    */

    sprinting =
        false;
}

void Player::heal(
    float amount
)
{
    if (amount <= 0.0f)
        return;

    if (!isAlive())
        return;

    health +=
        amount;

    if (health > maxHealth)
        health = maxHealth;
}

void Player::addArmor(
    float amount
)
{
    if (amount <= 0.0f)
        return;

    if (!isAlive())
        return;

    armor +=
        amount;

    if (armor > maxArmor)
        armor = maxArmor;
}

bool Player::isAlive() const
{
    return health > 0.0f;
}

bool Player::isOnGround() const
{
    return onGround;
}

bool Player::isSprinting() const
{
    return sprinting;
}

void Player::setSprinting(
    bool value
)
{
    if (!isAlive())
    {
        sprinting =
            false;

        return;
    }

    if (value &&
        stamina <= 0.0f)
    {
        sprinting =
            false;

        return;
    }

    sprinting =
        value;
}

Vec3 Player::getPosition() const
{
    return position;
}

Vec3 Player::getVelocity() const
{
    return velocity;
}

float Player::getHealth() const
{
    return health;
}

float Player::getMaxHealth() const
{
    return maxHealth;
}

float Player::getArmor() const
{
    return armor;
}

float Player::getMaxArmor() const
{
    return maxArmor;
}

float Player::getEyeHeight() const
{
    return eyeHeight;
}

float Player::getRadius() const
{
    return radius;
}

float Player::getHeight() const
{
    return height;
}

float Player::getYaw() const
{
    return yaw;
}

float Player::getPitch() const
{
    return pitch;
}

void Player::setYaw(
    float value
)
{
    yaw =
        value;
}

void Player::setPitch(
    float value
)
{
    pitch =
        value;

    constexpr float MAX_PITCH =
        1.5f;

    if (pitch > MAX_PITCH)
        pitch = MAX_PITCH;

    if (pitch < -MAX_PITCH)
        pitch = -MAX_PITCH;
}

float Player::getDamageFlash() const
{
    return damageFlash;
}

float Player::getStamina() const
{
    return stamina;
}

float Player::getMaxStamina() const
{
    return maxStamina;
}

void Player::applyGravity(
    float dt
)
{
    if (onGround)
    {
        /*
            Не тянем игрока вниз,
            пока он стоит на земле.
        */

        if (velocity.y < 0.0f)
        {
            velocity.y =
                0.0f;
        }

        return;
    }

    velocity.y -=
        gravity * dt;

    /*
        Ограничиваем скорость падения,
        чтобы игрок не "телепортировался"
        сквозь землю при большом dt.
    */

    if (velocity.y < -30.0f)
    {
        velocity.y =
            -30.0f;
    }
}

void Player::resolveHorizontalCollision(
    Vec3& desiredPosition,
    World& world
)
{
    /*
        Проверяем X и Z отдельно.

        Благодаря этому игрок может
        скользить вдоль стены вместо
        полной остановки.
    */

    Vec3 testX =
        position;

    testX.x =
        desiredPosition.x;

    if (world.isPositionFree(
            testX,
            radius
        ))
    {
        position.x =
            testX.x;
    }

    Vec3 testZ =
        position;

    testZ.z =
        desiredPosition.z;

    if (world.isPositionFree(
            testZ,
            radius
        ))
    {
        position.z =
            testZ.z;
    }
}

void Player::resolveVerticalCollision(
    World& world
)
{
    const float ground =
        world.getGroundHeight(
            position.x,
            position.z
        );

    if (position.y < ground)
    {
        position.y =
            ground;

        velocity.y =
            0.0f;

        onGround =
            true;
    }
}

bool Player::validSpawnPosition(
    const Vec3& p,
    World& world
) const
{
    if (!world.isPositionFree(
            p,
            radius
        ))
    {
        return false;
    }

    const float ground =
        world.getGroundHeight(
            p.x,
            p.z
        );

    /*
        Точка спавна должна находиться
        практически на поверхности.
    */

    if (std::fabs(
            p.y - ground
        ) > 0.25f)
    {
        return false;
    }

    return true;
}