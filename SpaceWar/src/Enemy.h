// Enemy.h
// Purpose: Defines the Enemy class — a hostile spaceship.
//
// Milestone 2: random wandering until the enemy enters the camera's
// visible area, then permanent chasing of the player. Movement only —
// shooting, health, and collision come in later milestones.
//
// Attributes:
// - position: World-space (x, y) of the enemy.
// - radius: Collision/draw radius of the placeholder shape.
// - m_destination: current random wander target.
// - m_isChasing: latched true once the enemy has entered the camera view.
//
// Methods:
// - Update(...): wander/chase movement, detection, boundary steering.
// - Draw(): Renders the enemy placeholder in world space.

#pragma once

#include "raylib.h"

// ----------------------------------------------------------------------------
// Enemy Movement Tuning (Milestone 2)
// ----------------------------------------------------------------------------
constexpr float ENEMY_SPEED_NORMAL = 320.0f;        // px/s — normal movement
constexpr float ENEMY_SPEED_PLAYER_ACCEL = 450.0f;  // px/s — while player accelerates
constexpr float ENEMY_ARRIVE_DISTANCE = 24.0f;  // px — pick a new wander target
constexpr float ENEMY_BOUNDARY_MARGIN = 60.0f;  // px — inward-steer zone at edges
constexpr float ENEMY_BOUNDARY_STEER = 1.5f;    // inward bias strength near edges
constexpr float ENEMY_CONTACT_AVOID_DISTANCE = 300.0f; // px — non-engaged enemy backs away this far
constexpr float ENEMY_MAX_HEALTH = 5.0f;         // HP — destroyed at 0

// Corner/edge awareness: when the player nears a world boundary, enemies
// hold a standoff ring instead of charging into the unreachable corner.
constexpr float ENEMY_CORNER_MARGIN = 300.0f;        // px — player this close to an edge triggers it
constexpr float ENEMY_HOLD_MIN_DIST = 100.0f;        // px — never closer than this
constexpr float ENEMY_HOLD_PREFERRED_DIST = 140.0f;  // px — ring distance enemies aim for
constexpr float ENEMY_HOLD_MAX_DIST = 200.0f;        // px — approach allowed up to this
constexpr float ENEMY_HOLD_HYSTERESIS = 60.0f;       // px — band exit margin prevents threshold flutter
constexpr float ENEMY_HOLD_DRIFT_SPEED = 80.0f;      // px/s — gentle drift while holding station

// Enemy-to-enemy separation: never overlap; soft steer zone + hard min gap.
constexpr float ENEMY_SEPARATION_MIN_DIST = 56.0f;   // px — hard minimum gap (2x radius + margin)
constexpr float ENEMY_SEPARATION_SOFT_DIST = 110.0f; // px — steering bias zone
constexpr float ENEMY_SEPARATION_STEER = 1.5f;       // repulsion bias strength
constexpr float ENEMY_SEPARATION_RESOLVE_SPEED = 300.0f; // px/s — smooth gap correction cap

// Ranged attack: moderately fast plasma orbs, one at a time, rapid fire.
constexpr float ENEMY_FIRE_INTERVAL = 0.8f;    // seconds between shots
constexpr float ENEMY_PROJ_SPEED   = 640.0f;  // px/s — must catch a fleeing player
constexpr float ENEMY_PROJ_TRAIL_SCALE = 1.0f; // trail length identity (Type 1)

class Enemy
{
    // Grant Game access to position for mutual enemy separation (same
    // friendship pattern as Player — no getters).
    friend class Game;

private:
    Vector2 position;
    float   radius;
    Vector2 m_destination;   // current random wander target
    bool    m_isChasing;     // true once the enemy has entered the camera view
    float   m_health;        // HP — enemy is destroyed at 0
    bool    m_cornerHolding; // true while holding station inside the corner distance band
    float   m_fireTimer;     // seconds until the next shot is ready
    bool    m_shotRequested; // one-frame flag: Game spawns a projectile when set

    // Picks a new random destination inside the given bounds
    void PickNewDestination(float minX, float minY, float maxX, float maxY);

public:
    Enemy(Vector2 spawnPos, float size = 20.0f);

    void Update(float dt, Vector2 playerPos, const Camera2D& camera,
                float worldWidth, float worldHeight,
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
