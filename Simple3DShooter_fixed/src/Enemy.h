#pragma once

#include "Collision.h"
#include "World.h"

#include <vector>
#include <string>

enum class EnemyType
{
    Soldier,
    Scout,
    Heavy,
    Sniper,
    Commander
};

enum class EnemyState
{
    Idle,
    Patrol,
    Alert,
    Chase,
    Attack,
    TakeCover,
    Dead
};

struct EnemyWeapon
{
    float damage;
    float fireRate;
    float range;

    int magazineSize;
    int ammo;

    float cooldown;
    float reloadTimer;
    float reloadDuration;

    bool reloading;

    EnemyWeapon();
};

struct Enemy
{
    EnemyType type;
    EnemyState state;

    Vec3 position;
    Vec3 velocity;

    float yaw;
    float pitch;

    float health;
    float maxHealth;

    float radius;
    float height;

    float moveSpeed;

    float detectionRange;
    float attackRange;

    float damage;

    float stateTimer;
    float decisionTimer;

    float hitFlash;
    float deathTimer;

    float attackCooldown;

    bool alerted;
    bool dead;

    EnemyWeapon weapon;

    Vec3 patrolTarget;
    Vec3 lastKnownPlayerPosition;

    int id;

    Enemy();

    // V25 combat helpers: safe damage, center point and alive-state query.
    bool isAlive() const;
    void applyDamage(float damage);
    Vec3 getCenter() const;

};

class EnemyManager
{
public:

    EnemyManager();

    void initialize(
        World* world
    );

    void clear();

    void spawnInitialEnemies(
        int count
    );

    int spawnEnemy(
        EnemyType type,
        const Vec3& preferredPosition
    );

    void update(
        float dt,
        const Vec3& playerPosition,
        float playerHealth
    );

    void updateEnemy(
        Enemy& enemy,
        float dt,
        const Vec3& playerPosition,
        float playerHealth
    );

    bool damageEnemy(
        int enemyId,
        float damage
    );

    bool damageEnemy(
        Enemy& enemy,
        float damage
    );

    Enemy* getEnemy(
        int id
    );

    const Enemy* getEnemy(
        int id
    ) const;

    const std::vector<Enemy>&
    getEnemies() const;

    std::vector<Enemy>&
    getEnemies();

    int getAliveCount() const;

    int getTotalCount() const;

    void removeDeadEnemies();

    bool canEnemySeePlayer(
        const Enemy& enemy,
        const Vec3& playerPosition
    ) const;

    bool enemyCanShoot(
        const Enemy& enemy,
        const Vec3& playerPosition
    ) const;

    bool getEnemyShot(
        const Vec3& playerPosition,
        Vec3& shotOrigin,
        Vec3& shotDirection,
        float& damage,
        int& enemyId
    );

private:

    World* world;

    std::vector<Enemy> enemies;

    int nextEnemyId;

    float globalAlertTimer;

    float spawnTimer;

    void configureEnemy(
        Enemy& enemy,
        EnemyType type
    );

    Vec3 findSpawnPosition(
        const Vec3& preferredPosition,
        float radius
    ) const;

    Vec3 randomSpawnPosition(
        float radius
    ) const;

    Vec3 randomPatrolPoint(
        const Vec3& origin
    ) const;

    bool moveEnemy(
        Enemy& enemy,
        const Vec3& direction,
        float distance
    );

    bool moveEnemyTowards(
        Enemy& enemy,
        const Vec3& target,
        float distance
    );

    void chooseNewState(
        Enemy& enemy,
        const Vec3& playerPosition
    );

    void updateIdle(
        Enemy& enemy,
        float dt,
        const Vec3& playerPosition
    );

    void updatePatrol(
        Enemy& enemy,
        float dt,
        const Vec3& playerPosition
    );

    void updateAlert(
        Enemy& enemy,
        float dt,
        const Vec3& playerPosition
    );

    void updateChase(
        Enemy& enemy,
        float dt,
        const Vec3& playerPosition
    );

    void updateAttack(
        Enemy& enemy,
        float dt,
        const Vec3& playerPosition
    );

    void updateTakeCover(
        Enemy& enemy,
        float dt,
        const Vec3& playerPosition
    );

    void updateDead(
        Enemy& enemy,
        float dt
    );

    void reloadWeapon(
        Enemy& enemy
    );

    void updateWeapon(
        Enemy& enemy,
        float dt
    );

    float distanceToPlayer(
        const Enemy& enemy,
        const Vec3& playerPosition
    ) const;

    Vec3 directionToPlayer(
        const Enemy& enemy,
        const Vec3& playerPosition
    ) const;

    void alertNearbyEnemies(
        const Enemy& source
    );

    bool isEnemyPositionSafe(
        const Vec3& position,
        float radius
    ) const;
};