// Game.cpp
// Purpose: Implements the methods declared in Game.h.
//
// This file coordinates the main game loop, state transitions, entity updates,
// rendering, collision detection, and enemy spawning.

#include "Game.h"
#include <cmath>
#include <algorithm>

// ----------------------------------------------------------------------------
// TEMPORARY DEBUG DAMAGE — remove once enemy/asteroid/boss damage exists.
// Press F1 once to deal DEBUG_DAMAGE_AMOUNT percent damage to the player.
// ----------------------------------------------------------------------------
constexpr int   DEBUG_DAMAGE_KEY = KEY_F1;
constexpr float DEBUG_DAMAGE_AMOUNT = 10.0f;

// ----------------------------------------------------------------------------
// Healing Stone Spawning (initial balancing values)
// ----------------------------------------------------------------------------
// No stones spawn while player health is above 50% — the timer re-checks
// periodically so spawning resumes as soon as health drops to 50% or below.
constexpr float HEALING_STONE_SPAWN_RECHECK_INTERVAL = 1.0f;
constexpr float HEALING_STONE_SPAWN_INTERVAL_MID  = 5.0f;  // health 31%-50%
constexpr float HEALING_STONE_SPAWN_INTERVAL_LOW  = 3.0f;  // health <= 30%
constexpr float HEALING_STONE_MIN_PLAYER_DISTANCE = 100.0f; // world units
constexpr int   HEALING_STONE_MAX_SPAWN_ATTEMPTS  = 20;

// Contact damage dealt by enemies (matches prototype: enemy contact = 10%)
constexpr float ENEMY_CONTACT_DAMAGE = 10.0f;

// Enemy projectile damage per source type + visual identity (enemy colors)
constexpr float ENEMY1_PROJ_DAMAGE = 2.0f;
constexpr float ENEMY2_PROJ_DAMAGE = 5.0f;
// Bomber bomb explosion: AoE hit at detonation, applied once per bomb.
// Radius lives in Bomb.h (BOMB_EXPLOSION_RADIUS, shared with warning).
constexpr float BOMB_EXPLOSION_DAMAGE = 20.0f;
constexpr Color ENEMY1_PROJ_COLOR = { 230, 60, 60, 255 };   // Type 1 red
constexpr Color ENEMY2_PROJ_COLOR = { 255, 150, 50, 255 };  // Type 2 orange

// ----------------------------------------------------------------------------
// Enemy (re)spawn positioning — random spots inside the visible camera area
// ----------------------------------------------------------------------------
constexpr float ENEMY_SPAWN_MIN_PLAYER_DIST = 240.0f; // px from player
constexpr float ENEMY_SPAWN_MIN_ENEMY_DIST  = 180.0f; // px between the two enemies
constexpr float ENEMY_SPAWN_VIEW_MARGIN = 80.0f;      // world-edge inset for spawns
constexpr int   ENEMY_SPAWN_MAX_ATTEMPTS = 30;

// Damage dealt to an enemy per player bullet hit
constexpr float ENEMY_BULLET_DAMAGE = 1.0f;

// ----------------------------------------------------------------------------
// Bomber (Type 3) initial spawn — one instance ANYWHERE in the world.
// Unlike Types 1/2 it is NOT restricted to the camera view: it may spawn
// far off-screen and is discovered by exploration.
// ----------------------------------------------------------------------------
constexpr float BOMBER_SPAWN_INSET = 80.0f;        // world-edge margin for spawn point
constexpr float BOMBER_MIN_PLAYER_DIST = 300.0f;   // never spawn on/near the player
constexpr int   BOMBER_SPAWN_MAX_ATTEMPTS = 40;

// Enemy respawn after destruction + initial AI activation delay
constexpr float ENEMY_RESPAWN_DELAY = 2.0f;     // seconds before a destroyed enemy returns
constexpr float BOMBER_RESPAWN_DELAY = 5.0f;    // seconds before a destroyed Bomber returns
constexpr float ENEMY_ACTIVATION_DELAY = 1.0f;  // seconds enemies wait before pursuing

// Score rewards per enemy type (awarded once at destruction)
constexpr int ENEMY1_KILL_SCORE = 100;
constexpr int ENEMY2_KILL_SCORE = 200;
constexpr int ENEMY3_KILL_SCORE = 300; // Bomber: substantial bonus, above Types 1/2

Game::Game(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_worldWidth(2400.0f)
    , m_worldHeight(1800.0f)
    , m_player({ 1200.0f, 900.0f }, 2400.0f, 1800.0f) // Spawn at center of arena
    , m_camera{ 0 }
    , m_stoneSpawnTimer(0.0f)
    , m_testEnemy({ 1300.0f, 620.0f })  // just above initial view, inside world
    , m_testEnemy2({ 1050.0f, 1150.0f }) // inside world, away from player spawn
    , m_bomber({ 500.0f, 350.0f })       // upper-left world area, far from player
    , m_contactOwner(0)
    , m_enemyActivateTimer(0.0f)
    , m_enemy1RespawnTimer(ENEMY_RESPAWN_DELAY)
    , m_enemy2RespawnTimer(ENEMY_RESPAWN_DELAY)
    , m_bomberRespawnTimer(BOMBER_RESPAWN_DELAY)
    , m_score(0)
    , m_t1Kills(0)
    , m_t2Kills(0)
{
    // Deterministic starfield generation (stable across runs)
    constexpr int STAR_COUNT = 400;
    m_stars.reserve(STAR_COUNT);

    unsigned int seed = 1337u;
    for (int i = 0; i < STAR_COUNT; ++i)
    {
        seed = seed * 1664525u + 1013904223u;
        const float fx = static_cast<float>(seed >> 8) / 16777216.0f;

        seed = seed * 1664525u + 1013904223u;
        const float fy = static_cast<float>(seed >> 8) / 16777216.0f;

        seed = seed * 1664525u + 1013904223u;
        const float fr = static_cast<float>(seed >> 8) / 16777216.0f;

        Star star;
        star.position = { fx * m_worldWidth, fy * m_worldHeight };
        star.radius = 0.5f + fr * 2.0f;
        star.alpha = static_cast<unsigned char>(80 + (seed & 0x7Fu));
        m_stars.push_back(star);
    }
}

void Game::Init()
{
    // ------------------------------------------------------------------------
    // Camera2D Initialization
    // Directly targets m_player.m_position via friendship (zero getters used)
    // ------------------------------------------------------------------------
    m_camera.target = m_player.m_position;
    m_camera.offset = Vector2{ static_cast<float>(m_screenWidth) / 2.0f,
                                 static_cast<float>(m_screenHeight) / 2.0f }; // Center on screen
    m_camera.rotation = 0.0f; // Fixed 0 rotation
    m_camera.zoom = 1.5f; // Fixed 1.5x zoom

    // Place both enemies at random positions inside the visible camera area
    RepositionEnemiesInView();

    // Bomber (Type 3): exactly one instance at a random WORLD position —
    // anywhere in the arena including off-camera, never on the player.
    // If no valid spot is found (practically impossible), the constructor
    // position is kept as a safe fallback.
    {
        Vector2 bomberPos;
        if (TryFindBomberSpawnPos(bomberPos))
        {
            m_bomber.ResetTo(bomberPos);
        }
    }

    // Enemies wait briefly before starting their AI pursuit
    m_enemyActivateTimer = ENEMY_ACTIVATION_DELAY;
}

void Game::Update(float dt)
{
    if (dt <= 0.0f) return;

    // 1. Update player (player reads camera to calculate world-space mouse target)
    m_player.Update(dt, m_camera);

    // 2. Spawn a bullet if the player requested one this frame
    if (m_player.m_shotRequested)
    {
        m_player.m_shotRequested = false;
        SpawnPlayerBullet();
    }

    // 3. Update projectiles
    UpdateBullets(dt);

    // 3a. Player bullets vs enemies (hit -> damage + bullet removal)
    CheckBulletEnemyHits();

    // 3b. Update healing stones (spawn timer, lifetime, collection)
    UpdateHealingStones(dt);

    // 3c. Activation delay: at game start and after player respawn, enemies
    // hold position inside the view for ENEMY_ACTIVATION_DELAY seconds
    // before their AI pursuit begins. Bullet hits still work during this.
    if (m_enemyActivateTimer > 0.0f)
    {
        m_enemyActivateTimer -= dt;
    }
    else
    {
        // Exclusive contact engagement: the enemy NOT owning contact must
        // keep a safe distance from the player this frame.
        const bool enemyAvoids  = (m_contactOwner == 2);
        const bool interAvoids  = (m_contactOwner == 1);

        // 3d. Update test enemy (wander -> chase; destroyed enemies stay inert)
        if (!m_testEnemy.IsDestroyed())
        {
            m_testEnemy.Update(dt, m_player.m_position, m_camera, m_worldWidth, m_worldHeight,
                               m_player.m_isAccelerating, m_player.m_isDead, enemyAvoids,
                               m_testEnemy2.position, !m_testEnemy2.IsDestroyed());

            // Enemy Type 1 firing: one plasma orb per shot, aimed at the
            // player's position at fire time. Fixed trajectory — no homing.
            if (m_testEnemy.m_shotRequested)
            {
                m_testEnemy.m_shotRequested = false;
                const float fdx = m_player.m_position.x - m_testEnemy.position.x;
                const float fdy = m_player.m_position.y - m_testEnemy.position.y;
                const float flen = std::hypot(fdx, fdy);
                if (flen > 1.0f)
                {
                    const Vector2 dir = { fdx / flen, fdy / flen };
                    const Vector2 spawnPos = { m_testEnemy.position.x + dir.x * 24.0f,
                                               m_testEnemy.position.y + dir.y * 24.0f };
                    const Vector2 velocity = { dir.x * ENEMY_PROJ_SPEED, dir.y * ENEMY_PROJ_SPEED };
                    m_enemyProjectiles.emplace_back(spawnPos, velocity,
                                                    ENEMY1_PROJ_COLOR, ENEMY1_PROJ_DAMAGE,
                                                    ENEMY_PROJ_TRAIL_SCALE);
                }
            }
        }

        // 3e. Update front interceptor (intercept ahead of player; same rules)
        const Vector2 playerFacing = { std::cos(m_player.m_rotation), std::sin(m_player.m_rotation) };
        if (!m_testEnemy2.IsDestroyed())
        {
            m_testEnemy2.Update(dt, m_player.m_position, playerFacing, m_player.m_velocity, m_camera,
                                m_worldWidth, m_worldHeight,
                                m_player.m_isAccelerating, m_player.m_isDead, interAvoids,
                                m_testEnemy.position, !m_testEnemy.IsDestroyed());

            // Enemy Type 2 firing: one plasma orb per shot, faster than
            // Type 1 with a longer-lived trail for high-speed readability.
            if (m_testEnemy2.m_shotRequested)
            {
                m_testEnemy2.m_shotRequested = false;
                const float fdx = m_player.m_position.x - m_testEnemy2.position.x;
                const float fdy = m_player.m_position.y - m_testEnemy2.position.y;
                const float flen = std::hypot(fdx, fdy);
                if (flen > 1.0f)
                {
                    const Vector2 dir = { fdx / flen, fdy / flen };
                    const Vector2 spawnPos = { m_testEnemy2.position.x + dir.x * 24.0f,
                                               m_testEnemy2.position.y + dir.y * 24.0f };
                    const Vector2 velocity = { dir.x * ENEMY2_PROJ_SPEED, dir.y * ENEMY2_PROJ_SPEED };
                    m_enemyProjectiles.emplace_back(spawnPos, velocity,
                                                    ENEMY2_PROJ_COLOR, ENEMY2_PROJ_DAMAGE,
                                                    ENEMY2_PROJ_TRAIL_SCALE);
                }
            }
        }

        // 3f. Contact engagement ownership + contact damage for both enemies
        UpdateEnemyContacts();
    }

    // 3g. Respawn destroyed enemies after ENEMY_RESPAWN_DELAY seconds
    UpdateEnemyRespawns(dt);

    // 3g2. Bomber respawn after BOMBER_RESPAWN_DELAY seconds
    UpdateBomberRespawn(dt);

    // 3g3. Bomber bombs: fly, warn, explode, then finish safely
    UpdateBombs(dt);

    // 3h. Enemy projectiles: move, trail, expire, and damage the player
    UpdateEnemyProjectiles(dt);

    // 3i. Bomber: always-active slow wander — no camera/activation gating,
    // no chasing; it functions anywhere in the world, even off-screen.
    m_bomber.Update(dt, m_player.m_position, m_worldWidth, m_worldHeight,
                    m_player.m_isDead);

    // 3i2. Bomber attack: the Bomber flags a launch; Game computes a target
    // clamped inside the VISIBLE camera area ∩ world bounds and spawns the
    // Bomb. The bomb then lives independently in m_bombs — destroying the
    // Bomber never affects an in-flight bomb.
    if (!m_bomber.IsDestroyed() && m_bomber.m_bombRequested)
    {
        m_bomber.m_bombRequested = false;

        // Visible world rectangle (same formula used for enemy confinement):
        // camera.offset = half screen size; /zoom = visible half-extents.
        const float halfViewW = m_camera.offset.x / m_camera.zoom;
        const float halfViewH = m_camera.offset.y / m_camera.zoom;
        const float viewMinX = std::max(m_camera.target.x - halfViewW, 0.0f);
        const float viewMaxX = std::min(m_camera.target.x + halfViewW, m_worldWidth);
        const float viewMinY = std::max(m_camera.target.y - halfViewH, 0.0f);
        const float viewMaxY = std::min(m_camera.target.y + halfViewH, m_worldHeight);

        // Target = player's position clamped into the visible area — the
        // player is normally at view center, so this is a safety clamp.
        const Vector2 target = {
            std::max(viewMinX, std::min(m_player.m_position.x, viewMaxX)),
            std::max(viewMinY, std::min(m_player.m_position.y, viewMaxY))
        };

        m_bombs.emplace_back(m_bomber.position, target, BOMB_SPEED);
    }

    // Temporary debug damage (single press per hit, not per frame)
    if (IsKeyPressed(DEBUG_DAMAGE_KEY))
    {
        m_player.TakeDamage(DEBUG_DAMAGE_AMOUNT);
    }

    // Temporary respawn: R only works while the player is dead (single press)
    if (m_player.m_isDead && IsKeyPressed(KEY_R))
    {
        m_player.Respawn(m_camera);
        // Reposition both enemies inside the (incoming) camera view,
        // release any stale contact engagement, and restart the
        // activation delay before they resume pursuing.
        RepositionEnemiesInView();
        m_contactOwner = 0;
        m_enemyActivateTimer = ENEMY_ACTIVATION_DELAY;
    }

    // 4. Smoothly follow player's new position
    UpdateCamera(dt);
}

void Game::UpdateCamera(float dt)
{
    constexpr float CAMERA_FOLLOW_SPEED = 5.0f;
    const float cameraBlend = 1.0f - std::exp(-CAMERA_FOLLOW_SPEED * dt);

    // Read m_player.m_position directly through class friendship (no getters)
    m_camera.target.x += (m_player.m_position.x - m_camera.target.x) * cameraBlend;
    m_camera.target.y += (m_player.m_position.y - m_camera.target.y) * cameraBlend;
}

void Game::SpawnPlayerBullet()
{
    // Facing direction from the ship's rotation convention:
    // 0 radians = facing right, nose at +x, rear at -x.
    const Vector2 facing = { std::cos(m_player.m_rotation), std::sin(m_player.m_rotation) };

    // Back mode: spawn at rear, fire opposite facing.
    // Front mode: spawn at nose, fire along facing.
    const bool frontMode = m_player.m_frontShooting;
    const float spawnOffset = frontMode ? 24.0f : 20.0f; // nose vs rear offset
    const float dirSign = frontMode ? 1.0f : -1.0f;

    const Vector2 spawnPos = { m_player.m_position.x + facing.x * dirSign * spawnOffset,
                               m_player.m_position.y + facing.y * dirSign * spawnOffset };

    // Shooting velocity + player's current velocity captured at this moment
    // (includes acceleration velocity while boosting).
    const Vector2 shootDir = { facing.x * dirSign, facing.y * dirSign };
    const Vector2 bulletVelocity = { shootDir.x * BULLET_SPEED + m_player.m_velocity.x,
                                     shootDir.y * BULLET_SPEED + m_player.m_velocity.y };

    m_bullets.emplace_back(spawnPos, bulletVelocity);
}

void Game::UpdateBullets(float dt)
{
    for (Bullet& bullet : m_bullets)
    {
        bullet.Update(dt);
    }

    // Remove expired bullets without invalidating iteration
    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
        [](const Bullet& bullet) { return bullet.IsExpired(); }),
        m_bullets.end());
}

// ----------------------------------------------------------------------------
// Healing Stones
// ----------------------------------------------------------------------------

void Game::TrySpawnHealingStone()
{
    // Spawn rules scale with current player health (Game is a Player friend)
    const float health = m_player.m_health;

    if (health > 50.0f)
    {
        // Above 50%: no spawning. Re-check soon so spawning resumes
        // promptly when health drops to 50% or below.
        m_stoneSpawnTimer = HEALING_STONE_SPAWN_RECHECK_INTERVAL;
        return;
    }

    float interval = HEALING_STONE_SPAWN_INTERVAL_MID; // 31%-50%
    if (health <= 30.0f)
    {
        interval = HEALING_STONE_SPAWN_INTERVAL_LOW;
    }
    m_stoneSpawnTimer = interval;

    Vector2 spawnPos;
    if (TryFindStoneSpawnPos(spawnPos))
    {
        m_healingStones.emplace_back(spawnPos);
    }
    // If no valid spot was found, this spawn attempt is safely skipped
}

bool Game::TryFindStoneSpawnPos(Vector2& outPos) const
{
    // Visible world rectangle: camera.target is the smoothed view center,
    // zoom scales the visible half-extents around it.
    const float halfViewW = (m_screenWidth * 0.5f) / m_camera.zoom;
    const float halfViewH = (m_screenHeight * 0.5f) / m_camera.zoom;

    // Intersect the view rect with the playable world bounds
    const float minX = std::max(m_camera.target.x - halfViewW, 0.0f);
    const float maxX = std::min(m_camera.target.x + halfViewW, m_worldWidth);
    const float minY = std::max(m_camera.target.y - halfViewH, 0.0f);
    const float maxY = std::min(m_camera.target.y + halfViewH, m_worldHeight);

    if (maxX <= minX || maxY <= minY)
    {
        return false;
    }

    const float minDistSq = HEALING_STONE_MIN_PLAYER_DISTANCE * HEALING_STONE_MIN_PLAYER_DISTANCE;

    // Limited attempts: give up rather than risk an infinite loop
    for (int attempt = 0; attempt < HEALING_STONE_MAX_SPAWN_ATTEMPTS; ++attempt)
    {
        const float rx = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;
        const float ry = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;

        const float x = minX + rx * (maxX - minX);
        const float y = minY + ry * (maxY - minY);

        const float dx = x - m_player.m_position.x;
        const float dy = y - m_player.m_position.y;
        if (dx * dx + dy * dy >= minDistSq)
        {
            outPos = { x, y };
            return true;
        }
    }

    return false;
}

void Game::UpdateHealingStones(float dt)
{
    // While the player is dead the whole system freezes: no spawning,
    // no lifetime countdown, no collection. Stones keep their positions
    // and resume normally after respawn.
    if (m_player.m_isDead)
    {
        return;
    }

    // Spawn countdown; interval is re-evaluated from health at each spawn
    m_stoneSpawnTimer -= dt;
    if (m_stoneSpawnTimer <= 0.0f)
    {
        TrySpawnHealingStone();
    }

    // Age stones, handle collection, and remove expired/collected safely
    for (auto it = m_healingStones.begin(); it != m_healingStones.end(); )
    {
        it->Update(dt);

        if (it->IsExpired())
        {
            it = m_healingStones.erase(it);
            continue;
        }

        // Collection: stone radius + ship collision radius overlap
        if (it->Overlaps(m_player.m_position, SHIP_COLLISION_RADIUS))
        {
            m_player.Heal(HEALING_AMOUNT);
            it = m_healingStones.erase(it);
            continue;
        }

        ++it;
    }
}

void Game::Draw()
{
    BeginDrawing();

    // Blue space gradient backdrop (screen space, covers entire window)
    DrawRectangleGradientV(0, 0, m_screenWidth, m_screenHeight,
        Color{ 10, 25, 70, 255 },   // deep space blue at top
        Color{ 25, 70, 140, 255 }); // brighter horizon blue at bottom

    // --------------------------------------------------------------------
    // 1. World Space (Inside Camera2D Transformation)
    // --------------------------------------------------------------------
    BeginMode2D(m_camera);

    // Starfield
    for (const Star& star : m_stars)
    {
        DrawCircleV(star.position, star.radius, Color{ 190, 215, 255, star.alpha });
    }

    // World background grid
    constexpr int gridSize = 80;
    const Color gridColor = Color{ 45, 90, 150, 255 };

    for (int x = 0; x <= static_cast<int>(m_worldWidth); x += gridSize)
    {
        DrawLine(x, 0, x, static_cast<int>(m_worldHeight), gridColor);
    }
    for (int y = 0; y <= static_cast<int>(m_worldHeight); y += gridSize)
    {
        DrawLine(0, y, static_cast<int>(m_worldWidth), y, gridColor);
    }

    // World boundary border (red outline)
    DrawRectangleLinesEx(Rectangle{ 0.0f, 0.0f, m_worldWidth, m_worldHeight }, 4.0f, Fade(RED, 0.6f));

    // Mouse target reticle in world space
    const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), m_camera);
    DrawCircleLinesV(mouseWorldPos, 10.0f, Color{ 80, 160, 240, 140 });
    DrawCircleV(mouseWorldPos, 2.0f, Color{ 120, 200, 255, 200 });

    // Spaceship
    m_player.Draw();

    // Player projectiles
    for (const Bullet& bullet : m_bullets)
    {
        bullet.Draw();
    }

    // Healing stones
    for (const HealingStone& stone : m_healingStones)
    {
        stone.Draw();
    }

    // Test enemy (world space — moves correctly with the camera)
    if (!m_testEnemy.IsDestroyed())
    {
        m_testEnemy.Draw();
    }

    // Front interceptor (orange placeholder)
    if (!m_testEnemy2.IsDestroyed())
    {
        m_testEnemy2.Draw();
    }

    // Bomber (purple triangle placeholder)
    if (!m_bomber.IsDestroyed())
    {
        m_bomber.Draw();
    }

    // Enemy projectiles (world space — trails + heads)
    for (const EnemyProjectile& proj : m_enemyProjectiles)
    {
        proj.Draw();
    }

    // Bomber bombs (world space — glowing orbs in flight)
    for (const Bomb& bomb : m_bombs)
    {
        bomb.Draw();
    }

    EndMode2D();

    // --------------------------------------------------------------------
    // 2. Screen Space (HUD / UI Outside Camera Mode)
    // --------------------------------------------------------------------
    DrawText("SPACEWAR", 20, 20, 20, RAYWHITE);
    DrawFPS(m_screenWidth - 100, 20);

    // HUD: score + per-type kill counters (screen space, top-right)
    DrawText(TextFormat("SCORE: %06d", m_score), m_screenWidth - 170, 116, 18, YELLOW);
    DrawText(TextFormat("T1: %d", m_t1Kills), m_screenWidth - 170, 142, 14, LIGHTGRAY);
    DrawText(TextFormat("T2: %d", m_t2Kills), m_screenWidth - 170, 162, 14, LIGHTGRAY);

    // HUD: player health bar (fixed screen position, drawn by Player)
    m_player.DrawHealthBar();
    m_player.DrawEnergyBar();

    // HUD: current shooting mode (screen space, updates instantly on S)
    DrawText(TextFormat("SHOOTING MODE: %s", m_player.m_frontShooting ? "FRONT" : "BACK"),
             20, 116, 16, m_player.m_frontShooting ? YELLOW : LIGHTGRAY);

    // HUD: controls guide (update this list when bindings change)
    DrawText("CONTROLS", 20, 260, 18, RAYWHITE);
    DrawText("Mouse             - Move & aim",               20, 286, 14, GRAY);
    DrawText("Left Click        - Toggle Shooting",          20, 304, 14, GRAY);
    DrawText("Left Double-Click - Activate Acceleration",    20, 322, 14, GRAY);
    DrawText("Space             - Accelerate (energy)",      20, 340, 14, GRAY);
    DrawText("S                 - Switch Shooting Mode",     20, 358, 14, GRAY);
    DrawText("F1                - Debug damage (temporary)",20, 376, 14, GRAY);
    if (m_player.m_isDead)
    {
        DrawText("R                - Respawn (temporary)",   20, 394, 14, YELLOW);
    }

    // GAME OVER overlay — visible only while the player is dead
    if (m_player.m_isDead)
    {
        constexpr const char* gameOverText = "GAME OVER";
        constexpr int fontSize = 64;
        const int textWidth = MeasureText(gameOverText, fontSize);
        DrawText(gameOverText,
                 (m_screenWidth - textWidth) / 2,
                 m_screenHeight / 2 - fontSize / 2,
                 fontSize, RED);
    }

    EndDrawing();
}

// ----------------------------------------------------------------------------
// Enemy (Re)spawn Positioning
// ----------------------------------------------------------------------------

// Finds a random spawn position inside the camera view (centered on
// viewCenter) intersected with world bounds. Avoids the player and any
// positions in avoidPos by at least ENEMY_SPAWN_MIN_* distances.
bool Game::TryFindEnemySpawnPos(Vector2 viewCenter, const std::vector<Vector2>& avoidPos,
                                Vector2& outPos) const
{
    // camera.offset = half the screen size; /zoom gives visible half-extents
    const float halfViewW = m_camera.offset.x / m_camera.zoom;
    const float halfViewH = m_camera.offset.y / m_camera.zoom;

    const float minX = std::max(viewCenter.x - halfViewW, ENEMY_SPAWN_VIEW_MARGIN);
    const float maxX = std::min(viewCenter.x + halfViewW, m_worldWidth - ENEMY_SPAWN_VIEW_MARGIN);
    const float minY = std::max(viewCenter.y - halfViewH, ENEMY_SPAWN_VIEW_MARGIN);
    const float maxY = std::min(viewCenter.y + halfViewH, m_worldHeight - ENEMY_SPAWN_VIEW_MARGIN);

    if (maxX <= minX || maxY <= minY)
    {
        return false;
    }

    const float minPlayerDistSq = ENEMY_SPAWN_MIN_PLAYER_DIST * ENEMY_SPAWN_MIN_PLAYER_DIST;
    const float minEnemyDistSq  = ENEMY_SPAWN_MIN_ENEMY_DIST * ENEMY_SPAWN_MIN_ENEMY_DIST;

    for (int attempt = 0; attempt < ENEMY_SPAWN_MAX_ATTEMPTS; ++attempt)
    {
        const float rx = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;
        const float ry = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;

        const float x = minX + rx * (maxX - minX);
        const float y = minY + ry * (maxY - minY);

        const float pdx = x - m_player.m_position.x;
        const float pdy = y - m_player.m_position.y;
        if (pdx * pdx + pdy * pdy < minPlayerDistSq)
        {
            continue;
        }

        bool tooClose = false;
        for (const Vector2& avoid : avoidPos)
        {
            const float adx = x - avoid.x;
            const float ady = y - avoid.y;
            if (adx * adx + ady * ady < minEnemyDistSq)
            {
                tooClose = true;
                break;
            }
        }
        if (tooClose)
        {
            continue;
        }

        outPos = { x, y };
        return true;
    }

    return false;
}

void Game::RepositionEnemiesInView()
{
    // Reposition the EXISTING two enemies at independent random positions
    // inside the visible camera area. Uses the player's position as the
    // view center — the camera converges there (at Init they coincide).
    const Vector2 viewCenter = m_player.m_position;
    std::vector<Vector2> used;
    Vector2 pos;

    // Destroyed enemies are NOT revived or repositioned — they stay gone
    if (!m_testEnemy.IsDestroyed() &&
        TryFindEnemySpawnPos(viewCenter, used, pos))
    {
        m_testEnemy.ResetTo(pos);
        used.push_back(pos);
    }
    if (!m_testEnemy2.IsDestroyed() &&
        TryFindEnemySpawnPos(viewCenter, used, pos))
    {
        m_testEnemy2.ResetTo(pos);
        used.push_back(pos);
    }
}

// ----------------------------------------------------------------------------
// Exclusive Contact Engagement + Contact Damage
// ----------------------------------------------------------------------------

void Game::UpdateEnemyContacts()
{
    if (m_player.m_isDead)
    {
        m_contactOwner = 0; // release lock while dead
        return;
    }

    // Destroyed enemies cannot touch the player or hold the engagement
    const bool touch1 = !m_testEnemy.IsDestroyed() &&
        m_testEnemy.Touches(m_player.m_position, SHIP_COLLISION_RADIUS);
    const bool touch2 = !m_testEnemy2.IsDestroyed() &&
        m_testEnemy2.Touches(m_player.m_position, SHIP_COLLISION_RADIUS);

    // Acquire: first toucher wins when nobody owns contact
    if (m_contactOwner == 0)
    {
        if (touch1)
        {
            m_contactOwner = 1;
        }
        else if (touch2)
        {
            m_contactOwner = 2;
        }
    }
    // Release: owner stopped touching -> engagement ends
    else if ((m_contactOwner == 1 && !touch1) ||
             (m_contactOwner == 2 && !touch2))
    {
        m_contactOwner = 0;
    }

    // Contact damage through the player's general damage path
    // (invulnerability prevents multi-hit during the same overlap)
    if (touch1)
    {
        m_player.TakeDamage(ENEMY_CONTACT_DAMAGE);
    }
    if (touch2)
    {
        m_player.TakeDamage(ENEMY_CONTACT_DAMAGE);
    }
}

// ----------------------------------------------------------------------------
// Bullet vs Enemy Combat
// ----------------------------------------------------------------------------

void Game::CheckBulletEnemyHits()
{
    for (Bullet& bullet : m_bullets)
    {
        if (bullet.IsExpired())
        {
            continue;
        }

        // Bullet world center from its collision bounds (no position getter)
        const Rectangle bounds = bullet.GetBounds();
        const Vector2 bulletCenter = { bounds.x + bounds.width * 0.5f,
                                       bounds.y + bounds.height * 0.5f };

        // Find which enemy this bullet actually overlaps. When both enemies
        // overlap the bullet (they can overlap each other), the CLOSEST one
        // takes the hit — otherwise Type 1 would always soak hits meant for
        // Type 2 and kills would be misattributed.
        int hitKind = 0; // 0 = none, 1 = Type 1, 2 = Type 2
        float bestDist = 1.0e9f;

        if (!m_testEnemy.IsDestroyed() &&
            m_testEnemy.Touches(bulletCenter, BULLET_RADIUS))
        {
            hitKind = 1;
            bestDist = m_testEnemy.DistanceTo(bulletCenter);
        }
        if (!m_testEnemy2.IsDestroyed() &&
            m_testEnemy2.Touches(bulletCenter, BULLET_RADIUS))
        {
            const float d = m_testEnemy2.DistanceTo(bulletCenter);
            if (d < bestDist)
            {
                hitKind = 2;
                bestDist = d;
            }
        }
        // Bomber is a valid bullet target too (hitKind 3); like the others
        // it takes damage, and a killing blow awards the Type 3 bonus.
        if (!m_bomber.IsDestroyed() &&
            m_bomber.Touches(bulletCenter, BULLET_RADIUS))
        {
            const float d = m_bomber.DistanceTo(bulletCenter);
            if (d < bestDist)
            {
                hitKind = 3;
                bestDist = d;
            }
        }

        // A bullet damages at most ONE enemy, then is destroyed
        if (hitKind == 1)
        {
            m_testEnemy.TakeDamage(ENEMY_BULLET_DAMAGE);
            bullet.Destroy();
            // Killing blow: award score + kill counter exactly once
            // (destroyed enemies are skipped by future hit checks)
            if (m_testEnemy.IsDestroyed())
            {
                m_score += ENEMY1_KILL_SCORE;
                ++m_t1Kills;
            }
        }
        else if (hitKind == 2)
        {
            m_testEnemy2.TakeDamage(ENEMY_BULLET_DAMAGE);
            bullet.Destroy();
            if (m_testEnemy2.IsDestroyed())
            {
                m_score += ENEMY2_KILL_SCORE;
                ++m_t2Kills;
            }
        }
        else if (hitKind == 3)
        {
            m_bomber.TakeDamage(ENEMY_BULLET_DAMAGE);
            bullet.Destroy();
            // Killing blow: award the Bomber bonus exactly once
            // (destroyed enemies are skipped by future hit checks)
            if (m_bomber.IsDestroyed())
            {
                m_score += ENEMY3_KILL_SCORE;
            }
        }
    }
}

void Game::UpdateEnemyProjectiles(float dt)
{
    for (EnemyProjectile& proj : m_enemyProjectiles)
    {
        proj.Update(dt, m_worldWidth, m_worldHeight);
    }

    // Player collision — skipped while dead; damage flows through the
    // existing TakeDamage path so invulnerability still applies.
    if (!m_player.m_isDead)
    {
        for (EnemyProjectile& proj : m_enemyProjectiles)
        {
            if (!proj.IsExpired() &&
                proj.Touches(m_player.m_position, SHIP_COLLISION_RADIUS))
            {
                proj.ApplyToPlayer(m_player); // projectile carries its own damage
                proj.Destroy();
            }
        }
    }

    // Remove expired shots without invalidating iteration
    m_enemyProjectiles.erase(std::remove_if(m_enemyProjectiles.begin(), m_enemyProjectiles.end(),
        [](const EnemyProjectile& proj) { return proj.IsExpired(); }),
        m_enemyProjectiles.end());
}

// ----------------------------------------------------------------------------
// Enemy Respawn After Destruction
// ----------------------------------------------------------------------------

void Game::UpdateEnemyRespawns(float dt)
{
    // Type 1: countdown while destroyed, then respawn inside the view
    if (m_testEnemy.IsDestroyed())
    {
        m_enemy1RespawnTimer -= dt;
        if (m_enemy1RespawnTimer <= 0.0f)
        {
            for (int attempt = 0; attempt < 10; ++attempt)
            {
                Vector2 pos;
                if (!TryFindEnemySpawnPos(m_camera.target, {}, pos))
                {
                    break; // no valid spot — retry next frame
                }
                // Keep separation from the other enemy when it's alive
                if (!m_testEnemy2.IsDestroyed() &&
                    m_testEnemy2.DistanceTo(pos) < ENEMY_SPAWN_MIN_ENEMY_DIST)
                {
                    continue;
                }
                m_testEnemy.RespawnAt(pos);
                m_enemy1RespawnTimer = ENEMY_RESPAWN_DELAY;
                break;
            }
        }
    }
    else
    {
        m_enemy1RespawnTimer = ENEMY_RESPAWN_DELAY; // re-arm for next death
    }

    // Type 2: same rules
    if (m_testEnemy2.IsDestroyed())
    {
        m_enemy2RespawnTimer -= dt;
        if (m_enemy2RespawnTimer <= 0.0f)
        {
            for (int attempt = 0; attempt < 10; ++attempt)
            {
                Vector2 pos;
                if (!TryFindEnemySpawnPos(m_camera.target, {}, pos))
                {
                    break;
                }
                if (!m_testEnemy.IsDestroyed() &&
                    m_testEnemy.DistanceTo(pos) < ENEMY_SPAWN_MIN_ENEMY_DIST)
                {
                    continue;
                }
                m_testEnemy2.RespawnAt(pos);
                m_enemy2RespawnTimer = ENEMY_RESPAWN_DELAY;
                break;
            }
        }
    }
    else
    {
        m_enemy2RespawnTimer = ENEMY_RESPAWN_DELAY;
    }
}

// ----------------------------------------------------------------------------
// Bomber (Type 3) Spawn & Respawn
// ----------------------------------------------------------------------------

// Finds a random spawn position ANYWHERE inside the world bounds (inset by
// BOMBER_SPAWN_INSET), at least BOMBER_MIN_PLAYER_DIST from the player.
// Unlike Types 1/2 there is no camera-view restriction — the Bomber is
// meant to be discovered by exploration, not spawned on-screen.
bool Game::TryFindBomberSpawnPos(Vector2& outPos) const
{
    const float minX = BOMBER_SPAWN_INSET;
    const float minY = BOMBER_SPAWN_INSET;
    const float maxX = m_worldWidth - BOMBER_SPAWN_INSET;
    const float maxY = m_worldHeight - BOMBER_SPAWN_INSET;

    const float minPlayerDistSq = BOMBER_MIN_PLAYER_DIST * BOMBER_MIN_PLAYER_DIST;

    for (int attempt = 0; attempt < BOMBER_SPAWN_MAX_ATTEMPTS; ++attempt)
    {
        const float rx = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;
        const float ry = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;

        const float x = minX + rx * (maxX - minX);
        const float y = minY + ry * (maxY - minY);

        const float dx = x - m_player.m_position.x;
        const float dy = y - m_player.m_position.y;
        if (dx * dx + dy * dy >= minPlayerDistSq)
        {
            outPos = { x, y };
            return true;
        }
    }

    return false;
}

void Game::UpdateBomberRespawn(float dt)
{
    // Same lifecycle pattern as Types 1/2, but the replacement spawns at a
    // random WORLD position (not the camera view) after the longer delay.
    if (m_bomber.IsDestroyed())
    {
        m_bomberRespawnTimer -= dt;
        if (m_bomberRespawnTimer <= 0.0f)
        {
            for (int attempt = 0; attempt < 10; ++attempt)
            {
                Vector2 pos;
                if (!TryFindBomberSpawnPos(pos))
                {
                    break; // no valid spot — retry next frame
                }
                m_bomber.RespawnAt(pos);
                m_bomberRespawnTimer = BOMBER_RESPAWN_DELAY;
                break;
            }
        }
    }
    else
    {
        m_bomberRespawnTimer = BOMBER_RESPAWN_DELAY; // re-arm for next death
    }
}

void Game::UpdateBombs(float dt)
{
    // Bombs are independent entities: they keep flying toward their locked
    // targets even if the Bomber that launched them is destroyed.
    for (Bomb& bomb : m_bombs)
    {
        bomb.Update(dt);

        // Detonation: the bomb flags its explosion exactly once. Game owns
        // the blast check — if the player is inside the radius AT THAT
        // MOMENT, apply damage through the existing TakeDamage path
        // (invulnerability/death rules stay inside Player). No damage
        // during flight, warning, or the explosion animation itself.
        if (bomb.ConsumeDetonation() && bomb.IsInsideBlast(m_player.m_position))
        {
            m_player.TakeDamage(BOMB_EXPLOSION_DAMAGE);
        }
    }

    // Remove finished bombs (explosion animation complete or lifetime expired)
    m_bombs.erase(std::remove_if(m_bombs.begin(), m_bombs.end(),
        [](const Bomb& bomb) { return bomb.IsFinished(); }),
        m_bombs.end());
}
