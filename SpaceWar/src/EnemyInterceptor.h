// EnemyInterceptor.h
// Purpose: Enemy Type 2 — Front Interceptor.
//
// Tries to position itself AHEAD of the player's movement direction to
// obstruct them, unlike Enemy Type 1 which chases from any direction.
// Shares the same wander/detection/death rules as Enemy Type 1.
//
// Attributes:
// - position, radius: world-space placeholder shape.
// - m_destination: wander target (used before detection and while player dead).
// - m_sideOffset: ±1 lateral offset for the intercept point (re-rolled on arrival).
// - m_isChasing: latched once the enemy enters the camera view.
//
// Methods:
// - Update(...): detection, interception, wandering, boundary steering.
// - Draw(): orange placeholder circle.
// - Touches(...): contact test for player damage.

#pragma once

#include "raylib.h"

// ----------------------------------------------------------------------------
// Front Interceptor Tuning
// ----------------------------------------------------------------------------
constexpr float ENEMY2_SPEED_NORMAL = 360.0f;        // px/s
constexpr float ENEMY2_SPEED_PLAYER_ACCEL = 540.0f;  // px/s while player accelerates
constexpr float ENEMY2_INTERCEPT_DISTANCE = 220.0f;  // px ahead of player to hold
constexpr float ENEMY2_SIDE_DISTANCE = 120.0f;       // px lateral offset range
constexpr float ENEMY2_ARRIVE_DISTANCE = 40.0f;      // px — re-roll side offset
constexpr float ENEMY2_WANDER_ARRIVE = 24.0f;        // px — new wander destination
constexpr float ENEMY2_CONFRONT_RANGE = 500.0f;     // px — react to direct player approach
constexpr float ENEMY2_CONFRONT_DOT = 0.6f;         // min alignment to count as head-on
constexpr float ENEMY2_BOUNDARY_MARGIN = 60.0f;
constexpr float ENEMY2_BOUNDARY_STEER = 1.5f;
constexpr float ENEMY2_CONTACT_AVOID_DISTANCE = 300.0f; // px — non-engaged enemy backs away this far
constexpr float ENEMY2_MAX_HEALTH = 9.0f;         // HP — destroyed at 0

// Corner/edge awareness: same standoff ring as Enemy Type 1.
constexpr float ENEMY2_CORNER_MARGIN = 300.0f;
constexpr float ENEMY2_HOLD_MIN_DIST = 100.0f;
constexpr float ENEMY2_HOLD_PREFERRED_DIST = 140.0f;
constexpr float ENEMY2_HOLD_MAX_DIST = 200.0f;
constexpr float ENEMY2_HOLD_HYSTERESIS = 60.0f;
constexpr float ENEMY2_HOLD_DRIFT_SPEED = 80.0f;

// Enemy-to-enemy separation: same rules as Enemy Type 1.
constexpr float ENEMY2_SEPARATION_MIN_DIST = 56.0f;
constexpr float ENEMY2_SEPARATION_SOFT_DIST = 110.0f;
constexpr float ENEMY2_SEPARATION_STEER = 1.5f;
constexpr float ENEMY2_SEPARATION_RESOLVE_SPEED = 300.0f;

// Ranged attack: very fast plasma orbs, one at a time, rapid fire.
constexpr float ENEMY2_FIRE_INTERVAL = 0.55f;   // seconds between shots
constexpr float ENEMY2_PROJ_SPEED   = 800.0f; // px/s — noticeably faster than Type 1
constexpr float ENEMY2_PROJ_TRAIL_SCALE = 1.3f; // longer trail stays visible at speed

class EnemyInterceptor
{
    // Grant Game access to position for mutual enemy separation (no getters).
    friend class Game;

private:
    Vector2 position;
    float   radius;
    Vector2 m_destination;
    float   m_sideOffset;   // -1 or +1: which side of the player's path to hold
    bool    m_isChasing;
    float   m_health;        // HP — enemy is destroyed at 0
    bool    m_cornerHolding; // true while holding station inside the corner distance band
    float   m_fireTimer;     // seconds until the next shot is ready
    bool    m_shotRequested; // one-frame flag: Game spawns a projectile when set

    void PickNewDestination(float minX, float minY, float maxX, float maxY);

public:
    EnemyInterceptor(Vector2 spawnPos, float size = 20.0f);

    void Update(float dt, Vector2 playerPos, Vector2 playerFacing, Vector2 playerVelocity,
                const Camera2D& camera, float worldWidth, float worldHeight,
                bool playerAccelerating, bool playerDead, bool avoidContact,
                Vector2 otherEnemyPos, bool otherEnemyActive);
    void Draw() const;

    // Contact test used by Game for player damage (no position getters)
    bool Touches(Vector2 otherPos, float otherRadius) const;

    // Health: bullet damage entry point + destruction query
    void TakeDamage(float damageAmount);
    bool IsDestroyed() const;

    // Reposition for (re)spawning inside the camera view
    void ResetTo(Vector2 spawnPos);

    // Full respawn after destruction: position + restored health + fresh AI
    void RespawnAt(Vector2 spawnPos);

    // Distance query used by Game for spawn separation (no position getter)
    float DistanceTo(Vector2 worldPos) const;
};
