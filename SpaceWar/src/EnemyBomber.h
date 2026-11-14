// EnemyBomber.h
// Purpose: Defines Enemy Type 3 — The Bomber.
//
// A slow, deliberate wanderer with a different role from Types 1 and 2:
// it roams the WHOLE world (never tied to the camera view), keeps a large
// standoff distance from the player, and never attempts contact.
// Bomb attacks: one at a time on a cooldown while the player is in range —
// Game turns the launch flag into an independent Bomb entity.
//
// Attributes:
// - position, radius: world-space placeholder shape.
// - m_destination: current random wander target.
// - m_health: HP — destroyed at 0 (same contract as Types 1 and 2).
//
// Methods:
// - Update(dt, playerPos, worldW, worldH): wander + keep-away + bounds.
// - Draw(): purple triangle placeholder (distinct from red/orange circles).
// - Touches / TakeDamage / IsDestroyed / DistanceTo: same API as other enemies.

#pragma once

#include "raylib.h"

// ----------------------------------------------------------------------------
// Bomber Tuning (Phase 1 — movement & health only)
// ----------------------------------------------------------------------------
constexpr float ENEMY3_SPEED = 120.0f;          // px/s — slow, deliberate
constexpr float ENEMY3_ARRIVE_DISTANCE = 30.0f; // px — pick a new wander target
constexpr float ENEMY3_KEEP_MIN_DIST = 550.0f;  // px — ease away inside this range
constexpr float ENEMY3_KEEPAWAY_STRENGTH = 0.9f;// repulsion bias strength
constexpr float ENEMY3_BOUNDARY_MARGIN = 60.0f; // px — inward-steer zone at edges
constexpr float ENEMY3_BOUNDARY_STEER = 1.5f;   // inward bias strength near edges
constexpr float ENEMY3_MAX_HEALTH = 8.0f;       // HP — destroyed at 0

// Bomb attack (Phase 4): one bomb at a time on a cooldown, only while the
// player is alive and inside attack range. Game turns the flag into a Bomb.
constexpr float ENEMY3_ATTACK_INTERVAL = 3.5f;  // seconds between bomb launches
constexpr float ENEMY3_ATTACK_RANGE    = 900.0f;// px — max distance to the player

class EnemyBomber
{
    // Grant Game access to position for spawn separation checks (same
    // friendship pattern as Player/Enemy — no getters).
    friend class Game;

private:
    Vector2 position;
    float   radius;
    Vector2 m_destination; // current random wander target
    float   m_health;      // HP — enemy is destroyed at 0
    float   m_attackTimer;   // seconds until the next bomb launch is ready
    bool    m_bombRequested; // one-frame flag: Game spawns a Bomb when set

    // Picks a new random destination inside the given world bounds
    void PickNewDestination(float minX, float minY, float maxX, float maxY);

public:
    EnemyBomber(Vector2 spawnPos, float size = 22.0f);

    // dt-scaled wander. playerPos is used ONLY for keep-away steering —
    // the Bomber never chases and works identically off-camera.
    void Update(float dt, Vector2 playerPos, float worldWidth, float worldHeight,
                bool playerDead);
    void Draw() const;

    // Contact/distance queries used by Game (no position getters)
    bool  Touches(Vector2 otherPos, float otherRadius) const;
    float DistanceTo(Vector2 worldPos) const;

    // Health: bullet damage entry point + destruction query
    void TakeDamage(float damageAmount);
    bool IsDestroyed() const;

    // Reposition for initial world spawn (Phase 2): new position + fresh
    // wander destination. Health and state are preserved.
    void ResetTo(Vector2 spawnPos);

    // Full respawn after destruction (Phase 3): fresh position, restored
    // health, and a clean wander state — behaves like a new Bomber.
    void RespawnAt(Vector2 spawnPos);
};
