#pragma once

#include "Collision.h"

#include <vector>

enum class WorldObjectType
{
    Wall,
    Column,
    Tree,
    Crate,
    Rock,
    Fence,
    Barrier
};

struct WorldObject
{
    WorldObjectType type;

    Vec3 position;
    Vec3 size;

    float rotation;

    float r;
    float g;
    float b;

    bool solid;
};

class World
{
public:
    World();

    void initialize();

    void update(
        float dt
    );

    float getGroundHeight(
        float x,
        float z
    ) const;

    bool isPositionFree(
        const Vec3& position,
        float radius
    ) const;

    Vec3 findNearestFreePosition(
        const Vec3& position,
        float radius
    ) const;

    bool raycast(
        const Vec3& origin,
        const Vec3& direction,
        float& hitDistance,
        int& objectIndex
    ) const;

    bool lineOfSight(
        const Vec3& from,
        const Vec3& to
    ) const;

    const std::vector<WorldObject>&
    getObjects() const;

    std::vector<WorldObject>&
    getObjects();

    int getObjectCount() const;

    const WorldObject*
    getObject(
        int index
    ) const;

    bool isInsideArena(
        const Vec3& position,
        float radius
    ) const;

private:
    std::vector<WorldObject> objects;

    float arenaHalfSize;
    float wallHeight;

    void createWalls();

    void createColumns();

    void createTrees();

    void createCrates();

    void createRocks();

    void createFences();

    void addObject(
        WorldObjectType type,
        const Vec3& position,
        const Vec3& size,
        float rotation,
        float r,
        float g,
        float b,
        bool solid
    );

    bool sphereIntersectsBox(
        const Vec3& spherePosition,
        float sphereRadius,
        const WorldObject& object
    ) const;

    bool rayBoxIntersection(
        const Vec3& origin,
        const Vec3& direction,
        const WorldObject& object,
        float& distance
    ) const;
};