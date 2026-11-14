// EnemyInterceptor.cpp
// Purpose: Implements the methods declared in EnemyInterceptor.h.

#include "EnemyInterceptor.h"
#include <algorithm>
#include <cmath>

EnemyInterceptor::EnemyInterceptor(Vector2 spawnPos, float size)
    : position(spawnPos)
    , radius(size)
    , m_destination(spawnPos)
    , m_sideOffset(1.0f)
    , m_isChasing(false)
    , m_health(ENEMY2_MAX_HEALTH)
    , m_cornerHolding(false)
{
}

// ----------------------------------------------------------------------------
// PickNewDestination
// Random point inside the given bounds (world area or camera view).
// ----------------------------------------------------------------------------
void EnemyInterceptor::PickNewDestination(float minX, float minY, float maxX, float maxY)
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
// Detection/wander/death rules mirror Enemy Type 1. While chasing, the
// target is an intercept point ahead of the player's facing direction.
// ----------------------------------------------------------------------------
void EnemyInterceptor::Update(float dt, Vector2 playerPos, Vector2 playerFacing,
                              Vector2 playerVelocity,
                              const Camera2D& camera, float worldWidth, float worldHeight,
                              bool playerAccelerating, bool playerDead, bool avoidContact,
                              Vector2 otherEnemyPos, bool otherEnemyActive)
{
    if (dt <= 0.0f || IsDestroyed()) return;

    // --- Detection: same camera-visible-area latch as Enemy Type 1 ---
    if (playerDead)
    {
        m_isChasing = false;
    }
    else if (!m_isChasing)
    {
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

    // --- Choose this frame's movement target ---
    Vector2 target;
    if (m_isChasing && !playerDead)
    {
        // Intercept point: ahead of the player along its facing, offset
        // sideways by the perpendicular. Recomputed every frame so the
        // enemy adapts as the player moves and turns.
        const Vector2 perp = { -playerFacing.y, playerFacing.x };

        // Distance to the CURRENT-side intercept point; on arrival, re-roll
        // the side so the enemy repositions left/right instead of orbiting
        // or oscillating.
        const Vector2 currentIntercept = {
            playerPos.x + playerFacing.x * ENEMY2_INTERCEPT_DISTANCE +
                perp.x * m_sideOffset * ENEMY2_SIDE_DISTANCE,
            playerPos.y + playerFacing.y * ENEMY2_INTERCEPT_DISTANCE +
                perp.y * m_sideOffset * ENEMY2_SIDE_DISTANCE
        };
        const float distToIntercept = std::hypot(currentIntercept.x - position.x,
                                                currentIntercept.y - position.y);
        if (distToIntercept <= ENEMY2_ARRIVE_DISTANCE)
        {
            m_sideOffset = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;
        }

        // Direct-approach reaction: if the player is moving toward this
        // enemy (head-on), confront them directly instead of holding the
        // intercept point. Computed from the player's actual velocity.
        const float distToPlayer = std::hypot(playerPos.x - position.x,
                                              playerPos.y - position.y);
        const float playerSpeed = std::hypot(playerVelocity.x, playerVelocity.y);
        bool playerApproaching = false;
        if (distToPlayer > 1.0f && distToPlayer <= ENEMY2_CONFRONT_RANGE &&
            playerSpeed > 50.0f)
        {
            const float dot = ((playerPos.x - position.x) * playerVelocity.x +
                               (playerPos.y - position.y) * playerVelocity.y) /
                              (distToPlayer * playerSpeed);
            playerApproaching = dot > ENEMY2_CONFRONT_DOT;
        }

        if (playerApproaching)
        {
            target = playerPos; // close the distance and confront
        }
        else
        {
            target = {
                playerPos.x + playerFacing.x * ENEMY2_INTERCEPT_DISTANCE +
                    perp.x * m_sideOffset * ENEMY2_SIDE_DISTANCE,
                playerPos.y + playerFacing.y * ENEMY2_INTERCEPT_DISTANCE +
                    perp.y * m_sideOffset * ENEMY2_SIDE_DISTANCE
            };
        }

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
                    position.x + (adx / adist) * ENEMY2_CONTACT_AVOID_DISTANCE,
                    position.y + (ady / adist) * ENEMY2_CONTACT_AVOID_DISTANCE
                };
            }
        }

        // Corner/edge awareness: distance band with hysteresis (same rule
        // as Enemy Type 1). Far -> approach to preferred distance; inside
        // [MIN, MAX] -> hold THIS enemy's own position (no shared ring
        // attractor -> no wall-corner shaking); too close -> ease away.
        // Hysteresis prevents threshold flutter; boundary steering and
        // separation still apply while holding.
        const bool playerNearEdge =
            playerPos.x < ENEMY2_CORNER_MARGIN ||
            playerPos.x > worldWidth - ENEMY2_CORNER_MARGIN ||
            playerPos.y < ENEMY2_CORNER_MARGIN ||
            playerPos.y > worldHeight - ENEMY2_CORNER_MARGIN;
        if (playerNearEdge)
        {
            const float rdx = position.x - playerPos.x;
            const float rdy = position.y - playerPos.y;
            const float rdist = std::hypot(rdx, rdy);
            if (rdist > 0.001f)
            {
                if (m_cornerHolding)
                {
                    if (rdist > ENEMY2_HOLD_MAX_DIST + ENEMY2_HOLD_HYSTERESIS ||
                        rdist < ENEMY2_HOLD_MIN_DIST - ENEMY2_HOLD_HYSTERESIS)
                    {
                        m_cornerHolding = false;
                    }
                    else
                    {
                        target = position; // hold station
                    }
                }
                if (!m_cornerHolding)
                {
                    if (rdist > ENEMY2_HOLD_MAX_DIST)
                    {
                        const float scale = ENEMY2_HOLD_PREFERRED_DIST / rdist;
                        target = {
                            playerPos.x + rdx * scale,
                            playerPos.y + rdy * scale
                        };
                    }
                    else if (rdist < ENEMY2_HOLD_MIN_DIST)
                    {
                        const float scale = ENEMY2_HOLD_MIN_DIST / rdist;
                        target = {
                            playerPos.x + rdx * scale,
                            playerPos.y + rdy * scale
                        };
                    }
                    else
                    {
                        m_cornerHolding = true;
                        target = position;
                    }
                }
            }
        }
    }
    else
    {
        // Wander: whole world normally; camera view while player is dead
        float minX = ENEMY2_BOUNDARY_MARGIN + radius;
        float minY = ENEMY2_BOUNDARY_MARGIN + radius;
        float maxX = worldWidth - ENEMY2_BOUNDARY_MARGIN - radius;
        float maxY = worldHeight - ENEMY2_BOUNDARY_MARGIN - radius;

        if (playerDead)
        {
            const float halfViewW = camera.offset.x / camera.zoom;
            const float halfViewH = camera.offset.y / camera.zoom;

            minX = std::max(minX, camera.target.x - halfViewW);
            maxX = std::min(maxX, camera.target.x + halfViewW);
            minY = std::max(minY, camera.target.y - halfViewH);
            maxY = std::min(maxY, camera.target.y + halfViewH);

            if (maxX <= minX) { minX = ENEMY2_BOUNDARY_MARGIN + radius; maxX = worldWidth - ENEMY2_BOUNDARY_MARGIN - radius; }
            if (maxY <= minY) { minY = ENEMY2_BOUNDARY_MARGIN + radius; maxY = worldHeight - ENEMY2_BOUNDARY_MARGIN - radius; }
        }

        const float toDestX = m_destination.x - position.x;
        const float toDestY = m_destination.y - position.y;
        const bool arrived = std::hypot(toDestX, toDestY) <= ENEMY2_WANDER_ARRIVE;
        const bool destOutside = m_destination.x < minX || m_destination.x > maxX ||
                                 m_destination.y < minY || m_destination.y > maxY;

        if (arrived || destOutside)
        {
            PickNewDestination(minX, minY, maxX, maxY);
        }
        target = m_destination;
    }

    // --- Keep the movement target reachable: clamp it inside world bounds
    // (radius inset). Intercept, hold-ring, and avoid-contact targets can
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
    if (position.x < ENEMY2_BOUNDARY_MARGIN)
    {
        dirX += ENEMY2_BOUNDARY_STEER * (ENEMY2_BOUNDARY_MARGIN - position.x) / ENEMY2_BOUNDARY_MARGIN;
    }
    else if (position.x > worldWidth - ENEMY2_BOUNDARY_MARGIN)
    {
        dirX -= ENEMY2_BOUNDARY_STEER * (position.x - (worldWidth - ENEMY2_BOUNDARY_MARGIN)) / ENEMY2_BOUNDARY_MARGIN;
    }
    if (position.y < ENEMY2_BOUNDARY_MARGIN)
    {
        dirY += ENEMY2_BOUNDARY_STEER * (ENEMY2_BOUNDARY_MARGIN - position.y) / ENEMY2_BOUNDARY_MARGIN;
    }
    else if (position.y > worldHeight - ENEMY2_BOUNDARY_MARGIN)
    {
        dirY -= ENEMY2_BOUNDARY_STEER * (position.y - (worldHeight - ENEMY2_BOUNDARY_MARGIN)) / ENEMY2_BOUNDARY_MARGIN;
    }

    // --- Enemy-to-enemy separation: soft repulsion bias inside the soft
    // zone, applied on top of whatever the AI chose this frame ---
    if (otherEnemyActive)
    {
        float sdx = position.x - otherEnemyPos.x;
        float sdy = position.y - otherEnemyPos.y;
        float sdist = std::hypot(sdx, sdy);
        if (sdist < 0.0001f) { sdx = 1.0f; sdy = 0.0f; sdist = 1.0f; }
        if (sdist < ENEMY2_SEPARATION_SOFT_DIST)
        {
            const float strength = (ENEMY2_SEPARATION_SOFT_DIST - sdist) / ENEMY2_SEPARATION_SOFT_DIST;
            dirX += (sdx / sdist) * ENEMY2_SEPARATION_STEER * strength;
            dirY += (sdy / sdist) * ENEMY2_SEPARATION_STEER * strength;
        }
    }

    const float steerLen = std::hypot(dirX, dirY);
    if (steerLen > 0.0001f)
    {
        dirX /= steerLen;
        dirY /= steerLen;
    }

    // --- Move (frame-rate independent); boost only while player is alive+accelerating ---
    const float currentSpeed = holdingStation
        ? ENEMY2_HOLD_DRIFT_SPEED
        : ((playerAccelerating && !playerDead)
            ? ENEMY2_SPEED_PLAYER_ACCEL : ENEMY2_SPEED_NORMAL);
    position.x += dirX * currentSpeed * dt;
    position.y += dirY * currentSpeed * dt;

    // --- Hard safety clamp inside world bounds ---
    position.x = std::max(radius, std::min(position.x, worldWidth - radius));
    position.y = std::max(radius, std::min(position.y, worldHeight - radius));

    // --- Smooth separation resolution: dt-capped positional trim instead
    // of an instant snap (which caused wall-side oscillation). ---
    if (otherEnemyActive)
    {
        float sdx = position.x - otherEnemyPos.x;
        float sdy = position.y - otherEnemyPos.y;
        float sdist = std::hypot(sdx, sdy);
        if (sdist < 0.0001f) { sdx = 1.0f; sdy = 0.0f; sdist = 1.0f; }
        if (sdist < ENEMY2_SEPARATION_MIN_DIST)
        {
            const float deficit = ENEMY2_SEPARATION_MIN_DIST - sdist;
            const float step = std::min(deficit, ENEMY2_SEPARATION_RESOLVE_SPEED * dt);
            position.x += (sdx / sdist) * step;
            position.y += (sdy / sdist) * step;
            position.x = std::max(radius, std::min(position.x, worldWidth - radius));
            position.y = std::max(radius, std::min(position.y, worldHeight - radius));
        }
    }
}

void EnemyInterceptor::Draw() const
{
    // Placeholder: orange circle — distinct from red Type 1 and cyan player
    DrawCircleV(position, radius, Color{ 255, 150, 50, 255 });
    DrawCircleLinesV(position, radius, Color{ 150, 70, 20, 255 });
}

bool EnemyInterceptor::Touches(Vector2 otherPos, float otherRadius) const
{
    const float dx = position.x - otherPos.x;
    const float dy = position.y - otherPos.y;
    const float reach = radius + otherRadius;
    return (dx * dx + dy * dy) <= (reach * reach);
}

void EnemyInterceptor::TakeDamage(float damageAmount)
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

bool EnemyInterceptor::IsDestroyed() const
{
    return m_health <= 0.0f;
}

void EnemyInterceptor::ResetTo(Vector2 spawnPos)
{
    // Reposition for (re)spawning: new position and wander destination.
    // Health and chase state are preserved.
    position = spawnPos;
    m_destination = spawnPos;
    m_sideOffset = 1.0f;
    m_cornerHolding = false;
}

void EnemyInterceptor::RespawnAt(Vector2 spawnPos)
{
    // Full respawn after destruction: fresh position, restored health,
    // and a clean AI state (re-detects immediately inside the view).
    position = spawnPos;
    m_destination = spawnPos;
    m_sideOffset = 1.0f;
    m_isChasing = false;
    m_health = ENEMY2_MAX_HEALTH;
    m_cornerHolding = false;
}

float EnemyInterceptor::DistanceTo(Vector2 worldPos) const
{
    return std::hypot(position.x - worldPos.x, position.y - worldPos.y);
}
