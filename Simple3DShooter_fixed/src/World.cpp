#include "World.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    constexpr float PI =
        3.14159265358979323846f;

    constexpr float EPSILON =
        0.0001f;
}

World::World()
    : objects(),
      arenaHalfSize(22.0f),
      wallHeight(6.0f)
{
}

void World::initialize()
{
    objects.clear();

    /*
        Порядок создания объектов важен только
        для удобства отладки. Физика не зависит
        от порядка.
    */

    createWalls();

    createColumns();

    createTrees();

    createCrates();

    createRocks();

    createFences();
}

void World::update(
    float dt
)
{
    /*
        Пока карта статичная.

        Метод оставлен специально, чтобы позже
        сюда можно было добавить:
        - разрушаемые ящики;
        - открывающиеся двери;
        - движущиеся платформы;
        - смену погоды;
        - динамические препятствия.
    */

    (void)dt;
}

float World::getGroundHeight(
    float x,
    float z
) const
{
    /*
        Базовая высота земли — 0.

        Проверяем только потенциальные
        платформы в будущем. Сейчас все
        объекты стоят на земле.
    */

    (void)x;
    (void)z;

    return 0.0f;
}

bool World::isPositionFree(
    const Vec3& position,
    float radius
) const
{
    /*
        Сначала проверяем границы арены.
    */

    if (!isInsideArena(
            position,
            radius
        ))
    {
        return false;
    }

    /*
        Затем проверяем столкновение
        со всеми твёрдыми объектами.
    */

    for (const WorldObject& object :
         objects)
    {
        if (!object.solid)
            continue;

        if (sphereIntersectsBox(
                position,
                radius,
                object
            ))
        {
            return false;
        }
    }

    return true;
}

Vec3 World::findNearestFreePosition(
    const Vec3& position,
    float radius
) const
{
    /*
        Сначала пробуем исходную точку.
    */

    if (isPositionFree(
            position,
            radius
        ))
    {
        return position;
    }

    /*
        Ищем свободную точку по
        концентрическим кольцам.

        Это особенно важно для спавна:
        даже если дизайнер случайно поставил
        spawn point слишком близко к колонне,
        игрок не окажется внутри неё.
    */

    constexpr int RINGS =
        20;

    constexpr int STEPS =
        32;

    for (int ring = 1;
         ring <= RINGS;
         ++ring)
    {
        const float distance =
            static_cast<float>(ring) *
            0.5f;

        for (int step = 0;
             step < STEPS;
             ++step)
        {
            const float angle =
                static_cast<float>(step) *
                (2.0f * PI /
                 static_cast<float>(STEPS));

            Vec3 candidate =
                position;

            candidate.x +=
                std::cos(angle) *
                distance;

            candidate.z +=
                std::sin(angle) *
                distance;

            candidate.y =
                getGroundHeight(
                    candidate.x,
                    candidate.z
                );

            if (isPositionFree(
                    candidate,
                    radius
                ))
            {
                return candidate;
            }
        }
    }

    /*
        Если рядом свободного места нет,
        используем гарантированно безопасную
        центральную точку.

        Она тоже проходит проверку.
    */

    const Vec3 center(
        0.0f,
        0.0f,
        0.0f
    );

    if (isPositionFree(
            center,
            radius
        ))
    {
        return center;
    }

    /*
        Крайний случай:
        ищем любую свободную точку
        сеткой по всей карте.
    */

    for (float z = -18.0f;
         z <= 18.0f;
         z += 2.0f)
    {
        for (float x = -18.0f;
             x <= 18.0f;
             x += 2.0f)
        {
            Vec3 candidate(
                x,
                getGroundHeight(
                    x,
                    z
                ),
                z
            );

            if (isPositionFree(
                    candidate,
                    radius
                ))
            {
                return candidate;
            }
        }
    }

    /*
        Теоретически сюда попасть невозможно
        при нормальной конфигурации карты.
    */

    return Vec3(
        0.0f,
        0.0f,
        0.0f
    );
}

bool World::raycast(
    const Vec3& origin,
    const Vec3& direction,
    float& hitDistance,
    int& objectIndex
) const
{
    hitDistance =
        std::numeric_limits<float>::max();

    objectIndex =
        -1;

    float length =
        std::sqrt(
            direction.x * direction.x +
            direction.y * direction.y +
            direction.z * direction.z
        );

    if (length < EPSILON)
        return false;

    Vec3 dir =
        direction;

    dir.x /= length;
    dir.y /= length;
    dir.z /= length;

    bool hit =
        false;

    for (int i = 0;
         i < static_cast<int>(
                 objects.size()
             );
         ++i)
    {
        const WorldObject& object =
            objects[i];

        if (!object.solid)
            continue;

        float distance = 0.0f;

        if (rayBoxIntersection(
                origin,
                dir,
                object,
                distance
            ))
        {
            if (distance >= 0.0f &&
                distance < hitDistance)
            {
                hitDistance =
                    distance;

                objectIndex =
                    i;

                hit =
                    true;
            }
        }
    }

    return hit;
}

bool World::lineOfSight(
    const Vec3& from,
    const Vec3& to
) const
{
    Vec3 direction;

    direction.x =
        to.x - from.x;

    direction.y =
        to.y - from.y;

    direction.z =
        to.z - from.z;

    const float distance =
        std::sqrt(
            direction.x * direction.x +
            direction.y * direction.y +
            direction.z * direction.z
        );

    if (distance < EPSILON)
        return true;

    direction.x /=
        distance;

    direction.y /=
        distance;

    direction.z /=
        distance;

    float hitDistance;
    int objectIndex;

    if (!raycast(
            from,
            direction,
            hitDistance,
            objectIndex
        ))
    {
        return true;
    }

    /*
        Если препятствие дальше цели,
        цель всё ещё видна.
    */

    return hitDistance >=
           distance - 0.05f;
}

const std::vector<WorldObject>&
World::getObjects() const
{
    return objects;
}

std::vector<WorldObject>&
World::getObjects()
{
    return objects;
}

int World::getObjectCount() const
{
    return static_cast<int>(
        objects.size()
    );
}

const WorldObject*
World::getObject(
    int index
) const
{
    if (index < 0)
        return nullptr;

    if (index >=
        static_cast<int>(
            objects.size()
        ))
    {
        return nullptr;
    }

    return &objects[index];
}

bool World::isInsideArena(
    const Vec3& position,
    float radius
) const
{
    /*
        Отступ от стены равен радиусу
        игрока/объекта.
    */

    const float limit =
        arenaHalfSize -
        radius;

    if (position.x < -limit)
        return false;

    if (position.x > limit)
        return false;

    if (position.z < -limit)
        return false;

    if (position.z > limit)
        return false;

    return true;
}

void World::createWalls()
{
    /*
        Левая стена.
    */

    addObject(
        WorldObjectType::Wall,

        Vec3(
            -arenaHalfSize,
            wallHeight * 0.5f,
            0.0f
        ),

        Vec3(
            0.6f,
            wallHeight,
            arenaHalfSize * 2.0f
        ),

        0.0f,

        0.35f,
        0.38f,
        0.45f,

        true
    );

    /*
        Правая стена.
    */

    addObject(
        WorldObjectType::Wall,

        Vec3(
            arenaHalfSize,
            wallHeight * 0.5f,
            0.0f
        ),

        Vec3(
            0.6f,
            wallHeight,
            arenaHalfSize * 2.0f
        ),

        0.0f,

        0.35f,
        0.38f,
        0.45f,

        true
    );

    /*
        Северная стена.
    */

    addObject(
        WorldObjectType::Wall,

        Vec3(
            0.0f,
            wallHeight * 0.5f,
            -arenaHalfSize
        ),

        Vec3(
            arenaHalfSize * 2.0f,
            wallHeight,
            0.6f
        ),

        0.0f,

        0.35f,
        0.38f,
        0.45f,

        true
    );

    /*
        Южная стена.
    */

    addObject(
        WorldObjectType::Wall,

        Vec3(
            0.0f,
            wallHeight * 0.5f,
            arenaHalfSize
        ),

        Vec3(
            arenaHalfSize * 2.0f,
            wallHeight,
            0.6f
        ),

        0.0f,

        0.35f,
        0.38f,
        0.45f,

        true
    );
}

void World::createColumns()
{
    /*
        Центральные и боковые укрытия.

        Они специально расположены так,
        чтобы центральная точка (0,0)
        оставалась свободной для спавна.
    */

    const Vec3 positions[] =
    {
        Vec3(-8.0f, 2.0f, -8.0f),
        Vec3( 8.0f, 2.0f, -8.0f),
        Vec3(-8.0f, 2.0f,  8.0f),
        Vec3( 8.0f, 2.0f,  8.0f),

        Vec3(-13.0f, 1.5f,  0.0f),
        Vec3( 13.0f, 1.5f,  0.0f),

        Vec3(0.0f, 1.5f, -13.0f),
        Vec3(0.0f, 1.5f,  13.0f)
    };

    for (const Vec3& p :
         positions)
    {
        const float height =
            p.y * 2.0f;

        addObject(
            WorldObjectType::Column,

            p,

            Vec3(
                1.6f,
                height,
                1.6f
            ),

            0.0f,

            0.48f,
            0.50f,
            0.56f,

            true
        );
    }

    /*
        Низкая центральная баррикада.
        Она не блокирует стартовую точку.
    */

    addObject(
        WorldObjectType::Barrier,

        Vec3(
            0.0f,
            0.65f,
            5.0f
        ),

        Vec3(
            5.0f,
            1.3f,
            0.8f
        ),

        0.0f,

        0.30f,
        0.34f,
        0.38f,

        true
    );
}

void World::createTrees()
{
    /*
        Деревья создаются как простые
        твёрдые стволы.

        Листва будет рисоваться отдельно
        рендерером.
    */

    const Vec3 treePositions[] =
    {
        Vec3(-17.0f, 1.2f, -16.0f),
        Vec3(-12.0f, 1.2f, -18.0f),
        Vec3( 12.0f, 1.2f, -18.0f),
        Vec3( 17.0f, 1.2f, -16.0f),

        Vec3(-17.0f, 1.2f,  16.0f),
        Vec3(-12.0f, 1.2f,  18.0f),
        Vec3( 12.0f, 1.2f,  18.0f),
        Vec3( 17.0f, 1.2f,  16.0f),

        Vec3(-18.0f, 1.2f, -5.0f),
        Vec3( 18.0f, 1.2f, -5.0f),
        Vec3(-18.0f, 1.2f,  5.0f),
        Vec3( 18.0f, 1.2f,  5.0f)
    };

    for (const Vec3& p :
         treePositions)
    {
        addObject(
            WorldObjectType::Tree,

            p,

            Vec3(
                1.2f,
                2.4f,
                1.2f
            ),

            0.0f,

            0.30f,
            0.18f,
            0.08f,

            true
        );
    }
}

void World::createCrates()
{
    const Vec3 cratePositions[] =
    {
        Vec3(-5.0f, 0.75f, -14.0f),
        Vec3(-3.0f, 0.75f, -14.0f),

        Vec3(5.0f, 0.75f, -14.0f),
        Vec3(7.0f, 0.75f, -14.0f),

        Vec3(-5.0f, 0.75f, 14.0f),
        Vec3(-3.0f, 0.75f, 14.0f),

        Vec3(5.0f, 0.75f, 14.0f),
        Vec3(7.0f, 0.75f, 14.0f)
    };

    for (const Vec3& p :
         cratePositions)
    {
        addObject(
            WorldObjectType::Crate,

            p,

            Vec3(
                1.5f,
                1.5f,
                1.5f
            ),

            0.0f,

            0.48f,
            0.30f,
            0.12f,

            true
        );
    }
}

void World::createRocks()
{
    const Vec3 rockPositions[] =
    {
        Vec3(-14.0f, 0.6f, -7.0f),
        Vec3(-15.0f, 0.5f, -5.5f),

        Vec3(14.0f, 0.6f, -7.0f),
        Vec3(15.0f, 0.5f, -5.5f),

        Vec3(-14.0f, 0.6f, 7.0f),
        Vec3(-15.0f, 0.5f, 5.5f),

        Vec3(14.0f, 0.6f, 7.0f),
        Vec3(15.0f, 0.5f, 5.5f)
    };

    for (const Vec3& p :
         rockPositions)
    {
        addObject(
            WorldObjectType::Rock,

            p,

            Vec3(
                1.8f,
                1.2f,
                1.5f
            ),

            0.0f,

            0.28f,
            0.29f,
            0.31f,

            true
        );
    }
}

void World::createFences()
{
    /*
        Небольшие декоративные ограждения.
    */

    addObject(
        WorldObjectType::Fence,

        Vec3(
            -4.0f,
            0.7f,
            5.5f
        ),

        Vec3(
            3.5f,
            1.4f,
            0.35f
        ),

        0.0f,

        0.20f,
        0.22f,
        0.25f,

        true
    );

    addObject(
        WorldObjectType::Fence,

        Vec3(
            4.0f,
            0.7f,
            5.5f
        ),

        Vec3(
            3.5f,
            1.4f,
            0.35f
        ),

        0.0f,

        0.20f,
        0.22f,
        0.25f,

        true
    );

    addObject(
        WorldObjectType::Fence,

        Vec3(
            -4.0f,
            0.7f,
            -5.5f
        ),

        Vec3(
            3.5f,
            1.4f,
            0.35f
        ),

        0.0f,

        0.20f,
        0.22f,
        0.25f,

        true
    );

    addObject(
        WorldObjectType::Fence,

        Vec3(
            4.0f,
            0.7f,
            -5.5f
        ),

        Vec3(
            3.5f,
            1.4f,
            0.35f
        ),

        0.0f,

        0.20f,
        0.22f,
        0.25f,

        true
    );
}

void World::addObject(
    WorldObjectType type,
    const Vec3& position,
    const Vec3& size,
    float rotation,
    float r,
    float g,
    float b,
    bool solid
)
{
    WorldObject object;

    object.type =
        type;

    object.position =
        position;

    object.size =
        size;

    object.rotation =
        rotation;

    object.r =
        r;

    object.g =
        g;

    object.b =
        b;

    object.solid =
        solid;

    objects.push_back(
        object
    );
}

bool World::sphereIntersectsBox(
    const Vec3& spherePosition,
    float sphereRadius,
    const WorldObject& object
) const
{
    /*
        Сейчас большинство объектов
        не поворачиваются.

        Но учитываем rotation для будущих
        объектов с поворотом.
    */

    const float angle =
        -object.rotation;

    const float c =
        std::cos(angle);

    const float s =
        std::sin(angle);

    const float dx =
        spherePosition.x -
        object.position.x;

    const float dz =
        spherePosition.z -
        object.position.z;

    /*
        Переводим центр сферы
        в локальную систему объекта.
    */

    const float localX =
        dx * c -
        dz * s;

    const float localZ =
        dx * s +
        dz * c;

    const float halfX =
        object.size.x *
        0.5f;

    const float halfY =
        object.size.y *
        0.5f;

    const float halfZ =
        object.size.z *
        0.5f;

    const float closestX =
        std::max(
            -halfX,
            std::min(
                localX,
                halfX
            )
        );

    const float closestY =
        std::max(
            object.position.y -
            halfY,

            std::min(
                spherePosition.y,

                object.position.y +
                halfY
            )
        );

    const float closestZ =
        std::max(
            -halfZ,
            std::min(
                localZ,
                halfZ
            )
        );

    const float worldClosestY =
        closestY;

    const float diffX =
        localX -
        closestX;

    const float diffY =
        spherePosition.y -
        worldClosestY;

    const float diffZ =
        localZ -
        closestZ;

    const float distanceSquared =
        diffX * diffX +
        diffY * diffY +
        diffZ * diffZ;

    return distanceSquared <
           sphereRadius *
           sphereRadius;
}

bool World::rayBoxIntersection(
    const Vec3& origin,
    const Vec3& direction,
    const WorldObject& object,
    float& distance
) const
{
    /*
        Переводим луч в локальные координаты
        объекта.
    */

    const float angle =
        -object.rotation;

    const float c =
        std::cos(angle);

    const float s =
        std::sin(angle);

    const float originDX =
        origin.x -
        object.position.x;

    const float originDZ =
        origin.z -
        object.position.z;

    Vec3 localOrigin;

    localOrigin.x =
        originDX * c -
        originDZ * s;

    localOrigin.y =
        origin.y -
        object.position.y;

    localOrigin.z =
        originDX * s +
        originDZ * c;

    Vec3 localDirection;

    localDirection.x =
        direction.x * c -
        direction.z * s;

    localDirection.y =
        direction.y;

    localDirection.z =
        direction.x * s +
        direction.z * c;

    const float hx =
        object.size.x *
        0.5f;

    const float hy =
        object.size.y *
        0.5f;

    const float hz =
        object.size.z *
        0.5f;

    float tMin =
        -std::numeric_limits<float>::max();

    float tMax =
        std::numeric_limits<float>::max();

    /*
        X.
    */

    if (std::fabs(
            localDirection.x
        ) < EPSILON)
    {
        if (localOrigin.x < -hx ||
            localOrigin.x > hx)
        {
            return false;
        }
    }
    else
    {
        float t1 =
            (-hx - localOrigin.x) /
            localDirection.x;

        float t2 =
            ( hx - localOrigin.x) /
            localDirection.x;

        if (t1 > t2)
            std::swap(
                t1,
                t2
            );

        tMin =
            std::max(
                tMin,
                t1
            );

        tMax =
            std::min(
                tMax,
                t2
            );

        if (tMin > tMax)
            return false;
    }

    /*
        Y.
    */

    if (std::fabs(
            localDirection.y
        ) < EPSILON)
    {
        if (localOrigin.y < -hy ||
            localOrigin.y > hy)
        {
            return false;
        }
    }
    else
    {
        float t1 =
            (-hy - localOrigin.y) /
            localDirection.y;

        float t2 =
            ( hy - localOrigin.y) /
            localDirection.y;

        if (t1 > t2)
            std::swap(
                t1,
                t2
            );

        tMin =
            std::max(
                tMin,
                t1
            );

        tMax =
            std::min(
                tMax,
                t2
            );

        if (tMin > tMax)
            return false;
    }

    /*
        Z.
    */

    if (std::fabs(
            localDirection.z
        ) < EPSILON)
    {
        if (localOrigin.z < -hz ||
            localOrigin.z > hz)
        {
            return false;
        }
    }
    else
    {
        float t1 =
            (-hz - localOrigin.z) /
            localDirection.z;

        float t2 =
            ( hz - localOrigin.z) /
            localDirection.z;

        if (t1 > t2)
            std::swap(
                t1,
                t2
            );

        tMin =
            std::max(
                tMin,
                t1
            );

        tMax =
            std::min(
                tMax,
                t2
            );

        if (tMin > tMax)
            return false;
    }

    if (tMax < 0.0f)
        return false;

    if (tMin < 0.0f)
        distance = 0.0f;
    else
        distance = tMin;

    return true;
}