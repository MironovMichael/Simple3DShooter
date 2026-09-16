// ============================================================
//  SIMPLE 3D SHOOTER - EXTENDED EDITION
//  GLFW + OpenGL fixed pipeline
//
//  ����������:
//    WASD       - ��������
//    SHIFT      - ���
//    SPACE      - ������
//    MOUSE      - �����
//    LMB        - ��������
//    R          - �����������
//    1          - ��������
//    2          - �������
//    3          - ��������
//    ESC        - �����
//
//  �����������:
//    - ������������� ����
//    - ����
//    - �������
//    - �����
//    - �������
//    - ����������
//    - 3 ������
//    - ������ �����
//    - �������� ������
//    - ����
//    - ������
//    - ����������
//    - ������������
//    - ���������� �����
//    - ����� �� ���������� ������ ��������
//    - ����� ������� ������
//    - �������� ���������
//    - HUD
// ============================================================

#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
// CONSTANTS
// ============================================================

static constexpr float WORLD_SCALE = 3.0f;
static const float ARENA = 630.0f;

static void createCityBiome();

// ============================================================
// MULTIVERSE TYPES / STATE
// These declarations must appear before helper functions that use them.
// ============================================================
enum WorldId { WORLD_MAIN = 0, WORLD_OCEAN = 1, WORLD_WINTER = 2 };
enum OtherWorldObjectType {
    OCEAN_OBJECT_BEACON = 1,
    OCEAN_OBJECT_WRECK = 2,
    OCEAN_OBJECT_ICEBERG = 3,
    WINTER_OBJECT_RADAR = 4,
    WINTER_OBJECT_PINE = 5,
    WINTER_OBJECT_ICE_CRYSTAL = 6
};

struct OtherWorldObject {
    float x, z, scale, yaw;
    int type;
    int world;
    float radius;
};

static std::vector<OtherWorldObject> g_otherWorldObjects;

static const int WORLD_LOCAL_COORDINATE_FRAME_V11 = 1;
static int g_world = WORLD_MAIN;
static bool g_worldSeeded[3] = { true, false, false };
static bool g_previousUsePortalKey = false;

struct WorldPortal
{
    float x, z;
    int targetWorld;
    float destX, destZ;
    float radius;
};
static std::vector<WorldPortal> g_worldPortals;



static float terrainHeight(float x, float z);
static float distance2D(float x1, float z1, float x2, float z2);
static void drawBox(float cx, float cy, float cz, float sx, float sy, float sz, float r, float g, float b);


static void createOtherWorldObjects()
{
    g_otherWorldObjects.clear();

    // Each world has its own object set. Coordinates are local to that world,
    // so they never overlap the main-world city/settlements/tunnels.
    // The minimap can draw these in the same local coordinate frame.
    for (int i = 0; i < 10; ++i)
    {
        const float a = i * 0.6283185f;
        g_otherWorldObjects.push_back({
            cosf(a) * 120.0f, sinf(a) * 120.0f, 1.0f + 0.25f*(i%3),
            a + 0.4f, OCEAN_OBJECT_BEACON, WORLD_OCEAN, 4.0f
        });
    }
    g_otherWorldObjects.push_back({-165.0f, 90.0f, 1.0f, 0.4f, OCEAN_OBJECT_WRECK, WORLD_OCEAN, 8.0f});
    g_otherWorldObjects.push_back({165.0f, -70.0f, 1.3f, -0.8f, OCEAN_OBJECT_WRECK, WORLD_OCEAN, 8.0f});
    g_otherWorldObjects.push_back({-95.0f, -155.0f, 1.4f, 0.0f, OCEAN_OBJECT_ICEBERG, WORLD_OCEAN, 10.0f});
    g_otherWorldObjects.push_back({115.0f, 145.0f, 1.1f, 0.0f, OCEAN_OBJECT_ICEBERG, WORLD_OCEAN, 10.0f});

    for (int i = 0; i < 14; ++i)
    {
        const float a = i * 0.448799f;
        const float r = 85.0f + 75.0f*(i%3);
        g_otherWorldObjects.push_back({
            cosf(a)*r, sinf(a)*r, 0.8f + 0.35f*(i%4),
            a, WINTER_OBJECT_PINE, WORLD_WINTER, 2.5f
        });
    }
    g_otherWorldObjects.push_back({-120.0f, -95.0f, 1.4f, 0.0f, WINTER_OBJECT_RADAR, WORLD_WINTER, 7.0f});
    g_otherWorldObjects.push_back({130.0f, 110.0f, 1.1f, 0.0f, WINTER_OBJECT_RADAR, WORLD_WINTER, 7.0f});
    g_otherWorldObjects.push_back({-45.0f, 165.0f, 1.5f, 0.0f, WINTER_OBJECT_ICE_CRYSTAL, WORLD_WINTER, 4.0f});
    g_otherWorldObjects.push_back({55.0f, -165.0f, 1.5f, 0.0f, WINTER_OBJECT_ICE_CRYSTAL, WORLD_WINTER, 4.0f});
}

static void drawOtherWorldObject(const OtherWorldObject& o)
{
    if (o.world != g_world) return;
    const float y = terrainHeight(o.x, o.z);

    glPushMatrix();
    glTranslatef(o.x, y, o.z);
    glRotatef(o.yaw * 57.2958f, 0, 1, 0);
    glScalef(o.scale, o.scale, o.scale);

    if (o.type == OCEAN_OBJECT_BEACON)
    {
        drawBox(0, 2.0f, 0, 1.2f, 4.0f, 1.2f, 0.08f,0.65f,0.95f);
        drawBox(0, 4.2f, 0, 2.0f, 0.18f, 2.0f, 0.12f,0.85f,1.0f);
    }
    else if (o.type == OCEAN_OBJECT_WRECK)
    {
        drawBox(0, 1.0f, 0, 7.0f, 1.8f, 2.4f, 0.20f,0.24f,0.27f);
        drawBox(2.0f, 2.4f, 0, 0.35f, 3.8f, 0.35f, 0.16f,0.18f,0.20f);
    }
    else if (o.type == OCEAN_OBJECT_ICEBERG)
    {
        drawBox(0, 3.0f, 0, 5.0f, 6.0f, 4.0f, 0.68f,0.86f,0.95f);
        drawBox(0.5f, 6.0f, -0.3f, 2.2f, 3.0f, 2.0f, 0.80f,0.92f,1.0f);
    }
    else if (o.type == WINTER_OBJECT_PINE)
    {
        drawBox(0, 2.5f, 0, 0.8f, 5.0f, 0.8f, 0.25f,0.19f,0.12f);
        drawBox(0, 5.0f, 0, 4.0f, 4.0f, 4.0f, 0.10f,0.32f,0.16f);
        drawBox(0, 7.0f, 0, 2.7f, 3.0f, 2.7f, 0.12f,0.38f,0.20f);
    }
    else if (o.type == WINTER_OBJECT_RADAR)
    {
        drawBox(0, 3.0f, 0, 3.0f, 6.0f, 3.0f, 0.22f,0.24f,0.27f);
        drawBox(0, 6.4f, 0, 5.5f, 0.35f, 5.5f, 0.70f,0.78f,0.86f);
    }
    else if (o.type == WINTER_OBJECT_ICE_CRYSTAL)
    {
        drawBox(0, 2.2f, 0, 1.4f, 4.4f, 1.4f, 0.55f,0.75f,0.92f);
    }
    glPopMatrix();
}

static bool otherWorldObjectBlocked(float x, float z, float radius)
{
    for (const auto& o : g_otherWorldObjects)
    {
        if (o.world != g_world) continue;
        if (distance2D(x,z,o.x,o.z) < radius + o.radius)
            return true;
    }
    return false;
}

static void createWorldPortals();
static void updateWorldPortals();
// ============================================================
// MULTIVERSE
// ============================================================
// 0 = main world (all existing biomes + city)
// 1 = ocean world
// 2 = winter world
// Separate urban biome. The city sits on a flattened plateau so the
// skyscrapers have clean foundations and predictable collision boxes.
static constexpr float CITY_CENTER_X = 260.0f;
static constexpr float CITY_CENTER_Z = -165.0f;
static constexpr float CITY_RADIUS = 145.0f;
static constexpr float CITY_ROAD_HALF = 3.8f;

static const float PLAYER_RADIUS = 0.45f;
static const float PLAYER_HEIGHT = 1.75f;
static const float EYE_HEIGHT = 1.60f;

static const float GRAVITY = 18.0f;
static const float JUMP_SPEED = 10.5f;

static GLFWwindow* g_window = nullptr;

// ============================================================
// PLAYER
// ============================================================

static float g_px = 0.0f;
static float g_py = 0.0f; // absolute world-space feet Y
static float g_pz = 0.0f;

static float g_vy = 0.0f;
static bool g_onGround = true;
static bool g_inTunnel = false;
static constexpr float PLAYER_STEP_HEIGHT = 1.25f;
static constexpr float PLAYER_JUMPABLE_HEIGHT = 2.60f;

static float g_yaw = 0.0f;
static float g_pitch = 0.0f;

static float g_health = 100.0f;
static float g_maxHealth = 100.0f;

static float g_damageFlash = 0.0f;
static float g_deadTimer = 0.0f;

static float g_mouseSensitivity = 0.0025f;

static bool g_firstMouse = true;
static float g_lastMouseX = 0.0f;
static float g_lastMouseY = 0.0f;

// ============================================================
// GAME
// ============================================================

static int g_score = 0;
static int g_kills = 0;

static float g_gameTime = 0.0f;
static const float DAY_NIGHT_CYCLE = 48.0f;
static constexpr float TERRAIN_RENDER_DISTANCE = 520.0f;
static constexpr float OBJECT_RENDER_DISTANCE = 620.0f;
static constexpr float ENEMY_RENDER_DISTANCE = 820.0f;

static int g_storyStage = 0;
static float g_storyTimer = 8.0f;

static std::string g_message =
    "MISSION: SURVIVE AND ELIMINATE THE HOSTILES";

// ============================================================
// WEAPONS
// ============================================================

enum WeaponType
{
    WEAPON_PISTOL = 0,
    WEAPON_RIFLE = 1,
    WEAPON_SHOTGUN = 2
};

struct Weapon
{
    const char* name;

    int magazineSize;
    int ammo;

    int reserve;

    float fireDelay;
    float cooldown;

    float damage;

    int pellets;

    float spread;

    float reloadTime;
    float reloadTimer;

    float recoil;

    float range;
};

static Weapon g_weapons[3] =
{
    {
        "PISTOL",
        12,
        12,
        72,
        0.25f,
        0.0f,
        25.0f,
        1,
        0.005f,
        1.1f,
        0.0f,
        0.8f,
        100.0f
    },

    {
        "RIFLE",
        30,
        30,
        150,
        0.085f,
        0.0f,
        13.0f,
        1,
        0.012f,
        1.7f,
        0.0f,
        0.45f,
        130.0f
    },

    {
        "SHOTGUN",
        8,
        8,
        48,
        0.75f,
        0.0f,
        13.0f,
        8,
        0.09f,
        2.2f,
        0.0f,
        2.5f,
        70.0f
    }
};

static int g_currentWeapon = WEAPON_PISTOL;

static float g_weaponRecoil = 0.0f;
static float g_muzzleFlash = 0.0f;
static float g_hitMarker = 0.0f;

// ============================================================
// WORLD OBJECTS
// ============================================================
// Forward declaration for the city biome builder, which is defined later.




enum ObjectType
{
    OBJECT_SOLID,
    OBJECT_TREE,
    OBJECT_AMMO,
    OBJECT_MEDKIT,
    OBJECT_BUILDING,
    OBJECT_SKYSCRAPER
};

struct WorldObject
{
    ObjectType type;

    float x;
    float y;
    float z;

    float sx;
    float sy;
    float sz;

    bool active;
    int world = WORLD_MAIN;
};

static std::vector<WorldObject> g_objects;

// Non-solid procedural vegetation/biome decorations. These are kept separate
// from collision objects so dense vegetation cannot clog physics or spawning.
enum DecorType { DECOR_GRASS, DECOR_BUSH, DECOR_REED, DECOR_FLOWER, DECOR_ROCK, DECOR_CACTUS, DECOR_CRYSTAL };
struct DecorObject
{
    DecorType type;
    float x, y, z;
    float scale;
    float rotation;
};
static std::vector<DecorObject> g_decor;

// ============================================================
// ENEMIES
// ============================================================

enum EnemyType
{
    ENEMY_SOLDIER,
    ENEMY_FAST,
    ENEMY_TANK,
    ENEMY_DRONE
};

struct Enemy
{
    float x;
    float y;
    float z;

    float hp;
    float maxHp;

    float speed;

    float radius;

    EnemyType type;

    float attackCooldown;

    float hitFlash;

    float attackRange;

    float damage;

    float respawn;
    float yaw;
    bool underground;
    int world;
    int variant = 0;

    bool alive;
};

static std::vector<Enemy> g_enemies;
static void configureWorldEnemyVariant(Enemy& e);
static void updateOtherWorldEnemyBehavior(Enemy& e, float dt);

// ============================================================
// PROJECTILES
// ============================================================

struct Projectile
{
    float x;
    float y;
    float z;

    float dx;
    float dy;
    float dz;

    float speed;

    float damage;

    bool active;
};

static std::vector<Projectile> g_projectiles;

struct TunnelSegment
{
    float ax, az;
    float bx, bz;
    float halfWidth;
};

static const TunnelSegment g_tunnels[] =
{
    {-555.0f,165.0f,-330.0f,165.0f,5.50f},
    {-330.0f,165.0f,-105.0f,165.0f,5.50f},
    {-105.0f,165.0f,120.0f,165.0f,5.50f},
    {120.0f,165.0f,345.0f,60.0f,5.50f},
    {345.0f,60.0f,525.0f,60.0f,5.50f},
    {120.0f,165.0f,30.0f,15.0f,5.00f},
    {30.0f,15.0f,30.0f,-210.0f,5.00f},
    {30.0f,-210.0f,-165.0f,-360.0f,5.50f},
    {-165.0f,-360.0f,-405.0f,-360.0f,5.50f},
    {-165.0f,-360.0f,-30.0f,-495.0f,5.00f},
    {-30.0f,-495.0f,255.0f,-495.0f,5.50f},
    {255.0f,-495.0f,420.0f,-360.0f,5.00f},
    {-330.0f,165.0f,-465.0f,30.0f,5.00f},
    {-465.0f,30.0f,-465.0f,-225.0f,5.00f},
    {-465.0f,-225.0f,-285.0f,-360.0f,5.00f},
    {345.0f,60.0f,405.0f,285.0f,5.00f},
    {405.0f,285.0f,195.0f,450.0f,5.00f},
    {195.0f,450.0f,-60.0f,450.0f,5.00f},
};

static constexpr int TUNNEL_COUNT = sizeof(g_tunnels) / sizeof(g_tunnels[0]);

// ============================================================
// UTILS
// ============================================================

static float clampf(float v, float a, float b)
{
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

static float normalizeAngle(float a)
{
    while (a > (float)M_PI) a -= 2.0f * (float)M_PI;
    while (a < -(float)M_PI) a += 2.0f * (float)M_PI;
    return a;
}

static float turnToward(float current, float target, float maxStep)
{
    const float delta = normalizeAngle(target - current);
    if (fabsf(delta) <= maxStep)
        return target;
    return current + (delta > 0.0f ? maxStep : -maxStep);
}

static float length2(float x, float z)
{
    return sqrtf(x * x + z * z);
}

static float distance2D(float x1, float z1, float x2, float z2)
{
    return length2(x1 - x2, z1 - z2);
}

static float randomFloat(float a, float b)
{
    return a + (float)rand() / (float)RAND_MAX * (b - a);
}


static bool isCityBiome(float x, float z)
{
    const float dx = x - CITY_CENTER_X;
    const float dz = z - CITY_CENTER_Z;
    return dx*dx + dz*dz <= CITY_RADIUS*CITY_RADIUS;
}

static float oceanTerrainHeight(float x, float z)
{
    // Low water plane with several broad islands. The portal lands on the
    // central island so the player always has a safe starting surface.
    const float island =
        13.0f * expf(-(x*x + z*z) / 2200.0f) +
        9.0f  * expf(-((x-145.0f)*(x-145.0f) + (z+90.0f)*(z+90.0f)) / 1800.0f) +
        7.0f  * expf(-((x+180.0f)*(x+180.0f) + (z-130.0f)*(z-130.0f)) / 1500.0f);
    const float waves = 0.22f * sinf(x*0.045f) * cosf(z*0.052f);
    return -1.5f + island + waves;
}

static float winterTerrainHeight(float x, float z)
{
    // Same broad landscape scale, but with smoother snow-covered valleys.
    const float a = 28.0f * expf(-((x-105.0f)*(x-105.0f) + (z+75.0f)*(z+75.0f)) / 1800.0f);
    const float b = 38.0f * expf(-((x+120.0f)*(x+120.0f) + (z-85.0f)*(z-85.0f)) / 1450.0f);
    const float c = 20.0f * expf(-((x+30.0f)*(x+30.0f) + (z+150.0f)*(z+150.0f)) / 1200.0f);
    const float rolling = 2.8f*sinf(x*0.035f)*cosf(z*0.041f) + 1.2f*cosf((x+z)*0.075f);
    return clampf(a+b+c+rolling-10.0f, -12.0f, 55.0f);
}

static float terrainHeight(float x, float z)
{
    if (g_world == WORLD_OCEAN)
        return oceanTerrainHeight(x, z);
    if (g_world == WORLD_WINTER)
        return winterTerrainHeight(x / WORLD_SCALE, z / WORLD_SCALE);

    // Terrain is evaluated in normalized coordinates so the entire landscape
    // scales with the enlarged 3x world instead of leaving a flat border.
    const float worldX = x;
    const float worldZ = z;
    x /= WORLD_SCALE;
    z /= WORLD_SCALE;

    // The city biome is deliberately level. A tiny procedural variation keeps
    // the ground from looking perfectly sterile while preserving flat roads.
    if (isCityBiome(worldX, worldZ))
    {
        const float edge = distance2D(worldX, worldZ, CITY_CENTER_X, CITY_CENTER_Z) / CITY_RADIUS;
        const float subtle = 0.35f * sinf(worldX * 0.025f) * cosf(worldZ * 0.021f);
        return 2.4f + subtle + std::max(0.0f, edge - 0.82f) * 1.2f;
    }

    // Large-scale mountain chains. The center remains mostly playable while
    // the outer sectors contain high peaks and valleys.
    const float m1 = 42.0f * expf(-((x - 92.0f)*(x - 92.0f) + (z + 55.0f)*(z + 55.0f)) / 1150.0f);
    const float m2 = 50.0f * expf(-((x + 105.0f)*(x + 105.0f) + (z - 72.0f)*(z - 72.0f)) / 900.0f);
    const float m3 = 36.0f * expf(-((x + 58.0f)*(x + 58.0f) + (z + 108.0f)*(z + 108.0f)) / 780.0f);
    const float m4 = 32.0f * expf(-((x - 112.0f)*(x - 112.0f) + (z - 88.0f)*(z - 88.0f)) / 700.0f);
    const float ridge = 11.0f * expf(-(x*x) / 2400.0f) * (0.55f + 0.45f * cosf(z * 0.055f));

    const float valley1 = 13.0f * expf(-((x + 28.0f)*(x + 28.0f) + (z - 18.0f)*(z - 18.0f)) / 950.0f);
    const float valley2 = 11.0f * expf(-((x - 50.0f)*(x - 50.0f) + (z + 34.0f)*(z + 34.0f)) / 820.0f);
    const float valley3 = 8.0f * expf(-((x + 78.0f)*(x + 78.0f) + (z + 8.0f)*(z + 8.0f)) / 720.0f);

    const float basin1 = 18.0f * expf(-((x + 10.0f)*(x + 10.0f) + (z + 115.0f)*(z + 115.0f)) / 1500.0f);
    const float basin2 = 16.0f * expf(-((x - 120.0f)*(x - 120.0f) + (z + 5.0f)*(z + 5.0f)) / 1200.0f);

    const float rolling =
        2.2f * sinf(x * 0.035f) * cosf(z * 0.041f) +
        1.1f * sinf((x - z) * 0.075f) +
        0.65f * cosf((x + z) * 0.12f);

    float h = m1 + m2 + m3 + m4 + ridge - valley1 - valley2 - valley3 - basin1 - basin2 + rolling;

    return clampf(h, -30.0f, 72.0f);
}

static float pointSegmentDistanceXZ(float px, float pz, float ax, float az, float bx, float bz)
{
    const float vx = bx - ax, vz = bz - az;
    const float wx = px - ax, wz = pz - az;
    const float vv = vx*vx + vz*vz;
    float t = 0.0f;
    if (vv > 0.000001f) t = clampf((wx*vx + wz*vz) / vv, 0.0f, 1.0f);
    const float qx = ax + vx*t, qz = az + vz*t;
    return sqrtf((px-qx)*(px-qx) + (pz-qz)*(pz-qz));
}

static int nearestTunnelSegment(float x, float z, float* outDistance = nullptr)
{
    int best = -1;
    float bestD = 1e30f;
    for (int i=0;i<TUNNEL_COUNT;++i)
    {
        const auto& t = g_tunnels[i];
        const float d = pointSegmentDistanceXZ(x,z,t.ax,t.az,t.bx,t.bz);
        if (d < bestD) { bestD=d; best=i; }
    }
    if (outDistance) *outDistance = bestD;
    return best;
}

static bool isTunnelZone(float x, float z)
{
    float d = 0.0f;
    const int i = nearestTunnelSegment(x,z,&d);
    return i >= 0 && d <= g_tunnels[i].halfWidth - 0.08f;
}

static constexpr float TUNNEL_DEPTH = 13.0f;
static constexpr float TUNNEL_ENTRANCE_LENGTH = 26.0f;
static constexpr float TUNNEL_WALL_H = 2.0f;
static constexpr float TUNNEL_ROOF_R = 3.6f;

static float smoothstepf(float a, float b, float x)
{
    if (b <= a) return x >= b ? 1.0f : 0.0f;
    x = clampf((x - a) / (b - a), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

static float tunnelFloorHeight(float x, float z)
{
    int seg = nearestTunnelSegment(x, z);
    if (seg < 0) return terrainHeight(x, z);

    const auto& t = g_tunnels[seg];
    const float da = distance2D(x, z, t.ax, t.az);
    const float db = distance2D(x, z, t.bx, t.bz);
    const float endDist = std::min(da, db);

    // The tunnel starts exactly at the surface and descends smoothly over
    // a long entrance ramp. This removes the old artificial vertical step.
    const float ramp = smoothstepf(0.0f, TUNNEL_ENTRANCE_LENGTH, endDist);
    const float depth = TUNNEL_DEPTH * ramp;
    return terrainHeight(x, z) - depth;
}

static float tunnelRoofHeight(float x, float z)
{
    return tunnelFloorHeight(x, z) + TUNNEL_WALL_H + TUNNEL_ROOF_R;
}

static bool nearestPortal(
    float x, float z,
    int& outSeg, bool& outStart,
    float& outDist,
    float& outDirX, float& outDirZ)
{
    outSeg = -1;
    outStart = false;
    outDist = 1e30f;
    outDirX = 0.0f;
    outDirZ = 0.0f;

    for (int i = 0; i < TUNNEL_COUNT; ++i)
    {
        const auto& t = g_tunnels[i];
        const float dx = t.bx - t.ax;
        const float dz = t.bz - t.az;
        const float len = sqrtf(dx*dx + dz*dz);
        if (len < 0.001f)
            continue;

        const float inv = 1.0f / len;
        const float dirX = dx * inv;
        const float dirZ = dz * inv;

        const float ds = distance2D(x, z, t.ax, t.az);
        if (ds < outDist)
        {
            outDist = ds;
            outSeg = i;
            outStart = true;
            outDirX = dirX;
            outDirZ = dirZ;
        }

        const float de = distance2D(x, z, t.bx, t.bz);
        if (de < outDist)
        {
            outDist = de;
            outSeg = i;
            outStart = false;
            outDirX = -dirX;
            outDirZ = -dirZ;
        }
    }

    return outSeg >= 0;
}

static bool tunnelPortalOpeningZone(float x, float z)
{
    int seg = -1;
    bool start = false;
    float d = 0.0f;
    float dirX = 0.0f, dirZ = 0.0f;
    if (!nearestPortal(x, z, seg, start, d, dirX, dirZ))
        return false;

    const float radius = g_tunnels[seg].halfWidth + 1.75f;
    return d <= radius;
}

static bool tunnelTransitionAllowed(
    float fromX, float fromZ,
    float toX, float toZ,
    bool& crossingPortal,
    float& portalX, float& portalZ)
{
    crossingPortal = false;
    portalX = toX;
    portalZ = toZ;

    const float mvX = toX - fromX;
    const float mvZ = toZ - fromZ;
    const float mvLen = sqrtf(mvX*mvX + mvZ*mvZ);
    if (mvLen < 0.0001f)
        return true;

    const float moveX = mvX / mvLen;
    const float moveZ = mvZ / mvLen;

    int seg = -1;
    bool start = false;
    float d = 0.0f;
    float dirX = 0.0f, dirZ = 0.0f;
    if (!nearestPortal(toX, toZ, seg, start, d, dirX, dirZ))
        return true;

    const float triggerRadius = g_tunnels[seg].halfWidth + 2.75f;
    if (d > triggerRadius)
        return true;

    // Entering: move in the portal's inward direction.
    // Exiting: move in the opposite direction.
    float desiredDot = moveX * dirX + moveZ * dirZ;
    if (g_inTunnel)
    {
        if (desiredDot > -0.10f)
            return true;

        const float px = start ? g_tunnels[seg].ax : g_tunnels[seg].bx;
        const float pz = start ? g_tunnels[seg].az : g_tunnels[seg].bz;
        portalX = px - dirX * 3.5f;
        portalZ = pz - dirZ * 3.5f;
        crossingPortal = true;
        return true;
    }

    if (desiredDot < 0.10f)
        return true;

    const float px = start ? g_tunnels[seg].ax : g_tunnels[seg].bx;
    const float pz = start ? g_tunnels[seg].az : g_tunnels[seg].bz;
    portalX = px + dirX * 3.5f;
    portalZ = pz + dirZ * 3.5f;
    crossingPortal = true;
    return true;
}

static bool isTunnelEntranceCarvedZone(float x, float z)
{
    // The surface mesh is never deleted over buried tunnels. This function is
    // used only for portal-clearance checks around the visible entrance.
    return tunnelPortalOpeningZone(x, z);
}

static float currentFloorHeight(float x, float z)
{
    // A buried tunnel is a separate underground level.  While the player is
    // on the surface, the terrain remains the floor even directly above a
    // tunnel.  Only an actual portal transition switches to the tunnel floor.
    if (g_inTunnel && isTunnelZone(x,z))
        return tunnelFloorHeight(x,z);
    return terrainHeight(x,z);
}

static float terrainColorScale(float h)
{
    return clampf(0.55f + h * 0.009f, 0.45f, 1.0f);
}

static float terrainSupportHeight(float x, float z, float radius)
{
    const float r = radius * 0.85f;
    return std::max(
        std::max(terrainHeight(x - r, z - r), terrainHeight(x + r, z - r)),
        std::max(terrainHeight(x - r, z + r), terrainHeight(x + r, z + r))
    );
}

static uint32_t decorHash(int x, int z, uint32_t salt)
{
    uint32_t h = static_cast<uint32_t>(x) * 0x9E3779B9u;
    h ^= static_cast<uint32_t>(z) * 0x85EBCA6Bu;
    h ^= salt * 0xC2B2AE35u;
    h ^= h >> 16; h *= 0x7FEB352Du; h ^= h >> 15;
    h *= 0x846CA68Bu; h ^= h >> 16;
    return h;
}

static float hash01(int x, int z, uint32_t salt)
{
    return (decorHash(x,z,salt) & 0x00FFFFFFu) / 16777215.0f;
}

static int biomeAt(float x, float z)
{
    if (g_world == WORLD_OCEAN) return 8;  // ocean
    if (g_world == WORLD_WINTER) return 9; // winter
    if (isCityBiome(x, z)) return 7; // city
    const float nx = x / (ARENA * 0.92f);
    const float nz = z / (ARENA * 0.92f);
    const float h = terrainHeight(x,z);
    const float wave = 0.5f + 0.5f * sinf(nx*5.7f + nz*2.9f) * cosf(nz*4.1f - nx*1.7f);
    if (h > 43.0f) return 5; // alpine
    if (h < -9.0f) return 4; // wetlands
    if (nx < -0.32f && nz > 0.18f) return 3; // desert
    if (nx > 0.28f && nz > 0.10f) return 2; // jungle
    if (nx < -0.18f && nz < -0.22f) return 6; // volcanic
    if (wave > 0.82f) return 1; // meadow
    return 0; // mixed grassland
}

// ------------------------------------------------------------
// Surface pattern helpers
// The terrain is intentionally not a single flat color: each biome
// gets a subtle multi-scale ground pattern (soil patches, gravel,
// moss, rock, snow variation and dry streaks).
// ------------------------------------------------------------
static float valueNoise(float x, float z)
{
    const float ix = floorf(x);
    const float iz = floorf(z);
    const float fx = x - ix;
    const float fz = z - iz;

    auto hashGrid = [](int gx, int gz)
    {
        uint32_t h = decorHash(gx, gz, 0x51F15EEDu);
        h ^= h >> 13;
        h *= 0x85EBCA6Bu;
        h ^= h >> 16;
        return (h & 0x00FFFFFFu) / 16777215.0f;
    };

    const float a = hashGrid((int)ix,     (int)iz);
    const float b = hashGrid((int)ix + 1, (int)iz);
    const float c = hashGrid((int)ix,     (int)iz + 1);
    const float d = hashGrid((int)ix + 1, (int)iz + 1);

    const float sx = fx * fx * (3.0f - 2.0f * fx);
    const float sz = fz * fz * (3.0f - 2.0f * fz);
    const float ab = a + (b - a) * sx;
    const float cd = c + (d - c) * sx;
    return ab + (cd - ab) * sz;
}

static float terrainPattern(float x, float z)
{
    const float coarse = valueNoise(x * 0.055f, z * 0.055f);
    const float medium = valueNoise(x * 0.14f + 17.0f, z * 0.14f - 11.0f);
    const float fine = valueNoise(x * 0.42f - 23.0f, z * 0.42f + 31.0f);
    return clampf(0.50f * coarse + 0.33f * medium + 0.17f * fine, 0.0f, 1.0f);
}

static void terrainBaseColor(float x, float z, float avgHeight, float& r, float& g, float& b)
{
    const int biome = biomeAt(x, z);
    const float p = terrainPattern(x, z);
    const float streak = 0.5f + 0.5f * sinf(x * 0.13f + sinf(z * 0.035f) * 2.4f);
    const float detail = clampf(0.86f + p * 0.24f + streak * 0.06f, 0.76f, 1.18f);

    switch (biome)
    {
        case 8: // ocean
            r=0.035f; g=0.19f; b=0.30f;
            break;
        case 9: // winter
            r=0.78f; g=0.84f; b=0.90f;
            break;
        case 1: // meadow
            r=0.20f; g=0.42f; b=0.12f;
            break;
        case 2: // jungle
            r=0.08f; g=0.34f; b=0.10f;
            break;
        case 3: // desert
            r=0.56f; g=0.40f; b=0.16f;
            break;
        case 4: // wetlands
            r=0.12f; g=0.25f; b=0.18f;
            break;
        case 5: // alpine
            r=0.42f; g=0.43f; b=0.46f;
            break;
        case 6: // volcanic
            r=0.17f; g=0.08f; b=0.07f;
            break;
        case 7: // city
            r=0.17f; g=0.19f; b=0.21f;
            break;
        default: // mixed grassland
            r=0.23f; g=0.44f; b=0.15f;
            break;
    }

    // Height adds a readable material transition without becoming a
    // single-color gradient.
    if (avgHeight < -8.0f)
    {
        r *= 0.72f; g *= 0.90f; b *= 1.10f;
    }
    else if (avgHeight > 32.0f)
    {
        r = 0.52f + r * 0.30f;
        g = 0.54f + g * 0.30f;
        b = 0.56f + b * 0.32f;
    }

    r *= detail;
    g *= detail;
    b *= detail;
}


static bool steepAt(float x, float z)
{
    const float s = 4.5f;
    const float hx = terrainHeight(x+s,z) - terrainHeight(x-s,z);
    const float hz = terrainHeight(x,z+s) - terrainHeight(x,z-s);
    return (hx*hx + hz*hz) > 130.0f;
}

static bool decorBlocked(float x, float z)
{
    if (distance2D(x,z,g_px,g_pz) < 2.5f) return true;
    if (tunnelPortalOpeningZone(x,z)) return true;
    if (g_inTunnel && isTunnelZone(x,z)) return true;
    return false;
}

static void generateDecor()
{
    g_decor.clear();
    if (g_world != WORLD_MAIN)
        return;
    // 10-unit cells: dense enough to look populated, sparse enough to render fast.
    const float cell = 10.0f;
    const int minC = static_cast<int>(floorf((-ARENA+cell)*0.5f/cell));
    const int maxC = static_cast<int>(ceilf((ARENA-cell)*0.5f/cell));
    g_decor.reserve(18000);

    for (int iz=minC; iz<=maxC; ++iz)
    {
        for (int ix=minC; ix<=maxC; ++ix)
        {
            const float bx = (ix*2.0f+1.0f)*cell*0.5f;
            const float bz = (iz*2.0f+1.0f)*cell*0.5f;
            const int biome = biomeAt(bx,bz);
            if (biome == 7) continue;
            const float density = (biome==1 || biome==0) ? 0.88f : 0.68f;
            if (hash01(ix,iz,11u) > density) continue;

            const float jx = (hash01(ix,iz,23u)-0.5f)*cell*0.88f;
            const float jz = (hash01(ix,iz,29u)-0.5f)*cell*0.88f;
            const float x = bx + jx;
            const float z = bz + jz;
            if (fabsf(x)>ARENA-5.0f || fabsf(z)>ARENA-5.0f) continue;
            if (decorBlocked(x,z) || steepAt(x,z)) continue;

            const float y = terrainSupportHeight(x,z,0.15f);
            const float sc = 0.65f + hash01(ix,iz,41u)*0.8f;
            DecorType type = DECOR_GRASS;
            if (biome==3) type = (hash01(ix,iz,51u)<0.64f) ? DECOR_CACTUS : DECOR_ROCK;
            else if (biome==2) type = (hash01(ix,iz,51u)<0.65f) ? DECOR_REED : DECOR_FLOWER;
            else if (biome==4) type = (hash01(ix,iz,51u)<0.72f) ? DECOR_REED : DECOR_BUSH;
            else if (biome==5) type = (hash01(ix,iz,51u)<0.68f) ? DECOR_ROCK : DECOR_GRASS;
            else if (biome==6) type = (hash01(ix,iz,51u)<0.60f) ? DECOR_CRYSTAL : DECOR_ROCK;
            else if (biome==1) type = (hash01(ix,iz,51u)<0.75f) ? DECOR_GRASS : DECOR_FLOWER;

            g_decor.push_back({type,x,y,z,sc,hash01(ix,iz,71u)*(float)M_PI*2.0f});

            // Extra small blades/clumps around the primary decor in fertile biomes.
            if (type==DECOR_GRASS || type==DECOR_FLOWER)
            {
                for (int k=0;k<3;++k)
                {
                    const float ox=(hash01(ix*3+k,iz,81u)-0.5f)*cell*0.9f;
                    const float oz=(hash01(ix,iz*3+k,87u)-0.5f)*cell*0.9f;
                    const float gx=x+ox, gz=z+oz;
                    if (fabsf(gx)>ARENA-3 || fabsf(gz)>ARENA-3 || decorBlocked(gx,gz) || steepAt(gx,gz)) continue;
                    g_decor.push_back({DECOR_GRASS,gx,terrainSupportHeight(gx,gz,0.08f),gz,
                                       0.45f+hash01(ix+k,iz+k,91u)*0.6f,hash01(ix+k,iz+k,93u)*(float)M_PI*2.0f});
                }
            }
        }
    }
}

static float terrainDayFactor()
{
    const float phase = fmodf(g_gameTime, DAY_NIGHT_CYCLE) / DAY_NIGHT_CYCLE;
    const float sun = 0.5f + 0.5f * sinf(phase * 2.0f * (float)M_PI - 0.35f*(float)M_PI);
    return clampf(sun,0.0f,1.0f);
}

static bool segmentSphereHit(
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    float cx, float cy, float cz,
    float radius)
{
    const float vx = x1 - x0;
    const float vy = y1 - y0;
    const float vz = z1 - z0;
    const float wx = cx - x0;
    const float wy = cy - y0;
    const float wz = cz - z0;
    const float vv = vx*vx + vy*vy + vz*vz;
    float t = 0.0f;
    if (vv > 0.000001f)
        t = clampf((wx*vx + wy*vy + wz*vz) / vv, 0.0f, 1.0f);

    const float qx = x0 + vx*t - cx;
    const float qy = y0 + vy*t - cy;
    const float qz = z0 + vz*t - cz;
    return qx*qx + qy*qy + qz*qz <= radius*radius;
}

static bool rayTerrainHit(float ox, float oy, float oz, float dx, float dy, float dz, float maxDistance, float& tHit)
{
    float prevT = 0.0f;
    float prevY = oy;
    float prevGround = currentFloorHeight(ox,oz);
    if (prevY <= prevGround + 0.02f) { tHit=0.0f; return true; }
    const int steps = 220;
    for (int i=1;i<=steps;++i)
    {
        const float t = maxDistance*(float)i/(float)steps;
        const float x=ox+dx*t, y=oy+dy*t, z=oz+dz*t;
        const float ground=currentFloorHeight(x,z);
        if (y <= ground+0.02f && prevY > prevGround+0.02f)
        {
            float lo=prevT, hi=t;
            for (int k=0;k<8;++k)
            {
                const float mid=0.5f*(lo+hi);
                const float mx=ox+dx*mid, my=oy+dy*mid, mz=oz+dz*mid;
                if (my <= currentFloorHeight(mx,mz)+0.02f) hi=mid; else lo=mid;
            }
            tHit=hi; return true;
        }
        prevT=t; prevY=y; prevGround=ground;
    }
    return false;
}

static bool tunnelWallHitSegment(float x0,float y0,float z0,float x1,float y1,float z1)
{
    const float dx=x1-x0, dy=y1-y0, dz=z1-z0;
    const int steps=24;
    for(int i=0;i<=steps;++i)
    {
        const float u=(float)i/(float)steps;
        const float x=x0+dx*u, y=y0+dy*u, z=z0+dz*u;
        if (isTunnelZone(x,z))
        {
            if (y < tunnelFloorHeight(x,z)+0.08f || y > tunnelRoofHeight(x,z)-0.08f) return true;
        }
        else if (isTunnelZone(x0,z0)) return true;
    }
    return false;
}

// ============================================================
// COLLISION
// ============================================================

static bool pointInsideBox(
    float x,
    float z,
    const WorldObject& o,
    float extra)
{
    return
        x >= o.x - o.sx * 0.5f - extra &&
        x <= o.x + o.sx * 0.5f + extra &&
        z >= o.z - o.sz * 0.5f - extra &&
        z <= o.z + o.sz * 0.5f + extra;
}

// Trees are visually large because of their foliage, but the player must
// collide only with the trunk.  Treating the whole foliage AABB as solid
// makes the player stop well before the trunk and can trap the player at
// corners.
static bool circleIntersectsTreeTrunk(
    float x,
    float z,
    const WorldObject& tree,
    float playerRadius)
{
    const float trunkHalf = 0.42f;
    const float minX = tree.x - trunkHalf;
    const float maxX = tree.x + trunkHalf;
    const float minZ = tree.z - trunkHalf;
    const float maxZ = tree.z + trunkHalf;

    float closestX = x;
    float closestZ = z;

    if (closestX < minX) closestX = minX;
    else if (closestX > maxX) closestX = maxX;

    if (closestZ < minZ) closestZ = minZ;
    else if (closestZ > maxZ) closestZ = maxZ;

    const float dx = x - closestX;
    const float dz = z - closestZ;
    const float radius = playerRadius + trunkHalf;
    return dx * dx + dz * dz < radius * radius;
}

static bool playerCollidesWithObject(
    float x,
    float z,
    const WorldObject& o,
    float radius)
{
    if (o.type == OBJECT_TREE)
        return circleIntersectsTreeTrunk(x, z, o, radius);

    return pointInsideBox(x, z, o, radius);
}

static bool tunnelConflictsObject(const WorldObject& o)
{
    if (o.type == OBJECT_AMMO || o.type == OBJECT_MEDKIT)
        return false;

    const float hx = o.sx * 0.5f + 1.0f;
    const float hz = o.sz * 0.5f + 1.0f;
    const float samples[][2] = {
        {o.x, o.z},
        {o.x-hx, o.z-hz}, {o.x+hx, o.z-hz},
        {o.x-hx, o.z+hz}, {o.x+hx, o.z+hz},
        {o.x-hx, o.z}, {o.x+hx, o.z},
        {o.x, o.z-hz}, {o.x, o.z+hz}
    };
    for (const auto& p : samples)
    {
        // Surface objects only conflict with the entrance cut/opening.
        // Objects above a buried section are allowed to remain on the surface.
        if (isTunnelEntranceCarvedZone(p[0], p[1]))
            return true;
    }
    return false;
}

static float playerFloorHeight(float x, float z)
{
    float floor = currentFloorHeight(x,z);
    const bool underground = g_inTunnel && isTunnelZone(x,z);
    for (const auto& o : g_objects)
    {
        if (!o.active || o.world != g_world) continue;
        if (o.type == OBJECT_AMMO || o.type == OBJECT_MEDKIT) continue;
        if (underground && tunnelConflictsObject(o)) continue;
        if (!pointInsideBox(x,z,o,PLAYER_RADIUS*0.35f)) continue;
        const float top=o.y+o.sy*0.5f;
        if(top>floor+0.03f && top-floor<=PLAYER_STEP_HEIGHT)
            floor=top;
        else if(!g_onGround && g_vy<=0.0f && top>floor+0.03f &&
                top-floor<=PLAYER_JUMPABLE_HEIGHT && g_py>=top-0.55f)
            floor=top;
    }
    return floor;
}

static bool playerInsideStepableObstacle(float x, float z)
{
    const float base = currentFloorHeight(x,z);
    for (const auto& o : g_objects)
    {
        if (!o.active || o.world != g_world) continue;
        if (o.type == OBJECT_AMMO || o.type == OBJECT_MEDKIT) continue;
        if (g_inTunnel && tunnelConflictsObject(o)) continue;
        if (!playerCollidesWithObject(x,z,o,PLAYER_RADIUS)) continue;
        const float top=o.y+o.sy*0.5f;
        if (o.type == OBJECT_TREE) continue;
        if(top-base<=PLAYER_STEP_HEIGHT+0.05f) return true;
    }
    return false;
}

static bool tunnelMovementClear(float x, float z, float radius)
{
    if (!isTunnelZone(x, z))
        return true;

    float d = 0.0f;
    const int i = nearestTunnelSegment(x, z, &d);
    if (i < 0)
        return false;

    return d <= g_tunnels[i].halfWidth - radius - 0.18f;
}

static bool playerBlocked(float x, float z)
{
    if (g_inTunnel && isTunnelZone(x, z) && !tunnelMovementClear(x, z, PLAYER_RADIUS))
        return true;

    const bool stepable = playerInsideStepableObstacle(x,z);
    if (stepable)
        return false;

    for (const auto& o : g_objects)
    {
        if (!o.active || o.world != g_world) continue;
        if (o.type == OBJECT_AMMO || o.type == OBJECT_MEDKIT) continue;
        if (g_inTunnel && tunnelConflictsObject(o)) continue;
        if (playerCollidesWithObject(x, z, o, PLAYER_RADIUS))
        {
            const float top=o.y+o.sy*0.5f;
            // While airborne, allow the player to pass over an obstacle once
            // the feet are above its top. This is what makes jumping onto
            // crates/cover possible without disabling side collision.
            if(!g_onGround && g_py>=top-0.05f) continue;
            return true;
        }
    }
    return false;
}

static bool enemyBlocked(float x, float z, float radius)
{
    for (const auto& o : g_objects)
    {
        if (!o.active || o.world != g_world)
            continue;

        if (o.type == OBJECT_AMMO ||
            o.type == OBJECT_MEDKIT)
            continue;

        if (pointInsideBox(x, z, o, radius))
            return true;
    }

    return false;
}

// ============================================================
// SAFE SPAWN
// ============================================================

static bool safeSpawn(
    float& outX,
    float& outZ,
    float minDistanceFromPlayer)
{
    for (int attempt = 0; attempt < 300; ++attempt)
    {
        float x = randomFloat(-ARENA + 3.0f, ARENA - 3.0f);
        float z = randomFloat(-ARENA + 3.0f, ARENA - 3.0f);

        if (distance2D(x, z, g_px, g_pz) < minDistanceFromPlayer)
            continue;

        if (playerBlocked(x, z))
            continue;

        bool tooCloseEnemy = false;

        for (const auto& e : g_enemies)
        {
            if (!e.alive)
                continue;

            if (distance2D(x, z, e.x, e.z) < 2.0f)
            {
                tooCloseEnemy = true;
                break;
            }
        }

        if (tooCloseEnemy)
            continue;

        outX = x;
        outZ = z;
        return true;
    }

    return false;
}

// ============================================================
// RAY / AABB
// ============================================================

static bool rayAABB(
    float ox,
    float oy,
    float oz,

    float dx,
    float dy,
    float dz,

    float cx,
    float cy,
    float cz,

    float hx,
    float hy,
    float hz,

    float& tHit)
{
    float tmin = -1e30f;
    float tmax =  1e30f;

    float o[3] =
    {
        ox, oy, oz
    };

    float d[3] =
    {
        dx, dy, dz
    };

    float c[3] =
    {
        cx, cy, cz
    };

    float h[3] =
    {
        hx, hy, hz
    };

    for (int i = 0; i < 3; ++i)
    {
        if (fabsf(d[i]) < 0.000001f)
        {
            if (o[i] < c[i] - h[i] ||
                o[i] > c[i] + h[i])
                return false;
        }
        else
        {
            float inv = 1.0f / d[i];

            float t1 =
                (c[i] - h[i] - o[i]) * inv;

            float t2 =
                (c[i] + h[i] - o[i]) * inv;

            if (t1 > t2)
            {
                float tmp = t1;
                t1 = t2;
                t2 = tmp;
            }

            if (t1 > tmin)
                tmin = t1;

            if (t2 < tmax)
                tmax = t2;

            if (tmin > tmax)
                return false;
        }
    }

    if (tmax < 0.0f)
        return false;

    tHit = tmin < 0.0f ? 0.0f : tmin;

    return true;
}

// ============================================================
// DRAWING
// ============================================================

static void drawBox(
    float cx,
    float cy,
    float cz,
    float sx,
    float sy,
    float sz,
    float r,
    float g,
    float b)
{
    float hx = sx * 0.5f;
    float hy = sy * 0.5f;
    float hz = sz * 0.5f;

    glBegin(GL_QUADS);

    // top
    glColor3f(r * 1.1f, g * 1.1f, b * 1.1f);

    glVertex3f(cx-hx, cy+hy, cz-hz);
    glVertex3f(cx-hx, cy+hy, cz+hz);
    glVertex3f(cx+hx, cy+hy, cz+hz);
    glVertex3f(cx+hx, cy+hy, cz-hz);

    // bottom
    glColor3f(r*0.35f, g*0.35f, b*0.35f);

    glVertex3f(cx-hx, cy-hy, cz-hz);
    glVertex3f(cx+hx, cy-hy, cz-hz);
    glVertex3f(cx+hx, cy-hy, cz+hz);
    glVertex3f(cx-hx, cy-hy, cz+hz);

    // front
    glColor3f(r*0.85f, g*0.85f, b*0.85f);

    glVertex3f(cx-hx, cy-hy, cz+hz);
    glVertex3f(cx+hx, cy-hy, cz+hz);
    glVertex3f(cx+hx, cy+hy, cz+hz);
    glVertex3f(cx-hx, cy+hy, cz+hz);

    // back
    glColor3f(r*0.55f, g*0.55f, b*0.55f);

    glVertex3f(cx+hx, cy-hy, cz-hz);
    glVertex3f(cx-hx, cy-hy, cz-hz);
    glVertex3f(cx-hx, cy+hy, cz-hz);
    glVertex3f(cx+hx, cy+hy, cz-hz);

    // left
    glColor3f(r*0.65f, g*0.65f, b*0.65f);

    glVertex3f(cx-hx, cy-hy, cz-hz);
    glVertex3f(cx-hx, cy-hy, cz+hz);
    glVertex3f(cx-hx, cy+hy, cz+hz);
    glVertex3f(cx-hx, cy+hy, cz-hz);

    // right
    glColor3f(r*0.9f, g*0.9f, b*0.9f);

    glVertex3f(cx+hx, cy-hy, cz+hz);
    glVertex3f(cx+hx, cy-hy, cz-hz);
    glVertex3f(cx+hx, cy+hy, cz-hz);
    glVertex3f(cx+hx, cy+hy, cz+hz);

    glEnd();
}

static void drawTree(float x, float baseY, float z)
{
    drawBox(x, baseY + 1.4f, z, 0.8f, 2.8f, 0.8f, 0.34f, 0.19f, 0.07f);
    drawBox(x, baseY + 3.35f, z, 3.2f, 2.5f, 3.2f, 0.05f, 0.42f, 0.10f);
    drawBox(x, baseY + 4.65f, z, 2.3f, 1.8f, 2.3f, 0.08f, 0.62f, 0.12f);
}

static void drawAmmo(float x, float baseY, float z)
{
    drawBox(x, baseY + 0.35f, z, 0.9f, 0.7f, 0.7f, 0.12f, 0.23f, 0.38f);
    drawBox(x, baseY + 0.72f, z, 0.5f, 0.08f, 0.5f, 0.95f, 0.72f, 0.05f);
}

static void drawMedkit(float x, float baseY, float z)
{
    drawBox(x, baseY + 0.4f, z, 1.0f, 0.8f, 1.0f, 0.90f, 0.90f, 0.90f);
    drawBox(x, baseY + 0.83f, z, 0.18f, 0.05f, 0.65f, 0.92f, 0.03f, 0.03f);
    drawBox(x, baseY + 0.83f, z, 0.65f, 0.05f, 0.18f, 0.92f, 0.03f, 0.03f);
}

static void drawBuildingWall(const WorldObject& o)
{
    // Large settlement walls use a warmer, house-like material.
    drawBox(o.x, o.y, o.z, o.sx, o.sy, o.sz, 0.52f, 0.30f, 0.16f);
}

static void drawHouseRoof(float x, float y, float z, float w, float d)
{
    drawBox(x, y, z, w + 0.9f, 0.65f, d + 0.9f, 0.22f, 0.13f, 0.10f);
    // raised ridge gives the houses a recognizable silhouette.
    drawBox(x, y + 0.38f, z, w * 0.72f, 0.22f, d + 0.35f, 0.30f, 0.16f, 0.12f);
}

static void addHouse(float cx, float cz, float rotation = 0.0f)
{
    // Collision-friendly rectangular house with a real doorway opening.
    // The opening is deliberately wide enough for the player and enemies.
    const float W = 13.0f;
    const float D = 10.0f;
    const float H = 5.5f;
    const float T = 0.65f;
    const float DOOR = 3.0f;
    const float side = (W - DOOR) * 0.5f;
    const float c = cosf(rotation), si = sinf(rotation);

    auto addPart = [&](float lx, float lz, float sx, float sy, float sz)
    {
        const float x = cx + lx*c - lz*si;
        const float z = cz + lx*si + lz*c;
        g_objects.push_back({OBJECT_BUILDING, x, sy*0.5f, z, sx, sy, sz, true});
    };

    // Front wall, split around the doorway.
    addPart(-(DOOR*0.5f + side*0.5f), -D*0.5f, side, H, T);
    addPart( +(DOOR*0.5f + side*0.5f), -D*0.5f, side, H, T);
    // Back and side walls.
    addPart(0.0f, D*0.5f, W, H, T);
    addPart(-W*0.5f, 0.0f, T, H, D);
    addPart( W*0.5f, 0.0f, T, H, D);
    // Roof collision keeps enemies from shooting straight down while
    // preserving a comfortable interior height.
    addPart(0.0f, 0.0f, W+0.35f, 0.45f, D+0.35f);

    // Door lintel above the entrance.
    addPart(0.0f, -D*0.5f, DOOR, 1.4f, T);
}

// ============================================================
// 7 SEGMENT HUD
// ============================================================

static void rect(
    float x1,
    float y1,
    float x2,
    float y2)
{
    glBegin(GL_QUADS);

    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);

    glEnd();
}

static void drawDigit(
    float x,
    float y,
    float s,
    int d)
{
    static const unsigned char seg[10] =
    {
        0x3F,
        0x06,
        0x5B,
        0x4F,
        0x66,
        0x6D,
        0x7D,
        0x07,
        0x7F,
        0x6F
    };

    unsigned char m = seg[d % 10];

    float t = s * 0.18f;

    float mid = y + s * 0.5f;

    if (m & 0x01)
        rect(
            x+t,
            y+s-t,
            x+s-t,
            y+s
        );

    if (m & 0x02)
        rect(
            x+s-t,
            mid,
            x+s,
            y+s-t
        );

    if (m & 0x04)
        rect(
            x+s-t,
            y+t,
            x+s,
            mid
        );

    if (m & 0x08)
        rect(
            x+t,
            y,
            x+s-t,
            y+t
        );

    if (m & 0x10)
        rect(
            x,
            y+t,
            x+t,
            mid
        );

    if (m & 0x20)
        rect(
            x,
            mid,
            x+t,
            y+s-t
        );

    if (m & 0x40)
        rect(
            x+t,
            mid-t*0.5f,
            x+s-t,
            mid+t*0.5f
        );
}

static void drawNumber(
    float x,
    float y,
    float s,
    int value)
{
    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "%d",
        value
    );

    float px = x;

    for (char* p = buffer; *p; ++p)
    {
        if (*p >= '0' && *p <= '9')
        {
            drawDigit(
                px,
                y,
                s,
                *p - '0'
            );

            px += s * 1.3f;
        }
    }
}

// ============================================================
// MOUSE
// ============================================================

static void mouseCallback(
    GLFWwindow*,
    double xpos,
    double ypos)
{
    if (g_firstMouse)
    {
        g_lastMouseX = (float)xpos;
        g_lastMouseY = (float)ypos;

        g_firstMouse = false;

        return;
    }

    float dx =
        (float)xpos - g_lastMouseX;

    float dy =
        (float)ypos - g_lastMouseY;

    g_lastMouseX = (float)xpos;
    g_lastMouseY = (float)ypos;

    // ������ ������ �� ��������� ������ ������
    if (fabsf(dx) > 100.0f)
        dx = 0.0f;

    if (fabsf(dy) > 100.0f)
        dy = 0.0f;

    g_yaw += dx * g_mouseSensitivity;

    g_pitch -= dy * g_mouseSensitivity;

    g_pitch =
        clampf(
            g_pitch,
            -1.45f,
            1.45f
        );
}

// ============================================================
// DAMAGE
// ============================================================

static bool enemyHasLineOfSight(const Enemy& e)
{
    const bool enemyTunnel = e.underground || isTunnelZone(e.x,e.z);
    const bool playerTunnel = isTunnelZone(g_px,g_pz);
    if (enemyTunnel != playerTunnel) return false;

    const float ox=e.x;
    const float oy=e.type==ENEMY_DRONE ? e.y : terrainSupportHeight(e.x,e.z,e.radius)+(e.type==ENEMY_TANK?1.9f:1.45f);
    const float oz=e.z;
    const float tx=g_px, ty=g_py+EYE_HEIGHT, tz=g_pz;
    float dx=tx-ox, dy=ty-oy, dz=tz-oz;
    const float len=sqrtf(dx*dx+dy*dy+dz*dz);
    if(len<0.001f) return true;
    dx/=len; dy/=len; dz/=len;
    for(const auto& o:g_objects)
    {
        if(!o.active || o.world != g_world || o.type==OBJECT_AMMO || o.type==OBJECT_MEDKIT) continue;
        float hitT=len;
        if(rayAABB(ox,oy,oz,dx,dy,dz,o.x,o.y,o.z,o.sx*0.5f,o.sy*0.5f,o.sz*0.5f,hitT) && hitT<len-0.1f) return false;
    }
    float terrainT=len;
    if(rayTerrainHit(ox,oy,oz,dx,dy,dz,len,terrainT) && terrainT<len-0.1f) return false;
    if(enemyTunnel && tunnelWallHitSegment(ox,oy,oz,tx,ty,tz)) return false;
    return true;
}

static void respawnPlayer()
{
    float x = 0.0f;
    float z = 20.0f;

    if (!safeSpawn(x, z, 6.0f))
    {
        x = 0.0f;
        z = 20.0f;
    }

    g_px = x;
    g_pz = z;
    g_py = currentFloorHeight(g_px, g_pz);
    g_vy = 0.0f;
    g_onGround = true;
    g_health = g_maxHealth;
    g_deadTimer = 0.0f;
    g_damageFlash = 0.0f;
    g_firstMouse = true;
    g_message = "RESPAWNED - KEEP FIGHTING";
}

static void damagePlayer(float damage)
{
    if (g_deadTimer > 0.0f || damage <= 0.0f)
        return;

    g_health -= damage;
    g_damageFlash = 1.0f;

    if (g_health <= 0.0f)
    {
        g_health = 0.0f;
        g_vy = 0.0f;
        g_onGround = true;
        g_deadTimer = 3.0f;
        g_message = "YOU DIED - RESPAWNING...";
    }
}

// ============================================================
// WEAPON SWITCH
// ============================================================

static void switchWeapon(int index)
{
    if (index < 0 || index > 2)
        return;

    g_currentWeapon = index;

    g_weaponRecoil = 0.0f;

    g_message =
        std::string("WEAPON: ") +
        g_weapons[index].name;
}

// ============================================================
// RELOAD
// ============================================================

static void reloadWeapon()
{
    Weapon& w =
        g_weapons[g_currentWeapon];

    if (w.reloadTimer > 0.0f)
        return;

    if (w.ammo >= w.magazineSize)
        return;

    if (w.reserve <= 0)
        return;

    w.reloadTimer =
        w.reloadTime;

    g_message = "RELOADING...";
}

// ============================================================
// FINISH RELOAD
// ============================================================

static void updateReload(float dt)
{
    Weapon& w =
        g_weapons[g_currentWeapon];

    if (w.reloadTimer <= 0.0f)
        return;

    w.reloadTimer -= dt;

    if (w.reloadTimer <= 0.0f)
    {
        int needed =
            w.magazineSize - w.ammo;

        int take =
            std::min(
                needed,
                w.reserve
            );

        w.ammo += take;

        w.reserve -= take;

        g_message = "READY";
    }
}

// ============================================================
// SHOOT
// ============================================================

static bool raySphere(float ox, float oy, float oz,
                      float dx, float dy, float dz,
                      float cx, float cy, float cz, float radius, float& tHit)
{
    const float bx = ox - cx;
    const float by = oy - cy;
    const float bz = oz - cz;
    const float b = bx*dx + by*dy + bz*dz;
    const float c = bx*bx + by*by + bz*bz - radius*radius;
    const float disc = b*b - c;
    if (disc < 0.0f) return false;
    const float sdisc = sqrtf(disc);
    float t = -b - sdisc;
    if (t < 0.0f) t = -b + sdisc;
    if (t < 0.0f) return false;
    tHit = t;
    return true;
}

static void shoot()
{
    Weapon& w =
        g_weapons[g_currentWeapon];

    if (w.reloadTimer > 0.0f)
        return;

    if (w.cooldown > 0.0f)
        return;

    if (w.ammo <= 0)
    {
        reloadWeapon();
        return;
    }

    w.ammo--;

    w.cooldown =
        w.fireDelay;

    g_weaponRecoil =
        w.recoil;

    g_muzzleFlash =
        1.0f;

    float cp = cosf(g_pitch);

    const float shotOriginY = g_py + EYE_HEIGHT;

    float baseDX =
        cp * sinf(g_yaw);

    float baseDY =
        sinf(g_pitch);

    float baseDZ =
        -cp * cosf(g_yaw);

    for (int pellet = 0;
         pellet < w.pellets;
         ++pellet)
    {
        float dx = baseDX;
        float dy = baseDY;
        float dz = baseDZ;

        dx += randomFloat(-w.spread, w.spread);
        dy += randomFloat(-w.spread, w.spread);
        dz += randomFloat(-w.spread, w.spread);

        float len =
            sqrtf(
                dx*dx +
                dy*dy +
                dz*dz
            );

        if (len < 0.0001f)
            continue;

        dx /= len;
        dy /= len;
        dz /= len;

        float bestT = w.range;
        float terrainT = w.range;
        if (rayTerrainHit(g_px,shotOriginY,g_pz,dx,dy,dz,w.range,terrainT) && terrainT > 0.02f)
            bestT = terrainT;

        int bestEnemy = -1;
        bool bestHeadshot = false;

        // world collision
        for (const auto& o : g_objects)
        {
            if (!o.active || o.world != g_world)
                continue;

            if (o.type == OBJECT_AMMO ||
                o.type == OBJECT_MEDKIT)
                continue;

            float t;

            if (rayAABB(
                    g_px,
                    shotOriginY,
                    g_pz,

                    dx,
                    dy,
                    dz,

                    o.x,
                    o.sy * 0.5f,
                    o.z,

                    o.sx * 0.5f,
                    o.sy * 0.5f,
                    o.sz * 0.5f,

                    t))
            {
                if (t < bestT)
                    bestT = t;
            }
        }

        // enemies
        for (size_t i = 0; i < g_enemies.size(); ++i)
        {
            Enemy& e = g_enemies[i];
            if (!e.alive)
                continue;

            const float baseY = e.type == ENEMY_DRONE ? e.y - 0.65f : terrainSupportHeight(e.x,e.z,e.radius);
            const float bodyHeight = (e.type == ENEMY_TANK) ? 2.35f : (e.type == ENEMY_DRONE ? 0.95f : 1.75f);
            const float bodyRadius = (e.type == ENEMY_TANK) ? 0.82f : (e.type == ENEMY_DRONE ? 0.55f : e.radius + 0.05f);
            const float headCenterY = (e.type == ENEMY_DRONE) ? (baseY + 1.30f) : (baseY + bodyHeight + 0.28f);
            const float headRadius = (e.type == ENEMY_TANK) ? 0.52f : (e.type == ENEMY_DRONE ? 0.28f : 0.36f);
            float bodyT = w.range, headT = w.range;
            const bool hitBody = rayAABB(g_px,shotOriginY,g_pz,dx,dy,dz,
                                         e.x,baseY+bodyHeight*0.5f,e.z,
                                         bodyRadius,bodyHeight*0.5f,bodyRadius,bodyT);
            const bool hitHead = raySphere(g_px,shotOriginY,g_pz,dx,dy,dz,
                                           e.x,headCenterY,e.z,headRadius,headT);
            float enemyT = w.range;
            bool enemyHeadshot = false;
            if (hitBody && bodyT < enemyT) { enemyT=bodyT; enemyHeadshot=false; }
            if (hitHead && headT < enemyT) { enemyT=headT; enemyHeadshot=(e.type != ENEMY_DRONE); }

            if (enemyT < bestT)
            {
                bestT = enemyT;
                bestEnemy = (int)i;
                bestHeadshot = enemyHeadshot;
            }
        }

        if (bestEnemy >= 0)
        {
            Enemy& e =
                g_enemies[bestEnemy];

            const float damageMultiplier = bestHeadshot ? 2.5f : 1.0f;
            e.hp -= w.damage * damageMultiplier;
            e.hitFlash = 1.0f;
            g_hitMarker = 1.0f;
            if (bestHeadshot)
                g_message = "HEADSHOT!";

            if (e.hp <= 0.0f)
            {
                e.hp = 0.0f;

                e.alive = false;

                e.respawn = 4.0f;

                g_score +=
                    e.type == ENEMY_TANK ? 300 :
                    e.type == ENEMY_FAST ? 150 :
                    100;

                g_kills++;

                if (g_kills == 1)
                {
                    g_message =
                        "GOOD SHOT. MORE HOSTILES INCOMING.";
                }

                if (g_kills == 5)
                {
                    g_message =
                        "COMMAND: HEAVY UNIT DETECTED.";
                }

                if (g_kills == 10)
                {
                    g_message =
                        "MISSION OBJECTIVE: HOLD THE AREA.";
                }
            }
        }
    }
}

// ============================================================
// CREATE WORLD
// ============================================================

static void createWorld()
{
    g_objects.clear();

    // No perimeter walls: the map edges are open.

    // central buildings / cover
    g_objects.push_back({
        OBJECT_SOLID,
        0.0f,
        1.5f,
        -10.0f,
        5.0f,
        3.0f,
        5.0f,
        true
    });

    g_objects.push_back({
        OBJECT_SOLID,
        -11.0f,
        1.5f,
        9.0f,
        4.0f,
        3.0f,
        4.0f,
        true
    });

    g_objects.push_back({
        OBJECT_SOLID,
        11.0f,
        1.5f,
        9.0f,
        4.0f,
        3.0f,
        4.0f,
        true
    });

    // columns
    const float columns[][2] =
    {
        {-8,-8},
        { 8,-8},
        {-8, 8},
        { 8, 8},
        { 0,  0}
    };

    for (int i = 0; i < 5; ++i)
    {
        g_objects.push_back({
            OBJECT_SOLID,
            columns[i][0],
            1.5f,
            columns[i][1],
            2.0f,
            3.0f,
            2.0f,
            true
        });
    }

    // crates
    const float crates[][2] =
    {
        {-15,-12},
        {-12,-15},
        { 14,-13},
        { 16,-10},
        {-15, 13},
        { 14, 14},
        {  4, 12},
        { -4,-13}
    };

    for (int i = 0; i < 8; ++i)
    {
        g_objects.push_back({
            OBJECT_SOLID,
            crates[i][0],
            0.8f,
            crates[i][1],
            1.6f,
            1.6f,
            1.6f,
            true
        });
    }

    // trees
    const float trees[][2] =
    {
        {-25,-20},
        {-20,-25},
        {-25, 20},
        {-20, 25},

        { 25,-20},
        { 20,-25},
        { 25, 20},
        { 20, 25},

        {-27, 0},
        { 27, 0},
        { 0, -27},
        { 0, 27}
    };

    for (int i = 0; i < 12; ++i)
    {
        g_objects.push_back({
            OBJECT_TREE,
            trees[i][0],
            2.0f,
            trees[i][1],
            2.5f,
            4.0f,
            2.5f,
            true
        });
    }

    // ammo
    g_objects.push_back({
        OBJECT_AMMO,
        -4.0f,
        0.0f,
        7.0f,
        1.0f,
        0.8f,
        1.0f,
        true
    });

    g_objects.push_back({
        OBJECT_AMMO,
        12.0f,
        0.0f,
        -4.0f,
        1.0f,
        0.8f,
        1.0f,
        true
    });

    g_objects.push_back({
        OBJECT_AMMO,
        -14.0f,
        0.0f,
        -4.0f,
        1.0f,
        0.8f,
        1.0f,
        true
    });

    // medkits
    g_objects.push_back({
        OBJECT_MEDKIT,
        -5.0f,
        0.0f,
        -7.0f,
        1.0f,
        0.8f,
        1.0f,
        true
    });

    g_objects.push_back({
        OBJECT_MEDKIT,
        14.0f,
        0.0f,
        8.0f,
        1.0f,
        0.8f,
        1.0f,
        true
    });

    // Settlements: large enterable houses with door openings.
    // Each cluster has several buildings so the player can break line of sight
    // and move from house to house under cover.
    const float settlements[][2] = {
        {-150.0f,-120.0f}, {145.0f,-105.0f}, {-155.0f,115.0f},
        {150.0f,125.0f}, {0.0f,155.0f}
    };
    for (const auto& s : settlements)
    {
        addHouse(s[0], s[1], 0.0f);
        addHouse(s[0] + 20.0f, s[1] + 4.0f, 0.12f);
        addHouse(s[0] - 17.0f, s[1] + 17.0f, -0.10f);
    }

    // Separate city biome with 26 detailed skyscrapers and conservative AABB hitboxes.
    createCityBiome();

    // Far cover, rocks and outposts make the enlarged map visually and tactically interesting.
    const float farCover[][3] = {
        {-72.0f, 4.0f, -52.0f}, {-58.0f, 2.5f, 66.0f}, {64.0f, 5.0f, 54.0f},
        {82.0f, 4.5f, -42.0f}, {-104.0f, 3.5f, 20.0f}, {108.0f, 3.0f, -18.0f},
        {-38.0f, 2.0f, -86.0f}, {44.0f, 2.5f, -92.0f}, {18.0f, 3.0f, 102.0f},
        {-84.0f, 2.5f, 100.0f}, {94.0f, 2.0f, 96.0f}, {-122.0f, 3.5f, -76.0f}
    };
    for (const auto& c : farCover)
        g_objects.push_back({OBJECT_SOLID, c[0] * WORLD_SCALE, c[1] * 0.5f, c[2] * WORLD_SCALE, c[1], c[1], c[1], true});

    const float farTrees[][2] = {
        {-78.0f,-88.0f},{-62.0f,-106.0f},{-18.0f,92.0f},{-72.0f,116.0f},
        {76.0f,90.0f},{118.0f,58.0f},{112.0f,-88.0f},{-118.0f,78.0f}
    };
    for (const auto& t : farTrees)
        g_objects.push_back({OBJECT_TREE, t[0] * WORLD_SCALE, 2.0f, t[1] * WORLD_SCALE, 2.5f, 4.0f, 2.5f, true});

    // Remove surface obstacles that would occupy the tunnel bore.
    // This guarantees the tunnel and its physics never intersect with old
    // world props such as crates, rocks or trees.
    for (auto& o : g_objects)
    {
        if (o.type == OBJECT_SOLID || o.type == OBJECT_TREE)
        {
            if (tunnelConflictsObject(o))
                o.active = false;
        }
    }

    // Snap surviving world objects to the actual terrain surface.
    for (auto& o : g_objects)
    {
        const bool wall = fabsf(o.x) > ARENA - 0.01f || fabsf(o.z) > ARENA - 0.01f;
        if (!wall && o.active)
        {
            const float support = isTunnelZone(o.x,o.z)
                ? tunnelFloorHeight(o.x,o.z)
                : terrainSupportHeight(o.x, o.z, std::max(o.sx, o.sz) * 0.5f);
            o.y = support + o.sy * 0.5f;
        }
    }
    // Dense procedural vegetation and biome-specific decorations are generated
    // after collision objects are placed, so decorative generation can reject
    // tunnel mouths, steep slopes and the player spawn area.
    generateDecor();
}

// ============================================================
// ENEMY CREATION
// ============================================================

static void spawnEnemy(EnemyType type)
{
    float x;
    float z;

    if (!safeSpawn(x, z, 12.0f))
        return;

    Enemy e;

    e.x = x;
    e.z = z;
    e.y = terrainSupportHeight(x, z, 0.4f) + 0.9f;

    e.type = type;

    e.attackCooldown =
        randomFloat(0.5f, 2.0f);

    e.hitFlash = 0.0f;

    e.respawn = 0.0f;
    e.yaw = atan2f(g_px - e.x, e.z - g_pz);
    e.underground = false;
    e.world = g_world;
    configureWorldEnemyVariant(e);

    e.alive = true;

    if (type == ENEMY_SOLDIER)
    {
        e.maxHp = 100.0f;
        e.hp = e.maxHp;

        e.speed = 2.0f;

        e.radius = 0.45f;

        e.attackRange = 28.0f;

        e.damage = 8.0f;
    }
    else if (type == ENEMY_FAST)
    {
        e.maxHp = 65.0f;
        e.hp = e.maxHp;

        e.speed = 3.8f;

        e.radius = 0.35f;

        e.attackRange = 12.0f;

        e.damage = 15.0f;
    }
    else if (type == ENEMY_DRONE)
    {
        e.maxHp = 80.0f;
        e.hp = e.maxHp;
        e.speed = 5.0f;
        e.radius = 0.40f;
        e.attackRange = 55.0f;
        e.damage = 10.0f;
        e.y = tunnelFloorHeight(x, z) + 2.6f;
        e.underground = true;
    }
    else
    {
        e.maxHp = 300.0f;
        e.hp = e.maxHp;

        e.speed = 1.1f;

        e.radius = 0.75f;

        e.attackRange = 36.0f;

        e.damage = 18.0f;
    }

    g_enemies.push_back(e);
}

// ============================================================
// RESET GAME
// ============================================================

static bool safeDroneSpawn(float& outX, float& outZ)
{
    // Pick a random tunnel segment instead of a fixed handful of world points.
    // This makes respawning work across the entire underground network.
    for (int attempt = 0; attempt < 120; ++attempt)
    {
        const int seg = rand() % TUNNEL_COUNT;
        const auto& t = g_tunnels[seg];
        const float dx = t.bx - t.ax;
        const float dz = t.bz - t.az;
        const float len = sqrtf(dx*dx + dz*dz);
        if (len < 0.01f) continue;

        const float along = randomFloat(0.10f, 0.90f);
        const float side = randomFloat(-t.halfWidth * 0.62f, t.halfWidth * 0.62f);
        const float nx = -dz / len;
        const float nz =  dx / len;

        const float x = t.ax + dx * along + nx * side;
        const float z = t.az + dz * along + nz * side;

        if (!isTunnelZone(x, z)) continue;
        if (distance2D(x, z, g_px, g_pz) < 14.0f) continue;

        bool occupied = false;
        for (const auto& e : g_enemies)
        {
            if (e.alive && e.type == ENEMY_DRONE && distance2D(x,z,e.x,e.z) < 3.0f)
            {
                occupied = true;
                break;
            }
        }
        if (occupied) continue;

        outX = x;
        outZ = z;
        return true;
    }
    return false;
}

static void resetGame()
{
    g_px = 0.0f;
    g_pz = 20.0f;
    g_py = currentFloorHeight(g_px, g_pz);

    g_vy = 0.0f;

    g_onGround = true;
    g_inTunnel = false;

    g_health = 100.0f;

    g_deadTimer = 0.0f;

    g_score = 0;

    g_kills = 0;

    g_gameTime = 0.0f;
    g_yaw = 0.0f;
    g_pitch = 0.0f;
    g_damageFlash = 0.0f;
    g_weaponRecoil = 0.0f;
    g_muzzleFlash = 0.0f;
    g_hitMarker = 0.0f;
    g_firstMouse = true;

    g_storyStage = 0;

    g_storyTimer = 8.0f;

    g_message =
        "MISSION: SURVIVE AND ELIMINATE THE HOSTILES";

    g_currentWeapon =
        WEAPON_PISTOL;

    for (int i = 0; i < 3; ++i)
    {
        g_weapons[i].ammo =
            g_weapons[i].magazineSize;

        g_weapons[i].reloadTimer = 0.0f;

        g_weapons[i].cooldown = 0.0f;
    }

    g_projectiles.clear();

    // Recreate pickups and world objects on restart.
    createWorld();
    createWorldPortals();
    createOtherWorldObjects();
    g_enemies.clear();
    g_world = WORLD_MAIN;
    g_worldSeeded[WORLD_MAIN] = true;
    g_worldSeeded[WORLD_OCEAN] = false;
    g_worldSeeded[WORLD_WINTER] = false;

    // 10x the previous surface force: 320 ground enemies total.
    // Composition is kept close to the previous mix, but with a much larger
    // armored contingent so the map stays dangerous at long range.
    for (int i = 0; i < 140; ++i) spawnEnemy(ENEMY_SOLDIER);
    for (int i = 0; i < 80; ++i)  spawnEnemy(ENEMY_FAST);
    for (int i = 0; i < 100; ++i) spawnEnemy(ENEMY_TANK);

    // Put drones throughout every tunnel segment, not at a single fixed
    // location. Two drones per segment gives the whole underground network
    // a persistent threat and also gives them diverse respawn positions.
    for (int seg = 0; seg < TUNNEL_COUNT; ++seg)
    {
        for (int n = 0; n < 2; ++n)
        {
            float x = 0.0f, z = 0.0f;
            if (!safeDroneSpawn(x, z)) continue;
            Enemy d{};
            d.x=x; d.z=z; d.y=tunnelFloorHeight(x,z)+2.6f;
            d.hp=d.maxHp=80.0f; d.speed=5.0f; d.radius=0.40f;
            d.type=ENEMY_DRONE; d.attackCooldown=randomFloat(0.8f,2.0f);
            d.hitFlash=0.0f; d.respawn=0.0f; d.attackRange=55.0f; d.damage=10.0f;
            d.yaw=randomFloat(-3.14159f,3.14159f); d.underground=true; d.world=g_world; d.alive=true;
            g_enemies.push_back(d);
        }
    }
}

// ============================================================
// UPDATE PLAYER
// ============================================================

static void updatePlayer(float dt)
{
    if (g_deadTimer > 0.0f)
        return;

    float fx =
        sinf(g_yaw);

    float fz =
        -cosf(g_yaw);

    float rx =
        cosf(g_yaw);

    float rz =
        sinf(g_yaw);

    float mx = 0.0f;
    float mz = 0.0f;

    if (glfwGetKey(
            g_window,
            GLFW_KEY_W) == GLFW_PRESS)
    {
        mx += fx;
        mz += fz;
    }

    if (glfwGetKey(
            g_window,
            GLFW_KEY_S) == GLFW_PRESS)
    {
        mx -= fx;
        mz -= fz;
    }

    if (glfwGetKey(
            g_window,
            GLFW_KEY_D) == GLFW_PRESS)
    {
        mx += rx;
        mz += rz;
    }

    if (glfwGetKey(
            g_window,
            GLFW_KEY_A) == GLFW_PRESS)
    {
        mx -= rx;
        mz -= rz;
    }

    float len =
        sqrtf(mx*mx + mz*mz);

    if (len > 0.001f)
    {
        mx /= len;
        mz /= len;

        float speed = 6.0f;

        if (glfwGetKey(
                g_window,
                GLFW_KEY_LEFT_SHIFT)
            == GLFW_PRESS)
        {
            speed = 9.0f;
        }

        // Underground corridors are always three times faster, including
        // while sprinting.
        if (g_inTunnel && isTunnelZone(g_px, g_pz))
            speed *= 3.0f;

        float amount = speed * dt;

        auto tryMove = [&](float nx, float nz)
        {
            bool crossingPortal = false;
            float portalX = nx;
            float portalZ = nz;

            if (!tunnelTransitionAllowed(
                    g_px, g_pz, nx, nz,
                    crossingPortal, portalX, portalZ))
                return false;

            const bool oldTunnelState = g_inTunnel;
            const bool newTunnelState = crossingPortal ? !oldTunnelState : oldTunnelState;
            float finalX = crossingPortal ? portalX : nx;
            float finalZ = crossingPortal ? portalZ : nz;

            // Seamless world wrapping: crossing any edge enters the opposite
            // side. This is applied before collision testing at the destination.
            bool wrapped = false;
            if (finalX > ARENA) { finalX = -ARENA + 0.75f; wrapped = true; }
            else if (finalX < -ARENA) { finalX = ARENA - 0.75f; wrapped = true; }
            if (finalZ > ARENA) { finalZ = -ARENA + 0.75f; wrapped = true; }
            else if (finalZ < -ARENA) { finalZ = ARENA - 0.75f; wrapped = true; }

            // Portal crossing is a controlled state change. We validate the
            // destination only against the collision layer that is active
            // after the transition, which prevents false blocking at the mouth.
            g_inTunnel = wrapped ? false : newTunnelState;

            if (!crossingPortal && (playerBlocked(finalX, finalZ) ||
                (g_world != WORLD_MAIN && otherWorldObjectBlocked(finalX, finalZ, 0.55f))))
            {
                // Trees use a compact trunk collider.  If a frame starts
                // slightly inside one because of a large dt or a previous
                // collision, try a small lateral slide before giving up.
                bool recovered = false;
                const float dx = finalX - g_px;
                const float dz = finalZ - g_pz;
                const float len = sqrtf(dx * dx + dz * dz);
                if (len > 0.0001f)
                {
                    const float inv = 1.0f / len;
                    const float sideX = -dz * inv;
                    const float sideZ = dx * inv;
                    const float sideStep = 0.18f;
                    const float candidates[2][2] = {
                        {finalX + sideX * sideStep, finalZ + sideZ * sideStep},
                        {finalX - sideX * sideStep, finalZ - sideZ * sideStep}
                    };
                    for (const auto& c : candidates)
                    {
                        if (!playerBlocked(c[0], c[1]))
                        {
                            g_px = c[0];
                            g_pz = c[1];
                            recovered = true;
                            break;
                        }
                    }
                }

                if (!recovered)
                {
                    g_inTunnel = oldTunnelState;
                    return false;
                }
            }

            g_px = finalX;
            g_pz = finalZ;

            if (crossingPortal)
            {
                if (g_inTunnel)
                {
                    g_py = tunnelFloorHeight(g_px, g_pz);
                    g_py += 0.02f;
                }
                else
                {
                    g_py = terrainHeight(g_px, g_pz);
                    g_py += 0.02f;
                }
                g_vy = 0.0f;
                g_onGround = true;
            }

            return true;
        };

        tryMove(g_px + mx*amount, g_pz);
        tryMove(g_px, g_pz + mz*amount);
    }

    // jump
    static bool previousSpace = false;

    bool space =
        glfwGetKey(
            g_window,
            GLFW_KEY_SPACE)
        == GLFW_PRESS;

    if (space &&
        !previousSpace &&
        g_onGround)
    {
        g_vy = JUMP_SPEED;

        g_onGround = false;
    }

    previousSpace = space;

    // g_py is absolute world-space feet Y. While grounded we snap directly
    // to the active floor; when airborne we integrate gravity in world space.
    const float floorY = playerFloorHeight(g_px, g_pz);
    if (g_onGround)
    {
        g_py = floorY;
        g_vy = 0.0f;
    }
    else
    {
        g_vy -= GRAVITY * dt;
        g_py += g_vy * dt;

        // Landing: never allow the player's feet below the actual floor.
        if (g_py <= floorY)
        {
            g_py = floorY;
            g_vy = 0.0f;
            g_onGround = true;
        }
    }

    // Hard ceiling collision inside tunnels: the player can never jump
    // through the arched roof into the terrain above.
    if (g_inTunnel && isTunnelZone(g_px, g_pz))
    {
        const float ceiling = tunnelRoofHeight(g_px, g_pz) - 0.18f;
        if (g_py + PLAYER_HEIGHT > ceiling)
        {
            g_py = ceiling - PLAYER_HEIGHT;
            if (g_vy > 0.0f)
                g_vy = 0.0f;
        }
    }
}

static void spawnEnemyProjectile(const Enemy& e)
{
    const float enemyY = e.type == ENEMY_DRONE ? e.y :
        terrainSupportHeight(e.x, e.z, e.radius) +
        (e.type == ENEMY_TANK ? 2.2f : 1.55f);
    const float targetY = g_py + 1.20f;

    float dx = g_px - e.x;
    float dy = targetY - enemyY;
    float dz = g_pz - e.z;
    const float len = sqrtf(dx*dx + dy*dy + dz*dz);
    if (len < 0.001f)
        return;

    dx /= len;
    dy /= len;
    dz /= len;

    Projectile p{};
    p.x = e.x;
    p.y = enemyY;
    p.z = e.z;
    p.dx = dx;
    p.dy = dy;
    p.dz = dz;
    p.speed = e.type == ENEMY_TANK ? 18.0f :
              e.type == ENEMY_FAST ? 28.0f : 22.0f;
    p.damage = e.damage;
    p.active = true;
    g_projectiles.push_back(p);
}

static void updateEnemyProjectiles(float dt)
{
    const float playerY = g_py + 0.90f;

    for (auto& p : g_projectiles)
    {
        if (!p.active)
            continue;

        const float oldX = p.x;
        const float oldY = p.y;
        const float oldZ = p.z;

        const float step = p.speed * dt;
        p.x += p.dx * step;
        p.y += p.dy * step;
        p.z += p.dz * step;

        const bool inTunnel = isTunnelZone(p.x, p.z);
        const float floorY = currentFloorHeight(p.x,p.z);
        const float ceilingY = inTunnel
            ? tunnelRoofHeight(p.x,p.z)
            : terrainHeight(p.x,p.z) + 80.0f;
        if (p.x < -ARENA - 2.0f || p.x > ARENA + 2.0f ||
            p.z < -ARENA - 2.0f || p.z > ARENA + 2.0f ||
            p.y < floorY + 0.05f || p.y > ceilingY - 0.05f)
        {
            p.active = false;
            continue;
        }

        const float vx = p.x - oldX;
        const float vy = p.y - oldY;
        const float vz = p.z - oldZ;
        const float segLen = sqrtf(vx*vx + vy*vy + vz*vz);
        bool blocked = false;

        if (segLen > 0.0001f)
        {
            for (const auto& o : g_objects)
            {
                if (!o.active || o.type == OBJECT_AMMO || o.type == OBJECT_MEDKIT)
                    continue;

                float hitT = 0.0f;
                if (rayAABB(oldX, oldY, oldZ,
                            vx / segLen, vy / segLen, vz / segLen,
                            o.x, o.y, o.z,
                            o.sx * 0.5f + 0.06f,
                            o.sy * 0.5f + 0.06f,
                            o.sz * 0.5f + 0.06f, hitT) &&
                    hitT <= segLen)
                {
                    blocked = true;
                    break;
                }
            }
        }

        if (!blocked && tunnelWallHitSegment(oldX,oldY,oldZ,p.x,p.y,p.z))
            blocked = true;

        if (blocked)
        {
            p.active = false;
            continue;
        }

        if (g_deadTimer <= 0.0f &&
            segmentSphereHit(oldX, oldY, oldZ, p.x, p.y, p.z,
                             g_px, playerY, g_pz, 0.55f))
        {
            damagePlayer(p.damage);
            p.active = false;
        }
    }

    g_projectiles.erase(
        std::remove_if(g_projectiles.begin(), g_projectiles.end(),
                       [](const Projectile& p) { return !p.active; }),
        g_projectiles.end());
}

// ============================================================
// UPDATE ENEMIES
// ============================================================

static void updateEnemies(float dt)
{
    for (auto& e : g_enemies)
    {
        if (e.world != g_world)
            continue;

        if (e.hitFlash > 0.0f)
        {
            e.hitFlash -= dt * 5.0f;

            if (e.hitFlash < 0.0f)
                e.hitFlash = 0.0f;
        }

        if (!e.alive)
        {
            e.respawn -= dt;

            if (e.respawn <= 0.0f)
            {
                float x;
                float z;

                bool ok = e.type == ENEMY_DRONE ? safeDroneSpawn(x,z) : safeSpawn(x,z,12.0f);
                if (ok)
                {
                    e.x = x;
                    e.z = z;
                    e.y = e.type == ENEMY_DRONE ? tunnelFloorHeight(x, z) + 2.6f : terrainSupportHeight(x, z, e.radius);

                    e.alive = true;

                    e.hp =
                        e.maxHp;

                    e.hitFlash = 0.0f;

                    e.attackCooldown =
                        randomFloat(0.5f, 2.0f);
                }
            }

            continue;
        }

        float dx =
            g_px - e.x;

        float dz =
            g_pz - e.z;

        float dist =
            sqrtf(dx*dx + dz*dz);

        // Enemy models face local -Z, so this is the yaw that points -Z
        // directly from the enemy toward the player.
        const float desiredYaw = atan2f(g_px - e.x, e.z - g_pz);
        const float turnRate = (e.type == ENEMY_TANK ? 2.5f : 5.5f);
        e.yaw = turnToward(e.yaw, desiredYaw, turnRate * dt);

        updateOtherWorldEnemyBehavior(e, dt);

        if (dist < 0.001f)
            dist = 0.001f;

        dx /= dist;
        dz /= dist;

        // underground drone enemy
        if (e.type == ENEMY_DRONE)
        {
            if (!isTunnelZone(g_px,g_pz))
                continue;
            const float targetY=tunnelFloorHeight(g_px,g_pz)+2.2f;
            const float ddx=g_px-e.x, ddz=g_pz-e.z;
            const float dist3=sqrtf(ddx*ddx+ddz*ddz);
            if(dist3>6.0f)
            {
                const float inv=1.0f/std::max(dist3,0.001f);
                const float step=e.speed*dt;
                const float nx=e.x+ddx*inv*step;
                const float nz=e.z+ddz*inv*step;
                if(isTunnelZone(nx,e.z)) e.x=nx;
                if(isTunnelZone(e.x,nz)) e.z=nz;
            }
            e.y=clampf(e.y+(targetY-e.y)*clampf(dt*3.0f,0.0f,1.0f),
                       tunnelFloorHeight(e.x,e.z)+1.1f,
                       tunnelRoofHeight(e.x,e.z)-1.0f);
            e.yaw=turnToward(e.yaw, atan2f(g_px-e.x, e.z-g_pz), 6.0f*dt);
            e.attackCooldown-=dt;
            if(e.attackCooldown<=0.0f && dist3<e.attackRange && enemyHasLineOfSight(e))
            { spawnEnemyProjectile(e); e.attackCooldown=1.25f; }
            continue;
        }

        // Fast enemy: move in the direction the model is facing. This makes
        // turning an actual part of locomotion rather than only a visual yaw.
        if (e.type == ENEMY_FAST)
        {
            if (dist > 1.6f)
            {
                const float step = e.speed * dt;
                const float fx = sinf(e.yaw);
                const float fz = -cosf(e.yaw);
                float nx = e.x + fx * step;
                float nz = e.z + fz * step;

                if (!enemyBlocked(nx, nz, e.radius))
                {
                    e.x = nx; e.z = nz;
                }
                else
                {
                    // Turn around obstacles instead of sliding sideways.
                    const float side = (rand() & 1) ? 1.0f : -1.0f;
                    e.yaw = normalizeAngle(e.yaw + side * 1.15f * dt);
                }
            }

            e.attackCooldown -= dt;

            if (dist < e.attackRange &&
                e.attackCooldown <= 0.0f &&
                enemyHasLineOfSight(e))
            {
                spawnEnemyProjectile(e);
                e.attackCooldown = 1.8f;
            }

            continue;
        }

        // Normal soldiers and tanks also drive along their current heading.
        // Tanks turn more slowly, which makes their large hulls feel heavier.
        if (dist > 7.0f)
        {
            const float step = e.speed * dt;
            const float fx = sinf(e.yaw);
            const float fz = -cosf(e.yaw);
            const float nx = e.x + fx * step;
            const float nz = e.z + fz * step;

            if (!enemyBlocked(nx, nz, e.radius))
            {
                e.x = nx;
                e.z = nz;
            }
            else
            {
                const float side = (rand() & 1) ? 1.0f : -1.0f;
                e.yaw = normalizeAngle(e.yaw + side * (e.type == ENEMY_TANK ? 0.75f : 1.45f) * dt);
            }
        }

        e.attackCooldown -= dt;

        if (e.attackCooldown <= 0.0f &&
            dist < e.attackRange &&
            enemyHasLineOfSight(e))
        {
            spawnEnemyProjectile(e);
            e.attackCooldown =
                e.type == ENEMY_TANK ?
                2.8f :
                1.8f;
        }
    }
}

// ============================================================
// UPDATE PICKUPS
// ============================================================

static void updatePickups()
{
    for (auto& o : g_objects)
    {
        if (!o.active)
            continue;

        if (o.type != OBJECT_AMMO &&
            o.type != OBJECT_MEDKIT)
            continue;

        float d =
            distance2D(
                g_px,
                g_pz,
                o.x,
                o.z
            );

        if (d > 1.5f)
            continue;

        if (o.type == OBJECT_AMMO)
        {
            for (int i = 0; i < 3; ++i)
            {
                g_weapons[i].reserve +=
                    i == WEAPON_SHOTGUN ?
                    8 :
                    30;
            }

            g_message =
                "AMMO PICKED UP";

            o.active = false;
        }
        else
        {
            if (g_health < g_maxHealth)
            {
                g_health =
                    clampf(
                        g_health + 35.0f,
                        0.0f,
                        g_maxHealth
                    );

                g_message =
                    "MEDKIT +35 HP";

                o.active = false;
            }
        }
    }
}

// ============================================================
// UPDATE
// ============================================================

static void updateGame(float dt)
{
    g_gameTime += dt;

    if (g_damageFlash > 0.0f)
    {
        g_damageFlash -= dt * 2.5f;

        if (g_damageFlash < 0.0f)
            g_damageFlash = 0.0f;
    }

    if (g_hitMarker > 0.0f)
    {
        g_hitMarker -= dt * 5.0f;

        if (g_hitMarker < 0.0f)
            g_hitMarker = 0.0f;
    }

    if (g_muzzleFlash > 0.0f)
    {
        g_muzzleFlash -= dt * 12.0f;

        if (g_muzzleFlash < 0.0f)
            g_muzzleFlash = 0.0f;
    }

    if (g_weaponRecoil > 0.0f)
    {
        g_weaponRecoil -= dt * 7.0f;

        if (g_weaponRecoil < 0.0f)
            g_weaponRecoil = 0.0f;
    }

    for (int i = 0; i < 3; ++i)
    {
        if (g_weapons[i].cooldown > 0.0f)
        {
            g_weapons[i].cooldown -= dt;

            if (g_weapons[i].cooldown < 0.0f)
                g_weapons[i].cooldown = 0.0f;
        }
    }

    updateReload(dt);

    if (g_deadTimer > 0.0f)
    {
        g_deadTimer -= dt;

        static bool prevEnter = false;
        const bool enter =
            glfwGetKey(
                g_window,
                GLFW_KEY_ENTER) == GLFW_PRESS;

        if (enter && !prevEnter)
        {
            g_deadTimer = 0.0f;
            respawnPlayer();
            prevEnter = enter;
            return;
        }

        prevEnter = enter;

        if (g_deadTimer <= 0.0f)
        {
            g_deadTimer = 0.0f;
            respawnPlayer();
        }

        return;
    }

    updatePlayer(dt);
    updateWorldPortals();

    updateEnemies(dt);
    updateEnemyProjectiles(dt);

    updatePickups();

    // shooting
    if (glfwGetMouseButton(
            g_window,
            GLFW_MOUSE_BUTTON_LEFT)
        == GLFW_PRESS)
    {
        shoot();
    }

    // weapon switching
    if (glfwGetKey(
            g_window,
            GLFW_KEY_1)
        == GLFW_PRESS)
    {
        switchWeapon(
            WEAPON_PISTOL
        );
    }

    if (glfwGetKey(
            g_window,
            GLFW_KEY_2)
        == GLFW_PRESS)
    {
        switchWeapon(
            WEAPON_RIFLE
        );
    }

    if (glfwGetKey(
            g_window,
            GLFW_KEY_3)
        == GLFW_PRESS)
    {
        switchWeapon(
            WEAPON_SHOTGUN
        );
    }

    static bool prevR = false;

    bool r =
        glfwGetKey(
            g_window,
            GLFW_KEY_R)
        == GLFW_PRESS;

    if (r && !prevR)
        reloadWeapon();

    prevR = r;

    // story
    g_storyTimer -= dt;

    if (g_storyTimer <= 0.0f)
    {
        g_storyStage++;

        g_storyTimer = 18.0f;

        if (g_storyStage == 1)
        {
            g_message =
                "COMMAND: HOSTILE REINFORCEMENTS DETECTED.";
        }
        else if (g_storyStage == 2)
        {
            g_message =
                "COMMAND: FIND THE SUPPLY CRATES.";
        }
        else if (g_storyStage == 3)
        {
            g_message =
                "COMMAND: HEAVY UNIT APPROACHING.";
        }
        else if (g_storyStage == 4)
        {
            g_message =
                "COMMAND: HOLD THE POSITION.";
        }
    }
}

// ============================================================
// SKY
// ============================================================

static void drawSky()
{
    glDisable(GL_DEPTH_TEST);
    const float day=terrainDayFactor();
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    const float hR=0.012f+0.30f*day, hG=0.020f+0.50f*day, hB=0.060f+0.86f*day;
    const float tR=0.002f+0.04f*day, tG=0.004f+0.16f*day, tB=0.018f+0.60f*day;
    glBegin(GL_QUADS);
    glColor3f(hR,hG,hB); glVertex2f(-1,-1); glVertex2f(1,-1);
    glColor3f(tR,tG,tB); glVertex2f(1,1); glVertex2f(-1,1);
    glEnd();
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

static void drawWorldStarField()
{
    const float day = terrainDayFactor();
    if (day > 0.5f)
        return;

    // World-space stars: their positions are fixed in the world, not in
    // screen space, so turning or moving the player does not drag the stars.
    const float radius = 900.0f;
    unsigned int seed = 0x7A1F39D1u;
    glDisable(GL_DEPTH_TEST);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 420; ++i)
    {
        seed = 1664525u * seed + 1013904223u;
        const float u = (seed & 0xFFFFu) / 65535.0f;
        seed = 1664525u * seed + 1013904223u;
        const float v = (seed & 0xFFFFu) / 65535.0f;
        seed = 1664525u * seed + 1013904223u;
        const float brightness = 0.45f + 0.55f * ((seed & 0xFFFFu) / 65535.0f);

        // Restrict stars to the upper hemisphere with a small band around
        // the horizon so mountains can still silhouette against them.
        const float theta = 2.0f * (float)M_PI * u;
        const float y = 0.08f + 0.92f * v;
        const float r = sqrtf(std::max(0.0f, 1.0f - y*y));
        const float sx = cosf(theta) * r * radius;
        const float sy = y * radius;
        const float sz = sinf(theta) * r * radius;

        const float c = brightness * (0.70f + 0.30f * (1.0f - day));
        glColor3f(0.70f*c, 0.78f*c, 1.00f*c);
        glVertex3f(sx, sy, sz);
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
}


// ============================================================
// DRAW WORLD
// ============================================================

static void createWorldPortals()
{
    g_worldPortals.clear();

    // Main world: spread portals across distant, open sectors. None is placed
    // on the city footprint, settlements, or tunnel centerlines.
    g_worldPortals.push_back({-520.0f, -470.0f, WORLD_OCEAN, 0.0f, 0.0f, 4.2f});
    g_worldPortals.push_back({ 500.0f, -455.0f, WORLD_OCEAN, 0.0f, 0.0f, 4.2f});
    g_worldPortals.push_back({-500.0f,  465.0f, WORLD_WINTER, 0.0f, 0.0f, 4.2f});
    g_worldPortals.push_back({ 475.0f,  470.0f, WORLD_WINTER, 0.0f, 0.0f, 4.2f});
    g_worldPortals.push_back({ 535.0f, -115.0f, WORLD_MAIN, CITY_CENTER_X, CITY_CENTER_Z, 4.2f});
    g_worldPortals.push_back({-535.0f,  115.0f, WORLD_MAIN, CITY_CENTER_X, CITY_CENTER_Z, 4.2f});
}

static const char* worldName(int w)
{
    if (w == WORLD_OCEAN) return "OCEAN WORLD";
    if (w == WORLD_WINTER) return "WINTER WORLD";
    return "MAIN WORLD";
}

static void drawWorldPortal(float x, float z, int targetWorld, bool returnPortal)
{
    const float ground = terrainHeight(x,z);
    const float spin = g_gameTime * 75.0f;
    glPushMatrix();
    glTranslatef(x, ground + 0.08f, z);

    // Four floating energy segments make the portal visible from a distance.
    const float glowR = returnPortal ? 0.95f : (targetWorld == WORLD_OCEAN ? 0.08f : 0.78f);
    const float glowG = returnPortal ? 0.16f : (targetWorld == WORLD_OCEAN ? 0.55f : 0.92f);
    const float glowB = returnPortal ? 0.92f : (targetWorld == WORLD_OCEAN ? 0.98f : 1.00f);

    glRotatef(spin,0,1,0);
    for (int i=0;i<4;++i)
    {
        const float a = i*(float)M_PI*0.5f;
        glPushMatrix();
        glTranslatef(cosf(a)*2.1f, 1.8f + 0.15f*sinf(g_gameTime*3.0f+i), sinf(a)*2.1f);
        glRotatef(-a*180.0f/(float)M_PI,0,1,0);
        drawBox(0,0,0,0.34f,3.5f,0.55f,glowR,glowG,glowB);
        glPopMatrix();
    }
    drawBox(0,1.8f,0,3.2f,0.08f,3.2f,glowR*0.45f,glowG*0.45f,glowB*0.45f);
    glPopMatrix();
}

static int nearestWorldPortal(float x, float z, float& outDist)
{
    int best = -1;
    outDist = 1e30f;
    if (g_world == WORLD_MAIN)
    {
        for (int i=0;i<(int)g_worldPortals.size();++i)
        {
            const auto& p=g_worldPortals[i];
            const float d=distance2D(x,z,p.x,p.z);
            if (d<outDist) { outDist=d; best=i; }
        }
    }
    return best;
}


static void configureWorldEnemyVariant(Enemy& e)
{
    e.variant = 0;
    if (e.world == WORLD_OCEAN)
    {
        // Aquatic enemies: faster strafing and periodic lateral drift.
        e.variant = (e.type == ENEMY_TANK ? 2 : 1);
    }
    else if (e.world == WORLD_WINTER)
    {
        // Frost enemies: slower, but stronger pursuit/ambush behavior.
        e.variant = (e.type == ENEMY_TANK ? 4 : 3);
    }
}

static void updateOtherWorldEnemyBehavior(Enemy& e, float dt)
{
    if (e.world == WORLD_OCEAN && e.variant == 1)
    {
        // Amphibious raiders weave while approaching the player.
        const float side = sinf(g_gameTime*2.4f + e.x*0.03f) * 0.65f;
        e.x += cosf(e.yaw) * side * dt;
        e.z -= sinf(e.yaw) * side * dt;
    }
    else if (e.world == WORLD_WINTER && e.variant == 3)
    {
        // Frost stalkers make short bursts, creating a different feel.
        const float burst = (fmodf(g_gameTime + e.x*0.01f, 2.8f) < 0.45f) ? 1.8f : 0.75f;
        e.x += cosf(e.yaw) * burst * dt;
        e.z -= sinf(e.yaw) * burst * dt;
    }
}

static void seedWorldEnemies(int world)
{
    if (world == WORLD_MAIN || g_worldSeeded[world])
        return;

    const int oldWorld = g_world;
    g_world = world;

    // Other worlds have their own hostile population.
    if (world == WORLD_OCEAN)
    {
        for (int i=0;i<48;++i) spawnEnemy(ENEMY_FAST);
        for (int i=0;i<18;++i) spawnEnemy(ENEMY_SOLDIER);
        for (int i=0;i<8;++i) spawnEnemy(ENEMY_TANK);
    }
    else if (world == WORLD_WINTER)
    {
        for (int i=0;i<28;++i) spawnEnemy(ENEMY_FAST);
        for (int i=0;i<32;++i) spawnEnemy(ENEMY_SOLDIER);
        for (int i=0;i<14;++i) spawnEnemy(ENEMY_TANK);
    }

    g_worldSeeded[world] = true;
    g_world = oldWorld;
}

static void enterWorldPortal(const WorldPortal& p)
{
    g_world = p.targetWorld;
    g_inTunnel = false;

    float dx = p.destX, dz = p.destZ;
    if (g_world == WORLD_MAIN)
    {
        // City destination gets a small offset from the central plaza.
        if (distance2D(dx,dz,CITY_CENTER_X,CITY_CENTER_Z) < 1.0f)
        {
            dx += 22.0f;
            dz += 22.0f;
        }
    }

    g_px = dx;
    g_pz = dz;
    if (g_world == WORLD_OCEAN && distance2D(g_px,g_pz,0,0) > 20.0f)
    {
        g_px=0.0f; g_pz=0.0f;
    }

    g_py = terrainHeight(g_px,g_pz) + 0.03f;
    g_vy = 0.0f;
    g_onGround = true;
    seedWorldEnemies(g_world);

    g_message = std::string("PORTAL: ENTERED ") + worldName(g_world);
}

static void updateWorldPortals()
{
    bool use = glfwGetKey(g_window, GLFW_KEY_E) == GLFW_PRESS;
    if (use && !g_previousUsePortalKey)
    {
        if (g_world != WORLD_MAIN)
        {
            WorldPortal back{0.0f,0.0f,WORLD_MAIN,0.0f,0.0f,4.2f};
            if (distance2D(g_px,g_pz,0.0f,0.0f) <= back.radius + 1.2f)
                enterWorldPortal(back);
        }
        else
        {
            float d=0.0f;
            const int idx=nearestWorldPortal(g_px,g_pz,d);
            if (idx>=0 && d <= g_worldPortals[idx].radius + 1.2f)
                enterWorldPortal(g_worldPortals[idx]);
        }
    }
    g_previousUsePortalKey = use;
}


static void drawPortalMouth(const TunnelSegment& t, bool start)
{
    const float dx = t.bx - t.ax;
    const float dz = t.bz - t.az;
    const float len = sqrtf(dx*dx + dz*dz);
    if (len < 0.001f)
        return;

    const float dirX = dx / len;
    const float dirZ = dz / len;
    const float outwardX = start ? -dirX : dirX;
    const float outwardZ = start ? -dirZ : dirZ;
    const float px = start ? t.ax : t.bx;
    const float pz = start ? t.az : t.bz;
    const float ground = terrainHeight(px, pz);
    const float side = std::max(3.8f, t.halfWidth - 0.35f);
    const float wallH = 2.15f;
    const float roofR = side;

    glPushMatrix();
    glTranslatef(px + outwardX*0.08f, ground + 0.04f, pz + outwardZ*0.08f);
    glRotatef(atan2f(dirX, -dirZ) * 180.0f / (float)M_PI + (start ? 180.0f : 0.0f), 0.0f, 1.0f, 0.0f);

    // Dark arch-shaped mouth. It is a real depth-tested surface, so foreground
    // trees, crates, terrain and other obstacles correctly occlude it instead
    // of allowing the black entrance to show through geometry.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glColor3f(0.015f, 0.003f, 0.02f);
    const int mouthRows = 18;
    const float mouthTop = wallH + roofR;
    glBegin(GL_QUADS);
    for (int row = 0; row < mouthRows; ++row)
    {
        const float y0 = mouthTop * (float)row / (float)mouthRows;
        const float y1 = mouthTop * (float)(row + 1) / (float)mouthRows;
        float hw0 = side;
        float hw1 = side;
        if (y0 > wallH)
        {
            const float q = clampf((y0 - wallH) / roofR, 0.0f, 1.0f);
            hw0 = roofR * sqrtf(std::max(0.0f, 1.0f - q*q));
        }
        if (y1 > wallH)
        {
            const float q = clampf((y1 - wallH) / roofR, 0.0f, 1.0f);
            hw1 = roofR * sqrtf(std::max(0.0f, 1.0f - q*q));
        }
        glVertex3f(-hw0, y0, 0.0f);
        glVertex3f( hw0, y0, 0.0f);
        glVertex3f( hw1, y1, 0.0f);
        glVertex3f(-hw1, y1, 0.0f);
    }
    glEnd();

    // Pink arched frame.

    glColor3f(0.98f, 0.04f, 0.68f);
    for (int i=0; i<16; ++i)
    {
        const float a0=(float)i/16.0f*(float)M_PI;
        const float a1=(float)(i+1)/16.0f*(float)M_PI;
        const float x0=cosf(a0)*side;
        const float y0=wallH+sinf(a0)*roofR;
        const float x1=cosf(a1)*side;
        const float y1=wallH+sinf(a1)*roofR;
        drawBox(0.5f*(x0+x1),0.5f*(y0+y1),0.0f,
                0.22f,std::max(0.18f,sqrtf((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0))+0.05f),0.52f,
                0.98f,0.04f,0.68f);
    }
    drawBox(-side,wallH*0.5f,0.0f,0.52f,wallH,0.52f,0.98f,0.04f,0.68f);
    drawBox(side,wallH*0.5f,0.0f,0.52f,wallH,0.52f,0.98f,0.04f,0.68f);
    glPopMatrix();
}


static void drawCityRoads()
{
    // Broad asphalt avenues and a central boulevard create a readable urban
    // grid. They are visual-only; building collision remains independent.
    const float span = CITY_RADIUS * 1.72f;
    const float ground = terrainHeight(CITY_CENTER_X, CITY_CENTER_Z) + 0.035f;
    for (int i=-5; i<=5; ++i)
    {
        const float offset = i * 25.0f;
        drawBox(CITY_CENTER_X + offset, ground, CITY_CENTER_Z,
                CITY_ROAD_HALF*2.0f, 0.07f, span, 0.055f,0.062f,0.070f);
        drawBox(CITY_CENTER_X, ground+0.005f, CITY_CENTER_Z + offset,
                span, 0.07f, CITY_ROAD_HALF*2.0f, 0.055f,0.062f,0.070f);
    }

    // Median strips / sidewalks.
    for (int i=-5; i<=5; ++i)
    {
        const float offset = i * 25.0f;
        drawBox(CITY_CENTER_X + offset + 5.0f, ground+0.045f, CITY_CENTER_Z,
                1.4f,0.10f,span, 0.34f,0.35f,0.37f);
        drawBox(CITY_CENTER_X, ground+0.045f, CITY_CENTER_Z + offset + 5.0f,
                span,0.10f,1.4f, 0.34f,0.35f,0.37f);
    }

    // Central civic plaza.
    drawBox(CITY_CENTER_X, ground+0.07f, CITY_CENTER_Z, 24.0f,0.14f,24.0f,
            0.28f,0.30f,0.33f);
    drawBox(CITY_CENTER_X, ground+0.14f, CITY_CENTER_Z, 11.0f,0.05f,11.0f,
            0.16f,0.21f,0.25f);
}

static void drawSkyscraper(const WorldObject& o)
{
    const float w=o.sx, d=o.sz, h=o.sy;
    const float baseY=o.y-h*0.5f;
    const int style = static_cast<int>(o.x*17.0f + o.z*31.0f) & 5;
    float br=0.16f, bg=0.20f, bb=0.25f;
    if (style==1) { br=0.10f; bg=0.17f; bb=0.24f; }
    if (style==2) { br=0.24f; bg=0.25f; bb=0.27f; }
    if (style==3) { br=0.13f; bg=0.23f; bb=0.29f; }
    if (style==4) { br=0.22f; bg=0.18f; bb=0.20f; }

    // Foundation, main tower, and stepped crown.
    drawBox(o.x, baseY+0.75f, o.z, w+2.2f, 1.5f, d+2.2f, 0.20f,0.22f,0.25f);
    drawBox(o.x, o.y, o.z, w, h, d, br,bg,bb);
    drawBox(o.x, baseY+h*0.88f, o.z, w*0.72f, h*0.18f, d*0.72f,
            br*1.12f,bg*1.12f,bb*1.12f);

    // Vertical glass ribbons.
    const float ribbon = 0.34f;
    drawBox(o.x-w*0.28f, o.y, o.z+d*0.505f, ribbon, h*0.86f, 0.06f, 0.05f,0.43f,0.65f);
    drawBox(o.x+w*0.28f, o.y, o.z+d*0.505f, ribbon, h*0.86f, 0.06f, 0.05f,0.43f,0.65f);
    drawBox(o.x-w*0.505f, o.y, o.z-d*0.18f, 0.06f, h*0.82f, ribbon, 0.05f,0.38f,0.58f);

    // Repeated window bands. The spacing is coarse enough to remain cheap,
    // but dense enough to sell a modern skyscraper silhouette.
    const int floors=std::max(5, (int)(h/4.0f));
    for(int f=1; f<floors; ++f)
    {
        const float yy=baseY + 1.7f + f*(h-3.0f)/floors;
        const float bandH=0.72f;
        const float frontY=o.z+d*0.511f;
        const float backY=o.z-d*0.511f;
        drawBox(o.x,yy,frontY,w*0.78f,bandH,0.045f,0.06f,0.30f,0.47f);
        drawBox(o.x,yy,backY,w*0.78f,bandH,0.045f,0.04f,0.20f,0.32f);
        drawBox(o.x-w*0.511f,yy,o.z,w*0.045f,bandH,d*0.72f,0.05f,0.27f,0.42f);
        drawBox(o.x+w*0.511f,yy,o.z,w*0.045f,bandH,d*0.72f,0.06f,0.34f,0.50f);
    }

    // Rooftop antenna / crown details.
    drawBox(o.x, baseY+h+1.0f, o.z, 0.34f,2.0f,0.34f,0.55f,0.57f,0.62f);
    drawBox(o.x, baseY+h+2.0f, o.z, 1.3f,0.22f,1.3f,0.08f,0.28f,0.40f);
}

static void addSkyscraper(float x, float z, float w, float d, float h)
{
    const float y=terrainHeight(x,z)+h*0.5f;
    // A single, conservative AABB is used for physics, so enemies and the
    // player never get snagged by decorative ledges or window geometry.
    g_objects.push_back({OBJECT_SKYSCRAPER,x,y,z,w,h,d,true});
}

static void createCityBiome()
{
    // 26 towers: three height tiers form a skyline with a clear central hero
    // tower and enough spacing for infantry/tank combat between buildings.
    const float P[][5] = {
        {-100,-75,18,18,42},{-50,-75,22,18,58},{0,-75,20,20,76},{50,-75,22,18,52},{100,-75,18,18,38},
        {-100,-25,24,20,64},{-50,-25,18,22,48},{0,-25,24,22,102},{50,-25,20,20,55},{100,-25,22,18,70},
        {-100,25,18,24,46},{-50,25,24,18,74},{50,25,24,20,62},{100,25,18,22,50},
        {-100,75,22,20,54},{-50,75,20,20,66},{0,75,22,18,88},{50,75,20,22,58},{100,75,24,20,72},
        {-75,115,18,18,40},{-25,115,22,18,57},{25,115,18,20,46},{75,115,22,18,64},
        {-125,35,16,18,36},{125,-10,18,20,44},{125,80,20,18,60}
    };
    for(const auto& p:P) addSkyscraper(CITY_CENTER_X+p[0],CITY_CENTER_Z+p[1],p[2],p[3],p[4]);
}

static void drawDecor()
{
    for (const auto& d : g_decor)
    {
        const float dx=d.x-g_px, dz=d.z-g_pz;
        if (dx*dx+dz*dz > OBJECT_RENDER_DISTANCE*OBJECT_RENDER_DISTANCE) continue;
        glPushMatrix();
        glTranslatef(d.x,d.y,d.z);
        glRotatef(d.rotation*180.0f/(float)M_PI,0,1,0);
        const float s=d.scale;
        switch(d.type)
        {
            case DECOR_GRASS:
                glBegin(GL_TRIANGLES);
                glColor3f(0.10f,0.48f,0.08f); glVertex3f(-0.04f,0,0); glVertex3f(0.0f,s*0.95f,0); glVertex3f(0.05f,0,0);
                glColor3f(0.14f,0.62f,0.10f); glVertex3f(0,0,-0.05f); glVertex3f(0.02f,s,0); glVertex3f(0.07f,0,0.05f);
                glEnd();
                break;
            case DECOR_BUSH:
                drawBox(0,s*0.35f,0,s*0.9f,s*0.7f,s*0.9f,0.08f,0.34f,0.10f);
                break;
            case DECOR_REED:
                for(int k=-1;k<=1;++k) drawBox(k*0.10f,s*0.55f,0.0f,0.07f,s*1.1f,0.07f,0.12f,0.48f,0.18f);
                break;
            case DECOR_FLOWER:
                drawBox(0,s*0.42f,0,0.04f,s*0.84f,0.04f,0.10f,0.42f,0.06f);
                drawBox(0,s*0.92f,0,0.20f,0.10f,0.20f,0.92f,0.18f,0.72f);
                break;
            case DECOR_ROCK:
                drawBox(0,s*0.28f,0,s*0.8f,s*0.55f,s*0.7f,0.30f,0.32f,0.34f);
                break;
            case DECOR_CACTUS:
                drawBox(0,s*0.75f,0,0.28f,s*1.5f,0.28f,0.18f,0.42f,0.12f);
                drawBox(-s*0.22f,s*0.82f,0,0.18f,s*0.55f,0.18f,0.18f,0.42f,0.12f);
                break;
            case DECOR_CRYSTAL:
                drawBox(0,s*0.55f,0,0.30f,s*1.1f,0.30f,0.18f,0.82f,0.94f);
                drawBox(0.22f*s,s*0.40f,0.06f,0.18f,s*0.8f,0.18f,0.64f,0.12f,0.92f);
                break;
        }
        glPopMatrix();
    }
}

static void drawWorld()
{
    const int GRID = 144;
    const float step = (ARENA * 2.0f) / GRID;
    const float day = terrainDayFactor();

    for (int ix = 0; ix < GRID; ++ix)
    {
        for (int iz = 0; iz < GRID; ++iz)
        {
            const float x0 = -ARENA + ix * step;
            const float x1 = x0 + step;
            const float z0 = -ARENA + iz * step;
            const float z1 = z0 + step;
            const float cx = 0.5f * (x0 + x1);
            const float cz = 0.5f * (z0 + z1);
            const float ddx = cx - g_px;
            const float ddz = cz - g_pz;
            if (ddx * ddx + ddz * ddz > TERRAIN_RENDER_DISTANCE * TERRAIN_RENDER_DISTANCE)
                continue;
            const float y00 = terrainHeight(x0, z0);
            const float y10 = terrainHeight(x1, z0);
            const float y11 = terrainHeight(x1, z1);
            const float y01 = terrainHeight(x0, z1);
            const float light00 = clampf((0.35f + 0.65f * day) * terrainColorScale(y00), 0.22f, 1.05f);
            const float light10 = clampf((0.35f + 0.65f * day) * terrainColorScale(y10), 0.22f, 1.05f);
            const float light11 = clampf((0.35f + 0.65f * day) * terrainColorScale(y11), 0.22f, 1.05f);
            const float light01 = clampf((0.35f + 0.65f * day) * terrainColorScale(y01), 0.22f, 1.05f);

            float r00,g00,b00, r10,g10,b10, r11,g11,b11, r01,g01,b01;
            terrainBaseColor(x0,z0,y00,r00,g00,b00);
            terrainBaseColor(x1,z0,y10,r10,g10,b10);
            terrainBaseColor(x1,z1,y11,r11,g11,b11);
            terrainBaseColor(x0,z1,y01,r01,g01,b01);

            glBegin(GL_QUADS);
            glColor3f(r00*light00,g00*light00,b00*light00); glVertex3f(x0,y00,z0);
            glColor3f(r10*light10,g10*light10,b10*light10); glVertex3f(x1,y10,z0);
            glColor3f(r11*light11,g11*light11,b11*light11); glVertex3f(x1,y11,z1);
            glColor3f(r01*light01,g01*light01,b01*light01); glVertex3f(x0,y01,z1);
            glEnd();
        }
    }

    // Inter-world portals are independent of ordinary collision objects, so
    // they cannot conflict with buildings, trees, pickups or tunnel physics.
    for (const auto& o : g_otherWorldObjects)
    {
        const float dx = o.x - g_px, dz = o.z - g_pz;
        if (o.world == g_world && dx*dx + dz*dz < OBJECT_RENDER_DISTANCE*OBJECT_RENDER_DISTANCE)
            drawOtherWorldObject(o);
    }

    if (g_world == WORLD_OCEAN)
    {
        // Calm ocean sheet below the island terrain. It is visual-only; the
        // terrain function supplies the walkable surface above it.
        const float waterY = -1.32f;
        const float span = ARENA * 2.0f;
        drawBox(0.0f, waterY, 0.0f, span, 0.05f, span, 0.025f,0.16f,0.24f);
    }

    if (g_world == WORLD_MAIN)
    {
        for (const auto& p : g_worldPortals)
        {
            const float pdx=p.x-g_px, pdz=p.z-g_pz;
            if (pdx*pdx+pdz*pdz < OBJECT_RENDER_DISTANCE*OBJECT_RENDER_DISTANCE)
                drawWorldPortal(p.x,p.z,p.targetWorld,false);
        }
    }
    else
    {
        // Every destination world has one return portal at its safe landing area.
        drawWorldPortal(0.0f,0.0f,WORLD_MAIN,true);
    }

    // Separate city biome: roads and plaza are drawn above the flat city terrain.
    if (g_world == WORLD_MAIN)
        drawCityRoads();

    // Underground maze: rounded, arched tunnels with closed ceilings and
    // explicit vertical side walls. The terrain remains visible above buried
    // tunnel sections, while the tunnel shell prevents looking out through
    // the roof or sides.
    auto drawTunnelSegment=[&](const TunnelSegment& t)
    {
        const float dx=t.bx-t.ax, dz=t.bz-t.az, len=sqrtf(dx*dx+dz*dz);
        if(len<0.01f) return;
        const float tcx=0.5f*(t.ax+t.bx), tcz=0.5f*(t.az+t.bz);
        const float tdx=tcx-g_px, tdz=tcz-g_pz;
        const float tunnelCull = TERRAIN_RENDER_DISTANCE + len;
        if(tdx*tdx+tdz*tdz > tunnelCull*tunnelCull) return;
        const int rings = std::max(3, (int)(len / 5.0f));
        const int arcSteps = 14;
        const float innerR = std::max(3.7f, t.halfWidth - 0.35f);
        const float wallH = 2.15f;
        const float roofR = innerR;
        const float nx = -dz / len;
        const float nz =  dx / len;

        // Arched ceiling.
        for(int r=0; r<rings; ++r)
        {
            const float u0=(float)r/(float)rings;
            const float u1=(float)(r+1)/(float)rings;
            const float x0=t.ax+dx*u0, z0=t.az+dz*u0;
            const float x1=t.ax+dx*u1, z1=t.az+dz*u1;
            const float f0=tunnelFloorHeight(x0,z0);
            const float f1=tunnelFloorHeight(x1,z1);

            glBegin(GL_QUADS);
            for(int s0=0; s0<arcSteps; ++s0)
            {
                const float a0=(float)s0/(float)(arcSteps)*M_PI;
                const float a1=(float)(s0+1)/(float)(arcSteps)*M_PI;
                const float lat0=cosf(a0)*innerR;
                const float lat1=cosf(a1)*innerR;
                const float off0=sinf(a0)*roofR;
                const float off1=sinf(a1)*roofR;
                const float y00=f0+wallH+off0;
                const float y01=f0+wallH+off1;
                const float y10=f1+wallH+off1;
                const float y11=f1+wallH+off0;
                const float shade=0.22f+0.12f*sinf(a0);
                glColor3f(shade,0.035f+0.03f*sinf(a0),0.25f+0.14f*sinf(a0));
                glVertex3f(x0+nx*lat0,y00,z0+nz*lat0);
                glVertex3f(x0+nx*lat1,y01,z0+nz*lat1);
                glVertex3f(x1+nx*lat1,y10,z1+nz*lat1);
                glVertex3f(x1+nx*lat0,y11,z1+nz*lat0);
            }
            glEnd();

            // Single continuous floor, slightly inset from the walls.
            const float floorInset=innerR-0.12f;
            glBegin(GL_QUADS);
            glColor3f(0.12f,0.055f,0.14f);
            glVertex3f(x0+nx*floorInset,f0+0.03f,z0+nz*floorInset);
            glVertex3f(x0-nx*floorInset,f0+0.03f,z0-nz*floorInset);
            glVertex3f(x1-nx*floorInset,f1+0.03f,z1-nz*floorInset);
            glVertex3f(x1+nx*floorInset,f1+0.03f,z1+nz*floorInset);
            glEnd();

            // Vertical lower walls up to the spring line of the arch.
            glBegin(GL_QUADS);
            glColor3f(0.19f,0.035f,0.22f);
            glVertex3f(x0+nx*innerR,f0,z0+nz*innerR);
            glVertex3f(x0+nx*innerR,f0+wallH,z0+nz*innerR);
            glVertex3f(x1+nx*innerR,f1+wallH,z1+nz*innerR);
            glVertex3f(x1+nx*innerR,f1,z1+nz*innerR);
            glColor3f(0.14f,0.025f,0.18f);
            glVertex3f(x0-nx*innerR,f0,z0-nz*innerR);
            glVertex3f(x1-nx*innerR,f1,z1-nz*innerR);
            glVertex3f(x1-nx*innerR,f1+wallH,z1-nz*innerR);
            glVertex3f(x0-nx*innerR,f0+wallH,z0-nz*innerR);
            glEnd();

            // Pink guide stripe at shoulder height.
            glBegin(GL_QUADS);
            glColor3f(0.98f,0.05f,0.72f);
            const float sy=f0+1.05f;
            const float sy1=f1+1.05f;
            const float sw=innerR+0.02f;
            glVertex3f(x0+nx*sw,sy,z0+nz*sw);
            glVertex3f(x0+nx*(sw-0.10f),sy,z0+nz*(sw-0.10f));
            glVertex3f(x1+nx*(sw-0.10f),sy1,z1+nz*(sw-0.10f));
            glVertex3f(x1+nx*sw,sy1,z1+nz*sw);
            glVertex3f(x0-nx*sw,sy,z0-nz*sw);
            glVertex3f(x1-nx*sw,sy1,z1-nz*sw);
            glVertex3f(x1-nx*(sw-0.10f),sy1,z1-nz*(sw-0.10f));
            glVertex3f(x0-nx*(sw-0.10f),sy,z0-nz*(sw-0.10f));
            glEnd();
        }

        // Solid arched entrance portal frames.
        const float ex[2] = {t.ax, t.bx};
        const float ez[2] = {t.az, t.bz};
        const float heading = atan2f(dx,dz)*180.0f/(float)M_PI;
        for(int e=0;e<2;++e)
        {
            const float px=ex[e], pz=ez[e];
            const float ground=terrainHeight(px,pz);
            const float floor=tunnelFloorHeight(px,pz);
            const float drop=std::max(0.0f, ground-floor);
            glPushMatrix();
            glTranslatef(px, ground-0.02f, pz);
            glRotatef(heading + (e==0?180.0f:0.0f),0,1,0);

            // Two pillars and a segmented semicircular crown. The crown is
            // kept above the actual ground profile so it never floats.
            const float pillarH=std::max(1.6f, std::min(3.8f, 1.8f+drop*0.35f));
            drawBox(-t.halfWidth+0.45f, pillarH*0.5f, 0.0f, 0.70f, pillarH, 0.90f, 0.68f,0.03f,0.42f);
            drawBox( t.halfWidth-0.45f, pillarH*0.5f, 0.0f, 0.70f, pillarH, 0.90f, 0.68f,0.03f,0.42f);
            const int crownSegs=12;
            const float crownR=t.halfWidth-0.45f;
            const float centerY=pillarH;
            for(int k=0;k<crownSegs;++k)
            {
                const float a0=(float)k/(float)crownSegs*M_PI;
                const float a1=(float)(k+1)/(float)crownSegs*M_PI;
                const float x0=cosf(a0)*crownR;
                const float y0=centerY+sinf(a0)*crownR;
                const float x1=cosf(a1)*crownR;
                const float y1=centerY+sinf(a1)*crownR;
                const float mx=0.5f*(x0+x1), my=0.5f*(y0+y1);
                const float segLen=sqrtf((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
                drawBox(mx,my,0.0f,0.52f,std::max(0.24f,segLen+0.10f),0.90f,0.86f,0.04f,0.58f);
            }
            glPopMatrix();
        }
    };
    if (g_world == WORLD_MAIN)
    {
        for(const auto& t:g_tunnels)
        {
            drawTunnelSegment(t);
            drawPortalMouth(t, true);
            drawPortalMouth(t, false);
        }
    }

    // Pink ceiling lamps with a small glow box, spaced along every segment.
    if (g_world == WORLD_MAIN) for(const auto& t:g_tunnels)
    {
        const float dx=t.bx-t.ax, dz=t.bz-t.az, len=sqrtf(dx*dx+dz*dz);
        const int lamps=std::max(1,(int)(len/18.0f));
        for(int i=1;i<lamps;++i)
        {
            const float u=(float)i/(float)lamps;
            const float x=t.ax+dx*u, z=t.az+dz*u;
            const float y=tunnelFloorHeight(x,z)+4.75f;
            drawBox(x,y,z,0.22f,0.16f,0.22f,0.95f,0.08f,0.70f);
            drawBox(x,y-0.08f,z,0.38f,0.06f,0.38f,0.65f,0.02f,0.45f);
        }
    }

    // No perimeter walls: the map edges are completely open.

    // Procedural vegetation and biome decorations.
    drawDecor();

    // objects
    for (const auto& o : g_objects)
    {
        if (!o.active || o.world != g_world)
            continue;

        const float odx=o.x-g_px, odz=o.z-g_pz;
        if(odx*odx+odz*odz > OBJECT_RENDER_DISTANCE*OBJECT_RENDER_DISTANCE)
            continue;

        if (o.type == OBJECT_BUILDING)
        {
            drawBuildingWall(o);
        }
        else if (o.type == OBJECT_SKYSCRAPER)
        {
            drawSkyscraper(o);
        }
        else if (o.type == OBJECT_SOLID)
        {
            float r = 0.42f, g = 0.46f, b = 0.52f;
            if (o.sx >= 4.5f) { r = 0.23f; g = 0.27f; b = 0.33f; }
            else if (o.sx <= 1.7f) { r = 0.50f; g = 0.28f; b = 0.10f; }
            drawBox(o.x, o.y, o.z, o.sx, o.sy, o.sz, r, g, b);
        }
        else if (o.type == OBJECT_TREE)
        {
            drawTree(o.x, o.y - o.sy * 0.5f, o.z);
        }
        else if (o.type == OBJECT_AMMO)
        {
            drawAmmo(o.x, o.y - o.sy * 0.5f, o.z);
        }
        else if (o.type == OBJECT_MEDKIT)
        {
            drawMedkit(o.x, o.y - o.sy * 0.5f, o.z);
        }
    }
}

static void drawEnemyProjectiles()
{
    for (const auto& p : g_projectiles)
    {
        if (!p.active)
            continue;

        drawBox(p.x, p.y, p.z,
                0.12f, 0.12f, 0.12f,
                0.95f, 0.25f, 0.05f);
    }
}

// ============================================================
// DRAW ENEMIES
// ============================================================

static void drawEnemies()
{
    for (const auto& e : g_enemies)
    {
        if (e.world != g_world || !e.alive) continue;
        const float edx=e.x-g_px, edz=e.z-g_pz;
        if(edx*edx+edz*edz > ENEMY_RENDER_DISTANCE*ENEMY_RENDER_DISTANCE) continue;
        const bool drone = e.type == ENEMY_DRONE;
        const float baseY = drone ? e.y - 0.65f : terrainSupportHeight(e.x, e.z, e.radius);
        const float bodyHeight = (e.type == ENEMY_TANK) ? 2.35f : (drone ? 1.3f : 1.75f);
        const float bodyWidth = (e.type == ENEMY_TANK) ? 1.65f : (drone ? 1.15f : 0.95f);
        float br=0.58f,bg=0.12f,bb=0.08f;
        if (e.type == ENEMY_FAST){br=0.95f;bg=0.44f;bb=0.05f;}
        if (e.type == ENEMY_TANK){br=0.33f;bg=0.08f;bb=0.68f;}
        if (drone){br=0.08f;bg=0.65f;bb=0.92f;}

        // Each destination world has its own visual faction.
        if (e.world == WORLD_OCEAN && !drone)
        {
            br = (e.type==ENEMY_TANK) ? 0.05f : 0.04f;
            bg = (e.type==ENEMY_TANK) ? 0.30f : 0.62f;
            bb = (e.type==ENEMY_TANK) ? 0.38f : 0.72f;
        }
        else if (e.world == WORLD_WINTER && !drone)
        {
            br = (e.type==ENEMY_TANK) ? 0.35f : 0.72f;
            bg = (e.type==ENEMY_TANK) ? 0.42f : 0.82f;
            bb = (e.type==ENEMY_TANK) ? 0.52f : 0.92f;
        }
        if (e.hitFlash > 0.0f){br=1.0f;bg=0.90f;bb=0.70f;}

        glPushMatrix();
        glTranslatef(e.x, baseY, e.z);
        // Movement uses heading (sin(yaw), -cos(yaw)). OpenGL
        // rotates local -Z by (-yaw), so the visible model now matches
        // the actual movement direction and visibly turns toward the player.
        glRotatef(-e.yaw * 180.0f / (float)M_PI, 0.0f, 1.0f, 0.0f);
        if (drone)
        {
            drawBox(0.0f,0.85f,0.0f,1.35f,0.62f,1.10f,br,bg,bb);
            drawBox(0.0f,0.85f,-0.78f,0.16f,0.16f,0.95f,0.12f,0.12f,0.14f);
            drawBox(-0.75f,0.95f,0.0f,0.10f,0.08f,0.65f,0.95f,0.18f,0.06f);
            drawBox(0.75f,0.95f,0.0f,0.10f,0.08f,0.65f,0.95f,0.18f,0.06f);
        }
        else if (e.type == ENEMY_TANK)
        {
            // Proper tank silhouette: hull, tracks, rotating turret and long gun.
            // The whole vehicle follows e.yaw, so tanks visibly turn into the
            // direction they are driving instead of sliding sideways.
            drawBox(0.0f, 0.62f, 0.0f, 2.15f, 0.72f, 2.85f, br, bg, bb);
            drawBox(-1.02f, 0.48f, 0.0f, 0.42f, 0.62f, 2.65f, 0.12f, 0.10f, 0.15f);
            drawBox( 1.02f, 0.48f, 0.0f, 0.42f, 0.62f, 2.65f, 0.12f, 0.10f, 0.15f);
            drawBox(0.0f, 1.12f, 0.05f, 1.45f, 0.48f, 1.55f, br*0.88f, bg*0.88f, bb*0.88f);
            drawBox(0.0f, 1.30f, -0.62f, 0.52f, 0.38f, 1.75f, 0.18f, 0.16f, 0.20f);
            drawBox(0.0f, 1.34f, -1.55f, 0.18f, 0.18f, 2.20f, 0.16f, 0.15f, 0.18f);
            // Wheels/track accents.
            for (int side = -1; side <= 1; side += 2)
                for (int k = -1; k <= 1; ++k)
                    drawBox(side*1.25f, 0.47f, k*0.78f, 0.08f, 0.30f, 0.42f, 0.05f,0.05f,0.06f);
        }
        else
        {
            const float headY=bodyHeight+0.28f;
            drawBox(0.0f,bodyHeight*0.5f,0.0f,bodyWidth,bodyHeight,bodyWidth,br,bg,bb);
            drawBox(0.0f,headY,0.0f,bodyWidth*0.65f,0.54f,bodyWidth*0.65f,br*0.92f,bg*0.92f,bb*0.92f);
            drawBox(0.0f,headY-0.04f,-bodyWidth*0.34f,bodyWidth*0.48f,0.20f,bodyWidth*0.14f,0.72f,0.84f,0.92f);
            drawBox(-bodyWidth*0.56f,bodyHeight*0.55f,0.0f,0.20f,bodyHeight*0.52f,0.28f,br*0.78f,bg*0.78f,bb*0.78f);
            drawBox(bodyWidth*0.56f,bodyHeight*0.55f,0.0f,0.20f,bodyHeight*0.52f,0.28f,br*0.78f,bg*0.78f,bb*0.78f);
            drawBox(bodyWidth*0.35f,1.20f,-0.55f,0.14f,0.14f,1.1f,0.10f,0.11f,0.13f);
        }

        // Ocean enemies carry fins/floatation gear; winter enemies carry ice armor.
        if (e.world == WORLD_OCEAN && !drone && e.type != ENEMY_TANK)
        {
            drawBox(-0.62f, 0.82f, 0.0f, 0.12f, 0.48f, 0.85f, 0.05f,0.38f,0.48f);
            drawBox( 0.62f, 0.82f, 0.0f, 0.12f, 0.48f, 0.85f, 0.05f,0.38f,0.48f);
        }
        else if (e.world == WORLD_WINTER && !drone && e.type != ENEMY_TANK)
        {
            drawBox(0.0f, 1.05f, 0.0f, 1.12f, 0.16f, 0.92f, 0.62f,0.78f,0.90f);
            drawBox(0.0f, 2.20f, -0.08f, 0.82f, 0.12f, 0.82f, 0.78f,0.90f,1.0f);
        }

        glPopMatrix();

        const float hp = clampf(e.hp / std::max(e.maxHp,1.0f),0.0f,1.0f);
        glPushMatrix();
        glTranslatef(e.x,baseY+bodyHeight+0.95f,e.z);
        glRotatef(-g_yaw*180.0f/(float)M_PI,0.0f,1.0f,0.0f);
        glRotatef(g_pitch*180.0f/(float)M_PI,1.0f,0.0f,0.0f);
        const float barW = drone ? 0.95f : bodyWidth*0.95f;
        glBegin(GL_QUADS);
        glColor3f(0.04f,0.04f,0.05f);
        glVertex3f(-barW,0,0);glVertex3f(barW,0,0);glVertex3f(barW,0.12f,0);glVertex3f(-barW,0.12f,0);
        glColor3f(drone?0.10f:0.10f,drone?0.80f:0.95f,0.16f);
        const float fill = (2*barW-0.06f)*hp;
        glVertex3f(-barW+0.03f,0.02f,0.01f);glVertex3f(-barW+0.03f+fill,0.02f,0.01f);glVertex3f(-barW+0.03f+fill,0.10f,0.01f);glVertex3f(-barW+0.03f,0.10f,0.01f);
        glEnd();
        glPopMatrix();
    }
}

// ============================================================
// DRAW WEAPON
// ============================================================

static void drawWeapon()
{
    int w =
        g_currentWeapon;

    float recoil =
        g_weaponRecoil;

    float x =
        0.22f;

    float y =
        -0.35f + recoil * 0.08f;

    float z =
        -0.70f;

    glPushMatrix();

    glTranslatef(
        x,
        y,
        z
    );

    glRotatef(
        -5.0f,
        0.0f,
        1.0f,
        0.0f
    );

    if (w == WEAPON_PISTOL)
    {
        drawBox(
            0.0f,
            0.0f,
            0.0f,
            0.25f,
            0.20f,
            0.75f,
            0.10f,
            0.11f,
            0.13f
        );

        drawBox(
            0.0f,
            -0.25f,
            0.15f,
            0.18f,
            0.55f,
            0.22f,
            0.08f,
            0.08f,
            0.10f
        );
    }
    else if (w == WEAPON_RIFLE)
    {
        drawBox(
            0.0f,
            0.0f,
            0.0f,
            0.24f,
            0.25f,
            1.25f,
            0.08f,
            0.09f,
            0.10f
        );

        drawBox(
            0.0f,
            -0.25f,
            0.15f,
            0.20f,
            0.65f,
            0.25f,
            0.07f,
            0.07f,
            0.08f
        );

        drawBox(
            0.0f,
            0.05f,
            -0.75f,
            0.12f,
            0.12f,
            0.70f,
            0.12f,
            0.13f,
            0.15f
        );
    }
    else
    {
        drawBox(
            0.0f,
            0.0f,
            0.0f,
            0.38f,
            0.30f,
            1.05f,
            0.12f,
            0.12f,
            0.13f
        );

        drawBox(
            0.0f,
            -0.25f,
            0.15f,
            0.26f,
            0.65f,
            0.28f,
            0.08f,
            0.08f,
            0.09f
        );

        drawBox(
            -0.10f,
            0.03f,
            -0.62f,
            0.10f,
            0.10f,
            0.65f,
            0.10f,
            0.10f,
            0.11f
        );

        drawBox(
            0.10f,
            0.03f,
            -0.62f,
            0.10f,
            0.10f,
            0.65f,
            0.10f,
            0.10f,
            0.11f
        );
    }

    glPopMatrix();

    // muzzle flash
    if (g_muzzleFlash > 0.0f)
    {
        glPushMatrix();

        glTranslatef(
            0.22f,
            -0.27f,
            -1.35f
        );

        float s =
            0.20f * g_muzzleFlash;

        drawBox(
            0.0f,
            0.0f,
            0.0f,
            s,
            s,
            s,
            1.0f,
            0.75f,
            0.10f
        );

        glPopMatrix();
    }
}

// ============================================================
// HUD
// ============================================================


static void drawRadar(int W, int H)
{
    const float radius = 92.0f;
    const float cx = W - radius - 24.0f;
    const float cy = H - radius - 24.0f;
    const float range = 180.0f;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,W,0,H,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    // Circular dark backing, built from horizontal quads.
    glColor3f(0.015f,0.025f,0.035f);
    for(int i=0;i<72;++i)
    {
        const float y0=-radius + (2.0f*radius)*(float)i/72.0f;
        const float y1=-radius + (2.0f*radius)*(float)(i+1)/72.0f;
        const float a0=sqrtf(std::max(0.0f,radius*radius-y0*y0));
        const float a1=sqrtf(std::max(0.0f,radius*radius-y1*y1));
        const float half=std::max(a0,a1);
        glBegin(GL_QUADS);
        glVertex2f(cx-half,cy+y0);
        glVertex2f(cx+half,cy+y0);
        glVertex2f(cx+half,cy+y1);
        glVertex2f(cx-half,cy+y1);
        glEnd();
    }

    // Terrain sampled locally around the player.
    const int cells=24;
    const float cell=(2.0f*radius)/(float)cells;
    for(int ix=0;ix<cells;++ix)
    {
        for(int iz=0;iz<cells;++iz)
        {
            const float sx=cx-radius+(ix+0.5f)*cell;
            const float sy=cy-radius+(iz+0.5f)*cell;
            const float rdx=(sx-cx), rdy=(sy-cy);
            if(rdx*rdx+rdy*rdy > (radius-1.0f)*(radius-1.0f)) continue;
            const float wx=g_px + (rdx/radius)*range;
            const float wz=g_pz + (rdy/radius)*range;
            const float h=terrainHeight(wx,wz);
            float r=0.18f,g=0.38f,b=0.16f;
            if(h<-5.0f){r=0.08f;g=0.20f;b=0.45f;}
            else if(h<3.0f){r=0.20f;g=0.48f;b=0.18f;}
            else if(h<13.0f){r=0.45f;g=0.30f;b=0.13f;}
            else if(h<25.0f){r=0.32f;g=0.32f;b=0.36f;}
            else {r=0.78f;g=0.82f;b=0.88f;}
            glColor3f(r,g,b);
            glBegin(GL_QUADS);
            glVertex2f(sx-cell*0.55f,sy-cell*0.55f);
            glVertex2f(sx+cell*0.55f,sy-cell*0.55f);
            glVertex2f(sx+cell*0.55f,sy+cell*0.55f);
            glVertex2f(sx-cell*0.55f,sy+cell*0.55f);
            glEnd();
        }
    }

    // Tunnel portal hints.
    glPointSize(4.0f);
    glColor3f(1.0f,0.08f,0.75f);
    glBegin(GL_POINTS);
    for(const auto& t:g_tunnels)
    {
        const float ends[2][2]={{t.ax,t.az},{t.bx,t.bz}};
        for(int k=0;k<2;++k)
        {
            const float dx=ends[k][0]-g_px, dz=ends[k][1]-g_pz;
            if(dx*dx+dz*dz > range*range) continue;
            const float px=cx+(dx/range)*radius, py=cy+(dz/range)*radius;
            const float qx=px-cx, qy=py-cy;
            if(qx*qx+qy*qy < radius*radius) glVertex2f(px,py);
        }
    }
    glEnd();

    // Inter-world portal markers.
    // Portals are drawn in the same local coordinate frame as the current world,
    // so switching worlds never causes markers from another world to overlap.
    glPointSize(7.0f);
    glBegin(GL_POINTS);
    if (g_world == WORLD_MAIN)
    {
        for (const auto& p : g_worldPortals)
        {
            const float dx=p.x-g_px, dz=p.z-g_pz;
            if(dx*dx+dz*dz > range*range) continue;
            const float px=cx+(dx/range)*radius, py=cy+(dz/range)*radius;
            const float qx=px-cx, qy=py-cy;
            if(qx*qx+qy*qy >= (radius-4.0f)*(radius-4.0f)) continue;
            if(p.targetWorld==WORLD_OCEAN) glColor3f(0.05f,0.80f,1.0f);
            else if(p.targetWorld==WORLD_WINTER) glColor3f(0.88f,0.95f,1.0f);
            else glColor3f(1.0f,0.25f,0.90f);
            glVertex2f(px,py);
        }
    }
    else
    {
        // Return portal at the center of each destination world.
        const float dx=-g_px, dz=-g_pz;
        if(dx*dx+dz*dz <= range*range)
        {
            const float px=cx+(dx/range)*radius, py=cy+(dz/range)*radius;
            glColor3f(1.0f,0.30f,0.90f);
            glVertex2f(px,py);
        }
    }
    glEnd();

    // Unique-object markers.
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    for (const auto& o : g_otherWorldObjects)
    {
        if(o.world != g_world) continue;
        const float dx=o.x-g_px, dz=o.z-g_pz;
        if(dx*dx+dz*dz > range*range) continue;
        const float px=cx+(dx/range)*radius, py=cy+(dz/range)*radius;
        const float qx=px-cx, qy=py-cy;
        if(qx*qx+qy*qy >= (radius-4.0f)*(radius-4.0f)) continue;
        if(g_world==WORLD_OCEAN) glColor3f(0.15f,0.85f,0.95f);
        else glColor3f(0.72f,0.88f,1.0f);
        glVertex2f(px,py);
    }
    glEnd();

    // Enemy markers.
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for(const auto& e:g_enemies)
    {
        if(!e.alive) continue;
        const float dx=e.x-g_px, dz=e.z-g_pz;
        if(dx*dx+dz*dz > range*range) continue;
        const float px=cx+(dx/range)*radius, py=cy+(dz/range)*radius;
        const float qx=px-cx, qy=py-cy;
        if(qx*qx+qy*qy >= (radius-4.0f)*(radius-4.0f)) continue;
        if(e.type==ENEMY_DRONE) glColor3f(0.10f,0.75f,1.0f);
        else if(e.type==ENEMY_TANK) glColor3f(0.78f,0.18f,1.0f);
        else glColor3f(1.0f,0.15f,0.12f);
        glVertex2f(px,py);
    }
    glEnd();

    // Player marker/orientation.
    glColor3f(0.95f,0.98f,1.0f);
    glPointSize(7.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx,cy);
    glEnd();

    // Radar ring, drawn as points so it remains circular with no extra GL state.
    glColor3f(0.55f,0.75f,0.85f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for(int i=0;i<96;++i)
    {
        const float a=(float)i/96.0f*2.0f*(float)M_PI;
        glVertex2f(cx+cosf(a)*radius,cy+sinf(a)*radius);
    }
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

static void drawHUD(
    int W,
    int H)
{
    glMatrixMode(GL_PROJECTION);

    glPushMatrix();

    glLoadIdentity();

    glOrtho(
        0,
        W,
        0,
        H,
        -1,
        1
    );

    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();

    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    // crosshair
    float cx =
        W * 0.5f;

    float cy =
        H * 0.5f;

    if (g_hitMarker > 0.0f)
    {
        glColor3f(
            1.0f,
            0.35f,
            0.15f
        );
    }
    else
    {
        glColor3f(
            0.3f,
            1.0f,
            0.4f
        );
    }

    rect(
        cx - 14,
        cy - 1.5f,
        cx + 14,
        cy + 1.5f
    );

    rect(
        cx - 1.5f,
        cy - 14,
        cx + 1.5f,
        cy + 14
    );

    // health background
    glColor3f(
        0.05f,
        0.05f,
        0.05f
    );

    rect(
        30,
        30,
        330,
        60
    );

    float hp =
        clampf(
            g_health / g_maxHealth,
            0.0f,
            1.0f
        );

    glColor3f(
        0.85f,
        0.08f,
        0.08f
    );

    rect(
        35,
        35,
        35 + 290 * hp,
        55
    );

    // ammo
    Weapon& weapon =
        g_weapons[g_currentWeapon];

    glColor3f(
        1.0f,
        0.9f,
        0.3f
    );

    drawNumber(
        W - 250,
        50,
        45,
        weapon.ammo
    );

    glColor3f(
        0.8f,
        0.8f,
        0.85f
    );

    drawNumber(
        W - 120,
        50,
        32,
        weapon.reserve
    );

    // score
    glColor3f(
        1.0f,
        0.85f,
        0.2f
    );

    drawNumber(
        35,
        H - 80,
        35,
        g_kills
    );

    // weapon indicator
    float bx =
        W * 0.5f - 110;

    float by =
        35.0f;

    glColor3f(
        0.08f,
        0.08f,
        0.10f
    );

    rect(
        bx,
        by,
        bx + 220,
        by + 38
    );

    glColor3f(
        0.7f,
        0.75f,
        0.85f
    );

    // small weapon bars
    for (int i = 0; i < 3; ++i)
    {
        if (i == g_currentWeapon)
        {
            glColor3f(
                1.0f,
                0.75f,
                0.15f
            );
        }
        else
        {
            glColor3f(
                0.3f,
                0.35f,
                0.40f
            );
        }

        rect(
            bx + 10 + i * 70,
            by + 10,
            bx + 60 + i * 70,
            by + 28
        );
    }

    // damage overlay
    if (g_damageFlash > 0.0f)
    {
        glColor4f(
            1.0f,
            0.0f,
            0.0f,
            g_damageFlash * 0.25f
        );

        glEnable(GL_BLEND);

        glBlendFunc(
            GL_SRC_ALPHA,
            GL_ONE_MINUS_SRC_ALPHA
        );

        rect(
            0,
            0,
            W,
            H
        );

        glDisable(GL_BLEND);
    }

    // death
    if (g_deadTimer > 0.0f)
    {
        glColor3f(
            1.0f,
            0.1f,
            0.1f
        );

        rect(
            W*0.5f - 160,
            H*0.5f - 30,
            W*0.5f + 160,
            H*0.5f + 30
        );
    }

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);

    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glPopMatrix();
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    srand(
        (unsigned)time(nullptr)
    );

    if (!glfwInit())
    {
        fprintf(
            stderr,
            "Failed to initialize GLFW\n"
        );

        return -1;
    }

    // fullscreen
    GLFWmonitor* monitor =
        glfwGetPrimaryMonitor();

    const GLFWvidmode* mode =
        glfwGetVideoMode(monitor);

    if (!mode)
    {
        glfwTerminate();
        return -1;
    }

    g_window =
        glfwCreateWindow(
            mode->width,
            mode->height,
            "Simple 3D Shooter - Extended Terrain",
            monitor,
            nullptr
        );

    if (!g_window)
    {
        fprintf(
            stderr,
            "Failed to create GLFW window\n"
        );

        glfwTerminate();

        return -1;
    }

    glfwMakeContextCurrent(
        g_window
    );

    glfwSwapInterval(1);

    // mouse
    glfwSetInputMode(
        g_window,
        GLFW_CURSOR,
        GLFW_CURSOR_DISABLED
    );

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(
            g_window,
            GLFW_RAW_MOUSE_MOTION,
            GLFW_TRUE
        );
    }

    glfwSetCursorPosCallback(
        g_window,
        mouseCallback
    );

    // OpenGL
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LEQUAL);

    glDisable(GL_CULL_FACE);

    glEnable(GL_COLOR_MATERIAL);

    // IMPORTANT:
    // player is deliberately placed in an empty area
    g_px = 0.0f;
    g_pz = 20.0f;
    g_py = currentFloorHeight(g_px, g_pz);

    resetGame();

    double lastTime =
        glfwGetTime();

    while (
        !glfwWindowShouldClose(
            g_window))
    {
        double now =
            glfwGetTime();

        float dt =
            (float)(
                now - lastTime
            );

        lastTime = now;

        if (dt > 0.1f)
            dt = 0.1f;

        // ----------------------------------------------------
        // EVENTS
        // ----------------------------------------------------

        glfwPollEvents();

        if (glfwGetKey(
                g_window,
                GLFW_KEY_ESCAPE)
            == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(
                g_window,
                GLFW_TRUE
            );
        }

        // ----------------------------------------------------
        // UPDATE
        // ----------------------------------------------------

        updateGame(dt);

        // ----------------------------------------------------
        // VIEWPORT
        // ----------------------------------------------------

        int W;
        int H;

        glfwGetFramebufferSize(
            g_window,
            &W,
            &H
        );

        if (H <= 0)
            H = 1;

        glViewport(
            0,
            0,
            W,
            H
        );

        // ----------------------------------------------------
        // CLEAR
        // ----------------------------------------------------

        const float day = terrainDayFactor();
        glClearColor(
            0.002f + 0.06f * day,
            0.004f + 0.12f * day,
            0.02f + 0.25f * day,
            1.0f
        );

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        // ----------------------------------------------------
        // SKY
        // ----------------------------------------------------

        drawSky();

        // ----------------------------------------------------
        // CAMERA
        // ----------------------------------------------------

        glMatrixMode(
            GL_PROJECTION
        );

        glLoadIdentity();

        float aspect =
            (float)W / (float)H;

        float fov =
            75.0f;

        float nearPlane =
            0.05f;

        float farPlane =
            650.0f;

        float top =
            nearPlane *
            tanf(
                fov *
                (float)M_PI /
                360.0f
            );

        float right =
            top * aspect;

        glFrustum(
            -right,
            right,
            -top,
            top,
            nearPlane,
            farPlane
        );

        glMatrixMode(
            GL_MODELVIEW
        );

        glLoadIdentity();

        // Camera is always reset here.
        // This fixes the common bug where
        // the sky is visible but WASD seems dead.

        glRotatef(
            -g_pitch *
            180.0f /
            (float)M_PI,
            1.0f,
            0.0f,
            0.0f
        );

        glRotatef(
            g_yaw *
            180.0f /
            (float)M_PI,
            0.0f,
            1.0f,
            0.0f
        );

        glTranslatef(
            -g_px,
            -(g_py + EYE_HEIGHT),
            -g_pz
        );

        // World-space stars: they are fixed in the game world.
        drawWorldStarField();

        // ----------------------------------------------------
        // WORLD
        // ----------------------------------------------------

        drawWorld();

        drawEnemyProjectiles();
        drawEnemies();

        // ----------------------------------------------------
        // WEAPON
        // ----------------------------------------------------

        glMatrixMode(
            GL_MODELVIEW
        );

        glPushMatrix();

        // weapon follows camera
        glLoadIdentity();

        drawWeapon();

        glPopMatrix();

        // ----------------------------------------------------
        // HUD
        // ----------------------------------------------------

        drawHUD(
            W,
            H
        );
        drawRadar(W, H);

        // ----------------------------------------------------
        // SWAP
        // ----------------------------------------------------

        glfwSwapBuffers(
            g_window
        );
    }

    glfwDestroyWindow(
        g_window
    );

    glfwTerminate();

    return 0;
}
