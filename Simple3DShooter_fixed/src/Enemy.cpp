#include "Enemy.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>

namespace
{
    constexpr float PI =
        3.14159265358979323846f;

    constexpr float EPS =
        0.0001f;

    float randomFloat(
        float minValue,
        float maxValue
    )
    {
        const float t =
            static_cast<float>(
                std::rand()
            ) /
            static_cast<float>(
                RAND_MAX
            );

        return minValue +
               (maxValue - minValue) *
               t;
    }

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

    // Исправлено:
    // не используем имя normalize,
    // чтобы не конфликтовать с другими
    // функциями normalize в проекте.
    Vec3 normalizeVec3(
        const Vec3& value
    )
    {
        const float length =
            std::sqrt(
                value.x * value.x +
                value.y * value.y +
                value.z * value.z
            );

        if (length < EPS)
        {
            return Vec3(
                0.0f,
                0.0f,
                0.0f
            );
        }

        return Vec3(
            value.x / length,
            value.y / length,
            value.z / length
        );
    }
}


// ============================================================
// EnemyWeapon
// ============================================================

EnemyWeapon::EnemyWeapon()
    : damage(10.0f),
      fireRate(1.0f),
      range(25.0f),
      magazineSize(12),
      ammo(12),
      cooldown(0.0f),
      reloadTimer(0.0f),
      reloadDuration(1.5f),
      reloading(false)
{
}


// ============================================================
// Enemy
// ============================================================

Enemy::Enemy()
    : type(EnemyType::Soldier),
      state(EnemyState::Idle),
      position(0.0f, 0.0f, 0.0f),
      velocity(0.0f, 0.0f, 0.0f),
      yaw(0.0f),
      pitch(0.0f),
      health(100.0f),
      maxHealth(100.0f),
      radius(0.45f),
      height(2.0f),
      moveSpeed(2.2f),
      detectionRange(18.0f),
      attackRange(12.0f),
      damage(10.0f),
      stateTimer(0.0f),
      decisionTimer(0.0f),
      hitFlash(0.0f),
      deathTimer(0.0f),
      attackCooldown(0.0f),
      alerted(false),
      dead(false),
      weapon(),
      patrolTarget(0.0f, 0.0f, 0.0f),
      lastKnownPlayerPosition(0.0f, 0.0f, 0.0f),
      id(-1)
{
}


// ============================================================
// EnemyManager
// ============================================================

EnemyManager::EnemyManager()
    : world(nullptr),
      enemies(),
      nextEnemyId(1),
      globalAlertTimer(0.0f),
      spawnTimer(0.0f)
{
    std::srand(
        static_cast<unsigned>(
            std::time(nullptr)
        )
    );
}


// ============================================================
// Initialize
// ============================================================

void EnemyManager::initialize(
    World* worldPtr
)
{
    world =
        worldPtr;

    clear();
}


// ============================================================
// Clear
// ============================================================

void EnemyManager::clear()
{
    enemies.clear();

    nextEnemyId =
        1;

    globalAlertTimer =
        0.0f;

    spawnTimer =
        0.0f;
}


// ============================================================
// Spawn initial enemies
// ============================================================

void EnemyManager::spawnInitialEnemies(
    int count
)
{
    if (!world)
        return;

    /*
        Расставляем противников вокруг
        арены, но не возле центра.
    */

    for (int i = 0;
         i < count;
         ++i)
    {
        EnemyType type =
            EnemyType::Soldier;

        switch (i % 5)
        {
            case 0:
                type =
                    EnemyType::Soldier;
                break;

            case 1:
                type =
                    EnemyType::Scout;
                break;

            case 2:
                type =
                    EnemyType::Heavy;
                break;

            case 3:
                type =
                    EnemyType::Sniper;
                break;

            case 4:
                type =
                    EnemyType::Commander;
                break;
        }

        Vec3 position =
            randomSpawnPosition(
                0.6f
            );

        spawnEnemy(
            type,
            position
        );
    }
}


// ============================================================
// Spawn enemy
// ============================================================

int EnemyManager::spawnEnemy(
    EnemyType type,
    const Vec3& preferredPosition
)
{
    if (!world)
        return -1;

    Enemy enemy;

    enemy.type =
        type;

    configureEnemy(
        enemy,
        type
    );

    enemy.position =
        findSpawnPosition(
            preferredPosition,
            enemy.radius
        );

    enemy.id =
        nextEnemyId++;

    enemy.patrolTarget =
        randomPatrolPoint(
            enemy.position
        );

    enemy.lastKnownPlayerPosition =
        enemy.position;

    enemy.state =
        EnemyState::Patrol;

    enemies.push_back(
        enemy
    );

    return enemy.id;
}


// ============================================================
// Configure enemy
// ============================================================

void EnemyManager::configureEnemy(
    Enemy& enemy,
    EnemyType type
)
{
    enemy.type =
        type;

    switch (type)
    {
        case EnemyType::Soldier:
        {
            enemy.maxHealth =
                100.0f;

            enemy.health =
                enemy.maxHealth;

            enemy.radius =
                0.45f;

            enemy.height =
                2.0f;

            enemy.moveSpeed =
                2.2f;

            enemy.detectionRange =
                20.0f;

            enemy.attackRange =
                14.0f;

            enemy.weapon.damage =
                8.0f;

            enemy.weapon.fireRate =
                3.5f;

            enemy.weapon.range =
                22.0f;

            enemy.weapon.magazineSize =
                15;

            enemy.weapon.ammo =
                15;

            enemy.weapon.reloadDuration =
                1.3f;

            break;
        }

        case EnemyType::Scout:
        {
            enemy.maxHealth =
                65.0f;

            enemy.health =
                enemy.maxHealth;

            enemy.radius =
                0.38f;

            enemy.height =
                1.9f;

            enemy.moveSpeed =
                4.0f;

            enemy.detectionRange =
                24.0f;

            enemy.attackRange =
                16.0f;

            enemy.weapon.damage =
                6.0f;

            enemy.weapon.fireRate =
                6.0f;

            enemy.weapon.range =
                24.0f;

            enemy.weapon.magazineSize =
                20;

            enemy.weapon.ammo =
                20;

            enemy.weapon.reloadDuration =
                1.0f;

            break;
        }

        case EnemyType::Heavy:
        {
            enemy.maxHealth =
                240.0f;

            enemy.health =
                enemy.maxHealth;

            enemy.radius =
                0.65f;

            enemy.height =
                2.3f;

            enemy.moveSpeed =
                1.25f;

            enemy.detectionRange =
                18.0f;

            enemy.attackRange =
                15.0f;

            enemy.weapon.damage =
                18.0f;

            enemy.weapon.fireRate =
                1.5f;

            enemy.weapon.range =
                25.0f;

            enemy.weapon.magazineSize =
                8;

            enemy.weapon.ammo =
                8;

            enemy.weapon.reloadDuration =
                2.2f;

            break;
        }

        case EnemyType::Sniper:
        {
            enemy.maxHealth =
                80.0f;

            enemy.health =
                enemy.maxHealth;

            enemy.radius =
                0.42f;

            enemy.height =
                2.0f;

            enemy.moveSpeed =
                1.7f;

            enemy.detectionRange =
                35.0f;

            enemy.attackRange =
                32.0f;

            enemy.weapon.damage =
                45.0f;

            enemy.weapon.fireRate =
                0.55f;

            enemy.weapon.range =
                45.0f;

            enemy.weapon.magazineSize =
                5;

            enemy.weapon.ammo =
                5;

            enemy.weapon.reloadDuration =
                2.8f;

            break;
        }

        case EnemyType::Commander:
        {
            enemy.maxHealth =
                350.0f;

            enemy.health =
                enemy.maxHealth;

            enemy.radius =
                0.58f;

            enemy.height =
                2.2f;

            enemy.moveSpeed =
                2.0f;

            enemy.detectionRange =
                30.0f;

            enemy.attackRange =
                20.0f;

            enemy.weapon.damage =
                12.0f;

            enemy.weapon.fireRate =
                3.0f;

            enemy.weapon.range =
                30.0f;

            enemy.weapon.magazineSize =
                25;

            enemy.weapon.ammo =
                25;

            enemy.weapon.reloadDuration =
                1.8f;

            break;
        }
    }

    enemy.weapon.cooldown =
        0.0f;

    enemy.weapon.reloading =
        false;

    enemy.weapon.reloadTimer =
        0.0f;

    enemy.attackCooldown =
        0.0f;
}


// ============================================================
// Find safe spawn position
// ============================================================

Vec3 EnemyManager::findSpawnPosition(
    const Vec3& preferredPosition,
    float radius
) const
{
    if (!world)
        return preferredPosition;

    /*
        Главная защита от ошибки спавна.

        World::findNearestFreePosition()
        дополнительно проверяет все объекты.
    */

    Vec3 safe =
        world->findNearestFreePosition(
            preferredPosition,
            radius
        );

    /*
        Дополнительная проверка.
    */

    if (world->isPositionFree(
            safe,
            radius
        ))
    {
        return safe;
    }

    /*
        Если точка стала занятой,
        ищем новую случайную.
    */

    for (int i = 0;
         i < 100;
         ++i)
    {
        Vec3 candidate =
            randomSpawnPosition(
                radius
            );

        if (world->isPositionFree(
                candidate,
                radius
            ))
        {
            return candidate;
        }
    }

    /*
        Последняя попытка.
    */

    Vec3 fallback(
        0.0f,
        0.0f,
        0.0f
    );

    fallback.y =
        world->getGroundHeight(
            fallback.x,
            fallback.z
        );

    if (world->isPositionFree(
            fallback,
            radius
        ))
    {
        return fallback;
    }

    return safe;
}


// ============================================================
// Random spawn position
// ============================================================

Vec3 EnemyManager::randomSpawnPosition(
    float radius
) const
{
    if (!world)
    {
        return Vec3(
            0.0f,
            0.0f,
            0.0f
        );
    }

    for (int attempt = 0;
         attempt < 100;
         ++attempt)
    {
        const float x =
            randomFloat(
                -18.0f,
                18.0f
            );

        const float z =
            randomFloat(
                -18.0f,
                18.0f
            );

        /*
            Не спавним прямо в центре.
        */

        if (x * x +
            z * z <
            7.0f * 7.0f)
        {
            continue;
        }

        Vec3 candidate(
            x,
            world->getGroundHeight(
                x,
                z
            ),
            z
        );

        if (world->isPositionFree(
                candidate,
                radius
            ))
        {
            return candidate;
        }
    }

    Vec3 fallback =
        world->findNearestFreePosition(
            Vec3(
                10.0f,
                0.0f,
                10.0f
            ),
            radius
        );

    fallback.y =
        world->getGroundHeight(
            fallback.x,
            fallback.z
        );

    return fallback;
}


// ============================================================
// Random patrol point
// ============================================================

Vec3 EnemyManager::randomPatrolPoint(
    const Vec3& origin
) const
{
    if (!world)
        return origin;

    for (int attempt = 0;
         attempt < 30;
         ++attempt)
    {
        Vec3 point =
            origin;

        point.x +=
            randomFloat(
                -6.0f,
                6.0f
            );

        point.z +=
            randomFloat(
                -6.0f,
                6.0f
            );

        point.x =
            std::max(
                -18.0f,
                std::min(
                    18.0f,
                    point.x
                )
            );

        point.z =
            std::max(
                -18.0f,
                std::min(
                    18.0f,
                    point.z
                )
            );

        point.y =
            world->getGroundHeight(
                point.x,
                point.z
            );

        if (world->isPositionFree(
                point,
                0.5f
            ))
        {
            return point;
        }
    }

    return origin;
}


// ============================================================
// Update all enemies
// ============================================================

void EnemyManager::update(
    float dt,
    const Vec3& playerPosition,
    float playerHealth
)
{
    if (!world)
        return;

    if (globalAlertTimer > 0.0f)
    {
        globalAlertTimer -= dt;

        if (globalAlertTimer < 0.0f)
            globalAlertTimer = 0.0f;
    }

    for (Enemy& enemy :
         enemies)
    {
        updateEnemy(
            enemy,
            dt,
            playerPosition,
            playerHealth
        );
    }
}


// ============================================================
// Update enemy
// ============================================================

void EnemyManager::updateEnemy(
    Enemy& enemy,
    float dt,
    const Vec3& playerPosition,
    float playerHealth
)
{
    if (enemy.dead)
    {
        updateDead(
            enemy,
            dt
        );

        return;
    }

    /*
        Эффекты попадания.
    */

    if (enemy.hitFlash > 0.0f)
    {
        enemy.hitFlash -=
            dt * 5.0f;

        if (enemy.hitFlash < 0.0f)
            enemy.hitFlash = 0.0f;
    }

    /*
        Таймеры.
    */

    if (enemy.stateTimer > 0.0f)
        enemy.stateTimer -= dt;

    if (enemy.decisionTimer > 0.0f)
        enemy.decisionTimer -= dt;

    if (enemy.attackCooldown > 0.0f)
        enemy.attackCooldown -= dt;

    updateWeapon(
        enemy,
        dt
    );

    /*
        Если здоровье игрока нулевое,
        противники перестают атаковать.
    */

    if (playerHealth <= 0.0f)
    {
        enemy.state =
            EnemyState::Idle;

        return;
    }

    /*
        Если игрок обнаружен.
    */

    const bool seesPlayer =
        canEnemySeePlayer(
            enemy,
            playerPosition
        );

    if (seesPlayer)
    {
        enemy.alerted =
            true;

        enemy.lastKnownPlayerPosition =
            playerPosition;

        if (enemy.state ==
                EnemyState::Idle ||
            enemy.state ==
                EnemyState::Patrol ||
            enemy.state ==
                EnemyState::Alert)
        {
            enemy.state =
                EnemyState::Chase;

            enemy.stateTimer =
                3.0f;
        }

        /*
            Commander предупреждает ближайших.
        */

        if (enemy.type ==
            EnemyType::Commander)
        {
            alertNearbyEnemies(
                enemy
            );
        }
    }

    /*
        Логика состояния.
    */

    switch (enemy.state)
    {
        case EnemyState::Idle:
            updateIdle(
                enemy,
                dt,
                playerPosition
            );
            break;

        case EnemyState::Patrol:
            updatePatrol(
                enemy,
                dt,
                playerPosition
            );
            break;

        case EnemyState::Alert:
            updateAlert(
                enemy,
                dt,
                playerPosition
            );
            break;

        case EnemyState::Chase:
            updateChase(
                enemy,
                dt,
                playerPosition
            );
            break;

        case EnemyState::Attack:
            updateAttack(
                enemy,
                dt,
                playerPosition
            );
            break;

        case EnemyState::TakeCover:
            updateTakeCover(
                enemy,
                dt,
                playerPosition
            );
            break;

        case EnemyState::Dead:
            updateDead(
                enemy,
                dt
            );
            break;
    }
}


// ============================================================
// Idle
// ============================================================

void EnemyManager::updateIdle(
    Enemy& enemy,
    float dt,
    const Vec3& playerPosition
)
{
    (void)dt;

    if (distanceToPlayer(
            enemy,
            playerPosition
        ) < enemy.detectionRange)
    {
        enemy.state =
            EnemyState::Alert;

        enemy.stateTimer =
            1.0f;
    }

    if (enemy.stateTimer <= 0.0f)
    {
        enemy.state =
            EnemyState::Patrol;

        enemy.patrolTarget =
            randomPatrolPoint(
                enemy.position
            );
    }
}


// ============================================================
// Patrol
// ============================================================

void EnemyManager::updatePatrol(
    Enemy& enemy,
    float dt,
    const Vec3& playerPosition
)
{
    const float distance =
        distanceToPlayer(
            enemy,
            playerPosition
        );

    if (distance <
        enemy.detectionRange)
    {
        enemy.state =
            EnemyState::Alert;

        enemy.stateTimer =
            0.8f;

        enemy.lastKnownPlayerPosition =
            playerPosition;

        return;
    }

    const float targetDistance =
        distanceXZ(
            enemy.position,
            enemy.patrolTarget
        );

    if (targetDistance < 0.7f)
    {
        enemy.patrolTarget =
            randomPatrolPoint(
                enemy.position
            );

        return;
    }

    moveEnemyTowards(
        enemy,
        enemy.patrolTarget,
        enemy.moveSpeed * dt
    );
}


// ============================================================
// Alert
// ============================================================

void EnemyManager::updateAlert(
    Enemy& enemy,
    float dt,
    const Vec3& playerPosition
)
{
    (void)dt;

    enemy.yaw =
        std::atan2(
            playerPosition.x -
                enemy.position.x,

            -(playerPosition.z -
                enemy.position.z)
        );

    if (canEnemySeePlayer(
            enemy,
            playerPosition
        ))
    {
        enemy.state =
            EnemyState::Chase;

        enemy.stateTimer =
            4.0f;

        enemy.lastKnownPlayerPosition =
            playerPosition;

        return;
    }

    if (enemy.stateTimer <= 0.0f)
    {
        enemy.state =
            EnemyState::Patrol;
    }
}


// ============================================================
// Chase
// ============================================================

void EnemyManager::updateChase(
    Enemy& enemy,
    float dt,
    const Vec3& playerPosition
)
{
    const float distance =
        distanceToPlayer(
            enemy,
            playerPosition
        );

    enemy.lastKnownPlayerPosition =
        playerPosition;

    if (distance <=
        enemy.attackRange &&
        enemyCanShoot(
            enemy,
            playerPosition
        ))
    {
        enemy.state =
            EnemyState::Attack;

        return;
    }

    /*
        Тяжёлый противник идёт напролом.
    */

    if (enemy.type ==
            EnemyType::Heavy &&
        distance < 5.0f)
    {
        moveEnemyTowards(
            enemy,
            playerPosition,
            enemy.moveSpeed * dt
        );

        return;
    }

    /*
        Scout старается заходить сбоку.
    */

    if (enemy.type ==
        EnemyType::Scout)
    {
        Vec3 direction =
            directionToPlayer(
                enemy,
                playerPosition
            );

        Vec3 side(
            -direction.z,
            0.0f,
            direction.x
        );

        const float wave =
            std::sin(
                static_cast<float>(
                    glfwGetTime()
                ) * 2.0f
            );

        direction.x +=
            side.x *
            wave *
            0.35f;

        direction.z +=
            side.z *
            wave *
            0.35f;

        /*
            Не используем normalize(),
            чтобы избежать конфликта имён.
        */

        direction =
            normalizeVec3(
                direction
            );

        moveEnemy(
            enemy,
            direction,
            enemy.moveSpeed * dt
        );

        return;
    }

    moveEnemyTowards(
        enemy,
        playerPosition,
        enemy.moveSpeed * dt
    );

    /*
        Если давно не видели игрока,
        возвращаемся к последней известной точке.
    */

    if (enemy.stateTimer <= 0.0f &&
        distance >
            enemy.detectionRange * 1.5f)
    {
        enemy.state =
            EnemyState::Alert;
    }
}


// ============================================================
// Attack
// ============================================================

void EnemyManager::updateAttack(
    Enemy& enemy,
    float dt,
    const Vec3& playerPosition
)
{
    (void)dt;

    const float distance =
        distanceToPlayer(
            enemy,
            playerPosition
        );

    if (distance >
        enemy.attackRange * 1.2f)
    {
        enemy.state =
            EnemyState::Chase;

        return;
    }

    if (!enemyCanShoot(
            enemy,
            playerPosition
        ))
    {
        enemy.state =
            EnemyState::Chase;

        return;
    }

    enemy.yaw =
        std::atan2(
            playerPosition.x -
                enemy.position.x,

            -(playerPosition.z -
                enemy.position.z)
        );

    /*
        После окончания магазина
        автоматически перезаряжаемся.
    */

    if (enemy.weapon.ammo <= 0)
    {
        reloadWeapon(
            enemy
        );

        return;
    }

    /*
        Сам выстрел обрабатывается
        через getEnemyShot().
    */
}


// ============================================================
// Take cover
// ============================================================

void EnemyManager::updateTakeCover(
    Enemy& enemy,
    float dt,
    const Vec3& playerPosition
)
{
    /*
        Простая система поиска укрытия.
    */

    const std::vector<
        WorldObject
    >& objects =
        world->getObjects();

    Vec3 bestCover =
        enemy.position;

    float bestDistance =
        std::numeric_limits<float>::max();

    for (const WorldObject& object :
         objects)
    {
        if (!object.solid)
            continue;

        Vec3 fromObjectToEnemy(
            enemy.position.x -
                object.position.x,

            0.0f,

            enemy.position.z -
                object.position.z
        );

        /*
            Исправлено имя normalize.
        */

        Vec3 dir =
            normalizeVec3(
                fromObjectToEnemy
            );

        if (std::fabs(dir.x) < EPS &&
            std::fabs(dir.z) < EPS)
        {
            continue;
        }

        Vec3 candidate =
            object.position;

        candidate.x +=
            dir.x * 1.5f;

        candidate.z +=
            dir.z * 1.5f;

        candidate.y =
            world->getGroundHeight(
                candidate.x,
                candidate.z
            );

        if (!world->isPositionFree(
                candidate,
                enemy.radius
            ))
        {
            continue;
        }

        const float d =
            distanceXZ(
                candidate,
                enemy.position
            );

        if (d < bestDistance)
        {
            bestDistance =
                d;

            bestCover =
                candidate;
        }
    }

    if (bestDistance <
        std::numeric_limits<float>::max())
    {
        moveEnemyTowards(
            enemy,
            bestCover,
            enemy.moveSpeed * dt
        );
    }
    else
    {
        enemy.state =
            EnemyState::Chase;
    }

    /*
        Через некоторое время возвращаемся
        в атаку.
    */

    if (enemy.stateTimer <= 0.0f)
    {
        enemy.state =
            EnemyState::Attack;
    }

    (void)playerPosition;
}


// ============================================================
// Dead
// ============================================================

void EnemyManager::updateDead(
    Enemy& enemy,
    float dt
)
{
    enemy.deathTimer +=
        dt;

    /*
        Через некоторое время труп
        можно удалить.
    */
}


// ============================================================
// Damage enemy by ID
// ============================================================

bool EnemyManager::damageEnemy(
    int enemyId,
    float damage
)
{
    Enemy* enemy =
        getEnemy(
            enemyId
        );

    if (!enemy)
        return false;

    return damageEnemy(
        *enemy,
        damage
    );
}


// ============================================================
// Damage enemy
// ============================================================

bool EnemyManager::damageEnemy(
    Enemy& enemy,
    float damage
)
{
    if (enemy.dead)
        return false;

    if (damage <= 0.0f)
        return false;

    enemy.health -=
        damage;

    enemy.hitFlash =
        1.0f;

    enemy.alerted =
        true;

    enemy.state =
        EnemyState::Chase;

    enemy.stateTimer =
        5.0f;

    if (enemy.health <= 0.0f)
    {
        enemy.health =
            0.0f;

        enemy.dead =
            true;

        enemy.state =
            EnemyState::Dead;

        enemy.deathTimer =
            0.0f;

        /*
            Враг больше не должен стрелять.
        */

        enemy.weapon.reloading =
            false;

        enemy.weapon.cooldown =
            0.0f;

        return true;
    }

    return false;
}


// ============================================================
// Get enemy
// ============================================================

Enemy* EnemyManager::getEnemy(
    int id
)
{
    for (Enemy& enemy :
         enemies)
    {
        if (enemy.id == id)
            return &enemy;
    }

    return nullptr;
}


// ============================================================
// Get const enemy
// ============================================================

const Enemy*
EnemyManager::getEnemy(
    int id
) const
{
    for (const Enemy& enemy :
         enemies)
    {
        if (enemy.id == id)
            return &enemy;
    }

    return nullptr;
}


// ============================================================
// Get enemies
// ============================================================

const std::vector<Enemy>&
EnemyManager::getEnemies() const
{
    return enemies;
}


// ============================================================
// Get mutable enemies
// ============================================================

std::vector<Enemy>&
EnemyManager::getEnemies()
{
    return enemies;
}


// ============================================================
// Alive count
// ============================================================

int EnemyManager::getAliveCount() const
{
    int result =
        0;

    for (const Enemy& enemy :
         enemies)
    {
        if (!enemy.dead &&
            enemy.health > 0.0f)
        {
            ++result;
        }
    }

    return result;
}


// ============================================================
// Total count
// ============================================================

int EnemyManager::getTotalCount() const
{
    return static_cast<int>(
        enemies.size()
    );
}


// ============================================================
// Remove dead enemies
// ============================================================

void EnemyManager::removeDeadEnemies()
{
    enemies.erase(
        std::remove_if(
            enemies.begin(),
            enemies.end(),

            [](const Enemy& enemy)
            {
                return enemy.dead &&
                       enemy.deathTimer >
                       5.0f;
            }
        ),

        enemies.end()
    );
}


// ============================================================
// Can enemy see player
// ============================================================

bool EnemyManager::canEnemySeePlayer(
    const Enemy& enemy,
    const Vec3& playerPosition
) const
{
    if (!world)
        return false;

    const float distance =
        distanceToPlayer(
            enemy,
            playerPosition
        );

    if (distance >
        enemy.detectionRange)
    {
        return false;
    }

    /*
        Смотрим примерно на уровень груди
        игрока.
    */

    Vec3 from(
        enemy.position.x,
        enemy.position.y +
            enemy.height * 0.75f,
        enemy.position.z
    );

    Vec3 to(
        playerPosition.x,
        playerPosition.y +
            1.0f,
        playerPosition.z
    );

    return world->lineOfSight(
        from,
        to
    );
}


// ============================================================
// Can enemy shoot
// ============================================================

bool EnemyManager::enemyCanShoot(
    const Enemy& enemy,
    const Vec3& playerPosition
) const
{
    if (enemy.dead)
        return false;

    const float distance =
        distanceToPlayer(
            enemy,
            playerPosition
        );

    if (distance >
        enemy.weapon.range)
    {
        return false;
    }

    if (!canEnemySeePlayer(
            enemy,
            playerPosition
        ))
    {
        return false;
    }

    if (enemy.weapon.reloading)
        return false;

    if (enemy.weapon.ammo <= 0)
        return false;

    return true;
}


// ============================================================
// Get enemy shot
// ============================================================

bool EnemyManager::getEnemyShot(
    const Vec3& playerPosition,
    Vec3& shotOrigin,
    Vec3& shotDirection,
    float& damage,
    int& enemyId
)
{
    /*
        Возвращает один выстрел за вызов.
    */

    for (Enemy& enemy :
         enemies)
    {
        if (enemy.dead)
            continue;

        if (!enemyCanShoot(
                enemy,
                playerPosition
            ))
        {
            continue;
        }

        if (enemy.weapon.cooldown >
            0.0f)
        {
            continue;
        }

        shotOrigin =
            Vec3(
                enemy.position.x,
                enemy.position.y +
                    enemy.height *
                    0.72f,
                enemy.position.z
            );

        Vec3 target(
            playerPosition.x,
            playerPosition.y +
                1.0f,
            playerPosition.z
        );

        /*
            Исправлено:
            normalize -> normalizeVec3
        */

        shotDirection =
            normalizeVec3(
                Vec3(
                    target.x -
                        shotOrigin.x,

                    target.y -
                        shotOrigin.y,

                    target.z -
                        shotOrigin.z
                )
            );

        /*
            Разброс оружия.
        */

        float spread =
            0.025f;

        if (enemy.type ==
            EnemyType::Scout)
        {
            spread =
                0.06f;
        }

        if (enemy.type ==
            EnemyType::Heavy)
        {
            spread =
                0.09f;
        }

        if (enemy.type ==
            EnemyType::Sniper)
        {
            spread =
                0.008f;
        }

        shotDirection.x +=
            randomFloat(
                -spread,
                spread
            );

        shotDirection.y +=
            randomFloat(
                -spread,
                spread
            );

        shotDirection.z +=
            randomFloat(
                -spread,
                spread
            );

        /*
            Исправлено:
            normalize -> normalizeVec3
        */

        shotDirection =
            normalizeVec3(
                shotDirection
            );

        damage =
            enemy.weapon.damage;

        enemyId =
            enemy.id;

        enemy.weapon.ammo--;

        enemy.weapon.cooldown =
            1.0f /
            std::max(
                0.1f,
                enemy.weapon.fireRate
            );

        if (enemy.weapon.ammo <= 0)
        {
            enemy.weapon.reloading =
                true;

            enemy.weapon.reloadTimer =
                enemy.weapon.reloadDuration;
        }

        return true;
    }

    return false;
}


// ============================================================
// Move enemy
// ============================================================

bool EnemyManager::moveEnemy(
    Enemy& enemy,
    const Vec3& direction,
    float distance
)
{
    if (!world)
        return false;

    /*
        Исправлено:
        normalize -> normalizeVec3
    */

    Vec3 dir =
        normalizeVec3(
            direction
        );

    if (std::fabs(dir.x) < EPS &&
        std::fabs(dir.z) < EPS)
    {
        return false;
    }

    Vec3 target =
        enemy.position;

    target.x +=
        dir.x * distance;

    target.z +=
        dir.z * distance;

    target.y =
        world->getGroundHeight(
            target.x,
            target.z
        );

    /*
        Враг не проходит через стены,
        деревья, ящики и колонны.
    */

    if (world->isPositionFree(
            target,
            enemy.radius
        ))
    {
        enemy.position =
            target;

        enemy.velocity =
            Vec3(
                dir.x *
                    enemy.moveSpeed,

                0.0f,

                dir.z *
                    enemy.moveSpeed
            );

        enemy.yaw =
            std::atan2(
                dir.x,
                -dir.z
            );

        return true;
    }

    /*
        Если прямо пройти нельзя,
        пробуем движение отдельно по X.
    */

    Vec3 xOnly =
        enemy.position;

    xOnly.x +=
        dir.x * distance;

    xOnly.y =
        world->getGroundHeight(
            xOnly.x,
            xOnly.z
        );

    if (world->isPositionFree(
            xOnly,
            enemy.radius
        ))
    {
        enemy.position =
            xOnly;

        enemy.yaw =
            std::atan2(
                dir.x,
                -dir.z
            );

        return true;
    }

    /*
        Затем отдельно по Z.
    */

    Vec3 zOnly =
        enemy.position;

    zOnly.z +=
        dir.z * distance;

    zOnly.y =
        world->getGroundHeight(
            zOnly.x,
            zOnly.z
        );

    if (world->isPositionFree(
            zOnly,
            enemy.radius
        ))
    {
        enemy.position =
            zOnly;

        enemy.yaw =
            std::atan2(
                dir.x,
                -dir.z
            );

        return true;
    }

    /*
        Не двигаемся, если всё заблокировано.
    */

    enemy.velocity =
        Vec3(
            0.0f,
            0.0f,
            0.0f
        );

    return false;
}


// ============================================================
// Move enemy towards target
// ============================================================

bool EnemyManager::moveEnemyTowards(
    Enemy& enemy,
    const Vec3& target,
    float distance
)
{
    Vec3 direction(
        target.x -
            enemy.position.x,

        0.0f,

        target.z -
            enemy.position.z
    );

    return moveEnemy(
        enemy,
        direction,
        distance
    );
}


// ============================================================
// Choose new state
// ============================================================

void EnemyManager::chooseNewState(
    Enemy& enemy,
    const Vec3& playerPosition
)
{
    const float distance =
        distanceToPlayer(
            enemy,
            playerPosition
        );

    if (distance <
        enemy.attackRange &&
        enemyCanShoot(
            enemy,
            playerPosition
        ))
    {
        enemy.state =
            EnemyState::Attack;

        return;
    }

    if (distance <
        enemy.detectionRange)
    {
        enemy.state =
            EnemyState::Chase;

        return;
    }

    enemy.state =
        EnemyState::Patrol;
}


// ============================================================
// Reload weapon
// ============================================================

void EnemyManager::reloadWeapon(
    Enemy& enemy
)
{
    if (enemy.weapon.reloading)
        return;

    enemy.weapon.reloading =
        true;

    enemy.weapon.reloadTimer =
        enemy.weapon.reloadDuration;
}


// ============================================================
// Update weapon
// ============================================================

void EnemyManager::updateWeapon(
    Enemy& enemy,
    float dt
)
{
    if (enemy.weapon.cooldown >
        0.0f)
    {
        enemy.weapon.cooldown -=
            dt;

        if (enemy.weapon.cooldown <
            0.0f)
        {
            enemy.weapon.cooldown =
                0.0f;
        }
    }

    if (!enemy.weapon.reloading)
        return;

    enemy.weapon.reloadTimer -=
        dt;

    if (enemy.weapon.reloadTimer <=
        0.0f)
    {
        enemy.weapon.reloading =
            false;

        enemy.weapon.ammo =
            enemy.weapon.magazineSize;

        enemy.weapon.reloadTimer =
            0.0f;
    }
}


// ============================================================
// Distance to player
// ============================================================

float EnemyManager::distanceToPlayer(
    const Enemy& enemy,
    const Vec3& playerPosition
) const
{
    return distanceXZ(
        enemy.position,
        playerPosition
    );
}


// ============================================================
// Direction to player
// ============================================================

Vec3 EnemyManager::directionToPlayer(
    const Enemy& enemy,
    const Vec3& playerPosition
) const
{
    /*
        Исправлено:
        normalize -> normalizeVec3
    */

    return normalizeVec3(
        Vec3(
            playerPosition.x -
                enemy.position.x,

            0.0f,

            playerPosition.z -
                enemy.position.z
        )
    );
}


// ============================================================
// Alert nearby enemies
// ============================================================

void EnemyManager::alertNearbyEnemies(
    const Enemy& source
)
{
    globalAlertTimer =
        5.0f;

    for (Enemy& enemy :
         enemies)
    {
        if (&enemy == &source)
            continue;

        if (enemy.dead)
            continue;

        const float distance =
            distanceXZ(
                enemy.position,
                source.position
            );

        if (distance < 12.0f)
        {
            enemy.alerted =
                true;

            enemy.state =
                EnemyState::Alert;

            enemy.stateTimer =
                1.0f;

            enemy.lastKnownPlayerPosition =
                source.lastKnownPlayerPosition;
        }
    }
}


// ============================================================
// Is enemy position safe
// ============================================================

bool EnemyManager::isEnemyPositionSafe(
    const Vec3& position,
    float radius
) const
{
    if (!world)
        return false;

    return world->isPositionFree(
        position,
        radius
    );
}
