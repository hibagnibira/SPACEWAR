// Enemy.cpp
// Purpose: Implements the methods declared in Enemy.h.

#include "Enemy.h"
#include <algorithm>
#include <cmath>

Enemy::Enemy(Vector2 spawnPos, float size)
    : position(spawnPos)
    , radius(size)
    , m_destination(spawnPos)   // first Update() immediately picks a real one
    , m_isChasing(false)
    , m_health(ENEMY_MAX_HEALTH)
    , m_cornerHolding(false)
    , m_fireTimer(ENEMY_FIRE_INTERVAL)
    , m_shotRequested(false)
{
}

// ----------------------------------------------------------------------------
// PickNewDestination
// Random point inside the given bounds (world area or camera view).
// ----------------------------------------------------------------------------
void Enemy::PickNewDestination(float minX, float minY, float maxX, float maxY)
{
    const float rx = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;
    const float ry = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;

    m_destination = {
        minX + rx * (maxX - minX),
        minY + ry * (maxY - minY)
    };
}

// ----------------------------------------------------------------------------
// Update
// Wander until the enemy enters the camera's visible world area, then
// chase the player permanently. All movement uses delta time.
// ----------------------------------------------------------------------------
void Enemy::Update(float dt, Vector2 playerPos, const Camera2D& camera,
                   float worldWidth, float worldHeight,
                   bool playerAccelerating, bool playerDead, bool avoidContact,
                   Vector2 otherEnemyPos, bool otherEnemyActive)
{
    if (dt <= 0.0f || IsDestroyed()) return;

    // --- Player detection: latches permanently on first entry into view ---
    // While the player is dead, chasing is suspended entirely: the latch
    // resets so enemies wander and must re-detect after respawn.
    if (playerDead)
    {
        m_isChasing = false;
    }
    else if (!m_isChasing)
    {
        // camera.offset = screen size / 2, so offset/zoom = visible half extents
        const float halfViewW = camera.offset.x / camera.zoom;
        const float halfViewH = camera.offset.y / camera.zoom;

        const bool inView =
            position.x >= camera.target.x - halfViewW &&
            position.x <= camera.target.x + halfViewW &&
            position.y >= camera.target.y - halfViewH &&
            position.y <= camera.target.y + halfViewH;

        if (inView)
        {
            m_isChasing = true;
        }
    }

    // --- Firing cooldown: only while chasing a live player. Game reads
    // m_shotRequested (friendship) and spawns the actual projectile. ---
    if (m_isChasing && !playerDead)
    {
        m_fireTimer -= dt;
        if (m_fireTimer <= 0.0f)
        {
            const float fdx = playerPos.x - position.x;
            const float fdy = playerPos.y - position.y;
            if (std::hypot(fdx, fdy) > 1.0f)
            {
                m_shotRequested = true;
            }
            m_fireTimer = ENEMY_FIRE_INTERVAL;
        }
    }

    // --- Choose this frame's movement target ---
    Vector2 target;
    if (m_isChasing)
    {
        target = playerPos; // continuously track the player's live position

        // Exclusive contact engagement: while the OTHER enemy owns contact,
        // keep backing away from the player for the duration of the
        // engagement, then normal AI resumes automatically.
        if (avoidContact)
        {
            const float adx = position.x - playerPos.x;
            const float ady = position.y - playerPos.y;
            const float adist = std::hypot(adx, ady);
            if (adist > 0.001f)
            {
                target = {
                    position.x + (adx / adist) * ENEMY_CONTACT_AVOID_DISTANCE,
                    position.y + (ady / adist) * ENEMY_CONTACT_AVOID_DISTANCE
                };
            }
        }

        // Corner/edge awareness: distance band with hysteresis instead of a
        // shared ring attractor. Far -> approach to preferred distance;
        // inside [MIN, MAX] -> hold THIS enemy's own position (no shared
        // target for separation to fight — fixes wall-corner shaking);
        // too close -> ease away to the minimum. Hysteresis keeps the hold
        // until the player clearly leaves the band. Boundary steering and
        // enemy separation still apply while holding, so the enemy never
        // gets stuck against a wall or inside another enemy.
        const bool playerNearEdge =
            playerPos.x < ENEMY_CORNER_MARGIN ||
            playerPos.x > worldWidth - ENEMY_CORNER_MARGIN ||
            playerPos.y < ENEMY_CORNER_MARGIN ||
            playerPos.y > worldHeight - ENEMY_CORNER_MARGIN;
        if (playerNearEdge)
        {
            const float rdx = position.x - playerPos.x;
            const float rdy = position.y - playerPos.y;
            const float rdist = std::hypot(rdx, rdy);
            if (rdist > 0.001f)
            {
                if (m_cornerHolding)
                {
                    if (rdist > ENEMY_HOLD_MAX_DIST + ENEMY_HOLD_HYSTERESIS ||
                        rdist < ENEMY_HOLD_MIN_DIST - ENEMY_HOLD_HYSTERESIS)
                    {
                        m_cornerHolding = false; // clearly outside band -> resume approach/retreat
                    }
                    else
                    {
                        target = position; // hold station
                    }
                }
                if (!m_cornerHolding)
                {
                    if (rdist > ENEMY_HOLD_MAX_DIST)
                    {
                        // Case A: approach to the preferred standoff distance
                        const float scale = ENEMY_HOLD_PREFERRED_DIST / rdist;
                        target = {
                            playerPos.x + rdx * scale,
                            playerPos.y + rdy * scale
                        };
                    }
                    else if (rdist < ENEMY_HOLD_MIN_DIST)
                    {
                        // Case C: too close -> ease away to the minimum distance
                        const float scale = ENEMY_HOLD_MIN_DIST / rdist;
                        target = {
                            playerPos.x + rdx * scale,
                            playerPos.y + rdy * scale
                        };
                    }
                    else
                    {
                        // Case B: inside the band -> hold station
                        m_cornerHolding = true;
                        target = position;
                    }
                }
            }
        }
    }
    else
    {
        // Destination bounds: whole world normally; camera-visible area
        // (clamped to the world) while the player is dead.
        float minX = ENEMY_BOUNDARY_MARGIN + radius;
        float minY = ENEMY_BOUNDARY_MARGIN + radius;
        float maxX = worldWidth - ENEMY_BOUNDARY_MARGIN - radius;
        float maxY = worldHeight - ENEMY_BOUNDARY_MARGIN - radius;

        if (playerDead)
        {
            const float halfViewW = camera.offset.x / camera.zoom;
            const float halfViewH = camera.offset.y / camera.zoom;

            minX = std::max(minX, camera.target.x - halfViewW);
            maxX = std::min(maxX, camera.target.x + halfViewW);
            minY = std::max(minY, camera.target.y - halfViewH);
            maxY = std::min(maxY, camera.target.y + halfViewH);

            // Degenerate case (view fully outside world): fall back to world
            if (maxX <= minX) { minX = ENEMY_BOUNDARY_MARGIN + radius; maxX = worldWidth - ENEMY_BOUNDARY_MARGIN - radius; }
            if (maxY <= minY) { minY = ENEMY_BOUNDARY_MARGIN + radius; maxY = worldHeight - ENEMY_BOUNDARY_MARGIN - radius; }
        }

        const float toDestX = m_destination.x - position.x;
        const float toDestY = m_destination.y - position.y;
        const bool arrived = std::hypot(toDestX, toDestY) <= ENEMY_ARRIVE_DISTANCE;
        const bool destOutside = m_destination.x < minX || m_destination.x > maxX ||
                                 m_destination.y < minY || m_destination.y > maxY;

        // Repick on arrival, or when dead-wandering leaves the view area
        if (arrived || destOutside)
        {
            PickNewDestination(minX, minY, maxX, maxY);
        }
        target = m_destination;
    }

    // --- Keep the movement target reachable: clamp it inside world bounds
    // (radius inset). Hold-ring, avoid-contact, and intercept targets can
    // otherwise land beyond a wall, making steering fight the hard clamp
    // every frame (visible jitter near boundaries). ---
    target.x = std::max(radius, std::min(target.x, worldWidth - radius));
    target.y = std::max(radius, std::min(target.y, worldHeight - radius));

    // --- Direction toward target (normalized) ---
    float dirX = target.x - position.x;
    float dirY = target.y - position.y;
    const float len = std::hypot(dirX, dirY);
    // No AI target this frame (hold-station): residual boundary/separation
    // biases must drift gently, not renormalize into a full-speed sprint.
    const bool holdingStation = (len <= 0.0001f);
    if (len > 0.0001f)
    {
        dirX /= len;
        dirY /= len;
    }

    // --- Smooth boundary steering: bias direction inward near edges ---
    if (position.x < ENEMY_BOUNDARY_MARGIN)
    {
        dirX += ENEMY_BOUNDARY_STEER * (ENEMY_BOUNDARY_MARGIN - position.x) / ENEMY_BOUNDARY_MARGIN;
    }
    else if (position.x > worldWidth - ENEMY_BOUNDARY_MARGIN)
    {
        dirX -= ENEMY_BOUNDARY_STEER * (position.x - (worldWidth - ENEMY_BOUNDARY_MARGIN)) / ENEMY_BOUNDARY_MARGIN;
    }
    if (position.y < ENEMY_BOUNDARY_MARGIN)
    {
        dirY += ENEMY_BOUNDARY_STEER * (ENEMY_BOUNDARY_MARGIN - position.y) / ENEMY_BOUNDARY_MARGIN;
    }
    else if (position.y > worldHeight - ENEMY_BOUNDARY_MARGIN)
    {
        dirY -= ENEMY_BOUNDARY_STEER * (position.y - (worldHeight - ENEMY_BOUNDARY_MARGIN)) / ENEMY_BOUNDARY_MARGIN;
    }

    // --- Enemy-to-enemy separation: soft repulsion bias inside the soft
    // zone, applied on top of whatever the AI chose this frame ---
    if (otherEnemyActive)
    {
        float sdx = position.x - otherEnemyPos.x;
        float sdy = position.y - otherEnemyPos.y;
        float sdist = std::hypot(sdx, sdy);
        if (sdist < 0.0001f) { sdx = 1.0f; sdy = 0.0f; sdist = 1.0f; } // degenerate overlap fallback
        if (sdist < ENEMY_SEPARATION_SOFT_DIST)
        {
            const float strength = (ENEMY_SEPARATION_SOFT_DIST - sdist) / ENEMY_SEPARATION_SOFT_DIST;
            dirX += (sdx / sdist) * ENEMY_SEPARATION_STEER * strength;
            dirY += (sdy / sdist) * ENEMY_SEPARATION_STEER * strength;
        }
    }

    // Re-normalize after adding the steering bias
    const float steerLen = std::hypot(dirX, dirY);
    if (steerLen > 0.0001f)
    {
        dirX /= steerLen;
        dirY /= steerLen;
    }

    // --- Move (frame-rate independent) ---
    // Enemies speed up only while the player is alive AND accelerating
    const float currentSpeed = holdingStation
        ? ENEMY_HOLD_DRIFT_SPEED
        : ((playerAccelerating && !playerDead)
            ? ENEMY_SPEED_PLAYER_ACCEL : ENEMY_SPEED_NORMAL);
    position.x += dirX * currentSpeed * dt;
    position.y += dirY * currentSpeed * dt;

    // --- Hard safety clamp inside world bounds (radius margin) ---
    position.x = std::max(radius, std::min(position.x, worldWidth - radius));
    position.y = std::max(radius, std::min(position.y, worldHeight - radius));

    // --- Smooth separation resolution: if the gap is under the minimum,
    // close it at a capped px/s speed (dt-scaled) instead of teleporting.
    // An instant snap here caused a close->snap->clamp limit cycle that
    // looked like shaking near walls; a steady small push cannot oscillate.
    // The soft repulsion above prevents deep overlap proactively, so this
    // only ever trims a few px per frame. ---
    if (otherEnemyActive)
    {
        float sdx = position.x - otherEnemyPos.x;
        float sdy = position.y - otherEnemyPos.y;
        float sdist = std::hypot(sdx, sdy);
        if (sdist < 0.0001f) { sdx = 1.0f; sdy = 0.0f; sdist = 1.0f; }
        if (sdist < ENEMY_SEPARATION_MIN_DIST)
        {
            const float deficit = ENEMY_SEPARATION_MIN_DIST - sdist;
            const float step = std::min(deficit, ENEMY_SEPARATION_RESOLVE_SPEED * dt);
            position.x += (sdx / sdist) * step;
            position.y += (sdy / sdist) * step;
            position.x = std::max(radius, std::min(position.x, worldWidth - radius));
            position.y = std::max(radius, std::min(position.y, worldHeight - radius));
        }
    }
}

void Enemy::Draw() const
{
    // Placeholder: red circle, clearly distinct from the cyan player ship
    DrawCircleV(position, radius, Color{ 220, 60, 60, 255 });
    DrawCircleLinesV(position, radius, Color{ 120, 20, 20, 255 });
}

bool Enemy::Touches(Vector2 otherPos, float otherRadius) const
{
    const float dx = position.x - otherPos.x;
    const float dy = position.y - otherPos.y;
    const float reach = radius + otherRadius;
    return (dx * dx + dy * dy) <= (reach * reach);
}

void Enemy::TakeDamage(float damageAmount)
{
    if (damageAmount <= 0.0f || IsDestroyed())
    {
        return;
    }
    m_health -= damageAmount;
    if (m_health < 0.0f)
    {
        m_health = 0.0f; // never negative — 0 HP means destroyed
    }
}

bool Enemy::IsDestroyed() const
{
    return m_health <= 0.0f;
}

void Enemy::ResetTo(Vector2 spawnPos)
{
    // Reposition for (re)spawning: new position and wander destination.
    // Health and chase state are preserved.
    position = spawnPos;
    m_destination = spawnPos;
    m_cornerHolding = false;
}

void Enemy::RespawnAt(Vector2 spawnPos)
{
    // Full respawn after destruction: fresh position, restored health,
    // and a clean AI state (re-detects immediately inside the view).
    position = spawnPos;
    m_destination = spawnPos;
    m_isChasing = false;
    m_health = ENEMY_MAX_HEALTH;
    m_cornerHolding = false;
}

float Enemy::DistanceTo(Vector2 worldPos) const
{
    return std::hypot(position.x - worldPos.x, position.y - worldPos.y);
}
