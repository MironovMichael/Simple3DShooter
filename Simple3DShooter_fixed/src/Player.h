#pragma once

#include "Collision.h"

class World;

class Player
{
public:
    Player();

    void initialize(
        const Vec3& spawnPosition
    );

    void update(
        float dt,
        World& world
    );

    void move(
        float forward,
        float right,
        float dt,
        World& world
    );

    void jump();

    void addDamage(
        float damage
    );

    void heal(
        float amount
    );

    void addArmor(
        float amount
    );

    void reset(
        const Vec3& spawnPosition
    );

    bool isAlive() const;

    bool isOnGround() const;

    bool isSprinting() const;

    void setSprinting(
        bool value
    );

    Vec3 getPosition() const;

    Vec3 getVelocity() const;

    float getHealth() const;

    float getMaxHealth() const;

    float getArmor() const;

    float getMaxArmor() const;

    float getEyeHeight() const;

    float getRadius() const;

    float getHeight() const;

    float getYaw() const;

    float getPitch() const;

    void setYaw(
        float value
    );

    void setPitch(
        float value
    );

    float getDamageFlash() const;

    float getStamina() const;

    float getMaxStamina() const;

private:
    Vec3 position;
    Vec3 velocity;

    float health;
    float maxHealth;

    float armor;
    float maxArmor;

    float radius;
    float height;
    float eyeHeight;

    float yaw;
    float pitch;

    float gravity;
    float jumpForce;

    float walkSpeed;
    float sprintSpeed;

    float stamina;
    float maxStamina;

    float staminaDrain;
    float staminaRecovery;

    float damageFlash;

    bool onGround;
    bool sprinting;

    void applyGravity(
        float dt
    );

    void resolveHorizontalCollision(
        Vec3& desiredPosition,
        World& world
    );

    void resolveVerticalCollision(
        World& world
    );

    bool validSpawnPosition(
        const Vec3& p,
        World& world
    ) const;
    // V25 movement helpers: horizontal speed, velocity reset and stamina ratio.
    float getHorizontalSpeed() const;
    void stopHorizontalMotion();
    float getStaminaRatio() const;

};